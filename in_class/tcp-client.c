#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) { perror("socket"); return 1; }

	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4444);
	if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
		fputs("invalid IPv4 address\n", stderr);
		close(fd);
		return 1;
	}
	if (connect(fd, (struct sockaddr *)&addr, sizeof addr) == -1) {
	    perror("connect");
	    close(fd);
	    return 1;
	}

	const char msg[] = "hello over TCP\n";
	size_t sent = 0;
	while (sent < sizeof msg - 1) {
	    ssize_t n = write(fd, msg + sent, sizeof msg - 1 - sent);
	    if (n == -1) {
		if (errno == EINTR) continue;
		perror("write");
		close(fd);
		return 1;
	    }
	    if (n == 0) { close(fd); return 1; }
	    sent += (size_t)n;
	}
	if (shutdown(fd, SHUT_WR) == -1) {
        	perror("shutdown");
        	close(fd);
        	return 1;
    	}

    char buf[128];
    for (;;) {
        ssize_t n = read(fd, buf, sizeof buf);
        if (n == 0) break;
        if (n == -1) {
            if (errno == EINTR) continue;
            perror("read");
            close(fd);
            return 1;
        }
        if (fwrite(buf, 1, (size_t)n, stdout) != (size_t)n) {	// write String output from server to stdout - NOT original message
            perror("fwrite");
            close(fd);
            return 1;
        }
    }
    if (close(fd) == -1) { perror("close"); return 1; }
    return 0;
}

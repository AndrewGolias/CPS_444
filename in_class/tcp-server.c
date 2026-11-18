#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/* Echo every byte until the client finishes sending. */
// Track how many bytes were written and continue until the entire buffer has been sent or an error occurs.
static int echo_stream(int fd)
{
	char buf[1024];
	for (;;) {
		ssize_t n = read(fd, buf, sizeof buf);
		if (n == 0) return 0;
		if (n == -1) {
			if (errno == EINTR) continue;
			perror("read");
            		return -1;
        	}
 		size_t sent = 0;
        	while (sent < (size_t)n) {
		    	ssize_t m = write(fd, buf + sent, (size_t)n - sent);
		    	if (m == -1) {
		        	if (errno == EINTR) continue;
		        	perror("write");
		        	return -1;
			}
			if (m == 0) return -1;
		    	sent += (size_t)m;
		}
	}
}

int main(void)
{
	int listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener == -1) { perror("socket"); return 1; }

	int reuse = 1;
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,
		           &reuse, sizeof reuse) == -1) {
		perror("setsockopt");
		close(listener);
		return 1;
	}
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4444);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (bind(listener, (struct sockaddr *)&addr, sizeof addr) == -1) {
		perror("bind");
		close(listener);
		return 1;
	}
	if (listen(listener, 5) == -1) {
		perror("listen");
		close(listener);
		return 1;
	}
	puts("listening on 127.0.0.1:4444");
	fflush(stdout);
	int peer = accept(listener, NULL, NULL);
	if (peer == -1) {
		perror("accept");
		close(listener);
		return 1;
	}
	puts("client connected");
	fflush(stdout);
	int result = echo_stream(peer);
	close(peer);
	close(listener);
	return result == 0 ? 0 : 1;
}

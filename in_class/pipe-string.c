#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char *value=malloc(1024);
	if (argc >= 2) {
		value[0]='\0';
		for (int i=1; i<argc; i++) {
			strcat(value, argv[i]);
			strcat(value, " ");
		}
		value[strlen(value) - 1] = '\0';
	} else {
		value = "XYZ";
	}

	int fd[2];
	if (pipe(fd) == -1) {
		perror("pipe");
		return 1;
	}
	pid_t child = fork();
	if (child == -1) {
		perror("fork");
		return 1;
	}
	
	// Child reads string into a buffer, then verifies stream ended
	if (child == 0) {
		close(fd[1]);
		char buf[128];
		ssize_t nread = read(fd[0], buf, sizeof(buf) - 1);
		if (nread <= 0) {
			close(fd[0]);
			_exit(1);
		}
		buf[nread] = '\0'; // Null-terminate string for printing
		dprintf(STDOUT_FILENO, "child read <%s>\n", buf);

		// Check that writer closed the pipe (EOF)
		char dummy;
		ssize_t after = read(fd[0], &dummy, 1);
		if (after == 0) {
			dprintf(STDOUT_FILENO, "reader saw EOF\n");
		}
		close(fd[0]);
		_exit(after == 0 ? 0 : 1);
	}
	
	// Parent sends string, closes write end, reaps child
	close(fd[0]);
	size_t len = strlen(value);
	
	int write_ok = (write(fd[1], value, len) == (ssize_t)len);
	if (!write_ok) {
		perror("write");
	}
	close(fd[1]);

	int status;
	if (waitpid(child, &status, 0) == -1) {
		perror("waitpid");
		return 1;
	}
	if (!write_ok)
		return 1;
	return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

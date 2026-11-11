#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
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
	
	// child consumes one bytes, then verifies stream ended
	if (child == 0) {
		close(fd[1]);
		char value;
		if (read(fd[0], &value, 1) != 1) {
			_exit(1);
		}
		dprintf(STDOUT_FILENO, "child read <%c>\n", value);
		ssize_t after = read(fd[0], &value, 1);
		if (after == 0) {
			dprintf(STDOUT_FILENO, "reader saw EOF\n");
		}
		close(fd[0]);
		_exit(after == 0 ? 0 : 1);
	}
	
	// parent sends a byte, closes its write, reaps child
	close(fd[0]);
	char value = 'X';
	int write_ok = (write(fd[1], &value, 1) == 1);
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

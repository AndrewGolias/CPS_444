#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

static void child_fail(const char *what) {
	perror(what);
	_exit(127);
}

// waits for child process to complete and returns status, else returns -1
static int collect(pid_t child) {
	int status;
	pid_t done;
	do {
		done = waitpid(child, &status, 0);
	}
	while (done == -1 && errno == EINTR);
	if (done == -1) {
		perror("waitpid");
		return -1;
	}
	if (WIFEXITED(status)) {
		return WEXITSTATUS(status);
	}
	return 1;	// if status is 0
}

int main(void) {
	// set up pipe
	int fd[2];
	if (pipe(fd) == -1) {
		perror("pipe");
		return 1;
	}
	
	// fork
	pid_t producer = fork();
	if (producer == -1) {	// fork fails
		perror("fork producer");
		close(fd[0]);
		close(fd[1]);
		return 1;
	}
	if (producer == 0) {	// fork succeeds
		if (dup2(fd[1], STDOUT_FILENO) == -1) {		// dup2: write to stdout
			child_fail("dup2 stdout");
		}
		close(fd[0]);	// close read fd
		close(fd[1]);	// close write fd
		char *const args[] = {"cat","colors.txt",NULL};
		execvp(args[0], args);		// exec
		child_fail("execvp cat");
	}
	
	// fork
	pid_t consumer = fork();
	if (consumer == -1) {	// fork fails
		perror("fork consumer");
		close(fd[0]);
		close(fd[1]);
		(void)collect(producer);
		return 1;
	}
	if (consumer == 0) {	// fork succeeds
		if (dup2(fd[0], STDIN_FILENO) == -1) {		// dup2: read to stdin
			child_fail("dup2 stdin");
		}
		close(fd[0]);	// close read fd
		close(fd[1]);	// close write fd
		char *const args[] = {"wc","-l",NULL};
		execvp(args[0], args);		// exec
		child_fail("execvp wc");
	}
	
	// parent closes copies and collects statuses
	close(fd[0]);
	close(fd[1]);
	int producer_code = collect(producer);
	int consumer_code = collect(consumer);
	if (producer_code < 0 || consumer_code < 0) {
		return 1;
	}
	printf("producer exit=%d; consumer exit=%d\n", producer_code, consumer_code);
	return (producer_code == 0 && consumer_code == 0) ? 0 : 1;
} 


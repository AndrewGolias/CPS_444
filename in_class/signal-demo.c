#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

// TEST: open new window
// kill -TERM <pid>

static volatile sig_atomic_t stop_requested = 0;

static void on_term(int signum) {
	(void) signum;	// unused; blocks warning error
	stop_requested = 1;
}

int main(void) {
	struct sigaction sa = {0};
	sa.sa_handler = on_term;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		perror("sigaction");
		return 1;
	}
	printf("pid=%ld\n", (long)getpid());
	fflush(stdout);
	while(!stop_requested) {
		sleep(1);
	}
	puts("cleanup in main");	// printed when killed by external process; NOT BY SIGINT
	return 0;
}

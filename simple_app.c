#include <stdio.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

int call_echo(char* message) {
	char* echo_name = "simple_echo";

	pid_t pid = 0;
	char* args[] = {echo_name, message, NULL};
	if (posix_spawnp(&pid, echo_name, NULL, NULL, args, NULL)) {
		perror("simple_app: posix_spawnp failed");
		return 1;
	}

	int status = -1;
	if (waitpid(pid, &status, 0) != pid) {
		perror("simple_app: waitpid failed");
		return 1;
	}

	if (status != 0) {
		printf("simple_app: subprocess failed: %d\n", status);
		return 1;
	}

	return 0;
}

int main(int argc, char** argv) {
	while (1) {
		printf("in app\n");
		fflush(stdout);
		if (call_echo("in echo")) return 1;
		sleep(1);
	}
}

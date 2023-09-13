#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <errno.h>

int open_journal(char* path, int* fd) {
    *fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (*fd < 0) {
        perror("simple_systemd: open failed");
        return 1;
    }

    return 0;
}

int open_pipe(int* read_pipe, int* write_pipe) {
	int pipefd[2];
	if (pipe(pipefd)) {
		perror("simple_systemd: pipe failed");
		return 1;
	}

	*read_pipe = pipefd[0];
	*write_pipe = pipefd[1];

    return 0;
}

int copy(int input_fd, int output_fd) {
    while (1) {
		char buf = '\0';

		ssize_t count_read;
        if ((count_read = read(input_fd, &buf, 1)) < 0) {
            perror("simple_systemd: read failed");
            return 1;
        }

        if (count_read == 0) {
            return 0;
        }

        ssize_t count_written;
        errno = 0;
        if ((count_written = write(output_fd, &buf, count_read)) <= 0) {
            perror("simple_systemd: write failed");
            return 1;
        }
    }
}

int close_fd(int fd) {
    if (close(fd)) {
        perror("simple_systemd: close failed");
        return 1;
    }

    return 0;
}

int spawnp_piped(char* command, char** args, char** env, int read_pipe, int write_pipe, pid_t* pid) {
	posix_spawn_file_actions_t file_actions;
	if (posix_spawn_file_actions_init(&file_actions)) {
		perror("simple_systemd: posix_spawn_file_actions_init failed");
		return 1;
	}

	if (posix_spawn_file_actions_addclose(&file_actions, read_pipe)) {
		perror("simple_systemd: posix_spawn_file_actions_addclose(read_pipe) failed");
		return 1;
	}

	if (posix_spawn_file_actions_adddup2(&file_actions, write_pipe, STDOUT_FILENO)) {
		perror("simple_systemd: posix_spawn_file_actions_adddup2(write_pipe, STDOUT_FILENO) failed");
		return 1;
	}

	if (posix_spawn_file_actions_addclose(&file_actions, write_pipe)) {
		perror("simple_systemd: posix_spawn_file_actions_addclose(write_pipe) failed");
		return 1;
	}

	if (posix_spawnp(pid, command, &file_actions, NULL, args, env)) {
		perror("simple_systemd: posix_spawnp failed");
		return 1;
	}

	if (posix_spawn_file_actions_destroy(&file_actions)) {
		perror("simple_systemd: posix_spawn_file_actions_destroy failed");
		return 1;
	}

    return 0;
}

int wait_and_check(pid_t pid) {
	int status = -1;
	if (waitpid(pid, &status, 0) != pid) {
		perror("simple_systemd: waitpid failed");
		return 1;
	}

	if (status != 0) {
		printf("simple_systemd: subprocess failed: %d\n", status);
		return 1;
	}

	return 0;
}

int main(int argc, char** argv, char** env) {
	if (argc != 2) {
		printf("usage: %s <command>\n", argv[0]);
		return 1;
	}

	char* command = argv[1];

    int journal_fd = -1;
    if (open_journal("journal.txt", &journal_fd)) return 1;

    int read_pipe = -1;
    int write_pipe = -1;
    if (open_pipe(&read_pipe, &write_pipe)) return 1;

	char* args[] = {command, NULL};
    pid_t pid = 0;
    if (spawnp_piped(command, args, env, read_pipe, write_pipe, &pid)) return 1;

    if (close_fd(write_pipe)) return 1;

    if (copy(read_pipe, journal_fd)) return 1;

    if (wait_and_check(pid)) return 1;

    if (close_fd(journal_fd)) return 1;
}

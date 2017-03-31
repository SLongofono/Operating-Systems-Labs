#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"

int main(int argc, char *argv[]){
	int ret, status;

	// N child processes -> n-1 pipes.  Each pipe requires an integer
	// array to map it to.
	int fd[2];
	int fd2[2];
	int fd3[2];
	int pid, pid2, pid3, pid4;
	char bufr[256];
	if(argc < 4){
		printf("Usage: finder DIR STR NUM_FILES\n");
		return -1;
	}

	// After calling pipe, the argument becomes and index into a kernel
	// managed file table.  It is still of type int, but some other
	// metadata exists (according to stackexchange)
	pipe(fd);
	pipe(fd2);
	pipe(fd3);

	// Create first child to run find
	pid = fork();
	if(0 == pid){
		// close stdout and replace it by the write end of the first
		// pipe
		dup2(fd[1], STDOUT_FILENO);
		
		// Close all others
		//close(fd[1]);
		close(fd[0]);
		close(fd2[0]);
		close(fd2[1]);
		close(fd3[0]);
		close(fd3[1]);
		// Since bash is already set up to parse a single string as
		// input, we encapsulate the first command and its options in
		// one string
		//sprintf(bufr, "%s %s -name \'*\'.[ch]", FIND_EXEC, argv[1]);
		char * args[] = {FIND_EXEC, argv[1], "-name", "*.[ch]", (char*)NULL};
		// Still no clue why we have to use BASH here?  It doesn't
		// work without it.
		//char * args[] = {BASH_EXEC, "-c", bufr, (char *)NULL};
		
		// Still need to pass BASH_EXEC.  The first is used to
		// actually select the executable.  The second is there since
		// by default, the 0th argument is the program being run.  To
		// avoid problems with programs that assume this it the case,
		// we follow this convention.
		ret = execv(FIND_EXEC, args);

		// If we got here, then exec did not replace the process image
		// (it failed).  If successful, the exectuable passed takes
		// over and handles exit and exit code.
		printf("Failed to exec: %d\n", ret);
		exit(ret);
	}
	pid2 = fork();
	if(0 == pid2){
		
		dup2(fd[0], STDIN_FILENO);
		dup2(fd2[1], STDOUT_FILENO);
		//close(fd2[1]);

		close(fd[1]);
		close(fd2[0]);
		close(fd3[0]);
		close(fd3[1]);
		
		// sprintf(bufr, "grep -c %s", argv[2]);
		// sprintf(bufr, "-c %s", argv[2]);
		// char* args[] = {XARGS_EXEC, bufr, (char*)NULL};
		char* args [] = {XARGS_EXEC, GREP_EXEC, "-c", argv[2], (char*)NULL};
		ret = execv(XARGS_EXEC, args);
		printf("Failed to exec: %d\n", ret);
		exit(ret);
	}
	pid3 = fork();
	if(0 == pid3){
		dup2(fd2[0], STDIN_FILENO);
		dup2(fd3[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		close(fd2[1]);
		close(fd3[0]);
		//sprintf(bufr, "-t : +1.0 -2.0 --numeric --reverse");
		char* args[] = {SORT_EXEC, "-t", ":", "+1.0", "-2.0", "--numeric", "--reverse", (char *)NULL};
		ret = execv(SORT_EXEC, args);
		printf("Failed to exec: %d\n", ret);
		exit(ret);
	}
	pid4 = fork();
	if(0 == pid4){
		dup2(fd3[0], STDIN_FILENO);
		close(fd3[1]);
		close(fd2[0]);
		close(fd2[1]);
		close(fd[0]);
		close(fd[1]);
		sprintf(bufr, "--lines=%s", argv[3]);
		char* args[] = {HEAD_EXEC, bufr, (char*)NULL};
		ret = execv(HEAD_EXEC, args);
		printf("Failed to exec: %d\n", ret);
		exit(ret);
	}
	else{
		// Since the child processes already have stdin/out/err by
		// default, there is no need to connect the final leg of the
		// pipe.
		//dup2(fd2[0], STDIN_FILENO);
		
		// All ends of the pipe must be closed, or none of them will
		// actually pass data
		close(fd[0]);
		close(fd[1]);
		close(fd2[0]);
		close(fd2[1]);
		close(fd3[0]);
		close(fd3[1]);

		// Need to wait for all to avoid creating zombies, and to
		// actually see their results properly
		if(waitpid(pid, &status, 0) < 0){
			printf("Child 1 failed: %d\n", status);	
		}
		if(waitpid(pid2, &status, 0) < 0){
			printf("Child 2 failed: %d\n", status);
		}
		if(waitpid(pid3, &status, 0) < 0){
			printf("Child 3 failed: %d\n", status);
		}
		if(waitpid(pid4, &status, 0) < 0){
			printf("Child 4 failed: %d\n", status);	
		}
	}
	
	return 0;
}

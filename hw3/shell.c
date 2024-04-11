#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

struct job{
	int jobid;
	pid_t pid;
	int status;//1 = dead, 2 = stopped, 3 = running, 4 = terminated, 5 = done
	char args[64];
};

typedef struct job job;

job* jobs;
int maxjobs = 64;

void initjobs(){
	jobs = malloc(sizeof(job) * 64);
	return;
}

void addjob(pid_t pid, char* cmdline){
	int i = 1;
	while(jobs[i].status >= 1 && jobs[i].status <= 5 ){
		if(i >= maxjobs - 1){jobs = realloc(jobs, sizeof(job) * maxjobs * 2); maxjobs = maxjobs * 2;}
		i++;
	}
	jobs[i].jobid = i;
	jobs[i].pid = pid;
	jobs[i].status = 3;
	strcpy(jobs[i].args, cmdline);
	//int j = 0; while(args[j]){strcpy(jobs[i].args[j], args[j]); j++;}
	return;
}

void deljob(pid_t pid){
	int i = 1;
	while(i < maxjobs){
		if(pid == jobs[i].pid){
			jobs[i].status = 1;
			break;
		}
		i++;
	}
	return;
}

int getjob(pid_t pid){
	int i = 1;
	while(i < maxjobs){
		if(jobs[i].pid == pid){
			return jobs[i].jobid;
			break;
		}
		i++;
	}
	return -1;
}

pid_t foreground;
int currentfore = 0;
int check;
int background = 0;
char* saveline;
char* saveampersand;

char* readline(){

	char* line = NULL;
	size_t length = 0;

	if(getline(&line, &length, stdin) > 1){
		//if(getline(&line, &length, stdin) == -1 && feof(stdin)){exit(EXIT_SUCCESS);}
		if(!line){fprintf(stderr, "input error"); exit(EXIT_FAILURE);}

		if(strlen(line) > 1){if(line[strlen(line) - 2] == 38){
			strcpy(saveampersand, line); saveampersand[strlen(saveampersand) - 1] = '\0'; 
			background = 1; line[strlen(line) - 2] = '\0';}else{background = 0; line[strlen(line) - 1] = '\0';}}

	}
	return line;
}

char** breakline(char* input){

	int maxargs = 32;
	char** args = malloc(sizeof(char*) * maxargs);
	char* currentarg = strtok(input, " \t\r\a\n");//Maybe add other delimiters?

	for(int i = 0;; i++){

		if(!currentarg){break;}

		if(i >= maxargs){maxargs = 2 * maxargs; args = realloc(args, sizeof(char*) * maxargs);}

		args[i] = currentarg;

		currentarg = strtok(NULL, " \n");
	}

	return args;
}

void caller(char** args, char* cmdline){

	sigset_t mask_all, mask_one, prev_one; //From lec16 slides, for masking in case child ends before parent
	sigfillset(&mask_all);
	sigemptyset(&mask_one);
	sigaddset(&mask_one, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask_one, &prev_one); //Block sig child before forking

	pid_t pid = fork();

	if(pid == 0){//If running in child process

		sigemptyset(&prev_one);
		sigprocmask(SIG_SETMASK, &prev_one, NULL); //Unblock SIGCHLD in child process

		if(execve(args[0], args, NULL) < 0){//Try to execute given args

			if(strchr(args[0], '/') != NULL){//If given a path and path not found

				printf("%s: No such file or directory\n", args[0]);
				exit(1);

			}

			char* bin = malloc(10 + strlen(args[0]));
			strcpy(bin, "/usr/bin/");
			strcat(bin, args[0]);

			if(execve(bin, args, NULL) < 0){//Try finding the command in usr/bin

				strcpy(bin, "/bin/");
				strcat(bin, args[0]);

				if(execve(bin, args, NULL) < 0){//Try finding the command in bin, if failed, print not found

					printf("%s: command not found\n", args[0]);

				}
			}
			free(bin);//WILL NOT REACH THIS FREE IF COMMAND FOUND, MEANING WE LEAK BIN IN CHILD

		}

		exit(1);

	}else if(pid > 0){//If in parent process, check for ampersand and execute accordingly

		//SIGCHLD still blocked, so don't have to worry about child finishing

		setpgid(pid, pid); //Change child pgid so signals are sent to child, not parent as well
		int checksignal;

		if(!background){
			sigemptyset(&prev_one);
			sigprocmask(SIG_SETMASK, &prev_one, NULL);
			foreground = pid;
			check = 1;
			currentfore = 1;
			addjob(pid, cmdline);
			waitpid(pid, &checksignal, WUNTRACED);
			if(WIFSIGNALED(checksignal)){printf("\n[%d] %d terminated by signal %d\n", getjob(pid), pid, WTERMSIG(checksignal));}
			if(currentfore){deljob(pid);}
			currentfore = 1;
			sigemptyset(&prev_one);
			sigprocmask(SIG_SETMASK, &prev_one, NULL);
		}else{
			sigprocmask(SIG_BLOCK, &mask_all, NULL); //If running in background, block all signals to job to list
			addjob(pid, saveampersand);
			printf("[%d] %d\n", getjob(pid), pid);
			sigemptyset(&prev_one);
			sigprocmask(SIG_SETMASK, &prev_one, NULL); //Unblock signals after adding to list
		}

	}else if(pid < 0){//Error

		printf("Forking error");

	}

	return;
}

void listjobs(); void fg(char** args); void bg(char** args); void exit2(); void kill2(char** args); void cd(char** args);

void chandler(int sig){
	if(check){
 		kill(foreground, SIGINT);//If foreground is running, otherwise do nothing
	}
	return;
}

void zhandler(int sig){//Using global variable here is risky, but im too lazy to figure out another way
	if(check){
		printf("\n");
		kill(foreground, SIGTSTP);
		jobs[getjob(foreground)].status = 2;
		currentfore = 0;
	}
	return;
}

void sigchildhandler(int sig){//From Lec16 slides, block signals while job list is updated

	//if(!currentfore){return;}
	//int olderrno = errno;
	sigset_t mask_all, prev_all;
	pid_t pid;
	int checksignal;

	sigfillset(&mask_all);
	if((pid = waitpid(-1, &checksignal, WNOHANG)) > 0){

		if(jobs[getjob(pid)].status != 1){//If job has not already been deleted, do so

			if(WTERMSIG(checksignal) != 15){if(WIFSIGNALED(checksignal)){printf("[%d] %d terminated by signal %d\n", getjob(pid), pid, WTERMSIG(checksignal));}}
			sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
			deljob(pid);
			sigprocmask(SIG_SETMASK, &prev_all, NULL);

		}
	}
}

void loop(){

	char *cmdline;
	saveline = malloc(64);
	saveampersand = malloc(64);
	char **args;
	initjobs();

	signal(SIGINT, chandler);
	signal(SIGTSTP, zhandler);
	signal(SIGCHLD, sigchildhandler);

	while(1){
		printf(">");
		cmdline = readline();
		if(strlen(cmdline) < 2 && cmdline[0] == '\n'){continue;}
		if(!strcmp(cmdline, "jobs")){listjobs(); continue;}
		strcpy(saveline, cmdline);
		args = breakline(cmdline);
		if(args[0][0] == 3){continue;}
		//handling built-ins
		if(!strcmp(args[0], "exit")){
			exit2(); return;
		}else if(!strcmp(args[0], "bg")){
			bg(args); continue;
		}else if(!strcmp(args[0], "fg")){
			fg(args); continue;
		}else if(!strcmp(args[0], "kill")){
			kill2(args); continue;
		}else if(!strcmp(args[0], "cd")){
			cd(args); continue;
		}
		caller(args, saveline);
	}

	//HAVE TO FREE EVERYTHING ELSE MALLOCED TOO
	free(saveline);
	free(jobs);
}

int main(){

	loop();

	return 1;
}

void listjobs(){
	int i = 1;
	while(jobs[i].status >= 1 && jobs[i].status <= 5){
		if(jobs[i].status >= 2){
			char a[16];
			int status = jobs[i].status;
			if(status == 3){strcpy(a, "Running");}else if(status == 2){strcpy(a, "Stopped");}
			printf("[%d] %d %s %s\n", jobs[i].jobid, jobs[i].pid, a, jobs[i].args);}
		i++;
	}
	return;
}

void bg(char** args){//MAYBE BLOCK SIGCHLD IN BG, FG, and KILL
	if(args[1][0] != '%'){printf("No job entered\n"); return;}
	args[1][0] = '0';
	int jobid = atoi(args[1]);
	if(jobid > 0 && jobid <= maxjobs && (jobs[jobid].status == 2 || jobs[jobid].status == 3)){
		sigset_t unblock;
		sigemptyset(&unblock);
		sigprocmask(SIG_SETMASK, &unblock, NULL);
		if(jobs[jobid].status == 2){
			pid_t pid = jobs[jobid].pid;
			kill(pid, SIGCONT);
			jobs[jobid].status = 3;
		}
	}else{
		printf("Job %s not found\n", args[1]);
	}
	return;
}

void fg(char** args){
	if(args[1][0] != '%'){printf("No job entered\n"); return;}
	args[1][0] = '0';
	int jobid = atoi(args[1]);
	if(jobid > 0 && jobid <= maxjobs && (jobs[jobid].status == 2 || jobs[jobid].status == 3)){
		sigset_t unblock;
		sigemptyset(&unblock);
		sigprocmask(SIG_SETMASK, &unblock, NULL);
		if(jobs[jobid].status == 2){
			pid_t pid = jobs[jobid].pid;
			int checksignal;
			foreground = pid;
			check = 1;
			currentfore = 1;
			kill(pid, SIGCONT);
			waitpid(pid, &checksignal, WUNTRACED);
			if(WIFSIGNALED(checksignal)){printf("[%d] %d terminated by signal %d\n", getjob(pid), pid, WTERMSIG(checksignal));}
			if(currentfore){deljob(pid);}
			currentfore = 1;
		}else if(jobs[jobid].status == 3){
			pid_t pid = jobs[jobid].pid;
			int checksignal;
			foreground = pid;
			check = 1;
			currentfore = 1;
			waitpid(pid, &checksignal, WUNTRACED);
			if(WIFSIGNALED(checksignal)){printf("[%d] %d terminated by signal %d\n", getjob(pid), pid, WTERMSIG(checksignal));}
			if(currentfore){deljob(pid);}
			currentfore = 1;
		}

	}else{
		printf("Job %s not found\n", args[1]);
	}
}

void exit2(){
	int i = 1;
	while(i < maxjobs){
		sigset_t block;
		sigemptyset(&block);
		sigaddset(&block, SIGCHLD);
		sigprocmask(SIG_BLOCK, &block, NULL);
		if(jobs[i].status == 2){
			kill(jobs[i].pid, SIGHUP);
			kill(jobs[i].pid, SIGCONT);
		}else if(jobs[i].status == 3){
			kill(jobs[i].pid, SIGHUP);
		}
		i++;
	}
	return;
}

void kill2(char** args){
	if(args[1][0] != '%'){printf("No job entered\n"); return;}
	args[1][0] = '0';
	int jobid = atoi(args[1]);
	kill(jobs[jobid].pid, SIGTERM);
	printf("[%d] %d terminated by signal %d\n", jobid, jobs[jobid].pid, SIGTERM);
	jobs[jobid].status = 4;
}

void cd(char** args){
	//attempt to change directory, if succesful update PWD env variable
	if(args[1] == NULL){
		if(chdir(getenv("HOME")) == 0) {
			setenv("PWD", getenv("HOME"), 69420);
		}
	}else{
		if(chdir(args[1]) == 0) {
			//get absolute path to set PWD to
			char absolute_path[200];
			realpath("./", absolute_path);
			setenv("PWD", absolute_path, 0xDAB);
		}
	}
}

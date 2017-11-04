#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/utsname.h>
#include <dirent.h>

#define MAX_LENGTH 1024
#define TRUE 1
#define COMPARELIMIT 1000

struct utsname unameData;
DIR *d;
struct dirent *dir;

char** tokenize(char args[]) {
	char **tokens = malloc(1000*sizeof(*tokens));
	args = strtok (args, "\n");
	args = strtok (args, " ");
	int counter = 0;
    	while (args != NULL)  {
        	tokens[counter] = args;
        	counter = counter + 1;
        	args = strtok (NULL, " ");
    	}
    	tokens[counter] = NULL;
    	return tokens;
}

void ls() {
	d = opendir(".");
	int extra = 0;
	int extra2 = 0;
	int empty = 0;
	while ((dir = readdir(d)) != NULL) {
		extra = (strncmp(dir->d_name, ".", COMPARELIMIT));
		extra2 = (strncmp(dir->d_name, "..", COMPARELIMIT));
		if (extra != 0 && extra2 != 0) {
			empty = (strncmp(dir->d_name, "", COMPARELIMIT));
			printf("%s ", dir->d_name);
		}
	}
	if (empty != 0) {
		printf("\n");
	}
	closedir(d);
}

void pwd() {
	char buffer[100];
	getcwd(buffer, 100);
	printf("%s\n", buffer);
}

void cd(char **arguments) {
	int counter = 1;
	char directory[100];
	while (arguments[counter] != NULL) {
		strcat(directory, arguments[counter]);
		counter = counter + 1;
	}
	//printf("Directory is %s\n", directory);
	chdir(directory);
}

void echo(char **arguments) {
	int counter = 1;
	while (arguments[counter] != NULL) {
		printf("%s ", arguments[counter]);
		counter = counter + 1;
	}
	printf("\n");
}

void makedir(char **arguments) {
	char buffer[100];
	getcwd(buffer, 100);
	strcat(buffer, "/");
	strcat(buffer, arguments[1]);
	//printf("Buffer is %s", buffer);
	mkdir(buffer, 0777);
}

void vim(char **arguments) {
	if (fork() != 0) {
		wait(NULL);
	} else {
		char *vim = "/usr/bin/";
		char command[1024];
		strcpy (command, vim);
		strcat (command, arguments[0]);
		arguments[2] = NULL;
		//printf("Command is: %s and args is %s\n", command, arguments[1]);
		execve(command, arguments, 0);
	}
}

int checkcmd(char *command) {
	const char* internalcmd[] = {"ls", "pwd", "cd"};
	const char* externalcmd[] = {"vim", "echo", "mkdir", "ps"};
	int found = 0;
	int result = 0;
	int counter = 0;
	while (counter <= 2) {
		result = strncmp(command, internalcmd[counter], COMPARELIMIT);
		if (result == 0) {
			found = 1;
			//printf("Found an internal command\n");
			return found;
		}
		counter = counter + 1;
	}
	counter = 0;
	while (counter <= 3) {
		result = strncmp(command, externalcmd[counter], COMPARELIMIT);
		if (result == 0) {
			found = 2;
			//printf("Found an external command\n");
			return found;
		} 
		counter = counter + 1;
	}
	//printf("Found no command");
	return found;
}

int main(int argc, char *argv[]) {
	uname(&unameData);
	//printf("OS is: %s\n", unameData.sysname);
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	char line[MAX_LENGTH];
	char* datetime = strndup(asctime(timeinfo), 24);
	int result;
	int status;
	int exit = 0;

	while (exit != 1) {
		printf("%s ", datetime);

		if (!fgets(line, MAX_LENGTH, stdin)) break;
		result = strncmp(line, "exit\n", COMPARELIMIT);

		if (result == 0) {
			exit = 1;
			break;
		}

		char **arguments = tokenize(line);
		int valid = checkcmd(arguments[0]);
		if (valid == 0) {
			printf("%s is not a supported in this shell\n", arguments[0]);
		} else {
			if ((strncmp(arguments[0], "ls", COMPARELIMIT)) == 0) {
				ls();
			}
			if ((strncmp(arguments[0], "pwd", COMPARELIMIT)) == 0) {
				pwd();
			}
			if ((strncmp(arguments[0], "cd", COMPARELIMIT)) == 0) {
				cd(arguments);
			} 
			if ((strncmp(arguments[0], "echo", COMPARELIMIT)) == 0) {
				echo(arguments);
			}
			if ((strncmp(arguments[0], "mkdir", COMPARELIMIT)) == 0) {
				makedir(arguments);
			}
			if ((strncmp(arguments[0], "vim", COMPARELIMIT)) == 0) {
				vim(arguments);
			}
		}
	}
	return 0;
}

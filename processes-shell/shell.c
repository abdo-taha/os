#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
char error_message[30] = "An error has occurred\n";
void error(){
	write(STDERR_FILENO, error_message, strlen(error_message));
}

char **path = NULL;
int cntpath = 0;
pid_t *pss = NULL;


/* seperates a string str by characters in ch and takes ret by refrence
 it will be an array of strings and the function returns its size */

int sep_bychar(char *str, char ***ret ,const char* ch){
	int pos = 0;
	(*ret) = malloc(sizeof(char*));
	char *tmp = strtok(str,ch);
	while(tmp != NULL){
		(*ret)[pos] = tmp;
		tmp = strtok(NULL,ch);
		++pos;
		(*ret) = realloc( (*ret), (pos+1) * sizeof(char*) );
	}
	(*ret)[pos] = NULL;
	return pos;
}

void cd(char **comm , int n){
	if(n != 2) {
		error();
		return;
	}
	int check = chdir(comm[1]);
	if(check) error();
}

void bye(char **comm, int n){
	if(n != 1){
		error();
		return;
	}
	exit(0);

}
void change_path(char **comm ,int n){
	free(path);
	cntpath = n-1;
	path = malloc((n-1)*sizeof(char*));
	for(int i = 0; i < n-1; ++i) {
		path[i] = (char*) malloc( strlen(comm[i+1])*sizeof(char) );
		strcpy(path[i],comm[i+1]);
	}

}
void init_path(){
	path = malloc(sizeof(char*));
	path[0] = "/bin";
	cntpath = 1;
}

// running process
// pr : command string seperated by parameters
// n : number of parameters
// red : redirection , k: number of process
void runp(char **pr, int n, int red,int k){
	if(!n) return;
	if(strcmp(pr[0], "exit")==0) bye(pr,n);
	else if(strcmp(pr[0] ,"cd")==0 ) cd(pr,n);
	else if(strcmp(pr[0],"path")==0) change_path(pr,n);
	else{
		pid_t child = fork();
		pss[k] = child;
		if(child == 0){
			char *tmp = NULL;
			if(red!= 0){
				if(red == -1 || ( red==1 && n <= 1 )) {
					error();
					return;
				}
				else {
					int fp = open(pr[n-1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
					dup2(fp,1);// output stream
					dup2(fp,2);// error stream
					close(fp);
					pr[n-1] = NULL;// the new end
				}
			}
			for(int i = 0; i <cntpath; ++i){// check for command in all defined paths
				tmp = (char*) malloc( (strlen(pr[0]) + strlen(path[i])+1 ) * sizeof(char) );// add program name to the path
				strcpy(tmp,path[i]);
				strcat(tmp,"/");
				strcat(tmp,pr[0]);
				if(access(tmp,X_OK) == 0) // check if the program exist
					execv(tmp,pr); // run it
			}
			error();
			exit(0);
		}
	}
}

int main(int argc , char *argv[] ){
	if(argc > 2) {
		error();
		exit(1);
	}
	FILE *input  = stdin;
	if(argc == 2) { // take input from file
		input = fopen(argv[1], "r");
		if(input == NULL){
			error();
			exit(1);
		}
	}
	init_path();
	while(1){
		if(argc == 1) printf("wish>");
		// get input line
		char *str = NULL;
		size_t n = 0,sz = 0;
		sz = getline(&str,&n,input);
		if((int)sz == -1) break;
		// seperate parallel commands
		char **input = NULL;
		int cntp = sep_bychar(str,&input,"&"); // number of parallel processes
		char ***parse  = malloc(cntp * sizeof(char**)); // will seperate every command to parameters
		int *parsesz = malloc(cntp * sizeof(int)); // number of parameters for every command
		int *red = malloc(cntp * sizeof(int)); // flag for redirections
		pss = malloc(cntp * sizeof(pid_t));// parallel processes ids
		int good = 1; // flag for good parse
		for(int i = 0; i < cntp; ++i){ // iterate over parallel processes
			red[i] = 0;
			int pos = 0;
			for(int j = 0; j < strlen(input[i]); ++j){ // check redirection
				if(input[i][j]=='>'){ red[i]++; pos = j;break;}
			}
			if(red[i]){ // check if there is more redirection
				for(int j = pos +1 ; j < strlen(input[i]);++j){
					if(input[i][j] == '>') red[i] = -1,good = 0;
				}
			}
			if(red[i] == 1){ // for redirection
				char* checkfile = (char*) malloc( (strlen(input[i])-pos-1) *sizeof(char));
				strcpy(checkfile,input[i]+pos); // string after '>'
				char ** checktmp = NULL;
				int checkn = sep_bychar(checkfile,&checktmp," \n\t\r\a>");
				if(checkn != 1) { // check there is exactly one path
					red[i] = -1;
					good = 0;
				}
			}
		}
		if(!good){
		 	error();
			continue;
		}
		for(int i = 0; i < cntp ; ++i) parsesz[i] = sep_bychar(input[i],&parse[i]," \n\t\r\a>");// seperating parameters for every process

		for(int i = 0; i < cntp; ++i) runp(parse[i],parsesz[i],red[i],i); // running process
		int tt;
		for(int i = 0; i < cntp;++i) waitpid(pss[i],&tt,WUNTRACED); // waiting for processes

	}

	if(argc == 2) fclose(input);// close input file

	return 0;
}

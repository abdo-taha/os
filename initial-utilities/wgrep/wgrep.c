#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(int argc , char *argv[] ){

	if(argc == 1){
		printf("wgrep: searchterm [file ...]\n");
		exit(1);
	}
	if(argc == 2){
		char *line = NULL;
           	size_t len = 0;
           	ssize_t nread;


                while((nread = getline(&line, &len, stdin) != -1)){
			char *ptr = strstr(line,argv[1]);
			if(ptr != NULL) printf("%s",line);
		}
		free(line);

	}
        for(int i = 2; i < argc; ++i){
                FILE *fl = fopen(argv[i], "r");
                if(fl == NULL){
                        printf("wgrep: cannot open file\n");
                        exit(1);
                }
		char *line = NULL;
           	size_t len = 0;
           	ssize_t nread;


                while((nread = getline(&line, &len, fl ) != -1)){
			char *ptr = strstr(line,argv[1]);
			if(ptr != NULL) printf("%s",line);
		}
		free(line);
                fclose(fl);

        }

        return 0;
}

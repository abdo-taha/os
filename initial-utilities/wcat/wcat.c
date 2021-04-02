#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE (512)

int main(int argc , char *argv[] ){

	char buffer[BUFFER_SIZE];

	for(int i = 1; i < argc; ++i){
		FILE *fl = fopen(argv[i], "r");
		if(fl == NULL){
			printf("wcat: cannot open file\n");
			exit(1);
		}
		while(fgets(buffer, BUFFER_SIZE, fl) != NULL )
			printf("%s", buffer);
		fclose(fl);

	}

	return 0;
}

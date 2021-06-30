#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {

	if(argc == 1) {
		printf("wunzip: file1 [file2 ...]\n");
		exit(1);
	}
	int cnt;
	char ch;
	for(int i = 1; i < argc; ++i){
		FILE *fp = fopen(argv[i], "r");
		while(fread(&cnt, sizeof(int),1,fp)){
			fread(&ch,sizeof(char),1,fp);
			while(cnt--) printf("%c",ch);
		}
		fclose(fp);
	}
    return 0;
}

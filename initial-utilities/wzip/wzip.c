#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {

	if(argc == 1) {
		printf("wzip: file1 [file2 ...]\n");
		exit(1);
	}
	int cnt = 0;
	char last = '.',ch[2];
	for(int i = 1; i < argc; ++i){
		FILE *fp = fopen(argv[i], "r");
		while(fgets(ch,2,fp) != NULL){
			if(ch[0] == last){
				++cnt;
			}
			else if(last != '.'){
				fwrite(&cnt, sizeof(int), 1, stdout);
				fwrite(&last, sizeof(char), 1, stdout);
				cnt = 1;
				last = ch[0];
			}else{
				cnt = 1;
				last = ch[0];
			}
		}
		fclose(fp);
	}
	if(cnt != 0){
		fwrite(&cnt, sizeof(int), 1, stdout);
		fwrite(&last , sizeof(char),1,stdout);
	}
    return 0;
}

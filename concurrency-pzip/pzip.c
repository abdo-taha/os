#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int nchunks = 0;
char* ptr = NULL;
int sz = 1024/sizeof(char);
int next = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
char** zipped = NULL;


void addstring(char** text1, char** text2){
	*text1 = (char*) realloc(*text1, (strlen(*text1)+strlen(*text2)+1) * sizeof(char) );
    strcat(*text1,*text2);
    strcat(*text1,"\0");
}



void *zip(void *arg){
	while (1)
	{
		int pos = 0;
		pthread_mutex_lock(&lock);
		pos = next;
		if(next >= nchunks) pos = -1;
		++next;
		pthread_mutex_unlock(&lock);

		if(pos == -1) return NULL;
		int start = pos*sz, end = start + sz;
		char *text = malloc(sizeof(char));
		strcpy(text,"");
		char *last = malloc(2*sizeof(char)),ch;
		strcpy(last,".");
		int cnt = 0;
		char* number = (char*) malloc(11*sizeof(char)) ;
		for (int i = start; i < end; i++)
		{
			ch = ptr[i]; 
			if(ch == '\0') break;
			if(ch == last[0]){
				++cnt;
			}
			else if(last[0] != '.'){
	 	   		sprintf(number,"%d",cnt);
				addstring(&text,(char**)&number);
				addstring(&text,(char**)&last);
				cnt = 1;
				last[0] = ch;
			}else{
				cnt = 1;
				last[0] = ch;
			}
		}
		if(cnt != 0){
			
				sprintf(number,"%d",cnt);
				addstring(&text,&number);
				addstring(&text,&last);
		}
		printf("%s\n",text);
	}
	

}

int main(int argc, char *argv[]) {

	if(argc == 1) {
		printf("wzip: file1 [file2 ...]\n");
		exit(1);
	}

	for(int i = 1; i < argc; ++i){
		const char* filepath = argv[i];
		int fd = open(filepath,O_RDWR);
		if(fd < 0) exit(1);
		struct stat statbuf;
		int err = fstat(fd,&statbuf);
		if(err < 0) exit(2);
		ptr = mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

		if (ptr == MAP_FAILED)
		{
			return 1;
		}
		close(fd);

		/*
			ptr is char pointer to the file in memory
			statbuf.st_size is the size
		*/
		nchunks = (statbuf.st_size +sz-1)/ sz;
		zip(NULL);







		err = munmap(ptr, statbuf.st_size);
    	if(err != 0) return 1;


		
	}


	// nchunks  = 1;
	// ptr = malloc(100);
	// strcpy(ptr,"hhhhaaalll");
	// zip(NULL);
	// // printf("%s\n",ptr);


    return 0;
}

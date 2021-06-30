#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/types.h>

int nchunks = 0;
char* ptr = NULL;
int sz = 0;
int next = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
// pthread_cond_t   done = PTHREAD_COND_INITIALIZER;
char** zipped = NULL;
char* ans = NULL;

void addstring(char** text1, char** text2){
	*text1 = (char*) realloc(*text1, (strlen(*text1)+strlen(*text2)+3) * sizeof(char) );
    strcat(*text1," ");
	strcat(*text1,*text2);
	strcat(*text1," ");
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
		if(pos == -1) {
			return NULL;
		}
		int start = pos*sz, end = start + sz;
		char *text = malloc(2*sizeof(char));
		strcpy(text,"");
		char *last = malloc(2*sizeof(char)),ch;
		strcpy(last,"\0");
		int cnt = 0;
		char* number = (char*) malloc(12*sizeof(char)) ;
		for (int i = start; i < end; i++)
		{
			ch = ptr[i]; 
			if(ch == '\0') break;
			if(ch == last[0]){
				++cnt;
			}
			else if(last[0] != '\0'){
	 	   		sprintf(number,"%d",cnt);
				addstring(&text,(char**)&number);
				sprintf(number,"%d",(int)last[0]);
				addstring(&text,(char**)&number);
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
				sprintf(number,"%d",(int)last[0]);
				addstring(&text,(char**)&number);;
		}
		int ln = strlen(text);
		zipped[pos] = malloc( (ln+1) * sizeof(char));
		zipped[pos][ln] = '\0';
		strcpy(zipped[pos],text);
		free(text);
		free(last);
		free(number);
	}
	

}

void print(int n, char ch){
	if(n && ch != '\0'){
		fwrite(&n, sizeof(int), 1, stdout);
		fwrite(&ch, sizeof(char), 1, stdout);
	}
}

void join(){
	int cnt = 0, lastcnt = 0;
	int ch = '\0', lastch = '\0';
	for (size_t i = 0; i < nchunks; i++)
	{
		char* ptr = zipped[i];
		int offset = 0;
		while (sscanf(ptr,"%d %d%n",&cnt,&ch,&offset) == 2)
		{
			ptr += offset;
			if(ch != lastch){
				print(lastcnt,(char)lastch);
				lastcnt = cnt;
				lastch = ch;
			}else{
				lastcnt += cnt;
			}
		}
		
	}
	if(ch == lastch) {
		print(lastcnt,(char)lastch);
	}
}

int main(int argc, char *argv[]) {

	if(argc == 1) {
		printf("wzip: file1 [file2 ...]\n");
		exit(1);
	}
	sz = 1024/sizeof(char);
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
		zipped = (char**) malloc(nchunks * sizeof(char*));


		int nthr = get_nprocs();
		pthread_t* threads = (pthread_t*) malloc( nthr * sizeof(pthread_t));
		for (int i = 0; i < nthr; i++)
		{
			pthread_create(&threads[i], NULL, zip, NULL);
		}

		for (int i = 0; i < nthr; i++)
		{
			pthread_join(threads[i],NULL);
		}
		
		
		
		join();
		for (int i = 0; i < nchunks; i++)
		{
			free(zipped[i]);
		}
		
		free(zipped);
		free(threads);
		err = munmap(ptr, statbuf.st_size);
    	if(err != 0) return 1;


		
	}


    return 0;
}

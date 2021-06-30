#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <fcntl.h>

int qsize = 32;
static int chunks;
int pgsize;
volatile int producer_finished;

static int fillptr = 0;
static int useptr = 0;
static int numfull = 0;

typedef struct {
	char *start;
	size_t index;
	size_t size;

}  chunk;

typedef struct {
	char *final;
	size_t size;

} reduced_chunk;

chunk *producer_consumer_Q;
reduced_chunk *final_Q;

pthread_mutex_t m  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;


void do_fill(chunk chunk){
producer_consumer_Q[fillptr] = chunk;
fillptr = (fillptr + 1) % qsize;
numfull++;
}

chunk do_get(){
	chunk tmp = producer_consumer_Q[useptr];
	useptr = (useptr + 1) % qsize;
	numfull--;
	return tmp;
}



void run_length_encoding(chunk chunk, reduced_chunk *tmp){
	char *start = chunk.start;
	char end = *start;
	int count = 0;
	char *output = malloc(chunk.size * 8);
	char *curr = output;
	for (int i = 0; i< chunk.size;++i){
		if (start[i] != end){
		*((int *)curr) = count;
		curr[4] = end;
		curr+=5;
		end = start[i];
		count = 1;
}

		else{count++;}
}
	*((int *)curr) = count;
        curr[4] = start[chunk.size - 1];
        curr+=5;
	
	size_t reduced_chunk_size = curr-output;
	char *res = malloc(reduced_chunk_size);
	memcpy(res, output, curr-output);
	tmp->size = reduced_chunk_size;
	tmp->final = res;
	
	free(output);
	munmap(chunk.start, chunk.size);
	return;
	

}

void *consumer(void *ptr) {

while(1) {
	chunk job;
	pthread_mutex_lock(&m);
	while(numfull == 0 && !producer_finished)
		pthread_cond_wait(&fill, &m);
	if (numfull ==0 && producer_finished){
		pthread_mutex_unlock(&m);
		pthread_exit(0);
}
	job = do_get();
	pthread_cond_signal(&empty);
	pthread_mutex_unlock(&m);
	reduced_chunk *tmp = &final_Q[job.index];
	run_length_encoding(job, tmp);

}
pthread_exit(0);
}

int find_size(const char *file_name)
{
    struct stat st; /*declare stat variable*/
     
    /*get the size using stat()*/
     
    if(stat(file_name,&st)==0)
        return (st.st_size);
    else
        return -1;
}




int total_file_size(int no_of_files, char *argv[]){

        int total_size = 0;
        for (int i = 1; i< no_of_files + 1; i++) {
                int fsize = find_size(argv[i]);
                if (fsize == -1){
                        printf("File not Found");
                        exit(1);
}
                total_size+=fsize;
}
		return total_size;

}







int main(int argc, char *argv[]){
	if (argc <=1){
		printf("pzip file1 file2 ... > file.z");
		exit(1);
	}
	pgsize = getpagesize() * 8;
	chunks = 0;

	int nprocs = get_nprocs();
	int chunk_size = 0;
	int no_of_files = argc - 1;
	int total_size = 0;
	int next = 1;
	int size_of_left_file = 0;
	int offset = 0;
	int fil = 0;
	int chunk_index = 0;
	void *map = NULL;
	producer_finished = 0;

	total_size = total_file_size(no_of_files, argv);
	chunks = total_size / pgsize;
	if ( ((size_t) total_size) % pgsize != 0)
		chunks++;
	
	
	
	//printf("total size: %d, pagesize: %d , chunks: %d\n", total_size,pgsize, chunks);
	
	producer_consumer_Q = malloc(sizeof(chunk) * qsize);
	final_Q = malloc(sizeof(reduced_chunk) * chunks);
	
	pthread_t threads[nprocs];
	for (int i = 0; i< nprocs; i++) {
		pthread_create(&threads[i],NULL,consumer, NULL);
	}

	// producer should be implemented here
	for (int file = 1; file < no_of_files + 1;){
	
		if (next) {
			fil = open(argv[file], O_RDONLY);

			struct stat statbuf;
			fstat(fil, &statbuf);
			size_of_left_file = (size_t) statbuf.st_size;
			offset = 0;
			next = 0;
		}
		chunk_size = size_of_left_file > pgsize ? pgsize : size_of_left_file;
		
		if (chunk_size == 0) {
			file++;
			close(fil);
			next = 1;
			continue;
		
		}
		
		map = mmap(NULL,chunk_size,PROT_READ, MAP_PRIVATE, fil, offset);
		chunk args;
		args.start = map;
		args.index = chunk_index;
		args.size = chunk_size;
		
		pthread_mutex_lock(&m);
		while(numfull == qsize)
			pthread_cond_wait(&empty, &m);
		do_fill(args);
		pthread_cond_signal(&fill);
		pthread_mutex_unlock(&m);
		
		size_of_left_file-=chunk_size;
		offset+=chunk_size;
		
		if (size_of_left_file <=0){
			file++;
			close(fil);
			next = 1;
		}
		chunk_index++;
	
	}	
	
	
	producer_finished = 1;
	pthread_cond_broadcast(&fill);
	for (int i = 0; i<nprocs; i++){
	pthread_join(threads[i],NULL);
}

// here iterate through final_Q to get compressed file
	char *end = NULL;
	for (int i = 0; i<chunks; i++){
		char *final = final_Q[i].final;
		int size = final_Q[i].size;
		if (end && end[4] == final[4]){

			*((int *)final)+= *((int *)end);
		}
		else if (end) {
			fwrite(end,5,1,stdout);
		}
		fwrite(final,size-5,1,stdout);
		end = final + size - 5;
}
	fwrite(end,5,1,stdout);

	return 0;
}



#include   "types.h"
#include   "stat.h"
#include   "user.h"
#include   "fcntl.h"
#include   "pstat.h"
//#include   "defs.h"
//#include   "proc.h"
#include   "param.h"


int main (int argc,char *argv[]){

  while(1){
  struct pstat *pst = malloc(sizeof(struct pstat));
  getpinfo(pst);
  printf(1,"used \t pid  \t tickets \t ticks \n");
  for(int i=0; i < 8 ; ++i){
  printf (1,"%d \t %d \t %d \t\t %d \n " , pst->inuse[i] , pst->pid[i],pst->tickets[i],
    pst->ticks[i]);
  }
  sleep(200);
  }  
  exit();
}


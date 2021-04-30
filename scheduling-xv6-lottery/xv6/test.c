#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"


int
main(void)
{
  int x = settickets(2);
  printf(1,"%d\n",x);
  x = settickets(0);
  printf(1,"%d\n",x);
  x = settickets(5);
  printf(1,"%d\n",x);
  x = settickets(-5);
  printf(1,"%d\n",x);


  struct pstat *pst = malloc(sizeof(struct pstat));
  getpinfo(pst);
  printf(1,"used \t pid  \t tickets \t ticks \n");
  for(int i=0; i < 8 ; ++i){
  printf (1,"%d \t %d \t %d \t\t %d \n " , pst->inuse[i] , pst->pid[i],pst->tickets[i],
    pst->ticks[i]);
  }  


  exit();

}

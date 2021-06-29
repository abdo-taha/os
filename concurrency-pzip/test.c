#include <stdio.h>
#include <stdlib.h>

#include <string.h>

void addstring(char** text1, char** text2){
	*text1 = (char*) realloc( *text1, (strlen(*text1)+strlen(*text2)+1) * sizeof(char) );
    strcat(*text1,*text2);
    strcat(*text1,"\0");
}

int main(int argc, char *argv[]){


    char *a = malloc(6*sizeof(char));
    strcpy(a,"abcde");
    char *b = malloc(3*sizeof(char));
    strcpy(b,"aa");
    // addstring(&a,&b);
    // printf("%d %s \n",(int) strlen(a), a);

    // char number[11] ;
    // sprintf(number,"%d",123);
    
    // printf("%s\n",number);
    // sprintf(number,"%d",456);
    // printf("%s\n",number);


    char* t = NULL;


    t = realloc(t,5);
    
    return 0;
}
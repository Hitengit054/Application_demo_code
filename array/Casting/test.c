#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct c1{
    char a;
    int b;
}c;

int main()
{
    char* ch;
    c c1;
    c* pter = (c*)malloc(sizeof(c));
    c* revert = (c*)malloc(sizeof(c));

    //pter = &c1;
    pter->a = 'a';
    pter->b=6;
    ch = (char *)pter;

    revert = (c*)ch;
    printf("priting revert c.a is %0xx\n",revert->a);
    printf("priting revert c.b is %d\n",revert->b);
    printf("priting revert size of c is %ld\n", sizeof(struct c1));


}
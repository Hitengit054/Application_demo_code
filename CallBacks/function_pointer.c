#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int add_two(int *a, int *b)
{
    return (*a + *b);
}

int main()
{
    int (*add)(int *, int *);
    add = add_two;
    int a = 5;
    int b = 6;
    int c  = (*add)(&a, &b);
    printf("c is = %d\n", c);

}

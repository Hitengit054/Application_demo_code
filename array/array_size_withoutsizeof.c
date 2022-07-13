#include <stdio.h>

int main() {
   int a[] = {6,7,5,3,1,4,2,10,9};
   int s = *(&a + 1) - a;
   printf("&a = %p \n ", &a);
   printf("(&a + 1) = %p \n", &a + 1);
   printf("(&a + 1) = %p \n", *(&a + 1));
   printf("a = %p\n", a);
   printf("a+9 = %p\n", a+9);


   printf("Number of elements in array is  %d", s);
}
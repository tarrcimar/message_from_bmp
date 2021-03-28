#include <stdio.h>
#include <stdlib.h>


int main(){

    int fos = 7;

    fos = fos << 5;
    fos ^= 1UL << 7;
    printf("%d\n",fos);

}
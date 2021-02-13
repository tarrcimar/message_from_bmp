#include <stdio.h>

#if defined(linux) || defined(_linux) || defined(__linux__) || defined(__unix__)
    #define OSTYPE 1
#elif defined(_WIN32) || defined(_WINDOWS) || defined(__WINDOWS__)
    #define OSTYPE 2
#elif defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__)
    #define OSTYPE 3
#elif (defined(__MACH__) && defined(__APPLE__))
    #define OSTYPE 4
#else
    #define OSTYPE 0
#endif

int main(){
    switch(OSTYPE){
        case 1:
            printf("This is a Unix/Linux system!\n");
            break;
        case 2:
            printf("This is a Windows system!\n");
            break;
        case 3:
            printf("This is a DOS system!\n");
            break;
        case 4:
            printf("This is a MAC system!\n");
            break;
        default:
            printf("I don't know what kind of operating system this is.\n");
        
    }
    return 0;
}
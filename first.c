#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "Version: 0.0 \nDate: 2021.02.13 \nAuthor: Tarr Marton"
#define HELP "What you can do"

FILE *open_file(char* filename, char* mode)
{
    FILE *f;
    f = fopen(filename, mode);
    if(f == NULL)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }
    fclose(f);
    printf("%s\n", "van siker");
}


int main(int argc, char* argv[]){
    char c;

    if(argc == 1){
        fprintf(stderr," %s: I need an argument.\n",argv[0]);
    }
    else if(argc == 2 && strcmp(argv[1],"--version") == 0){
        printf("%s\n", VERSION);
        exit(1);
    }
    else if(argc == 2 && strcmp(argv[1], "--help") == 0){
        printf("%s\n", HELP);
        exit(1);
    }
    else if(argc == 2){
        char *file_name = argv[1];
        char *mode = "w";
        open_file(file_name,mode);
    }

    return EXIT_SUCCESS;
}
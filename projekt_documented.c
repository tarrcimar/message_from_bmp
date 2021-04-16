#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include "mydef.h"

//Function: main
int main(int argc, char* argv[]){
    char c;
    int file = 0;
    char* extension;
    int num = 0;
    int response;


    signal(SIGALRM, WhatToDo); // attach function to alarm signal
    
    if(argc == 2){
    	extension = strrchr(argv[1], '.');
    }

    if(argc == 1){
        file = BrowseForOpen();
        if(file == 0){
        	fprintf(stderr, "File could not be opened. The program will now exit.\n");
        	exit(1);
        }
        signal(SIGINT, WhatToDo); //attach function to interrupt signal
        alarm(1);
        char* pixel_array = ReadPixels(file, &num);
    	char* hidden_text = Unwrap(pixel_array, num);
    	alarm(0);
		printf("%s\n",hidden_text);
		free(pixel_array);
		free(hidden_text);
    }
    else if(argc == 2 && strcmp(argv[1],"--version") == 0){
        printf("%s\n", VERSION);
        exit(1);
    }
    else if(argc == 2 && strcmp(argv[1], "--help") == 0){
        printf("%s\n", HELP);
        exit(1);
    }
    else if(argc == 2 && strcmp(extension + 1, "bmp") == 0){
        file = open(argv[1], O_RDONLY);
        if(file == 0){
        	fprintf(stderr, "File could not be opened. The program will now exit.\n");
        	exit(1);
        }
        signal(SIGINT, WhatToDo);
        alarm(1);
        char* pixel_array = ReadPixels(file, &num);
    	char* hidden_text = Unwrap(pixel_array, num);
    	alarm(0);
        printf("%s\n",hidden_text);
		response = Post("G8R7ZQ", hidden_text, num);
		if(response == 0) printf("Post successful! Text sent.\n");
		else if(response != 0) fprintf(stderr, "Text could not be posted!\n");
		free(pixel_array);
		free(hidden_text);
    }

    alarm(0);

    return EXIT_SUCCESS;
}
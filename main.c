#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include "mydef.h"



int main(int argc, char* argv[]){
    char c;
    int file = 0;
    char* extension;
    int num = 0;
    int response;


    
    if(argc == 2){
    	extension = strrchr(argv[1], '.');
    }

    if(argc == 1){
        file = BrowseForOpen();
        if(file == 0){
        	fprintf(stderr, "File could not be opened. The program will now exit.\n");
        	exit(1);
        }
        signal(SIGALRM, WhatToDo); // attach function to alarm signal
        signal(SIGINT, WhatToDo); //attach function to interrupt signal
        alarm(1);//1 second to execute
        char* pixel_array = ReadPixels(file, &num);
    	char* hidden_text = Unwrap(pixel_array, num);
    	alarm(0);
		printf("%s",hidden_text);

		response = Post("G8R7ZQ", hidden_text, num);


		free(hidden_text);
		free(pixel_array);


		if(response == 0){
			printf("Post successful! Text sent.\n");
			exit(0);
		}
		else if(response != 0){
			fprintf(stderr, "Text could not be posted!\n");
			exit(6);
		}

    }
    else if(argc == 2 && strcmp(argv[1],"--version") == 0){
        printf("%s\n", VERSION);
        exit(2);
    }
    else if(argc == 2 && strcmp(argv[1], "--help") == 0){
        printf("%s\n", HELP);
        exit(2);
    }
    else if(argc == 2 && strcmp(extension + 1, "bmp") == 0){ //if the extension is bmp
        file = open(argv[1], O_RDONLY);
        if(file == 0){
        	fprintf(stderr, "File could not be opened. The program will now exit.\n");
        	exit(1);
        }


		signal(SIGALRM, WhatToDo); // attach function to alarm signal
        signal(SIGINT, WhatToDo);
        alarm(1);
        char* pixel_array = ReadPixels(file, &num);
    	char* hidden_text = Unwrap(pixel_array, num);
    	alarm(0);
        printf("%s",hidden_text);


		response = Post("G8R7ZQ", hidden_text, num);

		free(hidden_text);
		free(pixel_array);

		if(response == 0){
			printf("Post successful! Text sent.");
			exit(0);
		}
		else if(response != 0){
			fprintf(stderr, "Text could not be posted!");
			exit(6);
		}
    }
    else if(argc == 2 && strcmp(extension + 1, "bmp") != 0){
    	fprintf(stderr, "Wrong file format!");
    	exit(9);
    }

    return EXIT_SUCCESS;
}
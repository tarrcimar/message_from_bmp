#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include <unistd.h>
#include "colors.h"

/*struct pixel{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
};

struct pixel *TestArray(int *NumCh){
    int character = 0b01100001; //a
    int original_mask = 0b11100000;


    int num = 3;
    NumCh = &num;

    int mask = original_mask;

    //encode the given character into randomized rgb values
    struct pixel *rgb_array = malloc(3 * sizeof(struct pixel*));
    for (int i = 0; i < num; i++)
    {
        rgb_array[i].blue = ((rand() % 255 + 0)<<3) + ((character&mask)>>5);
        mask = mask >> 3;
        rgb_array[i].green = ((rand() % 255 + 0)<<3) + ((character&mask)>>2);
        mask = mask >> 3;
        rgb_array[i].red = ((rand() % 255 + 0)<<3) + (character&mask);
        mask = mask >> 3;

        mask = original_mask;
        //change character a->b->c
        character += 1;
    }

    return rgb_array;
}*/


//second
char* Unwrap(char *Pbuff, int NumCh){
    char *letters = malloc(NumCh * sizeof(char) + 1);
    //Check for Failure
    if(letters == NULL){
        fprintf(stderr, "Memory could not be allocated! Program will exit..");
        exit(1);
    }

    //set the mask
    int mask = 0b00000111;
    int character;
    int j = 0;
    
    //Decode the rgb elements from the array
    for (int i = 0; i < NumCh * 3; i+=3)
    {
        character = ((Pbuff[i] & mask)<<6) | ((Pbuff[i+1] & mask)<<3) | ((Pbuff[i + 2] & mask));
        letters[j] = character;
        j++;
    }

    //terminate the string
    letters[NumCh-1] = '\n';
    
    return letters;
}

char* ReadPixels(int fd, int* numCh)
{
    char info[54];
    
    read(fd, info, 54*sizeof(char));

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];
    int offset = *(int*)&info[10];
    int text_length = *(int*)&info[6];

    *numCh = text_length;

    int padded_row =(width*3 + 3) & (~3); 
    int size = padded_row * height;

    char* data = malloc(size * sizeof(char));
    if(data == NULL){
        fprintf(stderr, "Memory could not be allocated! Program will exit..");
        exit(1);
    }
    //fread(data, sizeof(unsigned char), size, file); 
    read(fd, data, size * sizeof(char));
    
    return data;
}

int BrowseForOpen(){
	DIR* dir;
	struct dirent* entry;
	struct stat inode;
	char tmp[PATH_MAX];
	char pw[PATH_MAX] = "/";
	int regularfile;
	char* extension;
	
	
	dir = opendir("/home");
	chdir("/home");
	
	
	struct stat path_stat;
	
	while(1){
		while((entry = readdir(dir)) != NULL){
			stat((*entry).d_name, &inode);
			switch(inode.st_mode & S_IFMT){
				case S_IFDIR:
					printf("%s%s%s\t",BLU, (*entry).d_name, WHT);
					break;
				case S_IFREG:
					printf("%s%s%s \t",RED,(*entry).d_name, WHT);
					break;	
			}
		}
		
		//Scan the path that should be opened
		printf("\nWhat do you want to open?\n");
		scanf("%s", tmp);
		//if back is written, then execute "cd .."
		if(strcmp(tmp, "back") == 0){
			chdir(pw);
		}
		else{
			getcwd(pw, sizeof(pw));
			while(chdir(tmp) != 0){
				stat(tmp, &path_stat);
				//if not a directory, check if it's a regular file, if yes then open and return
    			if(S_ISREG(path_stat.st_mode) == 1){
					regularfile = open(tmp, O_RDWR);
					return regularfile;
    			}
    			//if the input does not exist, ask for another input
				printf("\nDirectory or file does not exist. Choose from the list above!\n");
				scanf("%s", tmp);
			}
		}
		dir = opendir(".");
	}
	closedir(dir);
}

/*extension = strrchr(tmp, '.');
    				if(strcmp(extension + 1, "bmp") == 0){	
						printf("%s\n",extension + 1);*/

int main(){
    setlocale(LC_ALL, "");

    //Create Test Array, and decode it
    ////struct pixel *array = TestArray(&num);
	
	int file = BrowseForOpen();
    char* pixel_array = ReadPixels(fasz, &num);
    char* hidden_text = Unwrap(pixel_array, num);
	printf("%s\n",hidden_text);
    free(pixel_array);
    free(hidden_text);

    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wchar.h>
#include <locale.h>


/*struct pixel{
    char blue;
    char green;
    char red;
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
    char character;
    int j = 0;
    
    //Decode the rgb elements from the array
    for (int i = 0; i < NumCh * 3; i+=3)
    {
        character = ((Pbuff[i] & mask)<<6) | ((Pbuff[i+1] & mask)<<3) | ((Pbuff[i + 2] & mask));
        strncat(letters, &character, 1);
        letters[j] = character;
        j++;
    }

    //terminate the string
    letters[NumCh - 1] = '\0';
    printf("%c", letters[8]);
    printf("%c",letters[9]);
    return letters;
}

char* ReadPixels(char* f, int* numCh)
{
    int i;
    FILE* file = fopen(f, "rb");
    char info[54];

    // read the 54-byte header
    fread(info, sizeof(char), 54, file); 

    int fd;
    fd = open(f, O_RDWR);
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
    
    //Check for Failure
    if(data == NULL){
        fprintf(stderr, "Memory could not be allocated! Program will exit..");
        exit(1);
    }

    read(fd, data, size * sizeof(char));

    return data;
}



int main(){
    int num = 0;

    //Create Test Array, and decode it

    char* pixel_array = ReadPixels("cpu02.bmp", &num);
    char* hidden_text = Unwrap(pixel_array, num);
    printf("%s", hidden_text);


    free(pixel_array);

    return EXIT_SUCCESS;
}
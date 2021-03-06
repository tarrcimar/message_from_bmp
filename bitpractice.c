#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>


struct pixel{
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
}

//char*

char* Unwrap(struct pixel *Pbuff, int NumCh){
    char *letters = malloc(NumCh * sizeof(char) + 1);

    //Check for Failure
    if(letters == NULL){
        fprintf(stderr, "Memory could not be allocated! Program will exit..");
        exit(1);
    }

    //set the mask
    int mask = 0b00000111;
    int character = 0;

    //Decode the rgb elements from the struct
    for (int i = 0; i < NumCh; i++)
    {
        character = ((Pbuff[i].blue & mask)<<5) + ((Pbuff[i].green & mask)<<2) + (Pbuff[i].red & mask);
        character &= ~(1UL << 7);
        letters[i] = (char)character;
    }

    //terminate the string
    letters[NumCh] = '\0';

    return letters;
}

char* ReadPixels(char* f) //+ int* numCh
{
    int i;
    FILE* file = fopen(f, "rb");
    unsigned char info[54];

    // read the 54-byte header
    fread(info, sizeof(unsigned char), 54, file); 

    /*int fd;
    fd = open(filename, O_RDWR);
    read(fd, info, 54*sizeof(unsigned char));*/

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];
    int offset = *(int*)&info[10];
    

    printf("width: %d height:%d offset:%d\n", width, height, offset);

    //int padded_row = (width*3 + 3) & (~3);

    // allocate 3 bytes per pixel
    //int size = 3 * width * height;
    //unsigned char* data = malloc(size * sizeof(unsigned char));
    int padded_row =(width*3 + 3) & (~3); //4 - ((width*3)%4);//
    int foska = 4 - ((width*3)%4);
    printf("padded: %d  foska: %d width: %d\n", padded_row, foska, width);
    unsigned char tmp;
    unsigned char* data;

    int sum = 0;

    for(int i = 0; i < 1; i++)
    {
        data = malloc(padded_row * sizeof(unsigned char));

        //struct pixel *rgb_array = malloc(width * sizeof(struct pixel*));
        fread(data, sizeof(unsigned char), padded_row, file);
        //read(fd, data, padded_row * sizeof(unsigned char));
        int i = 0;
        int mask = 0b00000111;
        for(int j = 0; j < width; j += 3)
        {

            /*int character = ((data[j] & mask)<<5) | ((data[j+1] & mask)<<2) | (data[j+2] & mask);
            character &= ~(1UL << 7);
            rgb_array[i].blue = data[j];
            rgb_array[i].green = data[j+1];
            rgb_array[i].red = data[j+2];
            i++;*/

            unsigned int character = ((data[j] & mask)<<6) | ((data[j+1] & mask)<<3) | ((data[j + 2] & mask));
            character &= ~(1UL << 7);
            /*if((char)character == '\n'){
                character = ((data[j + 5] & mask)<<6) | ((data[j+ 4] & mask)<<3) | ((data[j + 3] & mask));
                printf("%c", character);
                character = ((data[j + 8] & mask)<<6) | ((data[j+7] & mask)<<3) | ((data[j + 6] & mask));
                printf("%c", character);
                character = ((data[j + 11] & mask)<<6) | ((data[j+10] & mask)<<3) | ((data[j + 9] & mask));
                printf("%c", character);
            }*/
            printf("%c", character);

        }
    }

    fclose(file);
    return data;
}



int main(){

    srand(time(NULL));

    int num = 3;

    //Create Test Array, and decode it
    struct pixel *array = TestArray(&num);
    char *hidden_text = Unwrap(array, num);

    char* fos = ReadPixels("cpu02.bmp");


    //printf("%s", hidden_text);
    //Free allocated memory
    //free(hidden_text);
    free(array);

    return EXIT_SUCCESS;
}
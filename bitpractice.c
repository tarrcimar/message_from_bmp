#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <string.h>


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
        letters[i] = (char)character;
    }

    //terminate the string, and free the allocated memory
    letters[NumCh] = '\0';

    return letters;
}



int main(){

    srand(time(NULL));

    int num = 3;

    //Create Test Array, and decode it
    struct pixel *array = TestArray(&num);
    char *hidden_text = Unwrap(array, num);

    printf("%s", hidden_text);
    //Free allocated memory
    free(hidden_text);
    free(array);

    return EXIT_SUCCESS;
}
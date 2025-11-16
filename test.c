#include<stdio.h>
#include <string.h>
#include <stdlib.h>
#define SIZEOFBUFFER 1024 //1kb
char* userInput();
int main(){
    char* userString = userInput();
    //(const char*)userInput)
    //printf("Length of %s is: %li\n",userString,strlen((const char*)userString));
    if(strcmp(userString,"exit") == 0){
        printf("The same\n");
    }else{
        printf("Not the same\n");
    }
    free(userString);
    return 0;
}



char* userInput(){
    char c;
    size_t sizeOfBuffer = SIZEOFBUFFER;
    size_t currentSize = 0;
    char* userInputString = (char *) malloc(sizeof(char) * sizeOfBuffer);
    if(!userInputString){
        printf("Unable to allocate buffer");
        exit(-1);
    }
    // && ((c = getchar()) != EOF)
    printf("> ");
    while (((c = getchar()) != '\n')){
        if(currentSize >= sizeOfBuffer){
            sizeOfBuffer+=SIZEOFBUFFER;
            //reallocate
            userInputString = realloc(userInputString,sizeOfBuffer);
            if(!userInputString){
                printf("Unable to reallocate buffer");
                exit(-1);
            }
        }
        //could also do
        //*(userInputString+currentSize) = c;
        printf("%li\n",currentSize);
        userInputString[currentSize] = c;
        currentSize++;
    }
    userInputString[currentSize] = '\0';
    printf("Length is: %li\n",strlen(userInputString));
    return userInputString;
}
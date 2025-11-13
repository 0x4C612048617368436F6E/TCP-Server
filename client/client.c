#include"./client.h"
void client(){
    int clientSocket = socket(AF_INET,SOCK_STREAM,0);
    if((serverSocketFileDescriptor < 0)){
        perror(strerror(errno));
        return;
    }
    //get string
    char* userString = userInput();
    printf("%s\n",userString);

    //connect to server
    


    free(userString);
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
        userInputString[currentSize] = c;
        currentSize++;
    }
    printf("Hello world");
    return userInputString;
}

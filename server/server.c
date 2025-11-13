#include"./server.h"

void server(){
    printf("Server listening\n");
    //establish an unbound socker
    int serverSocketFileDescriptor = socket(AF_INET,SOCK_STREAM,0);

    if((serverSocketFileDescriptor < 0)){
        perror(strerror(errno));
        return;
    }

    int true = 1;
    if(setsockopt(serverSocketFileDescriptor,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) != 0){
        perror(strerror(errno));
        return;
    }

    //define address format
    struct sockaddr_in serverAddressFormat;
    serverAddressFormat.sin_family = AF_INET;
    serverAddressFormat.sin_port = htons(PORT);
    serverAddressFormat.sin_addr.s_addr = htonl(INADDR_ANY);

    if((bind(serverSocketFileDescriptor,(const struct sockaddr*)&serverAddressFormat,sizeof(serverAddressFormat)) < 0)){
        perror(strerror(errno));
        return;
    }
    //can listen on server
    if(listen(serverSocketFileDescriptor,PENDINGCLIENT) == -1){
        perror(strerror(errno));
        return;
    }
    
    //we can use thread, but modern approach is
    while(SERVERRUNNING){
        printf("\nSever is listening\n");
        // we wil start with blocking, then move to multithreading, and then move on to asynchronous

        //create clientAddress Format
        struct sockaddr_in clientAddressFormat;
        size_t sizeOfClientAddress = sizeof(clientAddressFormat);
        int clientSocketFileDescriptor = accept(serverSocketFileDescriptor,(struct sockaddr *)&clientAddressFormat,(unsigned int * restrict)&sizeOfClientAddress);
        if(clientSocketFileDescriptor == -1){
            perror(strerror(errno));
            return;
        }

    }
}
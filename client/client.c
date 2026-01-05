#include"client.h"
#if defined(_WIN32) || defined(_WIN64)
void client(void){
    //need to initialise the winsock2 API
	WORD currentWinSockAPISpecification = MAKEWORD((BYTE)2,(BYTE)2);
	//pointer to WSADATA struct
	WSADATA wsaDataStruct = {0};
	BYTE optvalTrue = TRUE;
    struct sockaddr_in serverSocketAddressStructure = {0};
	struct sockaddr_in clientSocketAddressStructure = {0};
    size_t send_all_output = 0;
    SOCKET clientSocket;

    if(WSAStartup(currentWinSockAPISpecification,&wsaDataStruct) != 0){
		//some error occured
		CUSTOM_ERROR("Unable to initialise winsock2.API: %d",WSAGetLastError());
		exit(-1);
	}else{
        CUSTOM_DEBUG("Initialised winsock2.API");
    }

	//check if the DLL version is supported
	if((LOBYTE(wsaDataStruct.wVersion)!=(BYTE)2) || (HIBYTE(wsaDataStruct.wVersion)!=(BYTE)2)){
		CUSTOM_ERROR("Provided DLL version not supported: %d",WSAGetLastError());
		exit(-1);
	}else{
        CUSTOM_DEBUG("Provided DLL version supported");
    }
	//can do socket related stuff

	clientSocket = socket(AF_INET,SOCK_STREAM,0);
	if(clientSocket == INVALID_SOCKET){
		//invalid socket
		CUSTOM_ERROR("Unable to create socket endpoint: %d",WSAGetLastError());
		exit(-1);	
	}else{
        CUSTOM_DEBUG("Socket created");
    }

    //configure server address
    serverSocketAddressStructure.sin_family = AF_INET; 
	serverSocketAddressStructure.sin_port = htons(PORT);
	serverSocketAddressStructure.sin_addr.s_addr = inet_addr("192.168.1.186");

    //connect client to server
    if(connect(clientSocket, (SOCKADDR*)&serverSocketAddressStructure, sizeof(serverSocketAddressStructure))!=0){
        CUSTOM_ERROR("Unable to connect to server: %d",WSAGetLastError());
        exit(-1);
    }
    char* getUserInput = userInput();
    //get length of input and send
    size_t lengthOfUserInput = htonl(strlen(getUserInput));
    send_all_output = send_all(clientSocket,&lengthOfUserInput,sizeof(size_t));
    if(send_all_output < 0){
        //nothing sent
        CUSTOM_ERROR("An error occured while trying to send data");
        exit(-1);
    }
    //send actual message
    memset((char*)((void*)&send_all_output),0,sizeof(size_t));
    send_all_output = send_all(clientSocket,getUserInput,(sizeof(char)*strlen(getUserInput)));
    if(send_all_output < 0){
        //nothing sent
        CUSTOM_ERROR("An error occured while trying to send data");
        exit(-1);
    }
    //free 
    free(getUserInput);


    //terminate contact with client
    if(closesocket(clientSocket) == SOCKET_ERROR){
        CUSTOM_ERROR("An Error occured while attempting to close socket: %d\n",WSAGetLastError());
        exit(-1);
    }

	//do cleanup
	if(WSACleanup() == SOCKET_ERROR){
        CUSTOM_ERROR("An Error occured while attempting to cleanup: %d\n",WSAGetLastError());
        exit(-1);
    }
}

char *userInput(void){
    size_t currentSizeOfBuffer = SIZEOFBUFFER;
    char c;
    char* userInput = (char*)malloc(sizeof(char)*currentSizeOfBuffer);
    size_t pointer = 0;
    if(!userInput){
        CUSTOM_ERROR("Unable to allocate buffer");
        exit(-1);
    }
    CUSTOM_INPUT("> ");
    while((c = getchar()) != '\n'){
        if(pointer == (size_t)(sizeof(userInput))){
            //reallocate buffer
            currentSizeOfBuffer+=SIZEOFBUFFER;
            userInput = (char*)realloc(userInput,currentSizeOfBuffer);
            if(!userInput){
                CUSTOM_ERROR("Unable to allocate buffer");
                exit(-1);
            }
        }
        *(userInput+pointer) = c;
        pointer++;
    }
    //check if pointer has reached end
    if(pointer == (size_t)(sizeof(userInput))){
        //reallocate for NULL terminating character
        userInput = (char*)realloc(userInput,currentSizeOfBuffer+1);
        if(!userInput){
            CUSTOM_ERROR("Unable to allocate buffer");
            exit(-1);
        }
    }
    //check if pointer is less than, and if yes, make so we do not waste so much space
    if(pointer < (size_t)(sizeof(userInput))){
        //reallocate based on differnce
        userInput = (char*)realloc(userInput,(userInput,currentSizeOfBuffer - pointer)+1); //plus 1 for the null termination
        if(!userInput){
            CUSTOM_ERROR("Unable to allocate buffer");
            exit(-1);
        }
    }

    *(userInput+pointer) = '\0';

    return userInput;
}

size_t send_all(int socket, void * buffer, size_t sizeOfBuffer){
    unsigned char* unsignedBuffer = (unsigned char*)buffer;
    size_t totalSent = 0;
    size_t bytesSent = 0;
    while(totalSent < sizeOfBuffer){
        bytesSent = send(socket,unsignedBuffer+totalSent,sizeOfBuffer-totalSent,0);
        if(bytesSent == 0){
            //probably socket was not alive
            CUSTOM_ERROR("Something went wrong: %d",WSAGetLastError());
            exit(-1);
        }
        totalSent+=bytesSent;
    }
    return totalSent;
}

#elif defined(__unix__) || defined(__linux__) || defined(_POSIX_VERSION)
void client(void) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { 
        perror("socket"); 
        return; 
    }

    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
  	//192.168.236.112 
     if (inet_pton(AF_INET, "192.168.1.186", &srv.sin_addr) != 1) {
        perror("inet_pton");
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr*)&srv, sizeof(srv)) == -1) {
        perror("connect");
        close(sock);
        return;
    }

    while (CLIENTINPUT) {
        char *msg = userInput();
        if (!msg) break;
        if (strcmp(msg, "exit") == 0) { 
            free(msg); 
            break; 
        }

        size_t payload_len = strlen(msg) + 1; // include '\0' if you want
        uint32_t netlen = htonl((uint32_t)payload_len);

        if (send_all(sock, &netlen, sizeof(netlen)) < 0) { 
            free(msg); 
            break; 
        }
        if (send_all(sock, msg, payload_len) < 0) { 
            free(msg); 
            break; 
        }

        free(msg);
    }

    close(sock);
    return;
}

char *userInput(void) {
    size_t cap = SIZEOFBUFFER;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) { 
        perror("malloc"); 
        return NULL; 
    }

    CUSTOM_INFO("> ");
    int c;
    while ((c = getchar()) != '\n') {
        if (len + 1 >= cap) {
            cap += SIZEOFBUFFER;
            char *tmp = realloc(buf, cap);
            if (!tmp) { free(buf); return NULL; }
            buf = tmp;
        }
        buf[len++] = (char)c;
    }
    buf[len] = '\0';
    return buf;
}


ssize_t send_all(int sock, const void *buf, size_t len) {
    const char *p = buf;
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(sock, p + total, len - total, 0);
        if (n < 0) {
            perror("send");
            return -1;
        }
        if (n == 0) return total;
        total += (size_t)n;
    }
    return (ssize_t)total;
}
#endif
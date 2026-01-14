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
    if(!getUserInput) break;
    char* convertToCaesarCipher = caesarCipher(getUserInput);
    if(!convertToCaesarCipher) break;
    //get length of input and send
    size_t lengthOfUserInput = htonl(strlen(convertToCaesarCipher));
    send_all_output = send_all(clientSocket,&lengthOfUserInput,sizeof(size_t));
    if(send_all_output < 0){
        //nothing sent
        CUSTOM_ERROR("An error occured while trying to send data");
        exit(-1);
    }
    //send actual message
    memset((char*)((void*)&send_all_output),0,sizeof(size_t));
    send_all_output = send_all(clientSocket,convertToCaesarCipher,(sizeof(char)*strlen(convertToCaesarCipher)));
    if(send_all_output < 0){
        //nothing sent
        CUSTOM_ERROR("An error occured while trying to send data");
        exit(-1);
    }
    //free 
    free(getUserInput);
    free(convertToCaesarCipher);


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

ssize_t send_all(int socket, void * buffer, size_t sizeOfBuffer){
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

char* caesarCipher(const char* string){
    //generate Random Shift (number between 0-25)
    //keep things consistent for now (vulnerable)
    //srand(5);
    unsigned int randomShift = 5;//rand();
    //left or right... left = 0, right = 1
    int shiftDirection = 1;//rand()%2;
    char* caesarCipher = (char*) malloc((sizeof(char)*strlen(string))+1);
    ssize_t pointerToEncodedString = 0;
    for(const char* i=string; *i!='\0';i++){
        //check if uppercase (between 65-90)
        if(((int)*i) >= 65 && ((int)*i) <= 90){
            CUSTOM_DEBUG("%c=%d is uppercase\n",*i,(int)*i);
            //map the random number to uppercase range
            int val = *i-65;
            if(shiftDirection == 1){ //move right
                val = (val+=randomShift)%26;
                //to decrypt based on shiftDirection Just add 65 to the val
                CUSTOM_DEBUG("Random Shift: %d\n",randomShift);
                CUSTOM_DEBUG("New: %c=%d\n",65+val,65+val);
                *(caesarCipher+pointerToEncodedString) = 65+val;
                pointerToEncodedString++;
            }else{
                //shiftDirection is negative
                val-=randomShift;
                if((val) < 0){
                    val=((26-abs(val))%26);
                }else{
                    val%=26;
                }
                *(caesarCipher+pointerToEncodedString) = 65+val;
                pointerToEncodedString++;
            }
        }//check lowercase (between 97-122)
        else if(((int)*i) >= 97 && ((int)*i) <= 122){
            CUSTOM_DEBUG("%c=%d is lowercase\n",*i,(int)*i);
            //map the random number to uppercase range
            int val = *i-97;
            if(shiftDirection == 1){ //move right
                val = (val+=randomShift)%26;
                //to decrypt based on shiftDirection Just add 65 to the val
                CUSTOM_DEBUG("Random Shift: %d\n",randomShift);
                CUSTOM_DEBUG("New: %c=%d\n",97+val,97+val);
                *(caesarCipher+pointerToEncodedString) = 97+val;
                pointerToEncodedString++;
            }else{
                //shiftDirection is negative
                val-=randomShift;
                if((val) < 0){
                    val=((26-abs(val))%26);
                }else{
                    val%=26;
                }
                printf("Random Shift: %d\n",randomShift);
                printf("New: %c=%d\n",97+val,97+val);
                *(caesarCipher+pointerToEncodedString) = 97+val;
                pointerToEncodedString++;
            }
        }else if((((int)*i) >= 0 && ((int)*i) <= 64)){
            //parse other stuff 0-64, 91-96, 123-127
            //add support to other values for later
            //map the random number to uppercase range
            int val = *i-0;
            if(shiftDirection == 1){ //move right
                val = (val+=randomShift)%63;
                //to decrypt based on shiftDirection Just add 65 to the val
                CUSTOM_DEBUG("Random Shift: %d\n",randomShift);
                CUSTOM_DEBUG("New: %c=%d\n",0+val,0+val);
                *(caesarCipher+pointerToEncodedString) = 0+val;
                pointerToEncodedString++;
            }else{
                //shiftDirection is negative
                val-=randomShift;
                if((val) < 0){
                    val=((63-abs(val))%63);
                }else{
                    val%=63;
                }
                printf("Random Shift: %d\n",randomShift);
                printf("New: %c=%d\n",0+val,0+val);
                *(caesarCipher+pointerToEncodedString) = 0+val;
                pointerToEncodedString++;
            }

        }else if(((int)*i) >= 91 && ((int)*i) <= 96){

            //map the random number to uppercase range
            int val = *i-91;
            if(shiftDirection == 1){ //move right
                val = (val+=randomShift)%5;
                //to decrypt based on shiftDirection Just add 65 to the val
                CUSTOM_DEBUG("Random Shift: %d\n",randomShift);
                CUSTOM_DEBUG("New: %c=%d\n",91+val,91+val);
                *(caesarCipher+pointerToEncodedString) = 91+val;
                pointerToEncodedString++;
            }
            else{
                //shiftDirection is negative
                val-=randomShift;
                if((val) < 0){
                    val=((5-abs(val))%5);
                }else{
                    val%=5;
                }
                printf("Random Shift: %d\n",randomShift);
                printf("New: %c=%d\n",91+val,91+val);
                *(caesarCipher+pointerToEncodedString) = 91+val;
                pointerToEncodedString++;
            }

        }else if(((int)*i) >= 123 && ((int)*i) <= 127){

            //map the random number to uppercase range
            int val = *i-123;
            if(shiftDirection == 1){ //move right
                val = (val+=randomShift)%4;
                //to decrypt based on shiftDirection Just add 65 to the val
                CUSTOM_DEBUG("Random Shift: %d\n",randomShift);
                CUSTOM_DEBUG("New: %c=%d\n",123+val,123+val);
                *(caesarCipher+pointerToEncodedString) = 123+val;
                pointerToEncodedString++;
            }else{
                //shiftDirection is negative
                val-=randomShift;
                if((val) < 0){
                    val=((4-abs(val))%4);
                }else{
                    val%=4;
                }
                printf("Random Shift: %d\n",randomShift);
                printf("New: %c=%d\n",123+val,123+val);
                *(caesarCipher+pointerToEncodedString) = 123+val;
                pointerToEncodedString++;
            }
        }else{
            CUSTOM_DEBUG("Can't be mapped to ASCII");
        }

    }

    *(caesarCipher+pointerToEncodedString) = '\0';
    return caesarCipher;
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
    //192.168.1.186
     if (inet_pton(AF_INET, "192.168.236.112", &srv.sin_addr) != 1) {
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

        char* convertToCaesarCipher = caesarCipher(msg);
        if(!convertToCaesarCipher) break;


        size_t payload_len = strlen(convertToCaesarCipher) + 1; // include '\0' if you want
        uint32_t netlen = htonl((uint32_t)payload_len);

        if (send_all(sock, &netlen, sizeof(netlen)) < 0) { 
            free(msg);
            free(convertToCaesarCipher);
            break; 
        }
        if (send_all(sock, convertToCaesarCipher, payload_len) < 0) { 
            free(msg);
            free(convertToCaesarCipher);
            break; 
        }

        free(msg);
        free(convertToCaesarCipher);
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

char* caesarCipher(const char* string){
    //generate Random Shift (number between 0-25)
    //keep things consistent for now (vulnerable)
    //srand(5);
    unsigned int randomShift = 5;//rand();
    //left or right... left = 0, right = 1
    int shiftDirection = 1;//rand()%2;
    char* caesarCipher = (char*) malloc((sizeof(char)*strlen(string))+1);
    ssize_t pointerToEncodedString = 0;
    for(const char* i=string; *i!='\0';i++){
        //check if uppercase (between 65-90)
        if(((int)*i) >= 65 && ((int)*i) <= 90){
            // CUSTOM_DEBUG("%c=%d is uppercase\n",*i,(int)*i);
            //map the random number to uppercase range
            int val = *i-65;
            if(shiftDirection == 1){ //move right
                val = (val+=randomShift)%26;
                //to decrypt based on shiftDirection Just add 65 to the val
                // CUSTOM_DEBUG("Random Shift: %d\n",randomShift);
                // CUSTOM_DEBUG("New: %c=%d\n",65+val,65+val);
                *(caesarCipher+pointerToEncodedString) = 65+val;
                pointerToEncodedString++;
            }else{
                //shiftDirection is negative
                val-=randomShift;
                if((val) < 0){
                    val=((26-abs(val))%26);
                }else{
                    val%=26;
                }
                *(caesarCipher+pointerToEncodedString) = 65+val;
                pointerToEncodedString++;
            }
        }//check lowercase (between 97-122)
        else if(((int)*i) >= 97 && ((int)*i) <= 122){
            // CUSTOM_DEBUG("%c=%d is lowercase\n",*i,(int)*i);
            //map the random number to uppercase range
            int val = *i-97;
            if(shiftDirection == 1){ //move right
                val = (val+=randomShift)%26;
                //to decrypt based on shiftDirection Just add 65 to the val
                // CUSTOM_DEBUG("Random Shift: %d\n",randomShift);
                // CUSTOM_DEBUG("New: %c=%d\n",97+val,97+val);
                *(caesarCipher+pointerToEncodedString) = 97+val;
                pointerToEncodedString++;
            }else{
                //shiftDirection is negative
                val-=randomShift;
                if((val) < 0){
                    val=((26-abs(val))%26);
                }else{
                    val%=26;
                }
                // printf("Random Shift: %d\n",randomShift);
                // printf("New: %c=%d\n",97+val,97+val);
                *(caesarCipher+pointerToEncodedString) = 97+val;
                pointerToEncodedString++;
            }
        }else if((((int)*i) >= 0 && ((int)*i) <= 64)){
            //parse other stuff 0-64, 91-96, 123-127
            //add support to other values for later
            //map the random number to uppercase range
            int val = *i-0;
            if(shiftDirection == 1){ //move right
                val = (val+=randomShift)%63;
                //to decrypt based on shiftDirection Just add 65 to the val
                // CUSTOM_DEBUG("Random Shift: %d\n",randomShift);
                // CUSTOM_DEBUG("New: %c=%d\n",0+val,0+val);
                *(caesarCipher+pointerToEncodedString) = 0+val;
                pointerToEncodedString++;
            }else{
                //shiftDirection is negative
                val-=randomShift;
                if((val) < 0){
                    val=((63-abs(val))%63);
                }else{
                    val%=63;
                }
                // printf("Random Shift: %d\n",randomShift);
                // printf("New: %c=%d\n",0+val,0+val);
                *(caesarCipher+pointerToEncodedString) = 0+val;
                pointerToEncodedString++;
            }

        }else if(((int)*i) >= 91 && ((int)*i) <= 96){

            //map the random number to uppercase range
            int val = *i-91;
            if(shiftDirection == 1){ //move right
                val = (val+=randomShift)%5;
                //to decrypt based on shiftDirection Just add 65 to the val
                // CUSTOM_DEBUG("Random Shift: %d\n",randomShift);
                // CUSTOM_DEBUG("New: %c=%d\n",91+val,91+val);
                *(caesarCipher+pointerToEncodedString) = 91+val;
                pointerToEncodedString++;
            }
            else{
                //shiftDirection is negative
                val-=randomShift;
                if((val) < 0){
                    val=((5-abs(val))%5);
                }else{
                    val%=5;
                }
                // printf("Random Shift: %d\n",randomShift);
                // printf("New: %c=%d\n",91+val,91+val);
                *(caesarCipher+pointerToEncodedString) = 91+val;
                pointerToEncodedString++;
            }

        }else if(((int)*i) >= 123 && ((int)*i) <= 127){

            //map the random number to uppercase range
            int val = *i-123;
            if(shiftDirection == 1){ //move right
                val = (val+=randomShift)%4;
                //to decrypt based on shiftDirection Just add 65 to the val
                // CUSTOM_DEBUG("Random Shift: %d\n",randomShift);
                // CUSTOM_DEBUG("New: %c=%d\n",123+val,123+val);
                *(caesarCipher+pointerToEncodedString) = 123+val;
                pointerToEncodedString++;
            }else{
                //shiftDirection is negative
                val-=randomShift;
                if((val) < 0){
                    val=((4-abs(val))%4);
                }else{
                    val%=4;
                }
                // printf("Random Shift: %d\n",randomShift);
                // printf("New: %c=%d\n",123+val,123+val);
                *(caesarCipher+pointerToEncodedString) = 123+val;
                pointerToEncodedString++;
            }
        }else{
            CUSTOM_DEBUG("Can't be mapped to ASCII");
        }

    }

    *(caesarCipher+pointerToEncodedString) = '\0';
    return caesarCipher;
}
#endif

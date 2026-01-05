#include"./server.h"

#if defined(_WIN32) || defined(_WIN64)
//window sepecific stuff
void server(void){
	//need to initialise the winsock2 API
	WORD currentWinSockAPISpecification = MAKEWORD((BYTE)2,(BYTE)2);
	//pointer to WSADATA struct
	WSADATA wsaDataStruct = {0};
	const char* optvalTrue = "TRUE";
	struct sockaddr_in serverSocketAddressStructure = {0};
	struct sockaddr_in clientSocketAddressStructure = {0};
    DWORD lengthOfClientSocketAddressStructure = sizeof(clientSocketAddressStructure);
    DWORD lengthOfMessage = 0;
    int recv_all_output = 0;
    SOCKET s, clientSocket;

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

	s = socket(AF_INET,SOCK_STREAM,0);
	if(s == INVALID_SOCKET){
		//invalid socket
		CUSTOM_ERROR("Unable to create socket endpoint: %d",WSAGetLastError());
		exit(-1);	
	}else{
        CUSTOM_DEBUG("Socket created");
    }
	//configure socket
	if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,optvalTrue,sizeof(BYTE)) != 0){
		//unaable to set soket option
		CUSTOM_ERROR("Unable to set socket option: %d",WSAGetLastError());
		exit(-1);
	}else{
        CUSTOM_DEBUG("Successfully set socket option");
    }

	serverSocketAddressStructure.sin_family = AF_INET; 
	serverSocketAddressStructure.sin_port = htons(PORT);
	serverSocketAddressStructure.sin_addr.s_addr = INADDR_ANY;

	//bind socket
	if(bind(s,(SOCKADDR*)&serverSocketAddressStructure,sizeof(serverSocketAddressStructure)) !=0){
		CUSTOM_ERROR("Unable to bind socket to all interface: %d",WSAGetLastError());
        exit(-1);
	}else{
        CUSTOM_DEBUG("Successfully bind socket");
    }
	//set socket to listen
	if(listen(s,BACKLOG) !=0){
		CUSTOM_ERROR("Unable to place socker in passive state: %d",WSAGetLastError());
        exit(-1);
	}else{
        CUSTOM_INFO("Server listening");
    }
	//accept client connections
	//will be blocking, until client connects
    clientSocket = accept(s,(SOCKADDR*)&clientSocketAddressStructure,&lengthOfClientSocketAddressStructure);
	if(clientSocket == INVALID_SOCKET){
		CUSTOM_ERROR("Unable to accept connecting entity: %d\n",WSAGetLastError());
        exit(-1);

	}else{
        CUSTOM_INFO("Client connected");
    }

	while(SERVERRUNNING){

        memset((char*)&lengthOfMessage,0,sizeof(size_t));
		//work on recv...Receive length
        recv_all_output = recv_all(clientSocket,&lengthOfMessage,sizeof(DWORD));
        
        if(recv_all_output != sizeof(DWORD)){
            CUSTOM_ERROR("Length of message received not satisfy what should be received");
            exit(-1);
        }
        
        //convert from network ordered byte to host
        DWORD msgLength = ntohl(lengthOfMessage);
        CUSTOM_DEBUG("LENGTH OF MESSAGE: %d",msgLength);

        //recieve actual message, and calculate actual length of message, but check if lengthOfMessage is ok
        if(msgLength <= 0 || msgLength >= MESSAGESIZELIMIT){
            //can not allocate space for message
            CUSTOM_ERROR("Message length does not satisfy message size limit: %d\n",msgLength);
            exit(-1);
        }

        char* messageBuffer = (char*)malloc(sizeof(char)*msgLength);
        if(!messageBuffer){
            CUSTOM_ERROR("Unable to allocate space for message buffer\n");
            exit(-1);
        }
        memset((char*)((void*)&recv_all_output),0,sizeof(size_t));

        recv_all_output = recv_all(clientSocket,messageBuffer,((size_t)(sizeof(char)*msgLength)));
        if(recv_all_output != (sizeof(char)*msgLength)){
            CUSTOM_ERROR("Length of message received not satisfy what should be received");
            exit(-1);
        }
        CUSTOM_INFO("%s\n",messageBuffer);
        break;
	}
    //C:\Users\benja\Desktop\tcp_server\server\server.c
    //terminate contact with client
    if(closesocket(s) == SOCKET_ERROR){
        CUSTOM_ERROR("An Error occured while attempting to close socket: %d\n",WSAGetLastError());
        exit(-1);
    }

	//do cleanup
	if(WSACleanup() == SOCKET_ERROR){
        CUSTOM_ERROR("An Error occured while attempting to cleanup: %d\n",WSAGetLastError());
        exit(-1);
    }
}

//recv_all
size_t recv_all(int socket, void * buffer, size_t sizeOfBuffer){
    unsigned char* unsignedBuffer = (unsigned char*)buffer;
    size_t pointerToBuffer = 0;
    DWORD i = 0;
    size_t totalRecv = 0;
    while(pointerToBuffer < sizeOfBuffer){
        totalRecv = recv(socket,unsignedBuffer+pointerToBuffer,sizeOfBuffer-pointerToBuffer,0);
        if(totalRecv == 0){
            CUSTOM_ERROR("Unable to receive payload from client: %d\n",WSAGetLastError());
            exit(-1);
        }
        pointerToBuffer+=totalRecv;
        CUSTOM_DEBUG("Read: %d bytes",pointerToBuffer);
    }
    return pointerToBuffer; //at this point should be the actual length of the message received, otherwise an error occured
}

#elif defined(__unix__) || defined(__linux__) || defined(_POSIX_VERSION)
//linux specific stuff
void server(void) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket"); return; 
    }

    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr*)&srv, sizeof(srv)) < 0) {
        perror("bind");
        close(s);
        return;
    }
    
    if (listen(s, BACKLOG) < 0) {
        perror("listen");
        close(s); 
        return; 
    }

    printf("Server listening on port %d\n", PORT);

    struct sockaddr_in cli;
    socklen_t cli_len = sizeof(cli);
    int c = accept(s, (struct sockaddr*)&cli, &cli_len);
    
    if(c < 0){
        perror("Error");
    }

    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli.sin_addr, ipstr, sizeof(ipstr));
    printf("Client connected: %s:%d\n", ipstr, ntohs(cli.sin_port));

    while(SERVERRUNNING) {
        // read 4-byte length
        uint32_t netlen;
        ssize_t r = recv_all(c, &netlen, sizeof(netlen));
        if (r <= 0) {
            close(c); 
            break; 
        }

        uint32_t msglen = ntohl(netlen);
        if (msglen == 0 || msglen > 10*1024*1024) { // sanity limit (10MB)
            fprintf(stderr, "bad msglen=%u\n", msglen);
            close(c);
            continue;
        }

        char *buf = malloc(msglen);
        if (!buf) { 
            perror("malloc"); 
            close(c); 
            break; 
        }

        r = recv_all(c, buf, msglen);
        if (r <= 0) { 
            free(buf); 
            close(c); 
            continue; 
        }

        // ensure printable string (if you included NUL)
        buf[msglen] = '\0';
        //buf[msglen] = '\0';
        printf("Received (%u bytes): %s\n", msglen, buf);

        free(buf);
        if(strcmp((const char*)buf,"exit\0") == 0){
            // keep connection open if you want multiple messages:
            // loop on recv_all for next length/payload
            printf("Yes\n");
            close(c); // or keep and loop
            free(buf);
            break;
        }else{
            char exit[5] = "exit";
            printf("%li\n",strlen(buf));
            printf("%li\n",strlen(exit));
            printf("No\n");
        }
    }

    close(s);
    return;
}

ssize_t recv_all(int sock, void *buf, size_t len) {
    char *p = buf;
    size_t total = 0;
    while (total < len) {
        ssize_t n = recv(sock, p + total, len - total, 0);
        if (n < 0) {
            perror("recv");
            return -1;
        }
        if (n == 0) return 0; // peer closed
        total += (size_t)n;
    }
    return (ssize_t)total;
}
#endif

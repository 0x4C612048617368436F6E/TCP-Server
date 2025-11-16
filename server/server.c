#include"server.h"

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
            char exit[5] = {'e','x','i','t','\0'};
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

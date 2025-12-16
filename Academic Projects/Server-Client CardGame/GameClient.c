#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12345

int main(int argc, char *argv []){

    int sock;
    struct sockaddr_in server_addr;
    char buffer[512];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr))==-1){
        perror("connect");
        exit(1);
    }
    while(1){
        int n=read(sock,buffer,sizeof(buffer)-1);
        if (n <= 0) break; 
        buffer[n]=0;
        printf("%s\n",buffer);

        if (strstr(buffer, "Pick an option") ||
                strstr(buffer, "Enter your name") ||
                strstr(buffer, "Choose an option") ||
                strstr(buffer, "Select Account") ||
                strstr(buffer, "How many cards") ||
                strstr(buffer, "are you ready") ||
                strstr(buffer, "Type:") ) 
        {
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            write(sock, buffer, strlen(buffer));
            
            // Αν επιλέξετε έξοδο, βγες
            if (strncmp(buffer, "7", 1) == 0 || strncasecmp(buffer, "Exit", 4) == 0) break;
        }
    
        if (strncmp(buffer, "Exit", 4) == 0) break;
    }
    close(sock);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    


    
    FILE* fp = fopen("hotel.txt", "r");
    if (fp == NULL) {
            perror("Could not open hotel.txt\n");
        }
    char line[1024];
    while(fgets(line, sizeof(line), fp)){
            printf("%s",line);
        }
    fclose(fp);
    printf("\nWELCOME!!!\n\n");
    while (1){
        printf("Enter command (-GET ROOMS /- BOOK <Room> <Start> <End>/ -SEARCH <beds> <max_price> / -EXIT):\n");
        
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        if (strcmp(buffer, "EXIT") == 0) {
            break;
        }
        int sock = 0;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }
        send(sock, buffer, strlen(buffer), 0);

        char response[1024];
        int bytes;
        while ((bytes = read(sock, response, sizeof(response) - 1)) > 0) {
            response[bytes] = '\0';
            printf("%s", response);
            }   
            
        close(sock);
        }

    
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "test.h"

#define PORT 8080
#define MAX_CLIENTS 10
Room rooms[9];
int r;
void load_rooms() {
    FILE* fp = fopen("room.txt", "r");
    if (!fp) return;

    char line[256], desc[200];
    int idx = 0;
    while (fgets(line, sizeof(line), fp) && idx < 9) {
        if (sscanf(line, "Room: %d,Available Beds: %d,Fall to Winter price: %f-Spring to Summer Price: %f\n",
            &rooms[idx].room_number,
            &rooms[idx].beds,
            &rooms[idx].price_low,
            &rooms[idx].price_high) == 4) {

            // Διάβασε την επόμενη γραμμή για την περιγραφή
            if (fgets(desc, sizeof(desc), fp)) {
                // Αφαιρείς τυχόν \n
                desc[strcspn(desc, "\n")] = 0;
                strncpy(rooms[idx].description, desc, sizeof(rooms[idx].description));
            }
            rooms[idx].reservation_count = 0;
            idx++;
        }
    }
    fclose(fp);

    for (int i = 0; i < 9; i++) {
    printf("Room %d: beds=%d prices=%.2f-%.2f desc=%s\n", rooms[i].room_number, rooms[i].beds, rooms[i].price_low, rooms[i].price_high, rooms[i].description);
}
}



int saved_reservation_count[9] = {0};  

void save_rooms() {
    FILE* fp = fopen("reservations.txt", "a");  
    if (!fp) {
        perror("Could not open reservations.txt");
        return;
    }

    for (int i = 0; i < 9; i++) {
        for (int j = saved_reservation_count[i]; j < rooms[i].reservation_count; j++) {
            fprintf(fp, RESERVATION_FORMAT,
                rooms[i].reservations[j].start_date,
                rooms[i].reservations[j].end_date,
                r+1
            );
        }
        saved_reservation_count[i] = rooms[i].reservation_count;
    }

    fclose(fp);
}

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void* handle_client(void* client_socket_ptr) {
    int client_socket = *(int*)client_socket_ptr;
    free(client_socket_ptr); 

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    read(client_socket, buffer, sizeof(buffer));
    printf("Received from client: %s\n", buffer);


    
    if (strcmp(buffer, "GET ROOMS") == 0) {
        pthread_mutex_lock(&file_mutex);
        FILE* fp = fopen("room.txt", "r");
        if (fp == NULL) {
            perror("Could not open room.txt\n");
            close(client_socket);
            pthread_exit(NULL);
        }

        char line[1024];
        while (fgets(line, sizeof(line), fp)) {
            send(client_socket, line, strlen(line), 0);
        }
        fclose(fp);
        pthread_mutex_unlock(&file_mutex);
    } 


    else if (strncmp(buffer, "BOOK", 4) == 0) {
        pthread_mutex_lock(&file_mutex);

        int d1, m1, y1, d2, m2, y2;
        if (sscanf(buffer, "BOOK %d %d/%d/%d %d/%d/%d", &r, &d1, &m1, &y1, &d2, &m2, &y2) != 7) {
            send(client_socket, "Invalid booking format\n", 24, 0);
            pthread_mutex_unlock(&file_mutex);
            pthread_exit(NULL);
            exit(EXIT_FAILURE);
        }

        if (r < 1 || r > 9) {
            send(client_socket, "Invalid room number\n", 21, 0);
            pthread_mutex_unlock(&file_mutex);
            pthread_exit(NULL);
            exit(EXIT_FAILURE);
        }

        r -= 1;
        char new_start[11], new_end[11];
        snprintf(new_start, sizeof(new_start), "%04d-%02d-%02d", y1, m1, d1);
        snprintf(new_end, sizeof(new_end), "%04d-%02d-%02d", y2, m2, d2);

        int overlap = 0;
        for (int i = 0; i < rooms[r].reservation_count; i++) {
            if (strcmp(new_start, rooms[r].reservations[i].end_date) <= 0 &&
                strcmp(new_end, rooms[r].reservations[i].start_date) >= 0) {
                overlap = 1;
                break;
            }
        }

        if (overlap) {
            send(client_socket, "Reservation overlaps with an existing one\n", 42, 0);
        } else {
            strcpy(rooms[r].reservations[rooms[r].reservation_count].start_date, new_start);
            strcpy(rooms[r].reservations[rooms[r].reservation_count].end_date, new_end);
            rooms[r].reservation_count++;
            save_rooms();
            send(client_socket, "Reservation confirmed\n", 23, 0);
        }

        pthread_mutex_unlock(&file_mutex);
        
    }

    
    else if(strncmp(buffer, "SEARCH", 6) == 0){
        pthread_mutex_lock(&file_mutex);

        int beds = 0;
        float price = 0.0;
        
        if (sscanf(buffer, "SEARCH %d %f", &beds, &price) != 2) {
            send(client_socket, "Invalid search format. Use: SEARCH <beds> <max_price>\n", 54, 0);
            pthread_mutex_unlock(&file_mutex);
            pthread_exit(NULL);
        }

        char results[1024];
        int found = 0;

        for(int i=0; i<9; i++){
            if(rooms[i].beds == beds && price >= rooms[i].price_low && price<=rooms[i].price_high){
                snprintf(results, sizeof(results),
                    "Room: %d,Available Beds: %d,Fall to Winter price: %.2f-Spring to Summer Price: %.2f\nRoom Description: %s\n",
                    rooms[i].room_number,
                    rooms[i].beds,
                    rooms[i].price_low,
                    rooms[i].price_high,
                    rooms[i].description
                );
                send(client_socket, results, strlen(results), 0);
                found = 1;
            }
        }
        if(!found){
            send(client_socket, "No rooms found matching criteria.\n\n", 33, 0);
        }
        pthread_mutex_unlock(&file_mutex);
    }





    
    else {
        send(client_socket, "Unknown command\n", 16, 0);
    }

    close(client_socket);
    pthread_exit(NULL);
}





int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    load_rooms();
    printf("Server is running and waiting for clients...\n");

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
            (socklen_t*)&addrlen)) < 0) 
        {
            perror("accept");
            continue;
        }

        int* client_sock = malloc(sizeof(int));
        *client_sock = new_socket;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, client_sock);
        pthread_detach(thread_id);  
    }

    close(server_fd);
    return 0;
}

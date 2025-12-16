#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_RESERVATIONS 100

FILE* hotel_file;
FILE* room_file;


const char* HOTEL_FORMAT="%s\n %s\n %s\n Rooms: %d \n\n";
const char* ROOM_FORMAT="Room: %d,Available Beds: %d,Fall to Winter price: %.2f-Spring to Summer Price: %.2f\nRoom Description: %s\n";
const char* RESERVATION_FORMAT = "Reservations:[%s to %s],Room %d\n\n ";

typedef struct {
    char name[100];
    char location[100];
    char description[256];
    int room_count;
    } Hotel;

    typedef struct{
        char start_date[11];
        char end_date[11]; 
    }Reservation;

typedef struct {
    int room_number;
    int beds;
    float price_low, price_high;
    Reservation reservations[MAX_RESERVATIONS];
    int reservation_count;
    char description[200];
} Room;


void initialize_data(){
    srand(time(0));
    Hotel THE_HOTEL={
        "Hotel Name: Fancy and Nice Hotel",
        "Location: Batman, Turkey",
        "Description: This prestigious(5-stars☪✪✭☆★) Hotel is globally known by its breathtaking view of the artificial lake in every room.",
        9};

    Room room[9]={0};
    for(int i=0;i<9;i++){
            room[i].room_number=i+1;
            
            if(i<3){
                room[i].price_low=19.99;
                room[i].price_high=39.99;
                strcpy(room[i].description,"Single-bed room with free Wi-Fi, including a TV with channels and a bathroom\n\n");
                room[i].beds=1;
                }
            if((i>=3)&&(i<6)){
                room[i].price_low=49.99;
                room[i].price_high=79.99;
                strcpy(room[i].description,"Double-bed room with free Wi-Fi, including a TV with channels and a bathroom.\n\n");
                room[i].beds=1;
                }
            if(i>=6){
                room[i].price_low=49.99;
                room[i].price_high=79.99;
                strcpy(room[i].description,"Two-bed room with free Wi-Fi, including a TV with channels and a bathroom.\n\n");
                room[i].beds=2;
                }
            
        }

        hotel_file=fopen("hotel.txt","w+");
        if(hotel_file==NULL){
            perror("Error opening file");
            }

        fprintf(hotel_file, HOTEL_FORMAT,
            THE_HOTEL.name,
            THE_HOTEL.location,
            THE_HOTEL.description,
            THE_HOTEL.room_count
        );
        fclose(hotel_file);

        room_file=fopen("room.txt","w+");
        for (int i = 0; i < THE_HOTEL.room_count; i++) {
            fprintf(room_file, ROOM_FORMAT,
                room[i].room_number,
                room[i].beds,
                room[i].price_low,
                room[i].price_high,
                room[i].description
                );
            
                
            
        }
        fclose(room_file);
        
        }
        
        


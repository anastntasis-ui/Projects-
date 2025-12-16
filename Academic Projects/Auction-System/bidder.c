#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>


#define MAX_BIDS 100
#define SHM_KEY 1234  

typedef struct {
    int highest_bid;
    int bidder_id;
    int bid_history[MAX_BIDS]; // Arbitrary value
    int bid_count;
    sem_t mutex;
    sem_t round;
} auction_data_t;

int main(int argc,char* argv[]){

    int my_id = atoi(argv[1]);
    srand(time(NULL) + my_id * 1000);

    int shmid=shmget(SHM_KEY,sizeof(auction_data_t),IPC_CREAT|S_IRWXU); 
    if (shmid<0) {
        perror("shmget"); exit(1);
    }
    auction_data_t *auction_data=(auction_data_t *)shmat(shmid, NULL,0);
    if (auction_data==(void*)-1){ 
        perror("shmat"); exit(1); 
    }
    

    for(int i= 0;i< 5;i++){
        sem_wait(&auction_data->round);

        time_t start_time = time(NULL);
        
        while(time(NULL) - start_time < 10){
            int bid=(rand()%100)+1;
            sleep((rand()%3)+1);

            
            sem_wait(&auction_data->mutex);
            if(bid>auction_data->highest_bid){
                auction_data->highest_bid=bid;
                auction_data->bidder_id=my_id;
                if (auction_data->bid_count<MAX_BIDS) {
                    auction_data->bid_history[auction_data->bid_count]=bid;
                    auction_data->bid_count++;
                }
                printf("Bidder %d placed a new highest bid: %d!!!!\n",my_id,bid);

            }else{
                 printf("Bidder %d's bid of %d was too low...\n",my_id,bid);
            }
            sem_post(&auction_data->mutex);
        }

    }
    shmdt(auction_data);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SHM_KEY 1234 
#define MAX_BIDS 100
#define bidders 3

typedef struct {
    int highest_bid;
    int bidder_id;
    int bid_history[MAX_BIDS]; // Arbitrary value
    int bid_count;
    sem_t mutex;
    sem_t round;
} auction_data_t;

int main(){
    int shmid=shmget(SHM_KEY,sizeof(auction_data_t),IPC_CREAT|S_IRWXU); 
    if (shmid<0) {
        perror("shmget"); exit(1);
    }
    auction_data_t *auction_data=(auction_data_t *)shmat(shmid, NULL,0);
    if (auction_data==(void*)-1){ 
        perror("shmat"); exit(1); 
    }
    for(int i=0;i<5;i++){
        sem_wait(&auction_data->mutex);

            auction_data->highest_bid = 0;
            auction_data->bidder_id=-1;
        
        sem_post(&auction_data->mutex);
        
        printf("Round %d is starting!!!!\n",i+1);
        for(int j=0;j<bidders;j++){
            sem_post(&auction_data->round);
        }

        sleep(10);

        sem_wait(&auction_data->mutex);
            printf("Round %d: Number %d has won with an offer of %d$\n", i+1, auction_data->bidder_id, auction_data->highest_bid);
        sem_post(&auction_data->mutex);
    
    }

    sem_wait(&auction_data->mutex);
    printf("Auction history:\n");
        for(int i=0;i<auction_data->bid_count;i++){
            printf("%d \t",auction_data->bid_history[i]);
        }
    sem_post(&auction_data->mutex);
    
    shmdt(auction_data);
    return 0;


}
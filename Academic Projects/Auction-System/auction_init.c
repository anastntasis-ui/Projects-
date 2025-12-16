#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>

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

int main(){ 
    
    int shmid=shmget(SHM_KEY,sizeof(auction_data_t),IPC_CREAT|S_IRWXU);   
    if (shmid<0) {
        perror("shmget");
        exit(1);
    }
    auction_data_t *auction_data=(auction_data_t *)shmat(shmid, NULL,0);
    if(auction_data==(void*)-1){
        perror("shmat");
    }

    
    if(sem_init(&auction_data->mutex,1,1)!=0){
        perror("sem_init mutex");
        exit(1);
    }
    if(sem_init(&auction_data->round, 1, 0)!=0){
        perror("sem_init round");
        exit(1);
    }


    auction_data->highest_bid=0;
    auction_data->bidder_id=-1;
    auction_data->bid_count=0;
    for(int i=0;i<MAX_BIDS;i++){
        auction_data->bid_history[i]=0;
    }
    shmdt(auction_data);

    return 0;
}
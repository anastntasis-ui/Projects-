#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

#define PORT 12345
#define MAX_CLIENTS 10

typedef struct {
    int rank; // Value from 2 to 14 (where 11 = Jack, 12 = Queen, 13 = King, 14 = Ace)
    char suit; // 'H' for Hearts, 'D' for Diamonds, 'C' for Clubs, 'S' for Spades
}Card;

typedef struct{
    char *id; // Dynamic array representing the player’s name
    Card *deck; // Dynamic array of Card structs representing the player's deck
    int deckSize; // Current number of cards in the player's deck
    int score; // Total score of the games a user has played
}Player;

typedef struct{
    Card *main_deck; // Game Deck
    Player player; // Human player
    Player computer; // Computer opponent
}GameState;


GameState cgame;
bool sign_in=false;
int player_count=0;
Player *players=NULL;
int account_select;
Player computer={NULL,NULL,0,0};
int p;
int deck_size=52;
char buffer[250];
int player_rounds[50];


int compareP(const void *a,const void *b){
    const Player *playerA = (const Player *)a;
    const Player *playerB = (const Player *)b;
    return strcmp(playerA->id,playerB->id);
}

void set_id(int client_sock){
    char *name;
    name = (char *)malloc(20 * sizeof(char)); 
    if (!name) {
        perror("malloc failed");
        exit(1);
    }
    char eyn[]="Enter your name! \n";
    send(client_sock, eyn, strlen(eyn), 0);
    
    int n=recv(client_sock,name,19,0);
    if(n > 0) {
        name[n] = '\0';
    }
    char welc[80];
    snprintf(welc,sizeof(welc),"Welcome %s\n!!Make sure to login to start playing!!\n",name);
    send(client_sock,welc,strlen(welc),0);
    
    players = realloc(players, (player_count + 1) * sizeof(Player));  
    players[player_count].id = name;
    players[player_count].deck = NULL; 
    players[player_count].deckSize = 0;
    players[player_count].score = 0;
    player_rounds[player_count]=0;
    
    player_count++;
}

void save_players(){
    qsort(players,player_count,sizeof(Player),compareP);
    FILE* pf=fopen("save_names.txt","w"); 
    if(!pf){
        perror("Cannot save file!");
        return;
    }
    fprintf(pf, "%d\n", player_count);
    for(int i=0; i<player_count; i++) {
        fprintf(pf,"%s\n",players[i].id);
    }
    fclose(pf);
}

void save_game(Card *mdeck,int client_sock) {
    qsort(players, player_count, sizeof(Player), compareP);
    FILE *file;
    FILE *pf;
    file = fopen("save_file.txt", "w");
    if (!file) {
        perror("Error Saving File\n");
        return;
    }
    for (int i = 0; i < player_count; i++){
        fprintf(file, "Player Name: %s\n", players[i].id);
        fprintf(file, "Player Deck Size: %d\n\n", players[i].deckSize);
        }
        
        
        fprintf(file, "\nMain Deck(%d Cards):\n", deck_size);
        for (int j = 0; j < deck_size; j++) {
            fprintf(file, "Card %d: %d %c\n", j + 1, mdeck[j].rank, mdeck[j].suit);
        }
        
    for (int i = 0; i < player_count; i++){
        fprintf(file, "\nPlayer Deck %s:\n", players[i].id);
        for (int j = 0; j < players[i].deckSize; j++) {
            fprintf(file, "Card %d: %d %c\n", j + 1, players[i].deck[j].rank, players[i].deck[j].suit);
        }      
    }
    fprintf(file,"\n-----------------------------------------------------\n");

    fprintf(file, "\nComputer Name: %s\n", computer.id);
    fprintf(file,"Computer Deck Size:%d",computer.deckSize);
    fprintf(file, "\nComputer Deck\n");
    for (int i = 0; i < computer.deckSize; i++) {
        fprintf(file, "Card %d: %d %c\n", i + 1, computer.deck[i].rank, computer.deck[i].suit);
    }
    fprintf(file,"\n-----------------------------------------------------\n");
    
     for (int i = 0; i < player_count; i++) {
        fprintf(file,"\nPlayer Score:%d\n",players[i].score);
        fprintf(file, "\nNumber of rounds: %d\n", player_rounds[i]);
    }
    
    fclose(file);
    char gs[]="Game Saved!\n";
    send(client_sock,gs,strlen(gs),0);
}

void load_game(Card *mdeck,int client_sock){
    FILE *file;
    FILE *pf;
    file=fopen("save_file.txt","r");
    if ((!file)) {
        perror("Error opening file");
        return;
    }
    if(sign_in==false){
        printf("Login to Continue!\n");
        fclose(file);
        return;
    }
    
    char buffer[100];
    char buffer1[100];
    for (int i = 0; i < player_count; i++){
        if (players[i].deck != NULL) free(players[i].deck);
        if (players[i].id != NULL) free(players[i].id);
        
        fscanf(file,"Player Name: %s\n",buffer);
        players[i].id = malloc((strlen(buffer) + 1) * sizeof(char));
        strcpy(players[i].id,buffer);
        fscanf(file, "Player Deck Size: %d\n\n", &players[i].deckSize);
        }
        
        fscanf(file,"\nMain Deck(%d Cards):\n",&deck_size);
    for (int j = 0; j < deck_size; j++){
        fscanf(file, "Card %*d: %d %c\n", &mdeck[j].rank, &mdeck[j].suit);
            }
            
    for (int i = 0; i < player_count; i++){
        fscanf(file, "\nPlayer Deck %s\n", buffer);
        players[i].deck = malloc(players[i].deckSize * sizeof(Card));
        for (int j = 0; j < players[i].deckSize; j++){
            fscanf(file, "Card %*d: %d %c\n", &players[i].deck[j].rank, &players[i].deck[j].suit);
            }
    }
    fscanf(file,"\n-----------------------------------------------------\n");

    char buf[100];
    fscanf(file, "\nComputer Name: %s\n", buf);
    computer.id = malloc(strlen(buf) + 1);
    strcpy(computer.id, buf);       
    fscanf(file,"Computer Deck Size:%d",&computer.deckSize);
    fscanf(file, "\nComputer Deck\n");
    computer.deck = malloc(computer.deckSize * sizeof(Card));
    for (int i = 0; i < computer.deckSize; i++){
        fscanf(file, "Card %*d: %d %c\n", &computer.deck[i].rank, &computer.deck[i].suit);
    }
    fscanf(file,"\n-----------------------------------------------------\n");

    for (int i = 0; i < player_count; i++){
        fscanf(file,"\nPlayer Score:%d\n",&players[i].score);
        fscanf(file, "\nNumber of rounds: %d\n", &player_rounds[i]);
    }

    fclose(file);
    char gl[]="Game Loaded!\n";
    send(client_sock,gl,strlen(gl),0);
    
}

void login(int client_sock){
    if (players) { // Free any old loaded players
        for (int i = 0; i < player_count; i++) {
            if (players[i].id) free(players[i].id);
            if (players[i].deck) free(players[i].deck);
        }
        free(players);
        players = NULL;
    }
    
    FILE *pf;
    pf = fopen("save_names.txt", "r");
    
    fscanf(pf,"%d\n",&player_count);
    players = malloc(player_count * sizeof(Player));

    if(!pf){
        char af[]="You must create an account first\n";
        send(client_sock,af,strlen(af),0);
        return;
    }
    else{
        if(player_count<1){
            char af[]="no existing players!!! CREATE AN ACCOUNT FIRST\n";
            send(client_sock,af,strlen(af),0);
            fclose(pf);
            return;
        }
        for (int i = 0; i < player_count; i++) {
            char buffer[50];
            fscanf(pf, "%s\n", buffer);
            players[i].id = malloc(strlen(buffer) + 1); 
            strcpy(players[i].id, buffer);
        }
        
        fclose(pf);
        qsort(players, player_count, sizeof(Player), compareP);

        char acc[]="Accounts:\n";
        send(client_sock,acc,strlen(acc),0);
        for (int i = 0; i < player_count; i++) {
            char pl[80];
            snprintf(pl,sizeof(pl),"%d. %s\n", i + 1, players[i].id);
            send(client_sock,pl,strlen(pl),0);
            
        }
        
        
        do{
            char sa[]="Select Account(By Number):";
            send(client_sock,sa,strlen(sa),0);

            char select_buf[16];
            recv(client_sock, select_buf, sizeof(select_buf)-1, 0); // διάβασε ως string
            select_buf[sizeof(select_buf)-1] = '\0'; // ασφάλεια για τερματισμό
            account_select = atoi(select_buf);  
            
            
            
            
                if (account_select<1||account_select>player_count)
                {
                    char ic[]="Invalid choice \n";
                    send(client_sock,ic,strlen(ic),0);
                }
            else
                {
                    char as[80];
                    snprintf(as,sizeof(as),"Account Selected: %s\n",players[account_select-1].id);
                    send(client_sock,as,strlen(as),0);
                    sign_in=true;
                }
        }while(account_select<1||account_select>player_count);
    }  
}

void shuffle(Card *mcard,int length){
    srand(time(NULL));
    for(int i=0;i<length;i++){
        int swap_index=rand()%length;
        Card temp=mcard[i];
        mcard[i]=mcard[swap_index];
        mcard[swap_index]=temp;
    }
}

void initialize_deck(Card *mdeck){
    char suits[]={'H','D','C','S'};
    int position=0;
        
        for(int i=0;i<4;i++){
            for(int j=2;j<=14;j++){
                mdeck[position].suit=suits[i];
                mdeck[position].rank=j;
                
                position++;
            }
        }
        shuffle(mdeck,deck_size);
       // printf("Deck initialized! \n");
}

void set_cards(Card *mdeck,int client_sock){
    FILE *pf;
    if((!pf)){
            char af[]="You must create an account first!\n";
            send(client_sock,af,strlen(af),0);
             }
    else if(sign_in==false){
        char lf[]="You must login first!\n";
        send(client_sock,lf,strlen(lf),0);
        
    }
    else{
        do {
            char question[]="How many cards does each player get?\n";
            send(client_sock,question,strlen(question),0);
            

            char select_buf[16];
            recv(client_sock, select_buf, sizeof(select_buf)-1, 0); // διάβασε ως string
            select_buf[sizeof(select_buf)-1] = '\0'; // ασφάλεια για τερματισμό
            p=atoi(select_buf);
            
            
            if ((p > 26)||(p==0)) {
                char invalid[]="Number of cards needs to be below 27 and above 0.\n";
                send(client_sock,invalid,strlen(invalid),0);
                
            }
            
            else{
                Card *player_hand = (Card*) malloc(p * sizeof(Card));
                Card *computer_hand = (Card*) malloc(p * sizeof(Card));
                computer.id="CPU";
                
                for(int i=0;i<p;i++){
                    
                player_hand[i] = mdeck[deck_size - 1];
                deck_size--;
                }  
                for(int i=0;i<p;i++){   
                computer_hand[i]=mdeck[deck_size-1];
                deck_size--;
                } 
                    players[account_select-1].deck = (Card *)malloc(p * sizeof(Card));
                    computer.deck = (Card *)malloc(p * sizeof(Card));

                    for(int i=0;i<p;i++){
                    players[account_select-1].deck[i] =player_hand[i]; 
                    
                    computer.deck[i]=computer_hand[i];
                    }           
                    players[account_select-1].deckSize = p;
                    computer.deckSize=p;
                                        
                                        /*
                                        printf("\nPlayer's Hand:\n");
                                            for (int i = 0; i < p; i++) {
                                                printf("Player Card %d: %d %c\n", i + 1, players[account_select-1].deck[i].rank, players[account_select-1].deck[i].suit);
                                            }

                                            // Display computer's cards
                                            printf("\nComputer's Hand:\n");
                                            for (int i = 0; i < p; i++) {
                                                printf("Computer Card %d: %d %c\n", i + 1, computer.deck[i].rank, computer.deck[i].suit);
                                            }
                                            
                                           printf("Main Deck:\n");
                                           for(int i=0;i<deck_size;i++){
                                            printf("Card %d:%d %c \n",i+1,mdeck[i].rank,mdeck[i].suit);
                                           }
                                          */ 
            }

            } while ((p > 26)||(p==0));
        }
}

void game(Card *mdeck,int client_sock){
    FILE *file;
    FILE *pf;
    if((!pf)&&(!file)){
        
        perror("error loading\n");
        return;
    }
    else if((sign_in==true)){
        char answer;
        int strikes=0;
        char lastWinner;
        do{
        char ready[]="are you ready to play CardGame?\nType:\tY for YES\tor\tN for NO\n";
        send(client_sock,ready,strlen(ready),0);

        char begin[]="are you ready to play CardGame?\nType:\tY for YES\tor\tN for NO\n";
        send(client_sock,begin,strlen(begin),0);
        
        recv(client_sock,&answer,1,0);
        switch (answer){
            case(('Y')):{
                char start[]="CardGame has started\n";
                send(client_sock,start,strlen(start),0);
                
                do{int i=0;
                    {   player_rounds[account_select-1]++;
                         char pc[80];
                         char cc[80];
                         snprintf(pc,sizeof(pc),"Player Card %d is :%d %c \n",i+1,players[account_select-1].deck[i].rank,players[account_select-1].deck[i].suit);
                         send(client_sock,pc,strlen(pc),0);
                         snprintf(cc,sizeof(cc),"Computer Card %d is:%d %c \n",i+1,computer.deck[i].rank,computer.deck[i].suit);
                         send(client_sock,cc,strlen(cc),0);
                         
                        
                        if(players[account_select-1].deck[i].rank>computer.deck[i].rank){
                                char wl[80];
                                snprintf(wl,sizeof(wl),"%s Wins(+3 Points) \t %s loses(-1 Point)\n",players[account_select-1].id,computer.id);
                                send(client_sock,wl,strlen(wl),0);
                                

                                players[account_select-1].score+=3;
                                computer.score-=1;

                                players[account_select-1].deckSize++;
                                players[account_select-1].deck = realloc(players[account_select-1].deck,players[account_select-1].deckSize * sizeof(Card));
                                

                                players[account_select-1].deck[players[account_select-1].deckSize - 1] = computer.deck[i];
                                for (int j = i; j < computer.deckSize - 1; j++) {
                                    computer.deck[j] = computer.deck[j + 1];
                                }

                                computer.deckSize--;
                                computer.deck = realloc(computer.deck, computer.deckSize * sizeof(Card));
                                i++;

                                if(lastWinner=='C'){
                                    strikes++;
                                }
                                else{strikes=0;}
                                lastWinner='P';

                                if(computer.deckSize==0){
                                    char pW[]="Player has won!!!\n";
                                    send(client_sock,pW,strlen(pW),0);
                                    break;
                                    }
                                int choice;
                               do{  
                                    char pick[]="1.Play Next Round\t2.Save And Exit\nChoose an option(By Number):";
                                    send(client_sock,pick,strlen(pick),0);

                                    char select_buf[16];
                                    recv(client_sock, select_buf, sizeof(select_buf)-1, 0); // διάβασε ως string
                                    select_buf[sizeof(select_buf)-1] = '\0'; // ασφάλεια για τερματισμό
                                    choice=atoi(select_buf);
                                    // scanf(" %d",&choice);
                                    if(choice==1){;}

                                    else if(choice==2){
                                        save_game(mdeck,client_sock);
                                        char ret[]="Returning to Menu\n";
                                        send(client_sock,ret,strlen(ret),0);
                                        // printf("Returning to Menu\n");
                                        return;
                                    }

                                    else{
                                        char nopt[]="Thats not an option\n";
                                        send(client_sock,nopt,strlen(nopt),0);
                                        //printf("Thats not an option\n");
                                    }

                                }while((choice!=2)&&(choice!=1));
                                

                        }
                        else if(players[account_select-1].deck[i].rank<computer.deck[i].rank){
                                char wl[80];
                                snprintf(wl,sizeof(wl),"%s Wins(+3 Points) \t %s loses(-1 Point)\n",computer.id,players[account_select-1].id);
                                send(client_sock,wl,strlen(wl),0);
                                // printf("%s Wins(+3 Points) \t %s loses(-1 Point)\n",computer.id,players[account_select-1].id);
                                players[account_select-1].score-=1;
                                computer.score+=3;
                                
                                computer.deckSize++;
                                computer.deck = realloc(computer.deck, computer.deckSize * sizeof(Card));

                                computer.deck[computer.deckSize-1]=players[account_select-1].deck[i];
                                for (int j = i; j < players[account_select-1].deckSize - 1; j++) {
                                    players[account_select-1].deck[j] = players[account_select-1].deck[j + 1];
                                }
                                
                                players[account_select-1].deckSize--;
                                players[account_select-1].deck = realloc(players[account_select-1].deck,players[account_select-1].deckSize * sizeof(Card));
                                i++;
                                if(lastWinner=='P'){
                                    strikes++;
                                }
                                else{strikes=0;}
                                lastWinner='C';
                                if(players[account_select-1].deckSize==0){
                                    char cW[]="Computer has won!!!\n";
                                    send(client_sock,cW,strlen(cW),0);
                                    // printf("Computer has won!!!\n");
                                    break;
                                }
                                int choice;
                                do{

                                    char pick[]="1.Play Next Round\t2.Save And Exit\nChoose an option(By Number):";
                                    send(client_sock,pick,strlen(pick),0);

                                    char select_buf[16];
                                    recv(client_sock, select_buf, sizeof(select_buf)-1, 0); // διάβασε ως string
                                    select_buf[sizeof(select_buf)-1] = '\0'; // ασφάλεια για τερματισμό
                                    choice=atoi(select_buf);
                                    /*
                                    printf("1.Play Next Round\t2.Save And Exit\nChoose an option(By Number):");
                                    scanf(" %d",&choice);
                                    */
                                    if(choice==1){;}

                                    else if(choice==2){
                                        save_game(mdeck,client_sock);
                                        char ret[]="Returning to Menu\n";
                                        send(client_sock,ret,strlen(ret),0);
                                        //printf("Returning to Menu\n");
                                        return;
                                    }

                                    else{
                                        char nopt[]="Thats not an option\n";
                                        send(client_sock,nopt,strlen(nopt),0);
                                        //printf("Thats not an option\n");
                                    }
                                }while((choice!=2)&&(choice!=1));
                                
                                
                        }
                        else{   char eq[]="Equal! Changing Cards...\n";
                                send(client_sock,eq,strlen(eq),0);
                                //printf("Equal! Changing Cards...\n");
                                if(deck_size>0){
                                    mdeck[deck_size]=players[account_select-1].deck[i];
                                    deck_size++;
                                    mdeck[deck_size]=computer.deck[i];
                                    deck_size++;
                                    shuffle(mdeck,deck_size);
                                    players[account_select-1].deck[i]=mdeck[deck_size-1];
                                    deck_size--;
                                    computer.deck[i]=mdeck[deck_size-1];
                                    deck_size--;
                                }
                                else{
                                    char ineq[]="Main Deck has no cards unable to solve inequality\nGAME ENDS HERE!\n";
                                    send(client_sock,ineq,strlen(ineq),0);
                                    //printf("Main Deck has no cards unable to solve inequality\nGAME ENDS HERE!\n");
                                    
                                    if(players[account_select-1].score>computer.score){
                                    char pW[]="Player has wins!\n";
                                    send(client_sock,pW,strlen(pW),0);
                                    //printf("Player Wins!\n");
                                }
                                else{
                                    char cW[]="Computer Wins!\n";
                                    send(client_sock,cW,strlen(cW),0);
                                   // printf("Computer Wins!\n");
                                }
                                    break;
                                }
                                
                                
                        }
                        if(strikes>=3){
                            shuffle(players[account_select-1].deck,players[account_select-1].deckSize);
                            shuffle(computer.deck,computer.deckSize);
                            strikes=0;
                        }
                }}while((computer.deckSize!=0)&&(players[account_select-1].deckSize!=0));
                break;
            }
            case(('N')):{
                char ret[]="Returning to Menu\n";
                send(client_sock,ret,strlen(ret),0);
                // printf("Returning to menu\n");
                break;
            }
            default:{
                char nopt[]="Thats not an option\n";
                send(client_sock,nopt,strlen(nopt),0);
                // printf("That is not an option\n");
            }
        }
    }while((answer!='Y')&&(answer!='N'));
    }
     
}

int binary_search(Player* players,char* e,int l, int r){
    int mid=l+(r-l)/2;

    if(l>r){
        return -1;
    }
    int cmp=strcmp(players[mid].id,e);
    if(cmp==0){
        return mid;
    }
    else if(cmp>0){
        return binary_search(players,e,l,mid-1);    
    }
    else{
        return binary_search(players,e,mid+1,r);
    }
        }

void score(Player* players,int client_sock){
        if (!sign_in) {
            char lf[]="You must log in first.\n";
            send(client_sock,lf,strlen(lf),0);
            // printf("You must log in first.\n");
            return;
        }
        int temp;
        
        Player* playersd = malloc(player_count * sizeof(Player));
        if (!playersd) {perror("malloc");exit(1);}
        for(int i=0;i<player_count;i++){
            playersd[i].score=players[i].score;
            playersd[i].id=strdup(players[i].id);
        }
        for(int i=0;i<player_count-1;i++){
            for(int j=0;j<player_count-1;j++){
                if(playersd[j].score<playersd[j+1].score){
                    temp=playersd[j].score;
                    playersd[j].score=playersd[j+1].score;
                    playersd[j+1].score=temp;
                }
            }
        }
        int show = player_count < 3 ? player_count : 3;
        for(int i=0;i<show;i++){
            char sc[100];
            snprintf(sc,sizeof(sc),"Number %d: %s, with a score of %d",i+1,playersd[i].id,playersd[i].score);
            send(client_sock,sc,strlen(sc),0);
            // printf("Number %d: %s, with a score of %d",i+1,players[i].id,players[i].score);
        }
        for (int i=0;i< player_count; i++){
            free(playersd[i].id);
        }
    }


void menu(int client_sock){
    Card *mdeck;
    mdeck=(Card*) malloc(52*sizeof(Card));
    int select;
    initialize_deck(mdeck);
    do{
        char pick[]="1. Create an account\n2. Login\n3. New Game\n4. Save Game\n5. Continue Game\n6. Show score\n7. Exit (and refresh the file)\nPick an option(by number):";
        send(client_sock,pick,strlen(pick),0);
        /*
        printf("1. Create an account\n2. Login\n3. New Game\n4. Save Game\n5. Continue Game\n6. Show score\n7. Exit (and refresh the file)\n");
        printf("Pick an option:");
        */
       char select_buf[16];
        recv(client_sock, select_buf, sizeof(select_buf)-1, 0); // διάβασε ως string
        select_buf[sizeof(select_buf)-1] = '\0'; // ασφάλεια για τερματισμό
        select = atoi(select_buf);
        // scanf("%d",&select);
        switch(select)
        {
        case 1:
        set_id(client_sock);
        save_players();
        break;
        
        case 2:
        login(client_sock);
        break;

        case 3:
        set_cards(mdeck,client_sock);
        game(mdeck,client_sock);
        break;

        case 4:
        save_game(mdeck,client_sock); 
        save_players(mdeck);   
        break;
        
        case 5:
        load_game(mdeck,client_sock);
        game(mdeck,client_sock);  
        break;

        case 6:
        score(players,client_sock);
        break;

        case 7: 
        save_game(mdeck,client_sock);
        save_players(mdeck);
        char exit[]="Exit Complete\n";
        send(client_sock,exit,strlen(exit),0);
        //printf("Exit Complete\n");
        free(mdeck);
        for (int i = 0; i < player_count; i++) {
            free(players[i].id);
        }
        free(players);
        break;
        
        case 8:
        
        break;
        default:
        char nopt[]="THATS NOT AN OPTION!!!!\n";
        send(client_sock,nopt,strlen(nopt),0);
        // printf("THATS NOT AN OPTION!!!!\n");
        break;
        }
    }while (select!=7);
}



int main(){
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_sock=socket(AF_INET,SOCK_STREAM,0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // οποιαδήποτε IP του μηχανήματος
    server_addr.sin_port = htons(PORT);       // πόρτα 12345    
    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(server_sock, MAX_CLIENTS);
    printf("The server has started.......!!!!\n");
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) continue;
        if (fork() == 0){
            close(server_sock);
            menu(client_sock);
            exit(0);
        }
    }
    return 0;
}
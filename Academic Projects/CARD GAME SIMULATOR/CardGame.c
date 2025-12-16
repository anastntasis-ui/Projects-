#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>



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


bool sign_in=false;
int player_count=0;
Player *players=NULL;
int account_select;
Player computer={NULL,NULL,0,0};
int p;
int deck_size=52;
FILE *pf;
char buffer[250];
int player_rounds[50];
FILE *file;

int compareP(const void *a,const void *b){
    const Player *playerA = (const Player *)a;
    const Player *playerB = (const Player *)b;
    return strcmp(playerA->id,playerB->id);
}


void set_id(){
    char *name;
    name = (char *)malloc(20 * sizeof(char)); 
    printf("Enter your name! \n");
    scanf("%s",name);

    printf("Welcome %s \n!!Make sure to login to start playing!!\n",name);
    
    players = realloc(players, (player_count + 1) * sizeof(Player));  
    players[player_count].id = name;
    players[player_count].deck = NULL; 
    players[player_count].deckSize = 0;
    players[player_count].score = 0;
    player_rounds[player_count]=0;
    
    player_count++;
}

void save_players(){
    int existing_count = 0;

    pf=fopen("save_names.txt","r+");
    if(!pf){
        pf=fopen("save_names.txt","w");
        fprintf(pf,"%d \n",player_count);
        }

    else{
        qsort(players, player_count, sizeof(Player), compareP);
        fscanf(pf, "%d", &existing_count);
        
        
        fprintf(pf, "%d\n", existing_count);
        fseek(pf, 0, SEEK_END);
        for (int i = existing_count; i < player_count; i++) {
            fprintf(pf, "%s\n", players[i].id);
        }
        rewind(pf);
        fprintf(pf, "%d\n", player_count);
        }

    fclose(pf);
}

void save_game(Card *mdeck) {
    qsort(players, player_count, sizeof(Player), compareP);

    file = fopen("save_file.txt", "w");
    if (!file) {
        perror("Error Saving File\n");
        return;
    }
    fprintf(file,"-----------------------------------------------------\n");
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

    fprintf(file, "\nComputer Name: %s \n", computer.id);
    fprintf(file,"Computer Deck Size:%d",computer.deckSize);
    fprintf(file, "\nComputer Deck:\n");
    for (int i = 0; i < computer.deckSize; i++) {
        fprintf(file, "Card %d: %d %c\n", i + 1, computer.deck[i].rank, computer.deck[i].suit);
    }
    fprintf(file,"\n-----------------------------------------------------\n");
    
     for (int i = 0; i < player_count; i++) {
        fprintf(file,"\nPlayer %s Score:%d\n",players[i].id, players[i].score);
        fprintf(file, "\nPlayer %s Number of rounds: %d\n",players[i].id, player_rounds[i]);
    }
    
    fclose(file);
    printf("Game Saved!\n");
}

void load_game(Card *mdeck){
    
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
    fscanf(file,"-----------------------------------------------------\n");
    for (int i = 0; i < player_count; i++){
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

    fscanf(file, "\nComputer Name: %s\n",buffer1);
    computer.id = malloc((strlen(buffer1) + 1) * sizeof(char));
    strcpy(computer.id,buffer1);

    fscanf(file,"Computer Deck Size:%d",&computer.deckSize);
    fscanf(file, "\nComputer Deck:\n");
    computer.deck = malloc(computer.deckSize * sizeof(Card));
    for (int i = 0; i < computer.deckSize; i++){
        fscanf(file, "Card %*d: %d %c\n", &computer.deck[i].rank, &computer.deck[i].suit);
    }
    fscanf(file,"\n-----------------------------------------------------\n");

    for (int i = 0; i < player_count; i++){
        fscanf(file,"\nPlayer %*s Score:%d\n",&players[i].score);
        fscanf(file, "\nPlayer %*s Number of rounds: %d\n", &player_rounds[i]);
    }

    fclose(file);
    printf("Game Loaded!\n");
    
}

void login(){
    pf = fopen("save_names.txt", "r");
    fscanf(pf,"%d\n",&player_count);
    players = malloc(player_count * sizeof(Player));

    if(((!pf)&&(!file))||player_count < 1){
    printf("You must create an account first\n"); }

    else{
        
        for (int i = 0; i < player_count; i++) {
            char buffer[50];
            fscanf(pf, "%s\n", buffer);
            players[i].id = malloc(strlen(buffer) + 1); 
            strcpy(players[i].id, buffer);
        }

        fclose(pf);
        qsort(players, player_count, sizeof(Player), compareP);

        printf("Accounts:\n");
        for (int i = 0; i < player_count; i++) {
            printf("%d. %s\n", i + 1, players[i].id);
        }
        
        
        do{
        printf("Select Account(By Number):");
        scanf("%d",&account_select);
        
            if (account_select<1||account_select>player_count)
            {
             printf("Invalid choice \n");
            }
         else
            {
             printf("Account Selected: %s\n",players[account_select-1].id);
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

void set_cards(Card *mdeck){
    
    if((!pf)){
            printf("You must create an account first!\n"); }
    else if(sign_in==false){
        printf("You must login first!\n");
    }
    else{
        do {
            printf("How many cards does each player get?\n");
            scanf("%d", &p);
            
            if ((p > 26)||(p==0)) {
                printf("Number of cards needs to be below 27 and above 0.\n");
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
                    players[account_select-1].deckSize = p;
                    
                    computer.deck[i]=computer_hand[i];
                    computer.deckSize=p;
                    }                  /*
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

void game(Card *mdeck){
    if((!pf)&&(!file)){
        printf("error loading\n");
    }
    else if((sign_in==true)){
        char answer;
        int strikes=0;
        char lastWinner;
        if(players[account_select-1].deckSize==0||computer.deckSize==0){
            printf("Current Game is Finished\n");
            return;
        }
        do{
        printf("are you ready to play CardGame?\nType:\tY for YES\tor\tN for NO\n");
        scanf(" %c",&answer);
        switch (answer){
            case(('Y')):{
                printf("CardGame has started\n");
                
                do{int i=0;
                {       
                        shuffle(players[account_select-1].deck,players[account_select-1].deckSize);
                        shuffle(computer.deck,computer.deckSize);
                        player_rounds[account_select-1]++;
                        printf("Player Card %d is :%d %c \n",i+1,players[account_select-1].deck[i].rank,players[account_select-1].deck[i].suit);
                        printf("Computer Card %d is:%d %c \n",i+1,computer.deck[i].rank,computer.deck[i].suit);
                        
                        if(players[account_select-1].deck[i].rank>computer.deck[i].rank){
                                printf("%s Wins(+3 Points) \t %s loses(-1 Point)\n",players[account_select-1].id,computer.id);

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
                                    printf("Player has won!!!\n");
                                    break;
                                    }
                                int choice;
                               do{
                                    printf("1.Play Next Round\t2.Save And Exit\nChoose an option(By Number):");
                                    scanf(" %d",&choice);
                                    if(choice==1){;}

                                    else if(choice==2){
                                        save_game(mdeck);
                                        printf("Returning to Menu\n");
                                        return;
                                    }

                                    else{printf("Thats not an option\n");}

                                }while((choice!=2)&&(choice!=1));
                                

                        }
                        else if(players[account_select-1].deck[i].rank<computer.deck[i].rank){
                                printf("%s Wins(+3 Points) \t %s loses(-1 Point)\n",computer.id,players[account_select-1].id);
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
                                    printf("Computer has won!!!\n");
                                    break;
                                }
                                int choice;
                                do{
                                    printf("1.Play Next Round\t2.Save And Exit\nChoose an option(By Number):");
                                    scanf(" %d",&choice);
                                    if(choice==1){;}

                                    else if(choice==2){
                                        save_game(mdeck);
                                        printf("Returning to Menu\n");
                                        return;
                                    }

                                    else{printf("Thats not an option\n");}
                                }while((choice!=2)&&(choice!=1));
                                
                                
                        }
                        else{
                                printf("Equal! Changing Cards...\n");
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
                                    printf("Main Deck has no cards unable to solve inequality\nGAME ENDS HERE!\n");
                                    
                                    if(players[account_select-1].score>computer.score){
                                    printf("Player Wins!\n");
                                    save_game(mdeck);
                                }
                                else{
                                    printf("Computer Wins!\n");
                                    save_game(mdeck);
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
                printf("Returning to menu\n");
                break;
            }
            default:{
                printf("That is not an option\n");
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

    void score(Player* players){
        if (!sign_in) {
            printf("You must log in first.\n");
            return;
        }

        char id[50];
        printf("Enter player id:");
        scanf("%s",id);

        qsort(players, player_count, sizeof(Player), compareP);
        int bsearch=binary_search(players,id,0,player_count-1);
        if(bsearch!=-1){
            printf("Player %s score is:%d \n",players[bsearch].id,players[bsearch].score);
        }
        else{
            printf("\nplayer not found\n");
        }
    }

void menu(Card *mdeck){

    int select;
    do{
        printf("1. Create an account\n2. Login\n3. New Game\n4. Save Game\n5. Continue Game\n6. Show score\n7. Exit (and refresh the file)\n");
        printf("Pick an option:");
        
        scanf("%d",&select);
        switch(select)
        {
        case 1:
        set_id();
        save_players();
        break;
        
        case 2:
        login();

        break;

        case 3:
        initialize_deck(mdeck);
        set_cards(mdeck);
        game(mdeck);
        break;

        case 4:
        save_game(mdeck);    
        break;
        
        case 5:
        load_game(mdeck);
        game(mdeck);  
        break;

        case 6:
        score(players);
        break;

        case 7: 
        save_game(mdeck);
        printf("Exit Complete\n");
        free(mdeck);
        for (int i = 0; i < player_count; i++) {
            free(players[i].id);
        }
        free(players);
        free(computer.id);
        break;

        default:
        printf("THATS NOT AN OPTION!!!!\n");
        break;


        case 8:
        printf("\n%d\n",player_count);
        break;
        }
        
    }while (select!=7);
}

int main(){

    Card *main_deck;
    main_deck=(Card*) malloc(52*sizeof(Card));
    menu(main_deck);
    
    return 0;
}
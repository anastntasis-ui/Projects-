#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "test.h"
int main(){
    initialize_data();
    FILE* fp = fopen("reservations.txt", "w");
    fprintf(fp,"\n");
    fclose(fp);
    
}



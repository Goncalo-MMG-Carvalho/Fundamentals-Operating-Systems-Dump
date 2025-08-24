//
// Created by fso team
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>

#include <errno.h>

#include "comm.h"

#define DISK_BLOCK_SIZE 4096  // assuming 4K pages and disk blocks
#define ARGVMAX 100
#define LINESIZE 1024

// int division rounding up
#define DIVUP(X,Y) ( (X+Y-1)/Y )

int server_inbox;
int my_inbox;
char *data_mem;  // buffer of accessed blocks


void mem_init(int size) {
    // Define with mmap a protected memory buffer to save file blocks received from
    // server and just request from server the really accessed blocks (not all the blocks)
    
    // use anonymous memory = not supported by a file
    // add one more page just in case the return pointer is not aligned with pages

    // pointer do mmaped memory aligned to a multiple of DISK_BLOCK_SIZE
    data_mem = (char*) mmap(NULL, size + DISK_BLOCK_SIZE, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1 , 0);
    
    if(data_mem == MAP_FAILED){
        perror("Mapping Failed\n");
        exit(0);
    }
}

void mem_free(int size) {
    munmap(data_mem, size);
}


void handle_SegFault(int sig, siginfo_t *info, void *idk){

    //use info to calculate the block
    int block = ((info->si_addr - ((void *) data_mem )) / DISK_BLOCK_SIZE); //parte inteira da divisao, nao se aredonda para cima pq os blocos comecao do 0
   
    // change protection with mprotect
    char * arrOffset = data_mem + (block * DISK_BLOCK_SIZE);

    if(mprotect(arrOffset, DISK_BLOCK_SIZE, PROT_READ | PROT_WRITE) == -1){
        perror("Deu erro no mprotect");
        exit(0);
    }
    
    //write to server the number of block needed
    send_int(server_inbox, block);

    // copy the contents to the page 
    read(my_inbox, arrOffset, DISK_BLOCK_SIZE);

    // copy the contents to the page
}

void sig_init() {
    struct sigaction act;
    // set handler to request a newly accessed block
    act.sa_flags = SA_SIGINFO; 
    act.sa_sigaction = handle_SegFault;
    sigaction(SIGSEGV, &act, NULL);
}

/* makeargv - build an argv vector from words in a string
 * in: s points a text string with words
 * out: argv[] points to all words in the string s (*s is modified!)
 * pre: argv is predefined as char *argv[ARGVMAX]
 * return: number of words pointed to by argv (or -1 in case of error)
 */
int makeargv(char *s, char *argv[ARGVMAX]) {
    int ntokens = 0;

    if (s == NULL || argv == NULL || ARGVMAX == 0)
        return -1;
    argv[ntokens] = strtok(s, " \t\n");
    while ((argv[ntokens] != NULL) && (ntokens < ARGVMAX)) {
        ntokens++;
        argv[ntokens] = strtok(NULL, " \t\n");
    }
    argv[ntokens] = NULL; // it must terminate with NULL
    return ntokens;
}


void prompt() {
    printf("sh> ");
    fflush(stdout); //writes the prompt
}

int main(int argc, char* argv[]) {
	int nWords;
	char *words[ARGVMAX];
	char line[LINESIZE];
	int lineNo;

    if (argc != 3 && argc != 5) {
        printf("usage: %s lineSize numberOfLines [client_inbox_name server_inbox_name]\n", argv[0]);
        return 1;
    }
	const int lineSize = atoi(argv[1]);
	const int numberOfLines = atoi(argv[2]);
    const int file_size = lineSize*numberOfLines;
    if (file_size == 0) {
        printf("wrong max file size");
        return 2;
    }
    mem_init(file_size);
    sig_init();

    const char* client_inbox_name = argc == 5 ? argv[3] : default_client_inbox_name;
    const char* server_inbox_name = argc == 5 ? argv[4] : default_server_inbox_name;
	
    create_inbox(client_inbox_name);
    printf ("Created inbox.\n");
    // connect to server
    server_inbox = connect(server_inbox_name);
    // create client pipe
    my_inbox = accept(client_inbox_name);

   while(1){ 
        prompt();
        if (fgets(line, LINESIZE, stdin) == NULL) break;

        nWords = makeargv( line, words );
        if ((nWords > 0) && (nWords <=2)){
			if(strcmp("q", words[0]) == 0){ // if words[0] == "q", then quit
				printf("Bye!!\n");
				break;
			}
			if( strcmp("s", words[0] )== 0){ // if words[0] == "s", then ask for a line in the mapped file
						// asking for register N
				lineNo = atoi(words[1]);
				// read line from memory buffer (data_mem) and print to stdout
                off_t offset = lineSize * lineNo;

                char buff[lineSize];
                memcpy(buff, &(data_mem[offset]), lineSize);
                write(1, buff, lineSize);
			}
			else
				printf("Unknown command\n");
		}
		else
			printf("Wrong format\n");
    }

    // close files/pipes
    close(server_inbox);
    unlink(client_inbox_name);
    mem_free(file_size);
    return 0;
}
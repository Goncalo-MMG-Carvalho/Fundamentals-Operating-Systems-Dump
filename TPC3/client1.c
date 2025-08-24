//
// Created by herve on 25-09-2022.
//

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include "comm.h"

// int division rounding up
#define DIVUP(X,Y) ( (X+Y-1)/Y )
#define BLOCK_SIZE (4096)

int main(int argc, char* argv[]) {
    if (argc != 4 && argc != 2) {
        printf("usage: %s max_file_size [client_inbox_name server_inbox_name]", argv[0]);
        return 1;
    }
    const int file_size = atoi(argv[1]);
    if (file_size==0) {
        printf("wrong max file size");
        return 2;
    }
    const char* client_inbox_name = argc == 4 ? argv[2] : default_client_inbox_name;
    const char* server_inbox_name = argc == 4 ? argv[3] : default_server_inbox_name;
    create_inbox(client_inbox_name);
    printf ("Created inbox.\n");
    // connect to server
    int server_inbox = connect(server_inbox_name);
    // create client pipe
    int my_inbox = accept(client_inbox_name);
    // request all file bytes (until file_size) and print to stdout
    int number_of_blocks = DIVUP(file_size, BLOCK_SIZE);
    
    int left_to_read = file_size;
    for (int i = 0; i < number_of_blocks; i++) // to handle all
    {
        char buff[BLOCK_SIZE];

        send_int(server_inbox, i);
        int nRead = read(my_inbox, buff, BLOCK_SIZE);
        nRead = nRead < left_to_read? nRead : left_to_read;
        write(1, buff, nRead);
        left_to_read -= nRead;
    }
    
    // close files/pipes
    close(server_inbox);
    unlink(client_inbox_name);

    return 0;
}


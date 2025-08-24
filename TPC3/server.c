//
// Created by FSO team
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include "comm.h"

#define DISK_BLOCK_SIZE (4096)          // assuming 4K pages and disk blocks
#define DIVUP(X, Y) ((X + Y - 1) / Y) // int division rounding up

char *server_inbox_name;

// handle SIGINT when Ctrl-C is pressed.
void handle_CTRLC(int sig)
{
    printf("Shutting down.\n");
    // remove server inbox
    unlink(server_inbox_name);
    // exit process
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 2 && argc != 4)
    {
        printf("usage: %s file_name [client_inbox_name server_inbox_name]", argv[0]);
        return 1;
    }
    signal(SIGINT, handle_CTRLC);

    const char *client_inbox_name = argc == 4 ? argv[2] : default_client_inbox_name;
    server_inbox_name = argc == 4 ? argv[3] : (char *)default_server_inbox_name;
    const int file = open(argv[1], O_RDONLY);
    if (file == -1)
    {
        perror(argv[1]);
        return 2;
    }
    struct stat st;
    fstat(file, &st);
    const long file_size = st.st_size;
    printf("File size %ld\n", file_size);
    // create inbox
    create_inbox(server_inbox_name);
    printf("Created inbox\n");

    size_t last_block = DIVUP(file_size, DISK_BLOCK_SIZE);

    while (1)
    {
        // Accept client connections
        int server_inbox = accept(server_inbox_name);
        printf("Accepted new connection.\n");

        // connect to client inbox
        int client_inbox = connect(client_inbox_name);

        // Handle client requests until client closes connections
        int nblock = 0;
		int nread = 0;
        while (recv_int(server_inbox, &nblock) > 0)
        {
            if(nblock <= last_block) {
                char buff[DISK_BLOCK_SIZE];

				lseek(file, DISK_BLOCK_SIZE*nblock, SEEK_SET);          
                nread = read(file, buff, DISK_BLOCK_SIZE);
                write(client_inbox, buff, nread);
            }
        }

        // close inboxs
        close(server_inbox);
        close(client_inbox);
    }
}

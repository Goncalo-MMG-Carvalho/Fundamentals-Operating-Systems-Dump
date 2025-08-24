/**
 * Created by the FSO 2022/23 team.
 */



#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "comm.h"
#include "request.h"



int main(int argc, char* argv[]) {

    if (argc > 3 || argc < 2) {
        printf("usage: %s file_name [server_inbox_name]", argv[0]);
        return 1;
    }

    const char* server_inbox_name = (argc == 3 ? argv[2] : default_server_inbox_name);
    int server_inbox = connect(server_inbox_name);     // : connect to server
    if (server_inbox == -1) {
        perror("connect");
        return errno;
    }
    printf ("Connected to server.\n"/*, server_inbox_name*/);

    char my_inbox_name[32];
    sprintf(my_inbox_name, "/tmp/fso_client_%d", getpid());

    // create client pipe
    int inbox_ok = create_inbox(my_inbox_name); // : create client inbox
    if (inbox_ok == -1) {
        perror("create inbox");
        return errno;
    }
    printf ("Created inbox.\n");

    request req;// TODO: create request req
    strcpy(req.reply_to, my_inbox_name);
    strcpy(req.file_name, argv[1]);

    // TODO: send request to server
    /*int e = */send_request(server_inbox, &req);
    /*if(!e){
        //Nao sei o q fazer aqui, nao encontro explicacao em lado nenhum
        //perror("send request");
        close(server_inbox);
        return e;
    }*/
    printf ("Sent request for file %s.\n", req.file_name);

    close(server_inbox);

    printf ("Waiting for the server's response.\n");

    int my_inbox = accept(my_inbox_name); // TODO: accept server connection
    if (my_inbox == -1) {
        perror("accept");
        return errno;
    }

    unsigned status;
    int reply = read(my_inbox,  &status, 4 ); // TODO: read the server's reply
    if (reply == 0)
        status = ENETUNREACH; // Problem with the connection of the server side. Error: network out of reach

    printf ("Response status: %s.\n", strerror(status));
    if (!status) {
        printf ("Waiting for file...");
        // TODO create local file and write cotents

        int localfile = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT);
        
        //off_t file_size = lseek(my_inbox, 0, SEEK_END); //don't know if it works with fifos
        
        unsigned char buff;
        while(read(my_inbox, &buff,1)){
            write(localfile, &buff, 1);
        }
        
        close(localfile);
        
        printf (" done.\n");
    }

    close(my_inbox);
    unlink(my_inbox_name);

    return 0;
}


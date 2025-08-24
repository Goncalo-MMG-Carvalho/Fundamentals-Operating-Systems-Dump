/**
 * Created by the FSO 2022/23 team.
 */


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include <pthread.h> // tive de acrescentar isto, preciso do pthread_create() e pthread_detatch()

#include "comm.h"
#include "request.h"


void* handle_request(void* req);

int main(int argc, char* argv[]) {

    if (argc > 2) {
        printf ("usage: %s [inbox_name]", argv[0]);
        return 1;
    }

    // create inbox
    const char* inbox_name = (argc == 1 ? default_server_inbox_name: argv[1]);
    int inbox_ok = create_inbox(inbox_name);
    if (inbox_ok == -1) {
        perror("create inbox");
        return errno;
    }

    printf ("Created inbox.\n");

    while (1) {

        // TODO: Accept client connections
        int inbox = accept(inbox_name);
        if (inbox == -1) {
            perror("accept");
            continue;
        }
        printf ("Accepted new connection.\n");

        // Receive request
        request req;

        

        int request_ok = recv_request(inbox, &req); // TODO: receive request
        if (request_ok == -1) {
            perror("Receive request");
            continue;
        }
        printf("New request: send file %s.\n", req.file_name);

        //2 LINHAS A ADICIONAR, sao estas
        read(inbox, NULL, 1);
        close(inbox);

        // Handle request
        
        //handle_request(&req);
        
        pthread_t id;
        pthread_create(&id, NULL, handle_request, (void*)(&req));
        pthread_detach(id);

        printf("Request (file %s) handled by new thread.\n", req.file_name);
        
    }
}

//client inbox is a file descriptor of an opened file with write settings
void write_status(int client_inbox){ 
    int status = errno;
    write(client_inbox, &status, 4);
    errno = status;
}

void print_end_status(int status){
    printf ("Thread ended with status: %s.\n", strerror(status));
}

void* handle_request(void* pedido) {
    // TODO: handle the request in a new execution flow: process or thread
    // this one is using threads
    
    const request* req = pedido;

    int client_inbox = connect(req->reply_to);

    if(client_inbox == -1){
        print_end_status(errno);
        return NULL;
    }
    printf("Connected to inbox.\n");

    int file = open(req->file_name, O_RDONLY);
    if(file == -1){
        write_status(client_inbox);
        print_end_status(errno);
        return NULL;
    }
    printf ("Opened file %s.\n", req->file_name);

    off_t file_size = lseek(file, 0, SEEK_END);
    lseek(file, 0, SEEK_SET);

    char buff[file_size];
    
    ssize_t readsize = read(file, buff, file_size);
    if(readsize != file_size){
        write_status(client_inbox);
        print_end_status(errno);
        return NULL;
    }

    errno = 0;
    write_status(client_inbox);
    /*ssize_t writesize = */write(client_inbox, buff, file_size);

    printf ("Sent file %s.\n", req->file_name);
    errno = 0;
    print_end_status(errno);
    close(client_inbox);
    close(file);
    
    return NULL;
}
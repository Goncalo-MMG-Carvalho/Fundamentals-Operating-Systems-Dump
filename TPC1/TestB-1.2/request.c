/**
 * Created by the FSO 2022/23 team.
 */
#include <string.h>
#include <unistd.h>

#include "request.h"

#define BUFF_INT_SIZE (4) 

int send_request(int inbox, const request *req) {
    // TODO: allocate buffer to store the serialization of the request to be sent to the server
    
    int size_reply_to = strlen(req->reply_to); // int because it needs to be 4 bytes long
    int size_file_name = strlen(req->file_name); // int because it it needs to be 4 bytes long
    int size_buff = BUFF_INT_SIZE + size_reply_to + BUFF_INT_SIZE + size_file_name;
    unsigned char buff[size_buff];
    unsigned char *start = buff;
    // TODO: fill the buffer with the request's data
    
    memcpy(start, &size_reply_to, BUFF_INT_SIZE); start += BUFF_INT_SIZE;
    memcpy(start, req->reply_to, size_reply_to); start += size_reply_to;
    
    memcpy(start, &size_file_name, BUFF_INT_SIZE); start += BUFF_INT_SIZE;
    memcpy(start, req->file_name, size_file_name);

    // TODO: write the request in the server's inbox

    return write(inbox, buff, size_buff) == size_buff ? 0 : -1;
}


int recv_request(int inbox, request *req) {
    // TODO: read the request from the inbox
    // TODO: fill req with the content's read from the inbox
    int size_reply_to = 0;
    int size_file_name = 0;
    
    if(read(inbox, &size_reply_to, BUFF_INT_SIZE) != BUFF_INT_SIZE)
        return -1;
    if(read(inbox, req->reply_to, size_reply_to) != size_reply_to)
        return-1;
    req->reply_to[size_reply_to] = '\0';

    if(read(inbox, &size_file_name, BUFF_INT_SIZE) != BUFF_INT_SIZE)
        return -1;
    if(read(inbox, req->file_name, size_file_name) != size_file_name)
        return -1;
    req->file_name[size_file_name] = '\0';

    return 0;
}

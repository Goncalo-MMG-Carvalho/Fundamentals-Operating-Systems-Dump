/**
 * Created by the FSO 2022/23 team.
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


int create_inbox(const char* inbox_name) {
    // TODO
    
    int i = mkfifo(inbox_name, 0666);
    
    if(errno == EEXIST){
        unlink(inbox_name);
        i = mkfifo(inbox_name, 0666);
    }
    
    //mkfifo already sets errno
    return i;
}

int accept(const char* inbox_name) { 
    // TODO
    return open(inbox_name, O_RDONLY); //open already sets errno
}

int connect(const char* inbox_name) {
    // TODO
    return open(inbox_name, O_WRONLY); //open already sets errno
}

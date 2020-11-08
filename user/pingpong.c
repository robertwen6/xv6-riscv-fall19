#include "kernel/types.h"
#include "user/user.h"

int main(int argn, char *argv[]){
	int pid;
    char buf[4];

    int p1[2];
    int p2[2];

    pipe(p1);
    pipe(p2);

    pid = fork();

    if(pid == 0){
        close(p2[0]);
        close(p1[1]);
        if(read(p1[0], buf, sizeof(buf)) == 4){
            printf("%d: received ping\n", getpid());
            buf[0] = 'p';
            buf[1] = 'o';
            buf[2] = 'n';
            buf[3] = 'g';
        }
        if(write(p2[1], buf, sizeof(buf)) != 4){
            printf("write error!");
        }
        exit();
    }
    else{
        buf[0] = 'p';
        buf[1] = 'i';
        buf[2] = 'n';
        buf[3] = 'g';
        close(p2[1]);
        close(p1[0]);
        if(write(p1[1], buf, sizeof(buf)) != 4){
            printf("write error!");
        }
        if(read(p2[0], buf, sizeof(buf)) == 4){
            printf("%d: received pong\n", getpid());
        }
    }
    exit();
}
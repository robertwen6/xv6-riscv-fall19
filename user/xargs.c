#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    int i, j, k, a, b;
    char buf[32];
    char buf2[32];
    char *p = buf;
    char *x[32];
    j = 0;
    a = 0;
    for(i=1; i<argc; i++){
        x[j] = argv[i];
        j++;
    }
    while((k = read(0, buf2, sizeof(buf2))) > 0){
        for(b=0; b<k; b++){
            if(buf2[b] == '\n'){
                buf[a] = 0;
                a = 0;
                x[j] = p;
                j++;
                p = buf;
                x[j] = 0;
                j = argc - 1;
                if(fork() == 0){
                    exec(argv[1], x);
                }                
                wait();
            }
            else if(buf2[b] == ' ') {
                buf[a] = 0;
                a++;
                x[j] = p;
                j++;
                p = &buf[a];
            }
            else {
                buf[a] = buf2[b];
                a++;
            }
        }
    }
    exit();
}
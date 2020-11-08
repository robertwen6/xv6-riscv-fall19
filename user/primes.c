#include "kernel/types.h"
#include "user/user.h"

void func(int *input, int num){
	int p[2];
	int prime = *input;
	int temp, i;
	if(num == 1){
		printf("prime %d\n", *input);
		return;
	}
	printf("prime %d\n", prime);
	pipe(p);
    if(fork() == 0){
        for(i=0; i<num; i++){
            temp = *(input + i);
			write(p[1], (char *)(&temp), 4);
		}
        exit();
    }
	close(p[1]);
	if(fork() == 0){
		int counter = 0;
		char buf[4];
		while(read(p[0], buf, 4) != 0){
			temp = *((int *)buf);
			if(temp % prime != 0){
				*input = temp;
				input++;
				counter++;
			}
		}
		func(input - counter, counter);
		exit();
    }
	wait();
	wait();
}

int main(){
    int input[34];
	int i;
	for(i=0; i<34; i++){
		input[i] = i + 2;
	}
	func(input, 34);
    exit();
}
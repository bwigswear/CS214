#include <stdio.h>
#include <stdlib.h>

//Given a single integer, prints its prime factors, excluding 1

int main(int argc, char **argv){
	int factor = atoi(argv[1]);
	int og = factor;
	while(factor != 1){
		for(int i = 2; ; i++){
			if(factor % i == 0){
				if(factor == og){printf("%d",i);}else{printf(" %d", i);}
				factor = factor / i;
				break;
			}
		}
	}
	printf("\n");
}

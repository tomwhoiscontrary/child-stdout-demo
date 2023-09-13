#include <stdio.h>

int main(int argc, char** argv) {
	int first = 1;
	for (int i = 1; i < argc; i++) {
		if (first) {
			first = 0;
		} else {
			printf(" ");
		}
		printf("%s", argv[i]);
	}
	printf("\n");
}

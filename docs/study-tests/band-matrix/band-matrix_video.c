/**
 * Exemplo de matriz banda abstraído do vídeo 
 * "bandmatrix: This video explains how to store a band matrix in a 1-D array and access elements."
 * url: https://youtu.be/iG6r-BhPH-Q
 * acesso em: 25/04/2026
 */

#include <stdio.h>
#include <stdlib.h>

int main() {
	int m[10][10] = {
		{1, 2, 0, 0, 0, 0, 0, 0, 0, 0},
		{3, 4, 5, 0, 0, 0, 0, 0, 0, 0},
		{0, 6, 7, 8, 0, 0, 0, 0, 0, 0},
		{0, 0, 9,10,11, 0, 0, 0, 0, 0},
		{0, 0, 0,12,13,14, 0, 0, 0, 0},
		{0, 0, 0, 0,15,16,17, 0, 0, 0},
		{0, 0, 0, 0, 0,18,19,20 ,0 ,0},
		{0, 0, 0, 0, 0, 0,21,22,23, 0},
		{0, 0, 0, 0, 0, 0, 0,24,25,26},
		{0, 0, 0, 0, 0, 0, 0, 0,27,28}
	};
	
	int s[28] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
			22, 23, 24, 25, 26, 27, 28};
	
	int i, j;
	scanf("%d%d", &i, &j);
	if (abs(i - j) > 1)
		printf("0\n");
	else if (i == 0) {
		printf("%d\n", s[j]);
	} else {
		printf("%d\n", s[i * 3 + (j - i)]);
	}

	return 0;
}
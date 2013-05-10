#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int add(int a, int b)
{
	return a + b;
}

int main()
{
	int a = 2;
	int b = 8;

	printf("%d + %d = %d\n", a, b, add(a, b));

	return 0;
}
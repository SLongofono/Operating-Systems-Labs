#include <stdio.h>
#include <stdlib.h>

#define A 6
#define B 3
#define magic_number 48

/* IMPLEMENT ME: Declare your functions here */
int add (int a, int b);
int sub (int a, int b);
int mul (int a, int b);
int dvi (int a, int b);

// Declare a function pointet type to point to functions
typedef int (*arith_op)(int a, int b);

arith_op math[4] = {add, sub, mul, dvi};


int fetch(){
	printf("Specify the operation to perform (0 : add | 1 : subtract | 2 : Multiply | 3 : divide): ");
	
	// Need two chars to catch null pointer also
	char c[2];
	if(NULL == fgets(c, 2, stdin)){
		printf("Error retrieving input, exiting...\n");
		return -1;
	}
	return c[0];
}

int main (void)
{

	printf("Operand 'a' : %d | Operand 'b' : %d\n", A, B);

	int input = fetch();
	
	int test = math[input - magic_number](A, B);
	printf("x = %d\n", test);
/*	switch(input){
		case 48:	// Case add
			printf("x = %d\n", add(A,B));		
			break;
		case 49:	// Case subtract
			printf("x = %d\n", sub(A,B));
			break;
		case 50:	// Case multiply
			printf("x = %d\n", mul(A,B));
			break;
		case 51:	// Case divide
			printf("x = %d\n", dvi(A,B));
			break;
		default:
			printf("Unknown option selected...\n");
			break;
	}
*/
	return 0;
}

/* IMPLEMENT ME: Define your functions here */
int add (int a, int b) { printf ("Adding 'a' and 'b'\n"); return a + b; }
int sub (int a, int b) { printf ("Subtracting 'b' from 'a'\n"); return a - b; }
int mul (int a, int b) { printf ("Multiplying 'a' and 'b'\n"); return a * b; }
int dvi (int a, int b) { printf ("Dividing 'a' by 'b'\n"); return a / b; }

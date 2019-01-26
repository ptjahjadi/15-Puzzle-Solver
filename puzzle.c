/* puzzle.c

15-PUZZLE GRAPH SEARCH
Program attempts to solve a 15-puzzle through IDA search using a .puzzle file

To run the program, type in the command prompt:
15puzzle [.puzzle file]

Program partly created by Nir Lipovetzky and 
completed by Patrick Tjahjadi (890003)
The University of Melbourne */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>


/** 
 * READ THIS DESCRIPTION
 *
 * node data structure: containing state, g, f
 * you can extend it with more information if needed
 */
typedef struct node{
	int state[16];
	int g;
	int f;
} node_t;

/**
 * Global Variables
 */

// used to track the position of the blank in a state,
// so it doesn't have to be searched every time we check if an operator is applicable
// When we apply an operator, blank_pos is updated
int blank_pos;

// Track the last move done in the puzzle. Used for Last Moves Heuristic.
int last_move;

// Initial node of the problem
node_t initial_node;

// Statistics about the number of generated and expendad nodes
unsigned long generated;
unsigned long expanded;

/**
 * The id of the four available actions for moving the blank (empty slot). e.x.
 * Left: moves the blank to the left, etc. 
 */

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3
#define INF 2147483647 // Alternative value for Infinity for this program.
#define SIZE 16 // Total size of the puzzle.
#define DIM 4 // Dimensions of the puzzle (4 x 4).


/*
 * Helper arrays for the applicable function
 * applicability of operators: 0 = left, 1 = right, 2 = up, 3 = down 
 */
int ap_opLeft[]  = { 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 };
int ap_opRight[]  = { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 };
int ap_opUp[]  = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
int ap_opDown[]  = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 };
int *ap_ops[] = { ap_opLeft, ap_opRight, ap_opUp, ap_opDown };


/* print state */
void print_state( int* s )
{
	int i;
	
	for( i = 0; i < 16; i++ )
		printf( "%2d%c", s[i], ((i+1) % 4 == 0 ? '\n' : ' ') );
}
      
void printf_comma (long unsigned int n) {
    if (n < 0) {
        printf ("-");
        printf_comma (-n);
        return;
    }
    if (n < 1000) {
        printf ("%lu", n);
        return;
    }
    printf_comma (n/1000);
    printf (",%03lu", n%1000);
}

/* return the sum of manhattan distances from state to goal */
int manhattan( int* state )
{
	int sum = 0;
	int i, value, expected_row, expected_col, curr_row, curr_col;
	for (i = 0; i < SIZE; i++) {

		// Calculate manhattan distance by summing differences of rows and cols
		value = state[i];
		// Manhattan distance is not applicable to the blank space 
		if (value != 0) {
			expected_row = value / DIM;
			expected_col = value % DIM;
			curr_row = i / DIM;
			curr_col = i % DIM;
			sum += (abs(expected_row - curr_row) + 
					abs(expected_col - curr_col));
		}
	}

	return( sum );
}


/* return 1 if op is applicable in state, otherwise return 0 */
int applicable( int op )
{
       	return( ap_ops[op][blank_pos] );
}


/* apply operator */
void apply( node_t* n, int op )
{
	int t;

	//find tile that has to be moved given the op and blank_pos
	t = blank_pos + (op == 0 ? -1 : (op == 1 ? 1 : (op == 2 ? -4 : 4)));

	//apply op
	n->state[blank_pos] = n->state[t];
	n->state[t] = 0;
	
	//update blank pos
	blank_pos = t;
}

/* Function returns the minimum between the two integers */
int min(int a, int b) {
	if (a > b) {
		return b;
	}
	return a;
}

/* Recursive IDA */
node_t* ida( node_t* node, int threshold, int* newThreshold )
{
	/* Initialise required nodes here */
	node_t* r;
	node_t* newNode = malloc(sizeof(node_t));
	if (newNode == NULL) {
		printf("Memory allocation failed!");
		exit(EXIT_FAILURE);
	} 

	int a, i;

	// Set state of newNode to be the same of the current node
	for (i = 0; i < SIZE; i++) {
		newNode->state[i] = node->state[i];
		if (node->state[i] == 0) {
				blank_pos = i;
		}
	}	

	// Check all possible moves: UP, DOWN, LEFT, RIGHT
	for (a = 0 ; a < DIM ; a++) {
		if (applicable(a)) {

			/* Implement the LAST MOVES HEURISTIC, preventing the puzzle from 
			moving back to its previous state */
			if ((last_move == LEFT && a == RIGHT) || 
			(last_move == RIGHT && a == LEFT) || 
			(last_move == UP && a == DOWN) || 
			(last_move == DOWN && a == UP)) {

			}

			else {
				// Perform the operation
				generated += 1;
				apply(newNode, a);
				newNode->g = (node->g) + 1;
				newNode->f = (newNode->g) + manhattan(newNode->state);

				// Find new threshold if minimum turns is higher than threshold
				if (newNode->f > threshold) {
					*newThreshold = min(newNode->f, *newThreshold);
				}

				else {
					last_move = a;
					expanded += 1;
					/* Return node if puzzle is solved */
					if (manhattan(newNode->state) == 0) {
						return newNode;
					}
					r = ida(newNode, threshold, newThreshold);
					if (r != NULL) {
						return r;
					}
				}

				// Revert node to the previous state BEFORE it was applied
				for (i = 0; i < SIZE; i++) {
					newNode->state[i] = node->state[i];
					if (newNode->state[i] == 0) {
						blank_pos = i;
					}
				}
				newNode->g = (node->g);
			}
		}
	}
	free(newNode);
	// Reset recorded last move if no solution is found.
	last_move = -1;
	return( NULL );
}


/* main IDA control loop */
int IDA_control_loop(  ){
	/* Initialise required nodes here */
	node_t* r = malloc(sizeof(node_t));
	node_t* n = malloc(sizeof(node_t));

	if (n == NULL || r == NULL) {
		printf("Memory allocation failed!");
		exit(EXIT_FAILURE);
	}

	int threshold = 0;	
	
	/* initialize statistics */
	generated = 0;
	expanded = 0;

	/* compute initial threshold B */
	threshold = manhattan( initial_node.state );
	initial_node.f = threshold;
	printf( "Initial Estimate = %d\nThreshold = %d ", threshold, threshold );

	// Condition to return for an already solved puzzle
	if (initial_node.f == 0) {
		return 0;
	}

	int alt_threshold, i;
	// Post test repetition - repeat until puzzle is solved
	do {
		alt_threshold = INF;
		/* Copies the initial state and find blank_pos for future thresholds */
		for (i = 0; i < SIZE; i++) {
			n->state[i] = initial_node.state[i];
			if (initial_node.state[i] == 0) {
				blank_pos = i;
			}
		}
		// Initialise the number of made moves from the initial state
		n->g = 0;
		last_move = -1;
		r = ida(n, threshold, &alt_threshold);
		/* If puzzle can't be solved within allocated threshold */
		if (r == NULL) {
			threshold = alt_threshold;
			printf("%d ", threshold);
		}

	} while (r == NULL);

	free(n);
	if(r)
		return r->g;
	else
		return -1;
}


static inline float compute_current_time()
{
	struct rusage r_usage;
	
	getrusage( RUSAGE_SELF, &r_usage );	
	float diff_time = (float) r_usage.ru_utime.tv_sec;
	diff_time += (float) r_usage.ru_stime.tv_sec;
	diff_time += (float) r_usage.ru_utime.tv_usec / (float)1000000;
	diff_time += (float) r_usage.ru_stime.tv_usec / (float)1000000;
	return diff_time;
}

int main( int argc, char **argv )
{
	int i, solution_length;

	/* check we have a initial state as parameter */
	if( argc != 2 )
	{
		fprintf( stderr, "usage: %s \"<initial-state-file>\"\n", argv[0] );
		return( -1 );
	}


	/* read initial state */
	FILE* initFile = fopen( argv[1], "r" );
	char buffer[256];

	if( fgets(buffer, sizeof(buffer), initFile) != NULL ){
		char* tile = strtok( buffer, " " );
		for( i = 0; tile != NULL; ++i )
			{
				initial_node.state[i] = atoi( tile );
				blank_pos = (initial_node.state[i] == 0 ? i : blank_pos);
				tile = strtok( NULL, " " );
			}		
	}
	else{
		fprintf( stderr, "Filename empty\"\n" );
		return( -2 );

	}
       
	if( i != 16 )
	{
		fprintf( stderr, "invalid initial state\n" );
		return( -1 );
	}

	/* initialize the initial node */
	initial_node.g=0;
	initial_node.f=0;

	print_state( initial_node.state );


	/* solve */
	float t0 = compute_current_time();
	
	solution_length = IDA_control_loop();				

	float tf = compute_current_time();

	/* report results */
	printf( "\nSolution = %d\n", solution_length);
	printf( "Generated = ");
	printf_comma(generated);
	printf("\nExpanded = ");
	printf_comma(expanded);		
	printf( "\nTime (seconds) = %.2f\nExpanded/Second = ", tf-t0 );
	printf_comma((unsigned long int) expanded/(tf+0.00000001-t0));
	printf("\n\n");

	/* aggregate all executions in a file named report.dat, for marking purposes */
	FILE* report = fopen( "report.dat", "a" );

	fprintf( report, "%s", argv[1] );
	fprintf( report, "\n\tSolution = %d, Generated = %lu, Expanded = %lu", solution_length, generated, expanded);
	fprintf( report, ", Time = %f, Expanded/Second = %f\n\n", tf-t0, (float)expanded/(tf-t0));
	fclose(report);
	// Minor correction here: Lack of semi-comma leading to compilation error
	return( 0 );
}



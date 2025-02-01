// Add your file header here
// Fix compiler error
// Implement remaining functionality


///////////////////////////////////////////////////////////////////////////////
// Copyright 2021-25 Deb Deppeler
// Posting or sharing this file is prohibited, including any changes/additions.
//
// We have provided comments and structure for this program to help you get 
// started.  Later programs will not provide the same level of commenting,
// rather you will be expected to add same level of comments to your work.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *DELIM = ",";  // commas ',' are a common delimiter character for data strings

/* Returns 1 if the value in the cell of a row/column is valid. Otherwise returns 0.
 * A valid row or column contains only blanks or the digits 1-size, 
 * with no duplicate digits, where size is the value 1 to 9.
 * 
 * digit: the value of the cell
 * size:  number of rows and columns in the board
 * seen:  an array of size 10 which stores the counts of each digit seen so far in a row/column
 */
int check_cell(int digit, int size, int *seen) {
	// blanks are always valid
	if (digit == 0) {
		return 1;
	}
	// digit must be 1 - size
    if (digit < 1 || digit > size) {
        return 0;
    }
	// digit cannot be a duplicate
    if (*(seen + digit) != 0) {
        return 0;
    }

    *(seen + digit) = 1;
    return 1;
}

/* Retrieves the value at board[row][col] using pointer arithmetic
 * 
 * board: heap allocated 2D array of integers 
 * row:   the row the cell is within
 * col:   the column the cell is within
 */
int get_cell_value(int **board, int row, int col) {
    return *(*(board + row) + col);
}

/* Returns 1 if and only if the 2D array of ints in board 
 * is in a valid Sudoku board state.  Otherwise returns 0.
 *
 * DOES NOT PRODUCES ANY PRINTED OUTPUT
 * 
 * A valid row or column contains only blanks or the digits 1-size, 
 * with no duplicate digits, where size is the value 1 to 9.
 * 
 * Note: This function requires only that each row and each column are valid.
 * 
 * board: heap allocated 2D array of integers 
 * size:  number of rows and columns in the board
 */
int valid_sudoku_board(int **board, int size) {

	// row checks i = row, j = col
	for (int i = 0; i < size; i++) {
		// use an array of index 0 - 9 with values of 0 to keep track of existing digits
		int *seen = calloc(10, sizeof(int));
		for (int j = 0; j < size; j++) {
			int valid = check_cell(get_cell_value(board, j, i), size, seen);
			
			if (valid == 0) {
				free(seen);
				return 0;
			}
		}
		free(seen);
	}

	// column checks i = col, j = row
	for (int i = 0; i < size; i++) {
		int *seen = calloc(10, sizeof(int));
		for (int j = 0; j < size; j++) {
			int valid = check_cell(get_cell_value(board, j, i), size, seen);

			if (valid == 0) {
				free(seen);
				return 0;
			}
		}
		free(seen);
	}

	return 1;   
}


/* COMPLETED (DO NOT EDIT)       
 * Read the first line of file to get the size of that board.
 * 
 * PRE-CONDITION #1: file exists
 * PRE-CONDITION #2: first line of file contains valid non-zero integer value
 *
 * fptr: file pointer for the board's input file
 * size: a pointer to an int to store the size
 *
 * POST-CONDITION: the integer whos address is passed in as size (int *) 
 * will now have the size (number of rows and cols) of the board being checked.
 */
void get_board_size(FILE *fptr, int *size) {      
	char *line = NULL;
	size_t len = 0;

	// 'man getline' to learn about <stdio.h> getline
	if ( getline(&line, &len, fptr) == -1 ) {
		printf("Error reading the input file.\n");
		free(line);
		exit(1);
	}

	char *size_chars = NULL;
	size_chars = strtok(line, DELIM); // 'man strtok' string tokenizer
	*size = atoi(size_chars);         // 'man atoi' alpha to integer
	free(line);                       // free memory allocated for line 
}


/* This program prints "valid" (without quotes) if the input file contains
 * a valid state of a Sudoku puzzle board wrt to rows and columns only.
 * It prints "invalid" (without quotes) if the input file is not valid.
 *
 * Usage: A single CLA that is the name of a file that contains data.
 *
 * argc: the number of command line args (CLAs)
 * argv: the CLA strings, includes the program name
 *
 * Returns 0 if file exists and is readable.
 * Exit with any non-zero result if unable to open and read the file given.
 */
int main( int argc, char **argv ) {              

	// Check if number of command-line arguments is correct.
	if (argc > 2) {
		printf("More arguments than expected.\n");
		exit(1);
	}
	// Open the file 
	FILE *fp = fopen(*(argv + 1), "r");
	if (fp == NULL) {
		printf("Can't open file for reading.\n");
		exit(1);
	}

	// will store the board's size, number of rows and columns
	int size;

	get_board_size(fp, &size);

	int **board = malloc(size * sizeof(int *));

	if (!board) {
        printf("Could not allocate memory for the board.\n");
        exit(1);
    }

	// For each row, allocate an array of `size` integers
    for (int i = 0; i < size; i++) {
        *(board + i) = malloc(size * sizeof(int));
        if (!(*(board + i))) {
            printf("Failed to allocate row\n");
            // Free already allocated rows
            for (int j = 0; j < i; j++) {
                free(*(board + j));
            }
            free(board);
            exit(1);
        }
    }

	// Read the remaining lines of the board data file.
	// Tokenize each line and store the values in your 2D array.
	char *line = NULL;
	size_t len = 0;
	char *token = NULL;
	for (int i = 0; i < size; i++) {

		// read the line
		if (getline(&line, &len, fp) == -1) {
			printf("Error while reading line %i of the file.\n", i+2);
			exit(1);
		}

		token = strtok(line, DELIM);
		for (int j = 0; j < size; j++) {
			*(*(board + i) + j) = atoi(token); 
			token = strtok(NULL, DELIM);
		}
	}

	if (valid_sudoku_board(board, size)) {
        printf("valid\n");
    } 
	else {
        printf("invalid\n");
    }

	for (int i = 0; i < size; i++) {
        free(*(board + i));
    }
	free(board);

	//Close the file.
	if (fclose(fp) != 0) {
		printf("Error while closing the file.\n");
		exit(1);
	} 

	return 0;       
}       

// s25-010


////////////////////////////////////////////////////////////////////////////////
// Main File:        my_magic_square.c
// This File:        my_magic_square.c
// Other Files:      N/A
// Semester:         CS 354 Lecture 10 Spring 2025
// Instructor/TA:    deppeler, asch
//
// Author:           Michael Hu
// UW NetID:         mhu97
// CS Login:         mhu
//
//////////////////// REQUIRED -- OTHER SOURCES OF HELP /////////////////////////
// Persons:          N/A
//
// Online sources:   Wikipedia for Simese Method
//
// AI chats:         N/A
//////////////////////////// 80 columns wide ///////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Copyright 2020 Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission, CS 354 Spring 2025, Deb Deppeler
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *DELIM = ",";

// Structure that represents a magic square
typedef struct {
    int size;            // dimension of the square
    int **magic_square;  // ptr to 2D heap array that stores magic square values
} MagicSquare;

/*
 * Prompts the user for the magic square's size, read size,
 * check if it's an odd number >= 3
 * (if not valid size, display the required error message and exit)
 *
 * return the valid number
 */
int getSize() {
    printf("Enter magic square's size (odd integer >=3)\n");

    int size;
    if (scanf("%d", &size) != 1) {
        // safe to assume input is integer - but in case something weird
        // happens we should exit
        printf("Error reading size.\n");
        exit(1);
    }

    // entering 2 should print the odd error message instead of the >=3 one
    if (size % 2 == 0) {
        printf("Magic square size must be odd.\n");
        exit(1);
    }
    if (size < 3) {
        printf("Magic square size must be >= 3.\n");
        exit(1);
    }

    return size;
}

/*
 * Creates a magic square of size n on the heap using
 * the Siamese Method.
 *
 * n - the number of rows and columns, 3 <= n <= 9
 *
 * returns a pointer to the completed MagicSquare struct.
 */
MagicSquare *generateMagicSquare(int n) {
    // allocate the struct
    MagicSquare *square = malloc(sizeof(MagicSquare));
    if (square == NULL) {
        printf("Could not allocate memory for the magic square.\n");
        free(square);
        exit(1);
    }

    square->size = n;

    // allocate the struct's square
    square->magic_square = malloc(n * sizeof(int *));
    if (square->magic_square == NULL) {
        printf("Could not allocate memory for the 2D array.\n");
        free(square->magic_square);
        free(square);
        exit(1);
    }

    // allocate each row in the square
    for (int i = 0; i < n; i++) {
        *(square->magic_square + i) = malloc(n * sizeof(int));
        if (*(square->magic_square + i) == NULL) {
            printf(
                "Could not allocate memory for the row within the 2D array.\n");
            // Free already allocated rows
            for (int j = 0; j < i; j++) {
                free(*(square->magic_square + j));
            }
            free(square->magic_square);
            free(square);
            exit(1);
        }
    }

    // initialize all values to 0 to avoid valgrind uinitialized warnings
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            *(*(square->magic_square + i) + j) = 0;
        }
    }

    // Siamese method to fill the magic square with values 1...n^2
    int row = 0;      // begin at row 0
    int col = n / 2;  // begin at center column
    int value;
    for (value = 1; value <= n * n; value++) {
        *(*(square->magic_square + row) + col) = value;

        // move diagonally up-right (row - 1, col + 1) wrapping as necessary (%
        // n)
        int next_row = (row - 1 + n) % n;
        int next_col = (col + 1) % n;

        // if next spot is filled, place one row below (row + 1)
        if (*(*(square->magic_square + next_row) + next_col) != 0) {
            next_row = (row + 1) % n;
            next_col = col;
        }

        // Update row, col
        row = next_row;
        col = next_col;
    }

    return square;
}

/*
 * Open a new file (overwrites the existing file)
 * and write the magic square values to the file
 * in the format specified by assignment.
 *
 * See assigntment for required file format.
 *
 * magic_square - the magic square to write to a file
 * filename - the name of the output file
 */
void fileOutputMagicSquare(MagicSquare *magic_square, char *filename) {
    if (magic_square == NULL || filename == NULL) {
        printf("magic_square and filename must not be null!\n");
        exit(1);
    }

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Can't open the file for writing\n");
        exit(1);
    }

    // first line contains the size
    fprintf(fp, "%d\n", magic_square->size);

    // each subsequent row is a row, there are n rows with n numbers in each
    // separated by commas
    int n = magic_square->size;
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            int val = *(*(magic_square->magic_square + row) + col);
            fprintf(fp, "%d", val);

            // every number should have a comma except the last column
            if (col < n - 1) {
                fprintf(fp, ",");
            }
        }
        fprintf(fp, "\n");
    }

    if (fclose(fp) != 0) {
        printf("Can't close the file\n");
        exit(1);
    }
}

/*
 * Call other functions to generate a magic square
 * of the user specified size and outputs
 * the created square to the output filename.
 *
 * Add description of required CLAs here
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: ./my_magic_square <output filename>\n");
        exit(1);
    }
    char *filename = *(argv + 1);
    int size = getSize();
    MagicSquare *sq = generateMagicSquare(size);
    fileOutputMagicSquare(sq, filename);

    for (int i = 0; i < size; i++) {
        free(*(sq->magic_square + i));
    }

    free(sq->magic_square);
    free(sq);

    return 0;
}

// 202501
////////////////////////////////////////////////////////////////////////////////
// Main File:        sequence.c
// This File:        sequence.c
// Other Files:      N/A
//
// Semester:         CS 354 Lecture 10? Spring 2025
// Instructor/TA:    deppeler, asch
// 
// Author:           Michael Hu
// UW NetID:         mhu97
// CS Login:         mhu
//
//////////////////// REQUIRED -- OTHER SOURCES OF HELP ///////////////////////// 
// Persons:          N/A
//
// Online sources:   N/A other than course materials
// 
// AI chats:         N/A
//////////////////////////// 80 columns wide ///////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2019 Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission, Fall 2020-Spring 2025, Deb Deppeler
//
////////////////////////////////////////////////////////////////////////////////

/*
 * This program prints a simple sequence of the numbers 1 through 10, separated by a comma
 */

#include <stdio.h>
#include <stdlib.h>

int compute(int m, int x, int b) {
    return (m * x) + b;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Invalid number of CLAs, exiting!");
        exit(1);
    }

    int n = atoi(argv[1]);
    int x0 = atoi(argv[2]);
    int m = atoi(argv[3]);
    int c = atoi(argv[4]);
    int prev = 0;

    for (int i = 0; i < n; i++) {
        if (i == 0) {
            prev = x0;
        }
        else {
            prev = compute(m, prev, c);
        }  
        if (i == n - 1) {
            printf("%i", prev);
        }
        else {
            printf("%i,", prev);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// Main File:        simple_sequence.c
// This File:        simple_sequence.c
// Other Files:      decode - .i .s .o *
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

int main(int argc, char *argv[]) {
    for (int i = 1; i <= 10; i++) {
        if (i == 10) {
            printf("%i", i);
        }
        else {
            printf("%i,", i);
        }
    }
}

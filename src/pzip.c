#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pzip.h"

// Global Variables
char *input;					// Holds the input
struct zipped_char *zchars;		// Holds zipped_chars
int zchars_count = 0;			// Counts how many zchars there currently are. This is used for copying into the correct indices
int *cfrequency;				// Holds char_frequency
int segment_size;				// The size of the segment to be looped over
int number_threads;				// Global variable for n_threads
pthread_t *threads;				// Array of threads
int *sizes;						// Holds the size of each localResult		
pthread_barrier_t barrier;

/*
	This function is called within threadCallback for the purpose 
	of summing the sizes of localResult over a range of threads.
	This is done so each thread can copy its data into the correct
	indices of zipped_chars.
*/
static int sumSizes(int end) {
    int sum = 0;
    for (int i = 0; i < end; i++) {
        sum += sizes[i];
    }
    return sum;
}

static void *thread_callback(void *arg) {
	long index = (long) arg;						// Cast the argument back to a long
	struct zipped_char *localResult = NULL;			// Define the localResult
	int local_index = -1;							// Used for indexing into localResult
	int size = 0;									// Used for allocting memory for localResult
	int start_index = index * segment_size;			// Where the thread should start iterating within the input
	int end_index = start_index + segment_size;		// Where the thread should stop iterating within the input
	char prev_char = '1';							// Initialize it to a value outside of a-z
	if ((localResult = malloc(segment_size * sizeof(struct zipped_char))) == NULL) {
		fprintf(stderr, "error: failed to realloc.\n");
		exit(EXIT_FAILURE);
	}
	for (int i = start_index; i < end_index; i++) {
		if (input[i] == prev_char) {				// If the letter is the same as what we previously read, increment its counter
			localResult[local_index].occurence++;
			continue;
		}
		size++;										// If the letter is not the same as what we previously read, make a new entry for it in the structure and increment its counter
		local_index++;
		localResult[local_index].character = input[i];
		localResult[local_index].occurence = 1;
		prev_char = input[i];
	}
	sizes[index] = size;
	pthread_barrier_wait(&barrier);					// Wait here so sizes contains the size of every thread's localResult
	int zchars_index = 0;							// The index to copy data to in zipped_chars
	if (index != 0)									// Copy results into zipped_chars
		zchars_index = sumSizes(index);
	for (int i = 0; i < size; i++, zchars_index++)
		zchars[zchars_index] = localResult[i];
	if (index == number_threads - 1)				// Sum all sizes to get the zipped_chars_count
		zchars_count = sumSizes(index) + size;
	free(localResult);								// Free localResult	
	return NULL;
}

/**
 * pzip() - zip an array of characters in parallel
 *
 * Inputs:
 * @n_threads:		   The number of threads to use in pzip
 * @input_chars:		   The input characters (a-z) to be zipped
 * @input_chars_size:	   The number of characaters in the input file
 *
 * Outputs:
 * @zipped_chars:       The array of zipped_char structs
 * @zipped_chars_count:   The total count of inserted elements into the zippedChars array.
 * @char_frequency[26]: Total number of occurences
 *
 *
 */
void pzip(int n_threads, char *input_chars, int input_chars_size, struct zipped_char *zipped_chars, int *zipped_chars_count, int *char_frequency)
{
	input = input_chars;									 // Initialize global variables
	zchars = zipped_chars;
	cfrequency = char_frequency;
	segment_size = input_chars_size / n_threads;
	number_threads = n_threads;
	if ((sizes = malloc(n_threads * sizeof(int))) == NULL) {
		fprintf(stderr, "error: malloc failed.\n");
		exit(EXIT_FAILURE);
	}
	if ((threads = malloc(n_threads * sizeof(pthread_t))) == NULL) {
		fprintf(stderr, "error: failed to allocate memory for threads.\n");
		exit(EXIT_FAILURE);
	}
	if (pthread_barrier_init(&barrier, NULL, n_threads)) {   // Initialize the barrier
        fprintf(stderr, "error: failed to create barrier.\n");
        exit(EXIT_FAILURE);
    }
	for (long i = 0; i < n_threads; i++) {
		if (pthread_create(&threads[i], NULL, thread_callback, (void *)i)) {
			fprintf(stderr, "error: failed to create thread.\n");
			exit(EXIT_FAILURE);
		}
	}
	for (long i = 0; i < n_threads; i++) {					// Join main thread with all sub-threads
		if (pthread_join(threads[i], NULL)) {
			fprintf(stderr, "error: failed to join thread %ld\n", i);
			exit(EXIT_FAILURE);
		}
	}
	for (int i = 0; i < sumSizes(n_threads-1); i++) {
		cfrequency[(zchars[i].character - 97)] += zchars[i].occurence;			// ASCII value of 'a' is 97, so subtract 97 from the char to get the correct index into char_frequency
	}
	pthread_barrier_destroy(&barrier);		// Destroy the barrier
	*zipped_chars_count = zchars_count;		// Set zipped_chars_count
	free(sizes);							// Free memory
	free(threads);
	input = NULL;								
	zchars = NULL;
	cfrequency = NULL;
}

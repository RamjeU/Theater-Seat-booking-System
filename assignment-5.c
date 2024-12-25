#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_CUSTOMERS 50         // Maximum number of customers (threads) allowed
#define MAX_SEATS 60             // Total seats in theater (5x12)
#define MAX_SEATS_PER_REQUEST 60 // Maximum seats one customer can request
#define MAX_LINES 1024           // Maximum lines in input file
#define ROWS 5                   // Number of rows (aisles) in theater
#define COLS 12                  // Number of columns (seats per aisle)

/**
 * Structure to represent a customer's booking request
 * Stores the customer ID and their requested seats
 */
typedef struct {
    int customer_id;
    int num_seats;
    int seats[MAX_SEATS_PER_REQUEST][2]; // Array of [aisle, seat] pairs
} BookingRequest;

// Global variables
pthread_mutex_t seat_mutexes[ROWS][COLS]; // Mutex for each seat in theater
int theater[ROWS][COLS] = {0}; // 0 represents empty seat (we'll use 000 for display)
BookingRequest requests[MAX_LINES];
int num_requests = 0;

// Function prototypes
void* process_booking(void* arg);
int try_book_seats(BookingRequest* request);
void print_theater();
void release_locks(int locks[][2], int num_locks);

/**
 * Main function - Entry point of the program
 * Handles initialization, file reading, thread creation, and cleanup
 */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Initialize random seed
    srand(time(NULL));

    // Initialize mutexes
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            pthread_mutex_init(&seat_mutexes[i][j], NULL);
        }
    }

    // Read input file
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("Error opening file %s\n", argv[1]);
        return 1;
    }
    // Read and parse input file line by line
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;
        
        char* comment = strchr(line, '#');
        if (comment) *comment = '\0';

        BookingRequest* req = &requests[num_requests];
        req->num_seats = 0;

        // Parse the line
        char* token = strtok(line, ",");
        if (token) {
            req->customer_id = atoi(token);
            
            while ((token = strtok(NULL, ", \n")) != NULL && req->num_seats < MAX_SEATS_PER_REQUEST) {
                int aisle = atoi(token);
                token = strtok(NULL, ", \n");
                if (!token) break;
                int seat = atoi(token);
                
                // Convert to 0-based indices
                req->seats[req->num_seats][0] = aisle - 1;
                req->seats[req->num_seats][1] = seat - 1;
                req->num_seats++;
            }
            
            if (req->num_seats > 0) {
                num_requests++;
            }
        }
    }
    fclose(file);

    // Create threads for each customer
    pthread_t threads[MAX_CUSTOMERS];
    int thread_ids[MAX_CUSTOMERS];

    for (int i = 0; i < num_requests; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, process_booking, &thread_ids[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_requests; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print final theater layout
    printf("\n");
    print_theater();

    // Cleanup mutexes
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            pthread_mutex_destroy(&seat_mutexes[i][j]);
        }
    }

    return 0;
}
/**
 * Thread function that processes a single booking request
 * Attempts to book seats and prints success/failure message
 * 
 * @param arg Pointer to the index of the request to process
 * @return NULL
 */
void* process_booking(void* arg) {
    int request_index = *((int*)arg);
    BookingRequest* request = &requests[request_index];
    
    // Try to book the seats
    if (try_book_seats(request)) {
        // Print success message
        printf("Customer %d - Successful - ", request->customer_id);
        for (int i = 0; i < request->num_seats; i++) {
            printf("Aisle %d, Seat %d", request->seats[i][0] + 1, request->seats[i][1] + 1);
            if (i < request->num_seats - 1) printf(", ");
        }
        printf("\n");
    } else {
        // Print failure message
        printf("Customer %d - Fail - ", request->customer_id);
        for (int i = 0; i < request->num_seats; i++) {
            printf("Aisle %d, Seat %d", request->seats[i][0] + 1, request->seats[i][1] + 1);
            if (i < request->num_seats - 1) printf(", ");
        }
        printf("\n");
    }

    return NULL;
}

/**
 * Attempts to book all requested seats for a customer
 * Uses mutex locks to ensure thread safety
 * 
 * @param request Pointer to the BookingRequest to process
 * @return 1 if booking successful, 0 if failed
 */
int try_book_seats(BookingRequest* request) {
    int locks[MAX_SEATS_PER_REQUEST][2];
    int num_locks = 0;

    // Try to acquire all necessary locks
    for (int i = 0; i < request->num_seats; i++) {
        int aisle = request->seats[i][0];
        int seat = request->seats[i][1];

        // Validate seat coordinates
        if (aisle < 0 || aisle >= ROWS || seat < 0 || seat >= COLS) {
            release_locks(locks, num_locks);
            return 0;
        }

        // Try to lock the seat
        if (pthread_mutex_trylock(&seat_mutexes[aisle][seat]) != 0) {
            // If we can't get the lock, release all acquired locks and fail
            release_locks(locks, num_locks);
            return 0;
        }

        // Check if seat is already taken
        if (theater[aisle][seat] != 0) {
            pthread_mutex_unlock(&seat_mutexes[aisle][seat]);
            release_locks(locks, num_locks);
            return 0;
        }

        // Add to our locks array
        locks[num_locks][0] = aisle;
        locks[num_locks][1] = seat;
        num_locks++;
    }

    // If we got here, we have all locks and seats are available
    // Sleep for 1-3 seconds to simulate customer activity
    sleep(rand() % 3 + 1);

    // Book all seats
    for (int i = 0; i < request->num_seats; i++) {
        int aisle = request->seats[i][0];
        int seat = request->seats[i][1];
        theater[aisle][seat] = request->customer_id;
    }

    // Release all locks
    release_locks(locks, num_locks);
    return 1;
}

/**
 * Releases all locks held for a booking attempt
 * 
 * @param locks Array of [aisle, seat] pairs to unlock
 * @param num_locks Number of locks to release
 */
void release_locks(int locks[][2], int num_locks) {
    for (int i = 0; i < num_locks; i++) {
        pthread_mutex_unlock(&seat_mutexes[locks[i][0]][locks[i][1]]);
    }
}

/**
 * Prints the current theater layout
 * Shows seat assignments with customer IDs
 */
void print_theater() {
    // Print column headers
    printf("        ");
    for (int j = 1; j <= COLS; j++) {
        printf("%3d ", j);
    }
    printf("\n");

    // Print each row
    for (int i = 0; i < ROWS; i++) {
        printf("Aisle %d ", i + 1);
        for (int j = 0; j < COLS; j++) {
            printf("%3d ", theater[i][j]);
        }
        printf("\n");
    }
}

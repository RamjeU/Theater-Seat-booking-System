# Theater Seat Booking System

This project simulates a multithreaded theater seat booking system where multiple customers can request specific seats simultaneously. It ensures thread safety and handles concurrency using mutexes.

## Features

- **Multithreading**: Each customer's booking request is processed in a separate thread.
- **Thread Safety**: Uses mutex locks to prevent race conditions when booking seats.
- **Dynamic Input**: Reads booking requests from a file.
- **Error Handling**: Validates input and handles invalid seat requests gracefully.
- **Theater Visualization**: Displays the final theater seating arrangement after all bookings are processed.

## How It Works

1. **Input File**: The program accepts an input file containing customer booking requests. Each request specifies the customer ID and the seats they wish to book (aisle and seat numbers).
2. **Seat Booking**: Each customer’s request is processed by a separate thread, which locks the requested seats, verifies their availability, and marks them as booked.
3. **Concurrency Management**: Mutex locks ensure that no two threads can book the same seat simultaneously.
4. **Output**: The program prints the success or failure of each booking request and displays the final seating layout.

## Input File Format

- Each line represents a customer request in the following format:
  ```
  <customer_id>, <aisle1>, <seat1>, <aisle2>, <seat2>, ...
  ```
- Lines starting with `#` are treated as comments and ignored.

Example:
```
1, 1, 5, 2, 6
2, 3, 4
3, 1, 1, 1, 2, 1, 3
# This is a comment
```

## Usage

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd <repository-name>
   ```
2. Compile the program:
   ```bash
   gcc -pthread -o theater_booking assignment-5.c
   ```
3. Run the program with an input file:
   ```bash
   ./theater_booking <input_file>
   ```

## Example Output

```
Customer 1 - Successful - Aisle 1, Seat 5, Aisle 2, Seat 6
Customer 2 - Successful - Aisle 3, Seat 4
Customer 3 - Fail - Aisle 1, Seat 1, Aisle 1, Seat 2, Aisle 1, Seat 3

        1   2   3   4   5   6   7   8   9  10  11  12
Aisle 1   3   3   3   0   1   0   0   0   0   0   0   0
Aisle 2   0   0   0   0   0   1   0   0   0   0   0   0
Aisle 3   0   0   0   2   0   0   0   0   0   0   0   0
Aisle 4   0   0   0   0   0   0   0   0   0   0   0   0
Aisle 5   0   0   0   0   0   0   0   0   0   0   0   0
```

## Dependencies

- GCC compiler with pthread support.

## Project Structure

- **`assignment-5.c`**: The main source code file for the project.

## Key Concepts

- **Mutex Locks**: Ensures thread-safe operations for seat booking.
- **Multithreading**: Simulates concurrent seat booking requests.
- **Data Structures**: Uses arrays and structures to manage booking requests and theater layout.

## License
This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments
This project was developed as part of a multithreading and concurrency assignment.


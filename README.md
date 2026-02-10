# Parallel Pizza Delivery Simulation

## Description

This project implements a parallel simulation of a pizza delivery system using POSIX threads (pthreads).
Each customer order is handled by a separate thread, while shared resources such as cooks, ovens, packers,
and delivery personnel are synchronized using mutexes and condition variables.

The simulation models a real-world service system and focuses on thread synchronization, shared resource
management, and performance behavior in a concurrent environment.

---

## Features

- Parallel handling of customer orders using threads
- Shared resource management (cooks, ovens, packers, delivery staff)
- Synchronization using mutexes and condition variables
- Collection of execution statistics (service time, waiting time, revenue)
- Automated benchmarking script for performance evaluation

---

## Technologies

- C
- POSIX Threads (pthreads)
- Mutexes & Condition Variables
- Bash scripting (benchmarking)

---

## Build & Run

The project uses POSIX threads and is intended to be built on Unix-like systems.

### Compile
gcc -pthread thread-handling-pizza.c -o pizza

### Run
./pizza <number_of_orders> <number_of_threads>

### Benchmarking
The test-res.sh script automates the execution of the program with different numbers of threads
in order to measure execution time and study scalability.


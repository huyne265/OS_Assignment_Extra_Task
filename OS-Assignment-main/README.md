<!-- # OS-Assignment -->
# README

## Overview 
This repository contains a program that simulates an operating system's basic components, including CPU scheduling, memory management, and process handling. The program takes a configuration file as input and executes according to its settings.

## Prerequisites 
Before running the program, ensure you have the following installed: 
- GCC compiler (for compiling the source code) 
- Bash shell (to run the provided script) 

## Files 
- **Makefile:** Build system to simplify compilation and manage dependencies. 
- **run.sh:** Script to compile and execute the program. 
- **os.c, cpu.c, timer.c, sched.c, loader.c, mm.c:** Source code files for the program. 
- **Configuration file:** Defines the settings for program execution. 

## Compile and Run program
### Using Makefile 

The Makefile supports the following commands:

- **`make all`**: Default target that compiles the entire OS simulation (os executable).
- **`make mem`**: Compiles only the memory management modules (mem executable).
- **`make sched`**: Compiles only the scheduler module (sched executable).
- **`make os`**: Compiles the entire OS simulation.
- **`make clean`**: Cleans up all generated files, including object files and executables.

If any changes are made to the .c or .h files, follow these steps to recompile the program using the Makefile: 
1. Open a terminal and navigate to the directory containing the source files and Makefile.
2. Run the following command to build the program:
    ```shell
    make all

Follow these steps to compile and run the program: 
### Compiling the Program 
1. Open a terminal and navigate to the directory containing the source files and Makefile. 
2. Compile the program using the make command: 
    ```shell
   make all 

This will ensure that the program is recompiled if any changes are made to the .c or .h files.  

3. After successful compilation, the os executable will be created. 

### Running the Program  
You can run the program using either the run.sh script or directly with the os executable. 
#### Using run.sh 
1. Ensure the script has executable permissions. If not, run the following command:
    ```shell
    chmod +x run.sh 
2. Run the script with the configuration file as an argument: 
    ```shell
    ./run.sh [path to configuration file] 
Replace [path to configuration file] with the actual path to your configuration file. Example: sched, sched_0, sched_1 
##### Running Directly 
1. Execute the program directly with the os binary: 
    ```shell
    ./os [path to configuration file] 
Replace [path to configuration file] with the actual path to your configuration file. 

## Example 
Assuming your configuration file is named sched and is located in the current directory, run the program as follows: 
1. To compile the program: 
    ```shell
    make all
2. To run the program using run.sh: 
    ```shell
    ./run.sh sched
3. Or run the program directly:
    ```shell
    ./os sched
    
## Notes
Always use make all to recompile the program if there are any changes to the .c or .h files.
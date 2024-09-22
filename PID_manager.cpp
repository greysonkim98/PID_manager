//Program name: "PIDmanager"                                                                                           
//Copyright (C) 2024 Minjae Kim                 
//                                                                             
//This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License  
//version 3 as published by the Free Software Foundation.                                                                    
//This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied         
//warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.     
//<https://www.gnu.org/licenses/>.
//
//Author information
//  Author name: Minjae Kim
//  Author email: minjaek@csu.fullerton.edu
//  Author CWID: 885206615
//  Author class: CPSC335
//
//Program information
//  Program name: PIDmanager
//  Programming languages: C++
//  Date of last update: 2024-09-08
//  Date of reorganization of comments: 2024-09-08
//  Files in this program: PIDmanager.cpp
//  Developed OS: Ubuntu 24.04.1 LTS
//  Tested OS: Ubuntu 24.04.1 LTS
//  Tested Compiler: g++ (Ubuntu 13.2.0-23ubuntu4) 13.2.0
//
//Program description
//  This program allocate PID in the range of 100 and 1000 to child process and parent process
//  Users can select the options of allocate space for PID information, allocate random PID and releasing PID

#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <ctime>

#define BUFFER_SIZE 25
#define READ_END 0
#define WRITE_END 1
#define MIN_PID 100
#define MAX_PID 1000

class PIDManager {
public:
    int allocate_map() {
        try{
            map = std::vector<PIDInfo>(MAX_PID - MIN_PID + 1);
            return 1;
        } catch (std::bad_alloc& e) {
        std::cerr << "Error: Failed to allocate PID map: " << e.what() << std::endl;
        return -1; // Initialization failed
    }
    }

    int allocate_pid() {
        if (!map.size()) {
            return -1;
        }

        // Search for an available PID randomly
        srand(time(nullptr));
        unsigned attempts = 0;
        while (attempts < map.size()) {
            int random_index = rand() % map.size();
            if (!map[random_index].isAllocated) {
                map[random_index].isAllocated = true;
                map[random_index].pid = MIN_PID + random_index;
                return map[random_index].pid;
            }
            attempts++;
        }

        // If no PID found after random attempts, return -1 (all PIDs allocated)
        return -1;
    }

    void release_pid(int pid) {
        // Checking if the users release PID before initializing
        if (!map.size()) {
            std::cerr << "Error: PID map not initialized. Cannot release PID." << std::endl;
            return;
        }
        // Release PID within the range
        if (pid >= MIN_PID && pid <= MAX_PID) {
            if (map[pid - MIN_PID].isAllocated) {
                map[pid - MIN_PID].isAllocated = false;
                map[pid - MIN_PID].pid = 0;
                std::cout << "\n" << pid << " has been released\n";
            } else {
                std::cerr << "Error: PID " << pid << " is not allocated." << std::endl;
                exit(EXIT_SUCCESS);
            }
        } else {
            std::cerr << "Error: Invalid PID to release." << std::endl;
        }
    }

private:
    struct PIDInfo {
        bool isAllocated;
        int pid;

        PIDInfo() : isAllocated(false), pid(0) {}
    };

    std::vector<PIDInfo> map;
};

int main() {
    PIDManager pid_manager;
    char user_input;
    char write_msg[BUFFER_SIZE];
    char read_msg[BUFFER_SIZE];
    int fd[2];

    // Checking the error of the pipe
    if (pipe(fd) == -1) {
        std::cerr << "Error: Pipe failed." << std::endl;
        return 1;
    }

    do {
        // Display selection options
        std::cout << "\nEnter your choice: \n'm' to allocate map \n'p' to allocate pid \n'r' to release pid \n'e' to exit";
        std::cout << "\nSelection:    ";
        std::cin >> user_input;

        // Initialize data structure for PID
        if (user_input == 'm') {
            if (pid_manager.allocate_map() == 1) {
                std::cout << "PID map allocated successfully." << std::endl;
            } else {
                std::cerr << "Error: Failed to allocate PID map." << std::endl;
            }
        }
        // Allocate random pid in the range 100 to 1000 
        else if (user_input == 'p') {
            int pid = pid_manager.allocate_pid();
            int ppid = pid_manager.allocate_pid();
            
            // Error of PID
            if (pid == -1) {
                std::cerr << "Error: No available PIDs." << std::endl;
            } 
            
            // When PID allocate is possible
            else {
                // Handle PID allocation in parent and child processes
                pid_t child_pid = fork();
                if (child_pid == -1) {
                    std::cerr << "Error: Failed to fork process." << std::endl;
                } else if (child_pid == 0) {
                    // Child process
                    std::cout << "Child process allocated PID: " << pid << std::endl;
                    // Simulate child process work here (using allocated PID if needed)
                    exit(0); // Child process exits
                } else {
                    // Parent process
                    std::cout << "Parent process allocated PID: " << ppid << " to child (PID: " << pid << ")" << std::endl;
                    // Wait for child process to finish
                    wait(nullptr);
                }
            }
        }
        // Release PID which user inputted 
        else if (user_input == 'r') {
            int pid_to_release;
            std::cout << "Enter PID to release: ";
            std::cin >> pid_to_release;
            pid_manager.release_pid(pid_to_release);
        } else if (user_input != 'p' && user_input != 'm' && user_input != 'r' && user_input != 'e'  ) {
            std::cout << "Invalid input. Please try again." << std::endl;
        }
    } 
    // Exit
    while (user_input != 'e');

    return 0;
}
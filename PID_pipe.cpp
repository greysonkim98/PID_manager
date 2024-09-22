//Program name: "PID_pipe"                                                                                           
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
//  Date of last update: 2024-09-22
//  Date of reorganization of comments: 2024-09-22
//  Files in this program: PIDmanager.cpp
//  Developed OS: Ubuntu 24.04.1 LTS
//  Tested OS: Ubuntu 24.04.1 LTS
//  Tested Compiler: g++ (Ubuntu 13.2.0-23ubuntu4) 13.2.0
//
//Program description
//  This program allocate PID, and release PID based on the commincation between parents process and child process
//  User can select the options of allocate space, allocate random PID and release it.
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <ctime>

#define MIN_PID 100
#define MAX_PID 1000
#define READ_END 0
#define WRITE_END 1

class PIDManager {
public:
    int allocate_map() {
        try {
            map = std::vector<PIDInfo>(MAX_PID - MIN_PID + 1);
            return 1;
        } catch (std::bad_alloc& e) {
            std::cerr << "Error: Failed to allocate PID map: " << e.what() << std::endl;
            return -1;
        }
    }

    int allocate_pid() {
        if (map.empty()) {
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
        if (map.empty()) {
            std::cerr << "Error: PID map not initialized. Cannot release PID." << std::endl;
            return;
        }
        // Release PID within the range
        if (pid >= MIN_PID && pid <= MAX_PID) {
            // Checking if the users release PID before initializing
            if (map[pid - MIN_PID].isAllocated) {
                map[pid - MIN_PID].isAllocated = false;
                map[pid - MIN_PID].pid = 0;
                std::cout << "\n" << pid << " has been released\n";
            } else {
                std::cerr << "Error: PID " << pid << " is not allocated." << std::endl;
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
    int write_pid{0};
    int read_pid{0};
    int request{3};
    int cpid{0};
    int ppid{0};
    
    // Allocate map after obtaining user input
    std::cout << "\nEnter 'm' to allocate map: ";
    std::cin >> user_input;
    if (user_input == 'm') {
        if (pid_manager.allocate_map() == 1) {
            std::cout << "PID map allocated successfully." << std::endl;
        } else {
            std::cerr << "Error: Failed to allocate PID map." << std::endl;
        }
    } else {
        std::cout << "You have ao allocate map" << std::endl;
        return 1;
    }
    
    // Allocate PID option
    std::cout << "\nEnter 'p' to allocate pid: ";
    std::cin >> user_input;
    if(user_input == 'p'){
        // Check availability of allocate function
        int pid = pid_manager.allocate_pid();
        if (pid == -1) {
                std::cerr << "Error: No available PIDs." << std::endl;
        }

        while(request != 0){ 
            if(request == 0) {
                return 1;
            }
           
            ppid = pid_manager.allocate_pid();
            // Create a pipe and checking error
            int pipe1[2];
            int pipe2[2];
            int pipe3[2];
            if (pipe(pipe1) == -1) {
                std::cerr << "Error: Pipe failed." << std::endl;
                return 1;
            }
            if (pipe(pipe2) == -1) {
                std::cerr << "Error: Pipe failed." << std::endl;
                return 1;
            }
            if (pipe(pipe3) == -1) {
                std::cerr << "Error: Pipe failed." << std::endl;
                return 1;
            }
                       
            
            // Obtain User Option
            sleep(1);
            std::cout << "\nEnter the Choice:\n1: Request PID\n2: Release PID\n0: Terminate" << std::endl;
            std::cout << "\nSelection: ";
            std::cin >> request;

            pid_t child_pid = fork();
                if (child_pid == -1) {
                    std::cerr << "Error: Failed to fork process." << std::endl;
                }
                else if(child_pid == 0) {  // Child process
                    
                    // Sending user's selection to parent process
                    close(pipe1[READ_END]); 
                    write(pipe1[WRITE_END], &request, sizeof(int));
                    close(pipe1[WRITE_END]);

                    if(request == 1) {  // Request a PID
                        close(pipe2[WRITE_END]);
                        read(pipe2[READ_END], &read_pid, sizeof(int));  // Receive PID
                        cpid = read_pid;
                        std::cout << "\nHello from child, received PID: " << cpid << std::endl <<std::endl;
                        close(pipe2[READ_END]);
                    } else if(request == 2) {  // Release PID
                        close(pipe3[READ_END]);
                        std::cout << "Enter PID to release: " ;
                        std::cin >> write_pid;
                        write(pipe3[WRITE_END], &write_pid, sizeof(int));  // Send the child's own PID to the parent
                        close(pipe3[WRITE_END]);
                    } else if (request == 0) {        // Terminate program               
                        std::cout << "Child sending 'Done' command. Terminating.";
                        exit(0);
                    }                    
                }
                else {  // Parent process
                    // Receive user selection from child process
                    close(pipe1[WRITE_END]); 
                    read(pipe1[READ_END], &request, sizeof(int));
                    close(pipe1[READ_END]);

                    if (request == 1) {  // Child requesting PID
                        write_pid = pid_manager.allocate_pid();
                        write(pipe2[WRITE_END], &write_pid, sizeof(int));  // Send PID back to child
                        close(pipe2[WRITE_END]);
                    } else if (request == 2) {  // Child requesting to release a PID
                        int pid_to_release{0};
                        read(pipe3[READ_END], &pid_to_release, sizeof(int));  // Receive PID to release
                        std::cout << "Parent received request to release PID: " << pid_to_release << std::endl;
                        pid_manager.release_pid(pid_to_release);
                        close(pipe3[READ_END]);
                    } else if (request == 0) {  //read_end Child sent Done to terminate
                        std::cout << "Parent received 'Done' command. Terminating.\n" << std::endl;
                        request = 0;
                        break;
                    }
                    wait(nullptr);
                }  
            }
    }     
    else{
        std::cout << "PID is not allocated\n";
        return 1;
    }
    sleep(1);
    
    // Wait for all processes to terminate
    while (waitpid(-1, NULL, 0) > 0) {
        std::cout << "Reaped a child process." << std::endl;
    }

    std::cout << "All processes terminated." << std::endl;

    exit(EXIT_SUCCESS);
    return 0; 
}
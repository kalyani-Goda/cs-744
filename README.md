### DECSProject

##Project DECServer

This project, "Project DECServer," is undertaken as part of the CS 744: Design and Engineering of Computing Systems course requirement for Autumn 2023. The project encompasses five versions of the client-server grading application implementations.

###File Structure

DECSProject
|-- README.md
|-- Version-1
|   |-- Bin
|   |-- Include
|   |-- Obj
|   |-- Output
|   |-- Src
|   |-- Test
|   |-- Makefile
|   |-- Readme.md
|   |-- tmp.txt
|-- Version-2
|   |-- ... (similar structure as Version-1)
|-- Version-3
|   |-- ... (similar structure as Version-1)
|-- Version-4
|   |-- ... (similar structure as Version-1)
|-- Version-5
|
|--loadtest/
   |
   |--loadtest_v1-3.sh
   |--loadtest_v4.sh
   |-- Version-1
       |-- Graph_results
       |-- Simulation_results
   |-- Version-2
   .
.
|--presentation
   |-- presentation report.pdf


All versions are organized using the make command, and the loadtest_script.sh is common for Version-1 to Version-3.

#Execution Instructions

##1. Version 1

Commands to Run Client and Server:

To run the server, navigate to the Version-1 folder and execute the following command on one machine:
$ ./bin/server <port_number>

To run the client, navigate to the Version-1 folder and execute the following command on another machine:
$ ./bin/client <ip_address> <port_number> <auto_grader_file> <loop_time> <sleep_time> <timeout_count>

Commands to Run Performance Experiments:
To Run the load test in the load test script we gave a sequence to run ‘m’ number of clients starting from ‘i’ to ‘m’ with some ‘n’ intervals.  
In the script the command we wrote was  “ loops=$(seq 2 2 10) “ it takes the loops starting from 2 to 10 with each interval with 2. The set of clients we are running the load test is {2, 4, 6, 8, 10}.

To run the load test for Version-1, go to the Version-1 folder and execute the following command on another machine:
$ ../loadtesting/loadtest_v1_3.sh <ip_address> <port_number> <loop_time> <sleep_time> <timeout_count>
After running the load test script, the results for each client are stored in the folder simulation_results/n_clients/client_number.txt. 
Ex: simulation_results/2_clients/1.txt means the test run for 2 clients and the 1st clients results are stored within the folder 2_clients with the file name 1.txt.
The performance-related data and graphs are stored in the "graph_results" folder.
In this folder the .txt files contain the data related to the metric, .png files contain the graphs.

##2. Version 2
Commands to Run Client and Server:
To run the server goto version-2 folder, then run this command in one machine to run the server.

$ ./bin/server <port_number>		

To run the client goto version-2 folder, then run this command in another machine to run the client.

$ ./bin/client <ip_address> <port_number> <auto_grader_file> <loop_time> <sleep_time> <timeout_count> 

COMMANDS TO RUN PERFORMANCE EXPERIMENTS:

To Run the load test in the load test script we gave a sequence to run ‘m’ number of clients starting from ‘i’ to ‘m’ with some ‘n’ intervals.  
In the script the command we wrote was  “ loops=$(seq 2 2 10) “ it takes the loops starting from 2 to 10 with each interval with 2. The set of clients we are running the load test is {2, 4, 6, 8, 10}.

To run the load test for the version-2, goto version-2 folder then execute the command given below which is need to run in another machine,
	
$ ../loadtesting/loadtest_v1_3.sh <ip_address> <port_number> <loop_time> <sleep_time> <timeout_count> 

After running this loadtest_v1_3 the results of the each clients are stored in the folder simulation_results/’n’_clients/’client_number’.txt

Ex: simulation_results/2_clients/1.txt means the test run for 2 clients and the 1st clients results are stored within the folder 2_clients with the file name 1.txt.

And the performance graph related data and the graphs are stored in the folder named “ graph _ results".

In this folder the .txt files contain the data related to the metric, .png files contain the graphs.

##3. Version 3:
COMMANDS TO RUN CLIENT AND SERVER:
To run the server goto version-3 folder, then run this command in one machine to run the server.

$ ./bin/server <port_number> <thread_pool_size>		

To run the client goto version-3 folder, then run this command in another machine to run the client.

$ ./bin/client <ip_address> <port_number> <auto_grader_file> <loop_time> <sleep_time> <timeout_count> 

COMMANDS TO RUN PERFORMANCE EXPERIMENTS:

To Run the load test in the load test script we gave a sequence to run ‘m’ number of clients starting from ‘i’ to ‘m’ with some ‘n’ intervals.  
In the script the command we wrote was  “ loops=$(seq 2 2 10) “ it takes the loops starting from 2 to 10 with each interval with 2. The set of clients we are running the load test is {2, 4, 6, 8, 10}.

To run the load test for the version-3, goto version-3 folder then execute the command given below which is need to run in another machine,
	
$ ../loadtesting/loadtest_v1_3.sh <ip_address> <port_number> <loop_time> <sleep_time> <timeout_count> 

After running this loadtest_v1_3 the results of the each clients are stored in the folder simulation_results/’n’_clients/’client_number’.txt

Ex: simulation_results/2_clients/1.txt means the test run for 2 clients and the 1st clients results are stored within the folder 2_clients with the file name 1.txt.

And the performance graph related data and the graphs are stored in the folder named “ graph _ results".

In this folder the .txt files contain the data related to the metric, .png files contain the graphs.

##4. VERSION 4:
COMMANDS TO RUN CLIENT AND SERVER:
To run the server goto version-4 folder, then run this command in one machine to run the server.
$ ./bin/server <port_number> <grader_thread_pool_size> <status_thread_pool_size>		
To run the client goto version-4 folder, then run this command in another machine to run the client.
     $ ./bin/client <ip_address> <port_number> request_type<new/status> <request_id/autograder>

If your are running for the new request then the command will be,
			$ ./bin/client <ip_address> <port_number> new <autograder_filename>

			It will give the request id as a return message.

Then run with the below command after some time interval to check the status of execution,
$ ./bin/client <ip_address> <port_number> status <request_id>

COMMANDS TO RUN PERFORMANCE EXPERIMENTS:

To run load test for v4, run the command given below from verision-4 directory
$ ../loadtesting/loadtest_v4.sh <ip_address> <port_number> <loop_time> <sleep_time> <timeout_count>

Github Repository Link:  

https://git.cse.iitb.ac.in/decs/decsproject.git

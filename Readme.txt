CSC361 Assignment 2:
/*
* Maxwell A. Borden
* V00757554
*
* Lab Section B06
*/

How To: From the directory this file resides in, simply run the 'make' command. This will generate two files, rdpr and rdps.
run them with the following commands.
./rdpr 10.10.1.100 8080 [your_file_copy]
./rdps 192.168.1.100 8080 10.10.1.100 8080 [your_file]

Implementation notes:
I originally planned to implement selective repeat, but changed to go-back-n due to time constraints. The way my programs work is by filling up priority queues on the sender side with every sent packet. The priority is based on the time at which the packet was created. Every time through my main loop it tries to pop off as many timed out or acknoledged packets as possible. timed out packets resent and restamped using the change priority function. When a packet gets to the receiver, they are immediatedly inserted into a priority queue based on their sequence numbers. every time I check the queue, any in order packets at the head of the queue are popped of and written to the receive file. As a result, a large portion of the logic of my programs resides in the filter_IB and filter_OB functions. Other important functions are the main functions of each program, and the handle_packet function.

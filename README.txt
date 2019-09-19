S3516 (A-Term 2019)
Programming Assignment 1: Socket Programming
Tuesday September 10, 2019

Nathan Bargman
nmbargman - 605103273

Setup
Running these programs will work best in terminal. To start find your way to the directory of this file and enter "ls". There should the following files/folders; README.txt, Makefile, server_files, client_files. Once you see that you can start by running the makefile.

Makefile
When in this directory typing and running "make" will compile and make both the client and the server.c files.

Running the Client
First enter the client_files directory with "cd client_files/". Then type in the command line "./client [-options] hostname/path port". Make sure to include the "/" in the hostname/path even if the path is "". If there are no options the command should look like "./client hostname/path port". Accepted options are -p (print out the RTT to the terminal before the server response), and -r (only print the RTT to the terminal). After the command is run the terminal will print out the servers response with any additional options added. After the program is done you may have to scroll up to find the RTT or response header as it will print out the received .html or page info.

Each run ends with the print:
---
END CLIENT
---

Running the Server
First enter the server_files directory with "cd server_files/". Then type in the command line "./server port". After that the server should run smoothly and can be connected to via the hostname it will print out. After each connection the terminal will display the clients ip. This server could should also be able to handle multiple clients at once through the use of the fork() method.

Additional Info
I wrote and tested this on ccc.wpi.edu. I was able to call "wget hostname:port/TMDG.html" from a second terminal window while inside ccc.wpi.edu and get the file. This however didn't work from outside ccc.wpi.edu. I am not sure what causes this issue.

If the Makefile fails you can always go into each folder and enter "gcc client/server.c -o client/server" to compile them.

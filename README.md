# Lets-Talk
A chatroom with many features built in C implemented with multithreading concepts This chatroom enables a user at one terminal to communicate with a user at another terminal

## Sample output

The user at terminal 1:
![[letstalk1.png]]

The user at terminal 2:
![[letstalk2.png]]

## Logic

![[letstalkLogic.png]]

### Required threads (in each of the processes):

One of the threads awaits input from the keyboard.
The other thread awaits a UDP datagram from another process.
There will also be a thread that prints messages (sent and received) to the screen.
Finally, a thread that sends data to the remote UNIX process over the network using UDP (use localhost IP(127.0.0.1) while testing it on the same machine with two terminals).

All four threads will share access to a list ADT.

The keyboard input thread, on receipt of the input, adds the input to the list of messages that need to be sent to the remote lets-talk client.
The UDP sender thread will take each message off this list and send it over the network to the remote client.
The UDP receiver thread, on receipt of input from the remote lets-talk client, will put the message onto the list of messages that need to be printed to the local screen.
The console output thread will take each message off this list and output it to the screen.

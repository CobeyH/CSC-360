# UVic Operating Systems
This course teaches to set up computing resources to be shared safely, efficiently, and effectively among many users, including the major concepts of modern operating systems and the relationship between the OS and the architecture of computer systems.

The first assignment involved the creation of a shell that could process tasks on different threads. Several custom commands were implemented to be able to bring tasks into the background of foreground, print the list of current tasks, etc.

The second assignment used semaphores and mutexes to create a train thread-safe train scheduler. The virtual trains are provided in a text file with a length, direction and arrival time and the program must prevent train collisions accross a bridge.
There is no central arbitor, so the trains have to broadcast their state to the conductors.

The final assignment is a basic file system implementation, similar to FAT32. It can read and write files into blocks and list the files contained in the file system.


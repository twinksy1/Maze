## ABOUT
> This is a small program that makes use of the X11 libraries to display a maze generated using prim's algorithm. This is achieved by using nodes & relating neighboring nodes. The main goal is to reach the end node, which is colored in red, with the player, the blue circle.
## BUILD
* X11 library needs to be installed
	* **Linux**: *sudo apt install libx11-dev*
* X11 extensions must also be installed
	* **Linux**: *sudo apt-get install libxtst-dev*
---
Then in cloned directory:
* **Build**: *make*
---
Execution: ***./main***
## FEATURES
* Press 's' at any time to display the route from the start of the maze to the end
* Press 'c' to clear the highlighted solution and resume player movement
* Once you reach the end of the maze, a new one is randomly generated
* Create a new maze at any time by pressing 'r'
* Move the player object with the arrow keys

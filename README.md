
# Box push game




##  Program Description

- This game simulates robots that must push boxes on a grid. The robots will be implemented as threads in the program. 
- In order to push a box, a robot must be standing on a grid square adjacent to the box, and push the box in the opposite direction. That means, every robot must move to the adjacent position  before push the its box.  There are therefore only four possible directions for a robot to push a box. We will call these displacement directions W, E, N, S (for West, East, North, and South respectively). In Figure 1, the boxes are represented as brown squares while the robots are represented as light green squares. 




![Figure1](https://github.com/YuHaoLao/Box-Pushing-game/blob/main/img/Figure1.png?raw=true)

- After the box is pushed into the designated door by the robot, the box and robot will disappear from the grid.






## Rules To Follow

- The robots only know three commands:
    - move direction (where direction is W, E, N, or S), to move to an unoccupied adjacent square in the chosen direction;
    - push direction (where direction is W, E, N, or S), to push an adjacent box into the unoccu- pied square in the chosen direction;
    - end when the robot has pushed its box to its destination and terminates.

- No robot is allowed to move through the box it is about to push. All the doors are allowed to go through. The path shown for the box in Figure 3 is therefore acceptable.
![Figure3](https://github.com/YuHaoLao/Box-Pushing-game/blob/main/img/Figure3.png?raw=true)

## Install MESA and FreeGlut

- MESA is a open-source implementation of OpenGL, the only free option on Linux. FreeGLUT is a more up-to-date version of GLUT (the GL Utility Toolkit), a library that provides binding between OpenGL and the OS3. MESA and FreeGLUT come installed by default on Ubuntu, so that you can run pre-built OpenGL, but in order to build a MESA project, you need to install the developer version of both libraries.
 Make sure first that your Ubuntu install is up to date, and then execute: 
```bash
• sudo apt-get install freeglut3-dev

• sudo apt-get install mesa-common-dev
```

## Compile and Run 

- For singel push:
```
 g++ robotV1.cpp gl_frontEnd.cpp -lm -framework OpenGL -framework GLUT -o robot

 ./robot width of grid, height of grid, number of boxes, number of doors

 Example :  ./robot 20 16 4 4

 GIF: 
```
![Figure3](https://github.com/YuHaoLao/Box-Pushing-game/blob/main/img/singel.gif?raw=true)
- For multithreading:
```
 g++ main.cpp gl_frontEnd.cpp -lm -framework OpenGL -framework GLUT -o robot

 ./robot width of grid, height of grid, number of boxes, number of doors
 
 Example :  ./robot 20 16 4 4

 GIF:
```
![Figure3](https://github.com/YuHaoLao/Box-Pushing-game/blob/main/img/mult.gif?raw=true)

- For .bash
```
 bash script.sh width of grid, height of grid, number of boxes, number of doors, number of execution

 Example: bash script.sh 20 16 4 4 2
```
## Output of the program
- A txt file contains the moving path of all the robot and box.


## If I have more time

As you can imagine, as the number of robots and boxes increases, the odds of encountering a deadlock increase. If I have more time, I will come up with a strategy for detecting a deadlock in the simulation.

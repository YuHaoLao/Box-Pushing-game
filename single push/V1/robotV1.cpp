//
//  main.cpp
//  Final Project CSC412
//
//  Created by Jean-Yves Hervé on 2018-12-05, Rev. 2021-12-01
//	This is public domain code.  By all means appropriate it and change is to your
//	heart's content.
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <random>
#include <fstream>
#include <unistd.h>
#include "guiChoice.h"
#if CSC412_FP_USE_GUI
	#include "gl_frontEnd.h"
#endif

using namespace std;

//==================================================================================
//	Function prototypes
//==================================================================================
#if CSC412_FP_USE_GUI
	void displayGridPane(void);
	void displayStatePane(void);
#endif
void printGrid(void);
void initializeApplication(void);


//==================================================================================
//	Application-level global variables
//==================================================================================

//	Don't touch
#if CSC412_FP_USE_GUI
	extern int	GRID_PANE, STATE_PANE;
	extern int	gMainWindow, gSubwindow[2];
	extern int GRID_PANE_WIDTH, GRID_PANE_HEIGHT;
	extern int STATE_PANE_WIDTH, STATE_PANE_HEIGHT;
#endif


//	Don't rename any of these variables
//-------------------------------------
//	The state grid and its dimensions (arguments to the program)
int** grid;
int numRows = -1;	//	height of the grid
int numCols = -1;	//	width
int numBoxes = -1;	//	also the number of robots
int numDoors = -1;	//	The number of doors.

int numLiveThreads = 0;		//	the number of live robot threads

//	robot sleep time between moves (in microseconds)
const int MIN_SLEEP_TIME = 1000;
int robotSleepTime = 100000;

//	An array of C-string where you can store things you want displayed
//	in the state pane to display (for debugging purposes?)
//	Dont change the dimensions as this may break the front end
const int MAX_NUM_MESSAGES = 8;
const int MAX_LENGTH_MESSAGE = 32;
char** message;

//-----------------------------
//	CHANGE THIS
//-----------------------------
//	Here I hard-code myself some data for robots and doors.  Obviously this code
//	must go away.  I just want to show you how information gets displayed.  
//	Obviously, you will need to allocate yourself some dynamic data structure to store 
//	that information once you know the dimensions of the grid, number of boxes/robots and
//	doors.  
//	Note that, even if you use the GUI version, it doesn't impose you a storage format at
//	all, since the drawing function draw a single robot/box/door at a time, and take that
//	object's parameters as individual argumenta.
//	So, feel free to go vectors all the way if you like it better than int**
//	Just don't feel free to declare oversized arrays, in the style of
//	int robotLoc[1000][2];
//	I can guarantee you that this kind of stuff will get penalized harshly (if might have
//	been convenient, borderline cute, in CSC211, but by now it's absolutely embarrassing)
//
//	Also:  Please note that because this is a grid-based problem, I never think of x and y but
//			row and column (well, the "I" dealing with the planning problem.  The "I" doing
//			the dirty work underneath has to translated all of that into x and y pixel
//			coordinates for OpenGL for rendering
//		   So,... In all of these arrays of 2 int values, the first value (index 0)
//			is a row coordinate and the second value (index 1) is a y coordinate.
  //                  0        1        2        3    

int** robotLoc;
int** boxLoc;
int* doorAssign;	//	door id assigned to each robot-box pair
int** doorLoc;




//for write data to file
ofstream myfile;


//  for the randam generate
random_device myRandDev;
default_random_engine myEngine(myRandDev());
uniform_int_distribution<int> rowdist,coldist;
uniform_int_distribution<int> boxrowdist,boxcoldist;
uniform_int_distribution<int> doorAssignGenerator;

/**
 * @brief function go generate random door location
 * 
 */
void generateDoors(void) {
	doorLoc = new int*[numDoors];
	for (int k=0; k<numDoors; k++)
	{
		doorLoc[k] = new int[2];
		// generate location for Door k
		bool goodLocation = false;
		while (!goodLocation) 
		{
			int row  = rowdist(myEngine);
			int col  = coldist(myEngine);
		
			// check against doors previously created 
			goodLocation = true;
			for (int i=0; i<k && goodLocation; i++) 
			{
				if ((doorLoc[i][0] == row) && (doorLoc[i][1] == col))
				{
					goodLocation = false;
				}
			}
			
			if (goodLocation)
			{
				doorLoc[k][0] = row;
				doorLoc[k][1] = col;
			}
		}
	}
}


/**
 * @brief function go generate random box location
 * 
 */
void generateBoxes(void) {
	boxLoc = new int*[numBoxes];
	for (int k=0; k<numBoxes; k++)
	{
		boxLoc[k] = new int[2];
		// generate location for box k
		bool goodLocation = false;
		while (!goodLocation) 
		{
			int row  = boxrowdist(myEngine);
			int col  = boxcoldist(myEngine);
	
			// check against all doors and boxes previously created 
			goodLocation = true;
			for (int i=0; i<numDoors && goodLocation; i++) 
			{
				if ((doorLoc[i][0] == row) && (doorLoc[i][1] == col))
				{
					goodLocation = false;
				}
			}
			for (int i=0; i<k && goodLocation; i++) 
			{
				if ((boxLoc[i][0] == row) && (boxLoc[i][1] == col))
				{
					goodLocation = false;
				}
			}
			//if true which mean the location unique and assign to the box
			if (goodLocation)
			{
				boxLoc[k][0] = row;
				boxLoc[k][1] = col;
			}
		}
	}
}
/**
 * @brief function go generate random robot location
 * 
 */
void generateRobot(void) {
	robotLoc = new int*[numBoxes];
	for (int k=0; k<numBoxes; k++)
	{
		robotLoc[k] = new int[2];
		// generate location for robot k
		bool goodLocation = false;
		while (!goodLocation) 
		{
			int row  = rowdist(myEngine);
			int col  = coldist(myEngine);

			
			// check against all doors,boxes and robots previously created 
			goodLocation = true;

			for (int i=0; i<numBoxes&& goodLocation ; i++) 
			{
				if ((boxLoc[i][0] == row) && (boxLoc[i][1] == col))
				{
					goodLocation = false;
				}
			}
			for (int i=0; i<numDoors && goodLocation; i++) 
			{
				if ((doorLoc[i][0] == row) && (doorLoc[i][1] == col))
				{
					goodLocation = false;
				}
			}
			for (int i=0; i<k && goodLocation; i++) 
			{
				if ((robotLoc[i][0] == row) && (robotLoc[i][1] == col))
				{
					goodLocation = false;
				}
			}
			
			//if true which mean the location unique and assign to the robot
			if (goodLocation)
			{
				robotLoc[k][0] = row;
				robotLoc[k][1] = col;
			}
		}
	}
}
/**
 * @brief function go generate random doorassign 
 * 
 */
void generateDoorassign(){
    doorAssign = new int[numBoxes];
    for(int i=0;i <numBoxes;i++){
        doorAssign[i] = doorAssignGenerator(myEngine);
    }

}

//struct type for robot
typedef struct info {
	pthread_t id;
	int index;
	bool is_running;
	int door_row;
	int door_col;
} info;

vector<info>info_list; 
info* boxinfo;

//==================================================================================
//	These are the functions that tie the simulation with the rendering.
//	Some parts are "don't touch."  Other parts need your intervention
//	to make sure that access to critical section is properly synchronized
//==================================================================================

#if CSC412_FP_USE_GUI

void displayGridPane(void)
{
	// single-threaded version
//	for (all robots)
//	{
//		execute one move
//	}
	
	//	This is OpenGL/glut magic.  Don't touch
	//---------------------------------------------
	glutSetWindow(gSubwindow[GRID_PANE]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// move to top of the pane
	glTranslatef(0.f, GRID_PANE_WIDTH, 0.f);
	// flip the vertiucal axis pointing down, in regular "grid" orientation
	glScalef(1.f, -1.f, 1.f);

	for (int i=0; i<numBoxes; i++)
	{
		//	here I would test if the robot thread is still live
		
		if(info_list[i].is_running){

			drawRobotAndBox(i, robotLoc[i][0], robotLoc[i][1], boxLoc[i][0], boxLoc[i][1], doorAssign[i]);
		}
		
		}

		

	for (int i=0; i<numDoors; i++)
	{
		//	here I would test if the robot thread is still alive
		drawDoor(i, doorLoc[i][0], doorLoc[i][1]);
	}

	//	This call does nothing important. It only draws lines
	//	There is nothing to synchronize here
	drawGrid();

	//	This is OpenGL/glut magic.  Don't touch
	glutSwapBuffers();
	
	glutSetWindow(gMainWindow);
}

void displayStatePane(void)
{
	//	This is OpenGL/glut magic.  Don't touch
	glutSetWindow(gSubwindow[STATE_PANE]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//	Here I hard-code a few messages that I want to see displayed
	//	in my state pane.  The number of live robot threads will
	//	always get displayed.  No need to pass a message about it.
	int numMessages = 3;
	sprintf(message[0], "We have %d doors", numDoors);
	sprintf(message[1], "I like cheese");
	sprintf(message[2], "System time is %ld", time(NULL));
	
	//---------------------------------------------------------
	//	This is the call that makes OpenGL render information
	//	about the state of the simulation.
	//
	//	You *must* synchronize this call.
	//
	//---------------------------------------------------------
	drawState(numMessages, message);
	
	
	//	This is OpenGL/glut magic.  Don't touch
	glutSwapBuffers();
	
	glutSetWindow(gMainWindow);
}

#endif // CSC412_FP_USE_GUI

//------------------------------------------------------------------------
//	You shouldn't have to touch this one.  Definitely if you don't
//	add the "producer" threads, and probably not even if you do.
//------------------------------------------------------------------------
void speedupRobots(void)
{
	//	decrease sleep time by 20%, but don't get too small
	int newSleepTime = (8 * robotSleepTime) / 10;
	
	if (newSleepTime > MIN_SLEEP_TIME)
	{
		robotSleepTime = newSleepTime;
	}
}

void slowdownRobots(void)
{
	//	increase sleep time by 20%
	robotSleepTime = (12 * robotSleepTime) / 10;
}

/**
 * @brief function to move robot to up
 * @param index robot index
 * @param distance row distance between robot and box
 */
void robot_move_up(int index, int distance){
	
	for (int i =0;i<distance;i++){
		robotLoc[index][0]-=1;
		usleep(robotSleepTime);
		displayGridPane();
		
		myfile<<"robot "<<index<<" "<<" move N"<<endl;
		

	}
}

/**
 * @brief function to move robot to down
 * @param index robot index
 * @param distance row distance between robot and box
 */
void robot_move_down(int index, int distance){
	
	for (int i =0;i<distance;i++){
		robotLoc[index][0]+=1;
		usleep(robotSleepTime);
		displayGridPane();
		
	
		myfile<<"robot "<<index<<" "<<" move S"<<endl;

	}
}

/**
 * @brief function to move robot to left
 * @param index robot index
 * @param distance col distance between robot and box
 */
void robot_move_left(int index, int distance){
	
	for (int i =0;i<distance;i++){
		robotLoc[index][1]-=1;
		usleep(robotSleepTime);
		displayGridPane();
	
		myfile<<"robot "<<index<<" "<<" move W"<<endl;
	}
}

/**
 * @brief function to move robot to right
 * @param index robot index
 * @param distance col distance between robot and box
 */
void robot_move_right(int index, int distance){
	
	for (int i =0;i<distance;i++){
		robotLoc[index][1]+=1;
		usleep(robotSleepTime);
		displayGridPane();
	
		myfile<<"robot "<<index<<" "<<" move E"<<endl;
	}
}

/**
 * @brief function to push box to up
 * 
 * @param index robot index
 * @param distance row distance between box and door 
 */
void robot_pushbox_up(int index, int distance){
	for (int i =0;i<distance;i++){
		boxLoc[index][0]-=1;
		robotLoc[index][0]-=1;
		usleep(robotSleepTime);
		displayGridPane();
		
		myfile<<"robot "<<index<<" "<<" push N"<<endl;
	}
}
/**
 * @brief function to push box to down
 * 
 * @param index robot index
 * @param distance row distance between box and door 
 */
void robot_pushbox_down(int index, int distance){
	for (int i =0;i<distance;i++){
		boxLoc[index][0]+=1;
		robotLoc[index][0]+=1;
		usleep(robotSleepTime);
		displayGridPane();
		
		myfile<<"robot "<<index<<" "<<" push S"<<endl;
		
	}
}


/**
 * @brief function to push box to left
 * 
 * @param index robot index
 * @param distance col distance between box and door 
 */
void robot_pushbox_left(int index, int distance){
	for (int i =0;i<distance;i++){
		boxLoc[index][1]-=1;
		robotLoc[index][1]-=1;
		usleep(robotSleepTime);
		displayGridPane();
	
		myfile<<"robot "<<index<<" "<<" push W"<<endl;
		
	}
}


/**
 * @brief function to push box to right
 * 
 * @param index robot index
 * @param distance col distance between box and door 
 */
void robot_pushbox_right(int index, int distance){
	for (int i =0;i<distance;i++){
		boxLoc[index][1]+=1;
		robotLoc[index][1]+=1;
		usleep(robotSleepTime);
		displayGridPane();
		
		myfile<<"robot "<<index<<" "<<" push E"<<endl;
		
	}
}

/**
 * @brief input the direction and postion col and row and its ID, that robot will move to the setup postion
 * 
 * @param direction direction only will be 0 and 1. 0 represents horizontal 1 represents vertical
 * @param rtnew_col_poistion destination  col
 * @param rtnew_row_poistion  destination row
 * @param index robot index 
 */
void robot_move_to_destination( int direction, int rtnew_col_poistion,int rtnew_row_poistion,int index){

	int rtb_col_distance,rtb_row_distance;
	// move horizontal first
	if (direction==0){
		if (robotLoc[index][1]>rtnew_col_poistion){
			rtb_col_distance=robotLoc[index][1]-rtnew_col_poistion;
			robot_move_left(index,rtb_col_distance);
		}
		else if (robotLoc[index][1]<rtnew_col_poistion){
			rtb_col_distance=rtnew_col_poistion-robotLoc[index][1];
			robot_move_right(index,rtb_col_distance);
		}
		// compare row (up and down)
		if (robotLoc[index][0]>rtnew_row_poistion){
			rtb_row_distance=robotLoc[index][0]-rtnew_row_poistion;
			robot_move_up(index,rtb_row_distance);
		}
		else if (robotLoc[index][0]<rtnew_row_poistion){
			rtb_row_distance=rtnew_row_poistion-robotLoc[index][0];
			robot_move_down(index,rtb_row_distance);
		}

	}
	// move vertical first
	else if (direction==1){

		if (robotLoc[index][0]>rtnew_row_poistion){
			rtb_row_distance=robotLoc[index][0]-rtnew_row_poistion;
			robot_move_up(index,rtb_row_distance);
		}

		else if (robotLoc[index][0]<rtnew_row_poistion){
			rtb_row_distance=rtnew_row_poistion-robotLoc[index][0];
			robot_move_down(index,rtb_row_distance);
		}

		if (robotLoc[index][1]>rtnew_col_poistion){
			rtb_col_distance=robotLoc[index][1]-rtnew_col_poistion;
			robot_move_left(index,rtb_col_distance);
		}

		else if (robotLoc[index][1]<rtnew_col_poistion){
			rtb_col_distance=rtnew_col_poistion-robotLoc[index][1];
			robot_move_right(index,rtb_col_distance);
		}
	}
}

/**
 * @brief function to close file and quit
 * 
 */
void quit(void)
{

	myfile.close();
	// exit(0);
}

/**
 * @brief function to write the box robot and door location
 * 
 */
void writeLocation(){
	for(int i=0;i<numDoors;i++){
		myfile<<"Door "<<i<<" ("<<doorLoc[i][0]<<","<<doorLoc[i][1]<<")"<<endl;
	}
	myfile<<"\n";
	for(int i=0;i<numBoxes;i++){
		myfile<<"Boxe "<<i<<" ("<<boxLoc[i][0]<<","<<boxLoc[i][1]<<")"<<endl;
	}
	myfile<<"\n";
	for(int i=0;i<numBoxes;i++){
		myfile<<"Robot "<<i<<" ("<<robotLoc[i][0]<<","<<robotLoc[i][1]<<")"<<" Destination door "<<doorAssign[i]<<endl;
	}
	myfile<<"\n";

}


//------------------------------------------------------------------------
//	You shouldn't have to change anything in the main function besides
//	the initialization of numRows, numCos, numDoors, numBoxes.
//------------------------------------------------------------------------
int main(int argc, char** argv)
{
	//	We know that the arguments  of the program  are going
	//	to be the width (number of columns) and height (number of rows) of the
	//	grid, the number of boxes (and robots), and the number of doors.
	//	You are going to have to extract these.  For the time being,
	//	I hard code-some values
	numCols = atoi(argv[1]);
	numRows = atoi(argv[2]);
	numBoxes = atoi(argv[3]);
	numDoors = atoi(argv[4]);
	


	// door
	rowdist = uniform_int_distribution<int>(0,numRows-1);
	coldist = uniform_int_distribution<int>(0,numCols-1);

	//box 
	boxrowdist = uniform_int_distribution<int>(1,numRows-2);
	boxcoldist = uniform_int_distribution<int>(1,numCols-2);


	//door assign
	doorAssignGenerator = uniform_int_distribution<int>(0,numDoors-1);


	//call to generate random door, box, robot, doorassign
	generateDoors();
	generateBoxes();
	generateRobot();
	generateDoorassign();

	
#if CSC412_FP_USE_GUI
	//	Even though we extracted the relevant information from the argument
	//	list, I still need to pass argc and argv to the front-end init
	//	function because that function passes them to glutInit, the required call
	//	to the initialization of the glut library.
	initializeFrontEnd(argc, argv, displayGridPane, displayStatePane);
#endif



initializeApplication();
myfile.open("robotSimulOut.txt");
writeLocation();






#if CSC412_FP_USE_GUI
	//	Now we enter the main loop of the program and to a large extend
	//	"lose control" over its execution.  The callback functions that 
	//	we set up earlier will be called when the corresponding event
	//	occurs
	glutMainLoop();
#endif
	
	//	Free allocated resource before leaving (not absolutely needed, but
	//	just nicer.  Also, if you crash there, you know something is wrong
	//	in your code.
	// for (int i=0; i< numRows; i++)
	// 	delete []grid[i];
	// delete []grid;
	// for (int k=0; k<MAX_NUM_MESSAGES; k++)
	// 	free(message[k]);
	// free(message);

	
	//	This will probably never be executed (the exit point will be in one of the
	//	call back functions).
	return 0;
}

void printHorizontalBorder(void)
{
	myfile << "+-";
	for (int j=1; j<numCols; j++)
	{
		myfile << "--";
	}
	myfile << "-+" << endl;
}
void printGrid(void)
{
	//	some ugly, hard-codedd stuff
	static string doorStr[] = {"D0", "D1", "D2", "D3", "DD4", "D5", "D6", "D7", "D8", "D9"};
	static string robotStr[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9"};
	static string boxStr[] = {"b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "b9"};

	if (numDoors > 9 || numBoxes > 9)
	{
		cout << "This function only works for small numbers of doors and robots" << endl;
		exit(1);
	}

	//	I use sparse storage for my grid
	map<int, map<int, string> > grid;
	
	//	addd doors
	for (int k=0; k<numDoors; k++)
	{
		grid[doorLoc[k][0]][doorLoc[k][1]] = doorStr[k];
	}
	//	add boxes
	for (int k=0; k<numBoxes; k++)
	{
		grid[boxLoc[k][0]][boxLoc[k][1]] = boxStr[k];
		grid[robotLoc[k][0]][robotLoc[k][1]] = robotStr[k];
	}
	
	//	print top border
	printHorizontalBorder();
	
	for (int i=0; i<numRows; i++)
	{
		myfile << "|";
		for (int j=0; j<numCols; j++)
		{
			if (grid[i][j].length() > 0)
				myfile << grid[i][j];
			else {
				myfile << " .";
			}
		}
		myfile << "|" << endl;
	}
	//	print bottom border
	printHorizontalBorder();

	grid.clear();
}


//==================================================================================
//
//	This is a part that you have to edit and add to.
//
//==================================================================================

void robotFunc(int index)
{

	// info* boxinfo =  (info*) arg;
//  box to door row dis box to door col dis rob to box rowdis rob to box col dis
	int btd_row_distance, btd_col_distance;
// the new position that rob should move to  row and col

	int rtnew_row_poistion,rtnew_col_poistion;
	int direction;


	//	do planning (generate list of robot commands (move/push)
	


//     			door
//               ^
//               |
	//           <..........  box<--rot  push left, move robot down, push up
	if ((boxLoc[info_list[index].index][0] > info_list[index].door_row) && (boxLoc[info_list[index].index][1]>info_list[index].door_col)){
		btd_row_distance=boxLoc[info_list[index].index][0]-info_list[index].door_row;
		btd_col_distance=boxLoc[info_list[index].index][1]-info_list[index].door_col;
		rtnew_row_poistion=boxLoc[info_list[index].index][0];
		rtnew_col_poistion=boxLoc[info_list[index].index][1]+1;
		direction=0;
		// if the rob in the same row with the box and in the middle between door and box
		if((robotLoc[info_list[index].index][0] == boxLoc[info_list[index].index][0]) &&(robotLoc[info_list[index].index][1] < boxLoc[info_list[index].index][1])){
			robot_move_down(info_list[index].index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		// if not the special case(not crossing its own box: move as usual)
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		//push 
		robot_pushbox_left(info_list[index].index,btd_col_distance);
		// //adjust rob position to under the box
		robot_move_down(info_list[index].index,1);
		robot_move_left(info_list[index].index,1);
		// push up the box
		robot_pushbox_up(info_list[index].index,btd_row_distance);
		if ((info_list[index].door_col==boxLoc[info_list[index].index][1])&& (info_list[index].door_row==boxLoc[info_list[index].index][0])){
                info_list[index].is_running=false;
        }
		 
	}
//     			    door
//                   ^
//                   |
//  rot->box .........  push right, move robot down, push up

	else if ((boxLoc[info_list[index].index][0] > info_list[index].door_row )&& (boxLoc[info_list[index].index][1] < info_list[index].door_col)){
		btd_row_distance=boxLoc[info_list[index].index][0]-info_list[index].door_row;
		btd_col_distance=info_list[index].door_col-boxLoc[info_list[index].index][1];
		rtnew_row_poistion=boxLoc[info_list[index].index][0];
		rtnew_col_poistion=boxLoc[info_list[index].index][1]-1;
		direction=0;
		// if rot and box same row and its col between box and door
		if((robotLoc[info_list[index].index][0] == boxLoc[info_list[index].index][0]) &&(robotLoc[info_list[index].index][1] > boxLoc[info_list[index].index][1])){
			robot_move_down(info_list[index].index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		//if not the special case(not crossing its own box: move as usual)
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		robot_pushbox_right(info_list[index].index,btd_col_distance);
		//adjust rob position to under the box
		robot_move_down(info_list[index].index,1);
		robot_move_right(info_list[index].index,1);
		// //push up
		robot_pushbox_up(info_list[index].index,btd_row_distance);
		if ((info_list[index].door_col==boxLoc[info_list[index].index][1])&& (info_list[index].door_row==boxLoc[info_list[index].index][0])){
                info_list[index].is_running=false;
        }
		//push 
			
	}

//   rot-> box.......
//      			|
//          		V       
//                 door        push right,  rot move up, push down

	else if ((boxLoc[info_list[index].index][0] < info_list[index].door_row )&& (boxLoc[info_list[index].index][1] < info_list[index].door_col)){
		
		btd_row_distance=info_list[index].door_row-boxLoc[info_list[index].index][0];
		btd_col_distance=info_list[index].door_col-boxLoc[info_list[index].index][1];

		rtnew_row_poistion=boxLoc[info_list[index].index][0];
		rtnew_col_poistion=boxLoc[info_list[index].index][1]-1;
		direction=0;

		if((robotLoc[info_list[index].index][0] == boxLoc[info_list[index].index][0]) &&(robotLoc[info_list[index].index][1] > boxLoc[info_list[index].index][1])){
			robot_move_up(info_list[index].index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}

		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		
		robot_pushbox_right(info_list[index].index,btd_col_distance);	
		//adjust rob position to top the box 
		robot_move_up(info_list[index].index,1);
		robot_move_right(info_list[index].index,1);

		robot_pushbox_down(info_list[index].index,btd_row_distance);
		if ((info_list[index].door_col==boxLoc[info_list[index].index][1])&& (info_list[index].door_row==boxLoc[info_list[index].index][0])){
                info_list[index].is_running=false;
        }

	}

//                  ....... box <-rot
//      			|
//          		V       
//                 door        push left,  rot move up, push down

	else if ((boxLoc[info_list[index].index][0] < info_list[index].door_row )&& (boxLoc[info_list[index].index][1] > info_list[index].door_col)){
		
		btd_row_distance= info_list[index].door_row - boxLoc[info_list[index].index][0];

		btd_col_distance= boxLoc[info_list[index].index][1] - info_list[index].door_col;
 
		rtnew_row_poistion=boxLoc[info_list[index].index][0];
		rtnew_col_poistion=boxLoc[info_list[index].index][1]+1;
		direction=0;
		if((robotLoc[info_list[index].index][0] == boxLoc[info_list[index].index][0]) &&(robotLoc[info_list[index].index][1] < boxLoc[info_list[index].index][1])){
			robot_move_up(info_list[index].index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		
		robot_pushbox_left(info_list[index].index,btd_col_distance);
		// adjust rob position, to the top of the box
		// //move 
		// robot_move_to_destination(rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		robot_move_up(info_list[index].index,1);
		robot_move_left(info_list[index].index,1);
		//push
		robot_pushbox_down(info_list[index].index,btd_row_distance);
		if ((info_list[index].door_col==boxLoc[info_list[index].index][1])&& (info_list[index].door_row==boxLoc[info_list[index].index][0])){
                info_list[index].is_running=false;
        }

	}


	// same col , door over box, push up
	else if((boxLoc[info_list[index].index][1] == info_list[index].door_col)  && ( boxLoc[info_list[index].index][0]> info_list[index].door_row)){
		btd_col_distance=0;
		btd_row_distance=boxLoc[info_list[index].index][0]-info_list[index].door_row; // use later for push box
		rtnew_col_poistion=boxLoc[info_list[index].index][1];
		rtnew_row_poistion=boxLoc[info_list[index].index][0]+1;
		direction=1;
		if((robotLoc[info_list[index].index][1] == boxLoc[info_list[index].index][1]) &&(robotLoc[info_list[index].index][0] < boxLoc[info_list[index].index][0])){
			robot_move_left(info_list[index].index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		robot_pushbox_up(info_list[index].index,btd_row_distance);
		if ((info_list[index].door_col==boxLoc[info_list[index].index][1])&& (info_list[index].door_row==boxLoc[info_list[index].index][0])){
                info_list[index].is_running=false;
        }
	}
	//same col, box over door, push down
	else if((boxLoc[info_list[index].index][1] == info_list[index].door_col)  && ( boxLoc[info_list[index].index][0]< info_list[index].door_row)){
		btd_col_distance=0;
		btd_row_distance=info_list[index].door_row- boxLoc[info_list[index].index][0]; // use later for push box
		rtnew_col_poistion=boxLoc[info_list[index].index][1];
		rtnew_row_poistion=boxLoc[info_list[index].index][0]-1;
		direction=1;
		if((robotLoc[info_list[index].index][1] == boxLoc[info_list[index].index][1]) &&(robotLoc[info_list[index].index][0] > boxLoc[info_list[index].index][0])){
			robot_move_left(info_list[index].index,1);
			// robot_move_up(info_list[index].index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		robot_pushbox_down(info_list[index].index,btd_row_distance);
		if ((info_list[index].door_col==boxLoc[info_list[index].index][1])&& (info_list[index].door_row==boxLoc[info_list[index].index][0])){
                info_list[index].is_running=false;
        }
	}

    // same row,  rot->box --->door  push right
	else if((boxLoc[info_list[index].index][0] = info_list[index].door_row)  && ( boxLoc[info_list[index].index][1] < info_list[index].door_col)){
		btd_row_distance=0;
		// if ( boxLoc[info_list[index].index][1] < info_list[index].door_col){
		btd_col_distance=info_list[index].door_col-boxLoc[info_list[index].index][1];
		rtnew_row_poistion=boxLoc[info_list[index].index][0];
		rtnew_col_poistion=boxLoc[info_list[index].index][1]-1;
		direction=0;
		if((robotLoc[info_list[index].index][0] == boxLoc[info_list[index].index][0]) &&(robotLoc[info_list[index].index][1] > boxLoc[info_list[index].index][1])){
			robot_move_up(info_list[index].index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		robot_pushbox_right(info_list[index].index,btd_col_distance);
		if ((info_list[index].door_col==boxLoc[info_list[index].index][1])&& (info_list[index].door_row==boxLoc[info_list[index].index][0])){
                info_list[index].is_running=false;
        }


	}
	// same row,  door<-----box<-rot  push left
	else if ((boxLoc[info_list[index].index][0] = info_list[index].door_row)  && ( boxLoc[info_list[index].index][1] > info_list[index].door_col)){
		btd_row_distance=0;
		btd_col_distance=boxLoc[info_list[index].index][1]-info_list[index].door_col;
		rtnew_row_poistion=boxLoc[info_list[index].index][0];
		rtnew_col_poistion=boxLoc[info_list[index].index][1]+1;
		direction=0;
		if((robotLoc[info_list[index].index][0] == boxLoc[info_list[index].index][0]) &&(robotLoc[info_list[index].index][1] < boxLoc[info_list[index].index][1])){
			robot_move_up(info_list[index].index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		// compare the rob col location and the location it should move to 
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,info_list[index].index);
		}
		
		robot_pushbox_left(info_list[index].index,btd_col_distance);
		if ((info_list[index].door_col==boxLoc[info_list[index].index][1])&& (info_list[index].door_row==boxLoc[info_list[index].index][0])){
                info_list[index].is_running=false;
        }
	
	}
	myfile << "robot " << info_list[index].index <<" end"<<endl;

	if(info_list[index].index+1 == numBoxes){
		quit();
	}
	
	
}

void initializeApplication()
{

	
	//	Allocate the grid
	grid = new int*[numRows];
	for (int i=0; i<numRows; i++)
		grid[i] = new int [numCols];

//make the door index same as the robot and box
int new_doorLoc[numBoxes][2];
for (int i=0;i<numBoxes;i++){
	int index=doorAssign[i];
	for (int k=0;k<2;k++){
		new_doorLoc[i][k]=doorLoc[index][k];
	}
	
} 


info e;
//compare the distance between door and box, get the distance.
for (int i =0;i<numBoxes;i++){
	e.index=i;
	e.door_row=new_doorLoc[i][0];
	e.door_col=new_doorLoc[i][1];

	e.is_running = true;
	info_list.push_back(e);
}

	message = (char**) malloc(MAX_NUM_MESSAGES*sizeof(char*));
	for (int k=0; k<MAX_NUM_MESSAGES; k++){
		message[k] = (char*) malloc((MAX_LENGTH_MESSAGE+1)*sizeof(char));}


}


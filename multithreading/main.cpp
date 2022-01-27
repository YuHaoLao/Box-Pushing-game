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
#include <mutex> // Version 3 
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
int robotSleepTime = 200000;

//	An array of C-string where you can store things you want displayed
//	in the state pane to display (for debugging purposes?)
//	Dont change the dimensions as this may break the front end
const int MAX_NUM_MESSAGES = 8;
const int MAX_LENGTH_MESSAGE = 32;
char** message;


int** robotLoc;
int** boxLoc;
int* doorAssign;	//	door id assigned to each robot-box pair
int** doorLoc;

// int robotLoc[][2] = {{13,2},{11,10}};
// int boxLoc[][2] = {{4,2},{7,2}};
// int doorAssign[] = {0,0};    //    door id assigned to each robot-box pair
// int doorLoc[][2] = {{1, 2},{9,9}};

//mutex lock for write data
pthread_mutex_t text_mutex;


//for write data to file
ofstream myfile;


// line 82 to 87 are for the randam generate
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
			//if true which mean the location unique and assign to the door
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
//jyh
//    First, why are you using numDoors?  It should be numBoxes!!!
//    You are assigning a door to each box/robot.  Not a door to another door.
//    Furthermore, why -1?  The last element gets nothing assigned?
//    for(int i=0;i <numDoors-1;i++){
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
		// live means the robot is moving or not finishing pushing the boxes
		if(boxinfo[i].is_running){
			drawRobotAndBox(i, robotLoc[i][0], robotLoc[i][1], boxLoc[i][0], boxLoc[i][1], doorAssign[i]);
		}
	}

	for (int i=0; i<numDoors; i++)
	{
		drawDoor(i, doorLoc[i][0], doorLoc[i][1]);
	}
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
	sprintf(message[1], "VERSION 2 by Jason/Sean");
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


bool can_moveup(int temp_move){
	for (int i=0;i<numBoxes;i++){
		if (temp_move==boxLoc[i][0]){
			
			return true;
		}
		else{
			cout<<i<<endl;
			return false;
		}
	}
}
void robot_move_up(int index, int distance){
	
	for (int i =0;i<distance;i++){
		robotLoc[index][0]-=1;
		usleep(robotSleepTime);
		//jyh second time e-mial
		//	No more direct calls to display in multithreaded version
		//		displayGridPane();
		pthread_mutex_lock(&text_mutex);
		myfile<<"robot "<<index<<" "<<" move N"<<endl;
		pthread_mutex_unlock(&text_mutex);

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
		pthread_mutex_lock(&text_mutex);
		myfile<<"robot "<<index<<" "<<" move S"<<endl;
		pthread_mutex_unlock(&text_mutex);

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
		pthread_mutex_lock(&text_mutex);
		myfile<<"robot "<<index<<" "<<" move W"<<endl;
		pthread_mutex_unlock(&text_mutex);

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
		pthread_mutex_lock(&text_mutex);
		myfile<<"robot "<<index<<" "<<" move E"<<endl;
		pthread_mutex_unlock(&text_mutex);

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
		pthread_mutex_lock(&text_mutex);
		myfile<<"robot "<<index<<" "<<" push N"<<endl;
		pthread_mutex_unlock(&text_mutex);
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
		pthread_mutex_lock(&text_mutex);
		myfile<<"robot "<<index<<" "<<" push S"<<endl;
		pthread_mutex_unlock(&text_mutex);
		
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
		pthread_mutex_lock(&text_mutex);
		myfile<<"robot "<<index<<" "<<" push W"<<endl;
		pthread_mutex_unlock(&text_mutex);
		
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
		pthread_mutex_lock(&text_mutex);
		myfile<<"robot "<<index<<" "<<" push E"<<endl;
		pthread_mutex_unlock(&text_mutex);
		
	}
}

/**
 * @brief input the direction and postion col and row and its ID, that robot will move to the setup postion
 * 
 * @param direction direction only will be 0 and 1. 0 represents horizontal 1 represents vertical
 * @param rtnew_col_poistion 
 * @param rtnew_row_poistion 
 * @param index robot index 
 */
void robot_move_to_destination( int direction, int rtnew_col_poistion,int rtnew_row_poistion,int index){

	int rtb_col_distance; //robot to box col distance
	int rtb_row_distance; // robot to box row distance
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

//jyh
//	I mmoved the thread-joining code into this function, called
//	when the user hits the ESC key
void quit(void)
{

	for(int k =0; k<numBoxes;k++){
		void* useless;
		pthread_join(boxinfo[k].id, &useless);
	}
	myfile.close();
	exit(0);
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
	myfile<<"\n";
	for(int i=0;i<numBoxes;i++){
		myfile<<"Boxe "<<i<<" ("<<boxLoc[i][0]<<","<<boxLoc[i][1]<<")"<<endl;
	}
	myfile<<"\n";
	myfile<<"\n";
	for(int i=0;i<numBoxes;i++){
		myfile<<"Robot "<<i<<" ("<<robotLoc[i][0]<<","<<robotLoc[i][1]<<")"<<" Destination door "<<doorAssign[i]<<endl;
	}
	myfile<<"\n";
	myfile<<"\n";

}


//------------------------------------------------------------------------
//	You shouldn't have to change anything in the main function besides
//	the initialization of numRows, numCos, numDoors, numBoxes.
//------------------------------------------------------------------------
int main(int argc, char** argv)
{
	numCols = atoi(argv[1]);
	numRows = atoi(argv[2]);
	numBoxes = atoi(argv[3]);
	numDoors = atoi(argv[4]);
	
	pthread_mutex_init(&text_mutex, NULL);

	//door
	rowdist = uniform_int_distribution<int>(0,numRows-1);
	coldist = uniform_int_distribution<int>(0,numCols-1);

	//box 
	boxrowdist = uniform_int_distribution<int>(1,numRows-2);
	boxcoldist = uniform_int_distribution<int>(1,numCols-2);

	//doorasign
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


int new_doorLoc[numBoxes][2];
for (int i=0;i<numBoxes;i++){
	int index=doorAssign[i];
	for (int k=0;k<2;k++){
		new_doorLoc[i][k]=doorLoc[index][k];
	}
} 
info e;

for (int i =0;i<numBoxes;i++){
	e.index=i;
	e.door_row=new_doorLoc[i][0];
	e.door_col=new_doorLoc[i][1];
	info_list.push_back(e);
}

myfile.open("robotSimulOut.txt");
writeLocation();
initializeApplication();


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
	for (int i=0; i< numRows; i++)
		delete []grid[i];
	delete []grid;
	for (int k=0; k<MAX_NUM_MESSAGES; k++)
		free(message[k]);
	free(message);

	
	//	This will probably never be executed (the exit point will be in one of the
	//	call back functions).
	return 0;
}

void printHorizontalBorder(void)
{
	cout << "+-";
	for (int j=1; j<numCols; j++)
	{
		cout << "--";
	}
	cout << "-+" << endl;
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
		cout<< "|";
		for (int j=0; j<numCols; j++)
		{
			if (grid[i][j].length() > 0)
				cout << grid[i][j];
			else {
				cout << " .";
			}
		}
		cout << "|" << endl;
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

void* robotFunc(void* arg)
{
	// bool isAlive = true;
	info* boxinfo =  (info*) arg;
//   define varibale
	int btd_row_distance; // box to door row distance
	int btd_col_distance; // box to door col distance
	// int rtb_row_distance; // robot to box  row distance
	// int rtb_col_distance; // robot  to box col distance
	int rtnew_row_poistion; //the new row position that rob should move to
	int rtnew_col_poistion; //the new col position that rob should move to
	int direction; //if =1: move 1 step horizontal first , if =0: move 1 step vertical first.

//     			door
//               ^
//               |
	//           <..........  box<--rot  push left, move robot down, push up
	if ((boxLoc[boxinfo->index][0] > boxinfo->door_row) && (boxLoc[boxinfo->index][1]>boxinfo->door_col)){
		btd_row_distance=boxLoc[boxinfo->index][0]-boxinfo->door_row;
		btd_col_distance=boxLoc[boxinfo->index][1]-boxinfo->door_col;
		rtnew_row_poistion=boxLoc[boxinfo->index][0];
		rtnew_col_poistion=boxLoc[boxinfo->index][1]+1;
		direction=0;
		// if the case is special:
		if((robotLoc[boxinfo->index][0] == boxLoc[boxinfo->index][0]) &&(robotLoc[boxinfo->index][1] < boxLoc[boxinfo->index][1])){
			robot_move_down(boxinfo->index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		//push 
		robot_pushbox_left(boxinfo->index,btd_col_distance);
		// //adjust rob position to under the box
		robot_move_down(boxinfo->index,1);
		robot_move_left(boxinfo->index,1);
		// // //push up
		robot_pushbox_up(boxinfo->index,btd_row_distance);
		if ((boxinfo->door_col==boxLoc[boxinfo->index][1])&& (boxinfo->door_row==boxLoc[boxinfo->index][0])){
				boxinfo->is_running=false;
		}
		 
	}
//     			    door
//                   ^
//                   |
//  rot->box .........  push right, move robot down, push up

	else if ((boxLoc[boxinfo->index][0] > boxinfo->door_row )&& (boxLoc[boxinfo->index][1] < boxinfo->door_col)){
		btd_row_distance=boxLoc[boxinfo->index][0]-boxinfo->door_row;
		btd_col_distance=boxinfo->door_col - boxLoc[boxinfo->index][1];
		rtnew_row_poistion=boxLoc[boxinfo->index][0];
		rtnew_col_poistion=boxLoc[boxinfo->index][1]-1;
		direction=0;
		// if the case is special:
		if((robotLoc[boxinfo->index][0] == boxLoc[boxinfo->index][0]) &&(robotLoc[boxinfo->index][1] > boxLoc[boxinfo->index][1])){
			robot_move_down(boxinfo->index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		robot_pushbox_right(boxinfo->index,btd_col_distance);
		//adjust rob position to under the box
		robot_move_down(boxinfo->index,1);
		robot_move_right(boxinfo->index,1);

		// //push up
		robot_pushbox_up(boxinfo->index,btd_row_distance);
		//push 
		if ((boxinfo->door_col==boxLoc[boxinfo->index][1])&& (boxinfo->door_row==boxLoc[boxinfo->index][0])){
				boxinfo->is_running=false;
		}
			
	}

//   rot-> box.......
//      			|
//          		V       
//                 door        push right,  rot move up, push down

	else if ((boxLoc[boxinfo->index][0] < boxinfo->door_row )&& (boxLoc[boxinfo->index][1] < boxinfo->door_col)){
		
		btd_row_distance=boxinfo->door_row-boxLoc[boxinfo->index][0];
		btd_col_distance=boxinfo->door_col-boxLoc[boxinfo->index][1];

		rtnew_row_poistion=boxLoc[boxinfo->index][0];
		rtnew_col_poistion=boxLoc[boxinfo->index][1]-1;
		direction=0;
		// if the case is special:
		if((robotLoc[boxinfo->index][0] == boxLoc[boxinfo->index][0]) &&(robotLoc[boxinfo->index][1] > boxLoc[boxinfo->index][1])){
			robot_move_up(boxinfo->index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		robot_pushbox_right(boxinfo->index,btd_col_distance);	
		//adjust rob position to top the box 
		robot_move_up(boxinfo->index,1);
		robot_move_right(boxinfo->index,1);

		robot_pushbox_down(boxinfo->index,btd_row_distance);
		if ((boxinfo->door_col==boxLoc[boxinfo->index][1])&& (boxinfo->door_row==boxLoc[boxinfo->index][0])){
				boxinfo->is_running=false;
		}

	}

//                  ....... box <-rot
//      			|
//          		V       
//                 door        push left,  rot move up, push down

	else if ((boxLoc[boxinfo->index][0] < boxinfo->door_row )&& (boxLoc[boxinfo->index][1] > boxinfo->door_col)){
		
		btd_row_distance= boxinfo->door_row - boxLoc[boxinfo->index][0];

		btd_col_distance= boxLoc[boxinfo->index][1] - boxinfo->door_col;
 
		rtnew_row_poistion=boxLoc[boxinfo->index][0];
		rtnew_col_poistion=boxLoc[boxinfo->index][1]+1;
		direction=0;
		// if the case is special:
		if((robotLoc[boxinfo->index][0] == boxLoc[boxinfo->index][0]) &&(robotLoc[boxinfo->index][1] < boxLoc[boxinfo->index][1])){
			robot_move_up(boxinfo->index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		robot_pushbox_left(boxinfo->index,btd_col_distance);
		// adjust rob position, to the top of the box
		robot_move_up(boxinfo->index,1);
		robot_move_left(boxinfo->index,1);

		//push
		robot_pushbox_down(boxinfo->index,btd_row_distance);
		if ((boxinfo->door_col==boxLoc[boxinfo->index][1])&& (boxinfo->door_row==boxLoc[boxinfo->index][0])){
				boxinfo->is_running=false;
		}

	}


	// same col , door over box, push up
	else if((boxLoc[boxinfo->index][1] == boxinfo->door_col)  && ( boxLoc[boxinfo->index][0]> boxinfo->door_row)){
		btd_col_distance=0;
		btd_row_distance=boxLoc[boxinfo->index][0]-boxinfo->door_row; // use later for push box
		rtnew_col_poistion=boxLoc[boxinfo->index][1];
		rtnew_row_poistion=boxLoc[boxinfo->index][0]+1;
		direction=1;
		// if the case is special:
		if((robotLoc[boxinfo->index][1] == boxLoc[boxinfo->index][1]) &&(robotLoc[boxinfo->index][0] < boxLoc[boxinfo->index][0])){
			robot_move_left(boxinfo->index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		robot_pushbox_up(boxinfo->index,btd_row_distance);
		if ((boxinfo->door_col==boxLoc[boxinfo->index][1])&& (boxinfo->door_row==boxLoc[boxinfo->index][0])){
				boxinfo->is_running=false;
		}
	}
	//same col, box over door, push down
	else if((boxLoc[boxinfo->index][1] == boxinfo->door_col)  && ( boxLoc[boxinfo->index][0]< boxinfo->door_row)){
		btd_col_distance=0;
		btd_row_distance=boxinfo->door_row- boxLoc[boxinfo->index][0]; // use later for push box
		rtnew_col_poistion=boxLoc[boxinfo->index][1];
		rtnew_row_poistion=boxLoc[boxinfo->index][0]-1;
		direction=1;
		
		// if the case is special:
		if((robotLoc[boxinfo->index][1] == boxLoc[boxinfo->index][1]) &&(robotLoc[boxinfo->index][0] > boxLoc[boxinfo->index][0])){
			robot_move_left(boxinfo->index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		robot_pushbox_down(boxinfo->index,btd_row_distance);
		if ((boxinfo->door_col==boxLoc[boxinfo->index][1])&& (boxinfo->door_row==boxLoc[boxinfo->index][0])){
				boxinfo->is_running=false;
		}
	}

    // same row,  rot->box --->door  push right
	else if((boxLoc[boxinfo->index][0] = boxinfo->door_row)  && ( boxLoc[boxinfo->index][1] < boxinfo->door_col)){
		btd_row_distance=0;
		// if ( boxLoc[boxinfo->index][1] < boxinfo->door_col){
		btd_col_distance=boxinfo->door_col-boxLoc[boxinfo->index][1];
		rtnew_row_poistion=boxLoc[boxinfo->index][0];
		rtnew_col_poistion=boxLoc[boxinfo->index][1]-1;
		direction=0;
		
		// if the case is special:
		if((robotLoc[boxinfo->index][0] == boxLoc[boxinfo->index][0]) &&(robotLoc[boxinfo->index][1] > boxLoc[boxinfo->index][1])){
			robot_move_up(boxinfo->index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		robot_pushbox_right(boxinfo->index,btd_col_distance);
		if ((boxinfo->door_col==boxLoc[boxinfo->index][1])&& (boxinfo->door_row==boxLoc[boxinfo->index][0])){
				boxinfo->is_running=false;
		}


	}
	// same row,  door<-----box<-rot  push left
	else if ((boxLoc[boxinfo->index][0] = boxinfo->door_row)  && ( boxLoc[boxinfo->index][1] > boxinfo->door_col)){
		btd_row_distance=0;
		btd_col_distance=boxLoc[boxinfo->index][1]-boxinfo->door_col;
		rtnew_row_poistion=boxLoc[boxinfo->index][0];
		rtnew_col_poistion=boxLoc[boxinfo->index][1]+1;
		direction=0;
		// if the case is special:
		if((robotLoc[boxinfo->index][0] == boxLoc[boxinfo->index][0]) &&(robotLoc[boxinfo->index][1] < boxLoc[boxinfo->index][1])){
			robot_move_up(boxinfo->index,1);
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		else{
			robot_move_to_destination(direction,rtnew_col_poistion,rtnew_row_poistion,boxinfo->index);
		}
		robot_pushbox_left(boxinfo->index,btd_col_distance);
		if ((boxinfo->door_col==boxLoc[boxinfo->index][1])&& (boxinfo->door_row==boxLoc[boxinfo->index][0])){
				boxinfo->is_running=false;
		}
	}
	pthread_mutex_lock(&text_mutex);
	myfile <<"robot " <<boxinfo->index<<" end"<<endl;
	pthread_mutex_unlock(&text_mutex);
	return nullptr;
}

void initializeApplication()
{

	
	//	Allocate the grid
	grid = new int*[numRows];
	for (int i=0; i<numRows; i++)
		grid[i] = new int [numCols];


	int numThreads=numBoxes;
	boxinfo = (info*) calloc(numThreads, sizeof(info));

	//initialize 
	for(int k =0; k<numThreads;k++){
		boxinfo[k].index=k;
		boxinfo[k].door_row=info_list[k].door_row;
		boxinfo[k].door_col=info_list[k].door_col;
		boxinfo[k].is_running = true;
	}

	for(int k =0; k<numThreads;k++){
    // // //     /**every threads runs the code below.*/

		pthread_create(&boxinfo[k].id,nullptr,robotFunc,boxinfo+k);
	
	}
	//jyh
	//	I moved the thread joing code out of this function (otherwise
	//	your threads typically complete before the GUI is actually
	//	launched (by glutMainLoop())
	message = (char**) malloc(MAX_NUM_MESSAGES*sizeof(char*));
	for (int k=0; k<MAX_NUM_MESSAGES; k++){
		message[k] = (char*) malloc((MAX_LENGTH_MESSAGE+1)*sizeof(char));}


}


//
//  gl_frontEnd.h
//  GL threads
//
//  Created by Jean-Yves Hervé on 2018-12-08
//

#ifndef GL_FRONT_END_H
#define GL_FRONT_END_H
//
#include "guiChoice.h"
#if CSC412_FP_USE_GUI


//------------------------------------------------------------------------------
//	Find out whether we are on Linux or macOS (sorry, Windows people)
//	and load the OpenGL & glut headers.
//	For the macOS, lets us choose which glut to use
//------------------------------------------------------------------------------
#if (defined(__dest_os) && (__dest_os == __mac_os )) || \
	defined(__APPLE_CPP__) || defined(__APPLE_CC__)
	//	Either use the Apple-provided---but deprecated---glut
	//	or the third-party freeglut install
	#if 1
		#include <GLUT/GLUT.h>
	#else
		#include <GL/freeglut.h>
		#include <GL/gl.h>
	#endif
#elif defined(linux)
	#include <GL/glut.h>
#else
	#error unknown OS
#endif

#include "dataTypes.h"


//-----------------------------------------------------------------------------
//	Function prototypes
//-----------------------------------------------------------------------------

//	I don't want to impose how you store the information about your robots,
//	boxes and doors, so the two functions below will have to be called once for
//	each pair robot/box and once for each door.

//	This function draws together a robot and the box it is supposed to move
//	Since a robot corresponds to a box, they should have the same index in their
//	respective array, so only one id needs to be passed.
//	We also pass the number of the door assigned to the robot/box pair, so that
//	can display them all with a matching color
void drawRobotAndBox(int id,
					 int robotRow, int robotCol,
					 int boxRow, int boxCol,
					 int doorNumber);

//	This function assigns a color to the door based on its number
void drawDoor(int doorNumber, int doorRow, int doorCol);


void drawGrid(void);
void drawState(int numMessages, char** message);

void initializeFrontEnd(int argc, char** argv,
						void (*gridCB)(void), void (*stateCB)(void));
void quit(void);

/**
 * @brief function to maneuver robot to push box to door
 *
 * @param index robot index
 */



void robotFunc(int index);

#endif // GL_FRONT_END_H

#endif //	CSC412_FP_USE_GUI

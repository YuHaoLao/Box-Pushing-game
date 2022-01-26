//
//  dataTypes.h
//  GL threads
//
//  Created by Jean-Yves Hervé on 2021-12-07
//

#ifndef DATA_TYPES_H
#define DATA_TYPES_H


//	Travel direction data type
//	Note that if you define a variable
//	TravelDirection dir = whatever;
//	you get the opposite directions from dir as (NUM_TRAVEL_DIRECTIONS - dir)
//	you get left turn from dir as (dir + 1) % NUM_TRAVEL_DIRECTIONS
typedef enum Direction {
								NORTH = 0,
								WEST,
								SOUTH,
								EAST,
								//
								NUM_TRAVEL_DIRECTIONS
} Direction;




#endif //	DATA_TYPES_H

/*
* Programming Assignment 4: A Game of Chance "Craps"
* 
* Program Description: 
*		A player rolls two dice. Each die has six faces. These faces contain 1, 2, 3, 4, 5, and 6 
*		spots. After the dice have come to rest, the sum of the spots on the two upward 
*		faces is calculated. If the sum is 7 or 11 on the first throw, the player wins. If the 
*		sum is 2, 3, or 12 on the first throw (called "craps"), the player loses (i.e. the "house" 
*		wins). If the sum is 4, 5, 6, 8, 9, or 10 on the first throw, then the sum becomes the 
*		player's "point." To win, you must continue rolling the dice until you "make your 
*		point." The player loses by rolling a 7 before making the point.
* 
* Author: DonnerTech
* Date: October 10, 2025
*/

#define CRT_SECURE_NO_WARNINGS

#include "craps.h"

int main(void)
{
	// seed the random number generator
	srand((unsigned int)time(NULL));

	// run the craps game
	craps_game();

	double bankBalance = 0;


}
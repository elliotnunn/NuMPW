/*
	File:		Library.c

	Contains:	Trivial 68K library code for the "calling 68K code from PowerPC" demo

	Written by:	Richard Clark

	Copyright:	© 1994 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

				 2/15/94	RC		Released

	To Do:
*/


#include "Library.h"
#include <Quickdraw.h>

pascal short LibFunction (long a)
{
	Rect 	bounds = {15, 15, 85, 185};
	
	while (!EmptyRect(&bounds)) {
		InvertRect(&bounds);
		InsetRect(&bounds, a, a);
	}
	
	return a & 0x0000FFFF;
}

/*------------------------------------------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#
#	MultiFinder-Aware Simple Application Framework
#
#	CPlusAppLib
#
#	TApplicationCommon.h	-	C++ and Rez source
#
#	Copyright © 1989 Apple Computer, Inc.
#	All rights reserved.
#
#	Versions:	
#			1.10 					07/89
#			1.00 					04/89
#
#	Components:
#			TApplicationCommon.h	July 9, 1989
#			TApplication.h			July 9, 1989
#			TDocument.h				July 9, 1989
#			TApplication.cp			July 9, 1989
#			TDocument.cp			July 9, 1989
#			TApplication.r			July 9, 1989
#
#	CPlusAppLib is a rudimentary application framework
#	for C++. The applications CPlusShapesApp and CPlusTESample
#	are built using CPlusAppLib.
#
------------------------------------------------------------------------------*/

#ifndef TApplicationCommon_Defs
#define TApplicationCommon_Defs

/*
	These definitions are shared by Rez and C++. We use #define statements
	instead of constants in this file because Rez doesn't support constants.
 */

/* Some constants for resource ID's */
#define kErrStrings	128				/* error string list */
#define	rUserAlert	129				/* user error alert */

/* The following are indicies into our error STR# resource. */
#define	eWrongMachine			1
#define	eSmallSize				2

#endif TApplicationCommon_Defs
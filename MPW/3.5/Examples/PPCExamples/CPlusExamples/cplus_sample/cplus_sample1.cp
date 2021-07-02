//
//	cplus_sample1 - main body of simple C++ program
//
//
//	Copyright © 1993, Apple Computer, Inc.  All rights reserved.
//

#include "cplus_sample.h"  

TSampleClass	gSampleClass;			// static class declaration (implies static constructor)

QDGlobals		qd;

WindowPtr		gWindow;
Rect			gRect;

void main ()
{		
	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitCursor ();
	SetRect (&gRect, 100, 100, 400, 200);
	gWindow = NewWindow (NULL, &gRect, (ConstStr255Param)"\pcplus_sample", true, rDocProc,
	                     (WindowPtr)-1, false, 0);
	gSampleClass.showMessage (gWindow);
	gSampleClass.myPause();
}
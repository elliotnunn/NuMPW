/*
 *	c_sample.h - header for simple C program
 *
 *
 *	Copyright © 1993, 1994 Apple Computer, Inc.  All rights reserved.
 */

#include <Windows.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <string.h>

#ifdef powerc
   #define APP_NAME_STRING "\pc_sample ppc version."
#else
   #define APP_NAME_STRING "\pc_sample 68K version."
#endif

void showMessage (WindowPtr theWindow);
void myPause (void);
/*
    File: mUPControl.h
    
    Description:
        Scrolling Text User Pane (mUP) control support routines.
	
	Routines defined in this header file are implemented in the
	file mUPControl.c
	
	These routines allow you to create (or use an existing) user
	pane control as a scrolling edit text field.
	
	These routines use the refcon field inside of the user pane
	record for storage of interal variables.  You should not
	use the reference value field in the user pane control if you
	are calling these routines.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright © 2000-2001 Apple Computer, Inc., All Rights Reserved
*/

#ifndef __mUPCONTROL__
#define __mUPCONTROL__

#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#else
#include <Carbon.h>
#endif

/* part codes */

/* kmUPTextPart is the part code we return to indicate the user has clicked
	in the text area of our control */
#define kmUPTextPart 1

/* kmUPScrollPart is the part code we return to indicate the user has clicked
	in the scroll bar part of the control. */
#define kmUPScrollPart 2


/* routines for using existing user pane controls.
	These routines are useful for cases where you would like to use an
	existing user pane control in, say, a dialog window as a scrolling
	text edit field.*/
	
/* mUPOpenControl initializes a user pane control so it will be drawn
	and will behave as a scrolling text edit field inside of a window.
	This routine performs all of the initialization steps necessary,
	except it does not create the user pane control itself.  theControl
	should refer to a user pane control that you have either created
	yourself or extracted from a dialog's control heirarchy using
	the GetDialogItemAsControl routine.  */
OSStatus mUPOpenControl(ControlHandle theControl);

/* mUPCloseControl deallocates all of the structures allocated
	by mUPOpenControl.  */
OSStatus mUPCloseControl(ControlHandle theControl);



/* routines for creating new scrolling text user pane controls.
	These routines allow you to create new scrolling text
	user pane controls. */

/* mUPCreateControl creates a new user pane control and then it passes it
	to mUPOpenControl to initialize it as a scrolling text user pane control. */
OSStatus mUPCreateControl(WindowPtr theWindow, Rect *bounds, ControlHandle *theControl);

/* mUPDisposeControl calls mUPCloseControl and then it calls DisposeControl. */
OSStatus mUPDisposeControl(ControlHandle theControl);


/* Utility Routines */

	/* mUPSetText replaces the contents of the selection with the unicode
	text described by the text and count parameters:.
		text = pointer to unicode text buffer
		count = number of bytes in the buffer.  */
OSStatus mUPSetText(ControlHandle theControl, char* text, long count);

/* mUPGetText returns the current text data being displayed inside of
	the mUPControl.  When noErr is returned, *theText contain a new
	handle containing all of the Unicode text copied from the current
	selection.  It is the caller's responsibiliby to dispose of this handle. */
OSStatus mUPGetText(ControlHandle theControl, Handle *theText);


/* mUPSetSelection sets the text selection and autoscrolls the text view
	so either the cursor or the selction is in the view. */
void mUPSetSelection(ControlHandle theControl, long selStart, long selEnd);



/* IsmUPControl returns true if theControl is not NULL
	and theControl refers to a mUP Control.  */
Boolean IsmUPControl(ControlHandle theControl);



/* Edit commands for mUP Controls. */
enum {
	kmUPCut = 1,
	kmUPCopy = 2,
	kmUPPaste = 3,
	kmUPClear = 4
};


/* mUPDoEditCommand performs the editing command specified
	in the editCommand parameter.  The mUPControl's text
	and scroll bar are redrawn and updated as necessary. */
void mUPDoEditCommand(ControlHandle theControl, short editCommand);


#endif

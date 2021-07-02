/*
	File:		Windows.c

	Contains:	Code to handle our application's windows

	Written by:	Richard Clark

	Copyright:	© 1993 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

				 1/27/94	RC		Set the clip region before dragging a tool window
				 					(in case we have something which will redraw the window
									completely after a move.)
				 1/26/94	RC		Made DrawClippedGrowIcon publically accessable,
				 					added code to set the clipping region to omit the
									grow icon.
				  11/28/93	RC		First release

	To Do:
*/

#ifndef __TYPES__
	#include <Types.h>
#endif

#ifndef __QUICKDRAW__
	#include <Quickdraw.h>
#endif

#ifndef __QDOFFSCREEN__
	#include <QDOffscreen.h>
#endif

#ifndef __DESK__
	#include <Desk.h>
#endif

#ifndef __LOWMEM__
	#include <LowMem.h> /* For MemError() */
#endif

#ifndef __TEXTUTILS__
	#include <TextUtils.h>
#endif

#ifndef __DIALOGS__
	#include <Dialogs.h>
#endif


#include "ModApp.h"
#include "Prototypes.h"

void AlertUser (short messageCode, short errorNum)
// Tell the user that something went wrong...
{
	Str255		messageString;
	Str255		numberString;
	
	if (messageCode > 0)
		GetIndString(messageString, rErrorStrings, messageCode);
	else
	 messageString[0] = '\0';
	 
	if (errorNum != 0)
		NumToString(errorNum, numberString);
	else
		numberString[0] = '\0';
	 
	ParamText (messageString, numberString, (StringPtr)"\p", (StringPtr)"\p");
	
	StopAlert(kErrorDialog, NULL);
}


void DrawClippedGrowIcon (WindowPtr wp)
{
	/* Draw the grow icon without drawing the scroll bar lines */
	Rect		portRect = wp->portRect;
	RgnHandle	oldClip = NULL;
	Rect		newClip;

	SetPort(wp);
	oldClip = NewRgn();
	GetClip(oldClip);
	
	/* Draw *only* the grow icon */
	newClip = portRect;
	newClip.top = newClip.bottom - 15;
	newClip.left = newClip.right - 15;
	ClipRect(&newClip);
	DrawGrowIcon(wp);

	SetClip(oldClip);
	DisposeRgn(oldClip);
}


void NewDisplayWindow ()
{
	DrawingWindowPeek	newWindow;
	Ptr					storage;

	storage = NewPtrClear(sizeof(DrawingWindow));
	newWindow = (DrawingWindowPeek)GetNewCWindow(kDisplayWindow, storage, (WindowPtr)-1L);
	if (newWindow) {
		SetPort((WindowPtr)newWindow);
		ShowWindow((WindowPtr)newWindow); /* Has to be visible for "GetGlobalBounds" to work */
	}
	gCurrentWindow = FrontWindow();
} /* NewDisplayWindow */


void CloseAWindow (WindowPtr wp)
{
	int16		wKind;
	OSErr		err;
	
	if (wp) {
		wKind = ((WindowPeek)wp)->windowKind;
		if (wKind < 0)
			CloseDeskAcc(wKind);
		else if (wKind == userKind) {
			// Release the tool
			err = UnloadTool(wp);
			CloseWindow(wp);
			DisposHandle(((DrawingWindowPeek)wp)->currMenuBar);
			DisposPtr((Ptr)wp);
			UseMenuBar(NULL);
			AdjustMenus();
		}
		gCurrentWindow = FrontWindow();
	}
} /* CloseAWindow */


/*----------------------- Handle window update events ----------------------*/

void DoUpdate (WindowPtr wp)
{	
	SetPort(wp);	 					/* make update window active grafPort	*/
	BeginUpdate(wp);					/* visRgn temporarily = updateRgn		*/
	EraseRect(&wp->portRect);
	
	if (isUserWindow(wp)) {
		DrawingWindowPeek	aWindow = (DrawingWindowPeek)wp;

		SetPort(wp);
		if ((aWindow->connectionID == 0) && (aWindow->toolResource == NULL)) {
			// We don't have a tool loaded
			Str255	noToolMessage = "\pPlease select an item from the Modules menu";
			
			TextFace(0);
			TextSize(0);
			TextFont(1);
			MoveTo(2, 20);
			TextBox(&noToolMessage[1],noToolMessage[0],&wp->portRect,teJustCenter);
		} else if (aWindow->toolRoutines.toolUpdateProc) {
			CallToolUpdateProc(aWindow->toolRoutines.toolUpdateProc, wp);
		}
		DrawClippedGrowIcon(wp);
	}
	EndUpdate(wp);						/* restore normal visRgn of grafport	*/
} /* DoUpdate */


void DoActivate (EventRecord *theEvent)
{
	WindowPtr	wp = (WindowPtr)theEvent->message;
	Boolean		activeFlag = theEvent->modifiers & 1;
	
	SetPort(wp);
	if (isUserWindow(wp)) {
		DrawingWindowPeek	aWindow = (DrawingWindowPeek)wp;

		DrawClippedGrowIcon(wp);
		if (activeFlag)
			UseMenuBar(aWindow->currMenuBar);
		else
			UseMenuBar(NULL);
		if (aWindow->toolRoutines.toolWindowActivateProc)
			CallToolWindowActivateProc(aWindow->toolRoutines.toolWindowActivateProc, wp, activeFlag);
	}
	AdjustMenus();
} /* DoActivate */


void DoContentClick (WindowPtr wp, EventRecord *theEvent)
{

	if (wp != FrontWindow())
		SelectWindow(wp);
	else {
		SetPort(wp);
		if (isUserWindow(wp)) {
			DrawingWindowPeek	aWindow = (DrawingWindowPeek)wp;
			if (aWindow->toolRoutines.toolClickProc)
				CallToolWindowClickProc(aWindow->toolRoutines.toolClickProc, wp, theEvent);
		}
	}
} /* DoContentClick */


void DoGrowWindow(WindowPtr wp, EventRecord *theEvent)
{
	Rect				sizeRect;
	long				newSize;

	sizeRect = qd.screenBits.bounds;
	sizeRect.top = 100;
	sizeRect.left = 100;
	newSize = GrowWindow(wp, theEvent->where, &sizeRect);
	if (newSize != 0) {
		SizeWindow(wp, newSize & 0x0000FFFF, newSize >> 16, true);
		SetPort(wp);
		InvalRect(&wp->portRect);
		if (isUserWindow(wp)) {
			DrawingWindowPeek	aWindow = (DrawingWindowPeek)wp;
			
			if (aWindow->toolRoutines.toolWindowResizedProc)
				CallToolWindowResizedProc(aWindow->toolRoutines.toolWindowResizedProc, wp);
		}
	}
}


void DoDragWindow(WindowPtr wp, EventRecord *theEvent)
{
	Rect				dragRect;

	dragRect = qd.screenBits.bounds;
	dragRect.top = 40;
	DragWindow(wp, theEvent->where, &dragRect);
	if (isUserWindow(wp)) {
		DrawingWindowPeek	aWindow = (DrawingWindowPeek)wp;

		/* Update the gWorld in case the screen depth changed */
		if (aWindow->toolRoutines.toolWindowMovedProc) {
			SetPort(wp);
			ClipToContentArea(wp);
			CallToolWindowMovedProc(aWindow->toolRoutines.toolWindowMovedProc, wp);
			ResetClip(wp);
		}
	}
}


void DoZoomWindow(WindowPtr wp, EventRecord *theEvent, int16 windowPart)
{
	Point				globalPt = theEvent->where;

	SetPort(wp);
	if (TrackBox(wp, globalPt, windowPart)) {
		ZoomWindow(wp, windowPart, true);
		InvalRect(&wp->portRect);
		if (isUserWindow(wp)) {
			DrawingWindowPeek	aWindow = (DrawingWindowPeek)wp;

			if (aWindow->toolRoutines.toolWindowResizedProc)
				CallToolWindowResizedProc(aWindow->toolRoutines.toolWindowResizedProc, wp);
		}
	}
}


void DoIdleWindow (WindowPtr wp)
{
	
	if (isUserWindow(wp)) {
		DrawingWindowPeek	aWindow = (DrawingWindowPeek)wp;

		if (aWindow->toolRoutines.toolIdleProc) {
			SetPort(wp);
			ClipToContentArea(wp);
			CallToolIdleProc(aWindow->toolRoutines.toolIdleProc, wp);
			ResetClip(wp);
		}
	}
} /* DoIdleWindow */


void ClipToContentArea (WindowPtr wp)
// Set the window's clipping region to exclude the grow icon
{
	RgnHandle	newClip = NewRgn();
	Rect		growIconRect;
	RgnHandle	growIconRgn = NewRgn();
	
	if ((newClip != NULL) && (growIconRgn != NULL)) {
		SetPort(wp);
			growIconRect = wp->portRect;
			growIconRect.top = growIconRect.bottom - 15;
			growIconRect.left = growIconRect.right - 15;
			RectRgn(growIconRgn, &growIconRect);
			RectRgn(newClip, &wp->portRect);
			DiffRgn(newClip, growIconRgn, newClip);
			SetClip(newClip);
	}
	if (newClip)
		DisposeRgn(newClip);
	if (growIconRgn)
		DisposeRgn(growIconRgn);
}


void ResetClip (WindowPtr wp)
{
	Rect	maxClip = {-32767, -32767, 32767, 32767};
	
	SetPort(wp);
	ClipRect(&maxClip);
}

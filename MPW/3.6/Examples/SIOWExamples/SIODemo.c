/* SIODemo.c */
/* 			"SIOdemo by C. Keith Ray"					*/
/*			"Copyright 1996 Apple Computer, Inc."		*/

#include <Quickdraw.h>
#include <Fonts.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Gestalt.h>
#include <AppleEvents.h>
#include <Menus.h>
#include <Devices.h>
#include <Events.h>
#include <Sound.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>

#include "SIO.h"
#include "SIODemo.h"

// function prototypes ****************************************************************

pascal void sioDemoInit( int * mainArgc, char *** mainArgv );

pascal void sioDemoRead(	char 	* buffer, 
							SInt32 	nCharsDesired, 
							SInt32 	* nCharsUsed, 
							SInt16 	* eofFlag );

pascal void sioDemoWrite( SInt16 filenum, char * buffer, SInt32 nChars );

pascal void sioDemoExit( void );

pascal OSErr HandleOapp ( AEDescList* aevt, AEDescList* reply, long refCon );
pascal OSErr HandleQuit ( AEDescList* aevt, AEDescList* reply, long refCon );
pascal OSErr HandleOdoc ( AEDescList* aevt, AEDescList* reply, long refCon );
pascal OSErr HandlePdoc ( AEDescList* aevt, AEDescList* reply, long refCon );

OSErr InstallAppleEventHandlers ( void );
void EventLoop ( void );
void DoMouseDown ( EventRecord* theEvent );
void MenuDispatch ( SInt32 menuResult );

// global function pointers  **********************************************************

pascal void (*__sioInit)( int * mainArgc, char *** mainArgv ) = sioDemoInit;

pascal void (*__sioRead)(	char 	* buffer, 
							SInt32 	nCharsDesired, 
							SInt32 	* nCharsUsed, 
							SInt16 	* eofFlag ) = sioDemoRead;


pascal void (*__sioWrite)( SInt16 filenum, char * buffer, SInt32 nChars ) = sioDemoWrite;

pascal void (*__sioExit)( void ) = sioDemoExit;

// TRowContent  **********************************************************

typedef struct TRowContent
{
	char 	c[80]; // a row contains 80 chars
	short	cols;
	long	color; // pre-color-quickdraw color
} TRowContent;

static void TRowContent_ctor( TRowContent* that ) // contructor
{
	int i;

	that->cols = 0;
	that->color = blackColor;
	for ( i = 0; i < 80; i++ )
	{
		that->c[i] = 0;
	}
}

// TOutputWindow  **********************************************************

/*
	This is NOT a recommended way to display text -- in particular, 
	this does not do the right thing for non-roman text systems.
*/

typedef struct TOutputWindow
{
	WindowPtr	myWindow;
	short		row;
	short		charWidth;
	short		charHeight;
	FontInfo 	myFontInfo;

	TRowContent* content[24]; // pointers to 24 rows.
} TOutputWindow;

void TOutputWindow_ctor( short windRsrc, TOutputWindow* that );
void TOutputWindow_dtor( TOutputWindow* that );
void output( char * buffer, long nChars, long color, TOutputWindow* that );
void updateWindowContent( TOutputWindow* that );
void scrollContent( TOutputWindow* that );

void TOutputWindow_ctor( short windRsrc, TOutputWindow* that ) /* constructor */
{
	int i;
	int j;

	that->myWindow = GetNewWindow( windRsrc, NULL, (WindowPtr)(-1) );
	that->row = 0;
	
	j = sizeof(TRowContent);
	for ( i = 0; i < 24; i++ )
	{
		that->content[i] = (TRowContent*) calloc( 1, sizeof(TRowContent) );
		if ( that->content[i] != NULL )
		{
			TRowContent_ctor( that->content[i] );
		}
		else
		{
			DebugStr("\p failed to calloc that->content[i]");
		}
	}
	SetPort( that->myWindow );
	TextFont( kFontIDMonaco );
	TextSize( 9 );
	TextMode( srcCopy );
	GetFontInfo( &(that->myFontInfo) );

	that->charWidth = that->myFontInfo.widMax;
	that->charHeight = that->myFontInfo.ascent + that->myFontInfo.descent 
		+ that->myFontInfo.leading;

	SizeWindow( that->myWindow, that->charWidth * 80, 
		that->charHeight * 24, false );

	SelectWindow ( that->myWindow );
	ShowWindow ( that->myWindow );
}

void TOutputWindow_dtor( TOutputWindow* that ) /* destructor */
{
	int i;

	DisposeWindow( that->myWindow );

	for ( i = 0; i < 24; i++ )
	{
		free ( that->content[i] );
	}	
}

void output( char * buffer, long nChars, long color, TOutputWindow* that )
{
	long i;

	for ( i = 0; i < nChars; i++ )
	{
		if ( buffer[i] == '\n' )
		{
			that->row++;
			if ( that->row == 24 )
			{
				that->row--;
				scrollContent( that );
			}
		}
		else
		{
			if ( that->content[that->row]->color != color )
			{
				that->content[that->row]->color = color;
			}
			if ( isprint( buffer[i] ) )
			{
				that->content[that->row]->c[that->content[that->row]->cols] 
					= buffer[i];
			}
			else
			{
				that->content[that->row]->c[that->content[that->row]->cols ] = ' ';
			}
			that->content[that->row]->cols++;
			if ( that->content[that->row]->cols == 80 )
			{
				that->row++;
				if ( that->row == 24 )
				{
					that->row--;
					scrollContent( that );
				}
			}
		}
	}
	updateWindowContent( that );
}

void updateWindowContent( TOutputWindow* that )
{
	short	irow;
	short 	icol;
	short	charX;
	short	charY;

	GrafPtr	savePort;
	
	GetPort( &savePort );
	SetPort( that->myWindow );
	
	ClipRect( &(that->myWindow->portBits.bounds) );
	EraseRect( &(that->myWindow->portBits.bounds) );

	charY = that->myFontInfo.ascent;
	for ( irow = 0; irow <= that->row; irow++ )
	{
		charX = 0;
		ForeColor( that->content[irow]->color );
		for ( icol = 0; icol < that->content[irow]->cols; icol++ )
		{
			if ( that->content[irow]->c[icol] != 0 )
			{
				MoveTo( charX, charY );
				DrawChar( that->content[irow]->c[icol] );
			}
			charX += that->myFontInfo.widMax;
		}
		charY += that->myFontInfo.ascent + that->myFontInfo.descent 
			+ that->myFontInfo.leading;
	}
	// irow == row
	SetPort( savePort );
}

void scrollContent( TOutputWindow* that )
{
	int	i;

	free(  that->content[0] );
	for ( i = 1; i < 24; i++ )
	{
		that->content[i-1] = that->content[i];
	}
	that->content[23] = (TRowContent*) calloc( 1, sizeof(TRowContent) );
	if ( that->content[23] != NULL )
	{
		TRowContent_ctor( that->content[23] );
	}
	else
	{
		DebugStr("\p failed to calloc that->content[23]");
	}
}

// global vars  **********************************************************

enum
{
	kNothingDone = 0,
	kReadingDone = 1,
	kEndOfFileDone = 2,
	kQuitDone = 3
};

short	gActionTaken = kNothingDone;

TOutputWindow* 	gStdOutWindow = 0;
// TOutputWindow* 	gStdErrWindow = 0;

Boolean			gInBackground;

DialogPtr 		gMyReadDialog = 0;
DialogRecord 	gMyDialogRecord;

QDGlobals	qd;

// sioDemoInit  **********************************************************

pascal void sioDemoInit( int * mainArgc, char *** mainArgv )
{
	static char		progName[] = "SIODemo";
	static char		progArg[] = "arg1 and only 1";
	static char*	argv[3];

	*mainArgc = 2;
	argv[0] = progName;
	argv[1] = progArg;
	argv[2] = NULL;
	*mainArgv = argv;

	MaxApplZone();

	InitGraf ( &qd.thePort );
	InitFonts ( );
	InitWindows ( );
	InitMenus ( );
	TEInit ( );
	InitDialogs ( nil );
	InitCursor ( );
	
	FlushEvents ( everyEvent, 0 );

	SetMenuBar ( GetNewMBar ( kDemoMBAR ) );
	AppendResMenu ( GetMenuHandle ( kAppleMenu ), 'DRVR' );
	DrawMenuBar ( );
	
	InstallAppleEventHandlers();

	gStdOutWindow = (TOutputWindow*) calloc ( 1, sizeof(TOutputWindow) );
	if ( gStdOutWindow != NULL )
	{
		TOutputWindow_ctor( kStdOutWindow, gStdOutWindow );
	}
	else
	{
		DebugStr("\p failed to calloc gStdOutWindow");
	}
}

// sioDemoExit  **********************************************************
	
pascal void sioDemoExit( void )
{
	do{ 
		// if Quitting aborted the app, then gActionTaken still == kQuitDone
		// Otherwise, we loop until user quits.
		EventLoop();
	} while ( gActionTaken != kQuitDone );

	if ( gMyReadDialog != 0 )
	{
		CloseDialog( gMyReadDialog );
		gMyReadDialog = 0;
	}

	TOutputWindow_dtor( gStdOutWindow );
	free ( gStdOutWindow );
	gStdOutWindow = 0;

	__sioInit = 0;
	__sioRead = 0;
	__sioWrite = 0;
	__sioExit = 0;
}

// sioDemoRead  **********************************************************

pascal void sioDemoRead(	char 	* buffer, 
							long 	nCharsDesired, 
							long 	* nCharsUsed, 
							short 	* eofFlag )
{
	Boolean 	reallyStop = false;
	long 		i;

	gMyReadDialog = GetNewDialog( kInputWindowDLOG, &gMyDialogRecord, (WindowPtr) -1 );

continueLoop:

	gActionTaken = kNothingDone;
	do {
		EventLoop();
	} while ( gActionTaken == kNothingDone );

	if ( gActionTaken == kReadingDone )
	{
		short	iType;
		Handle	iHandle;
		Rect	iRect;
		long	handleSize;

		GetDialogItem ( gMyReadDialog, kInputWindowEditText, &iType, &iHandle, &iRect);

		HLock( iHandle );
		handleSize = GetHandleSize( iHandle );

		for ( i = 0; i < nCharsDesired-1 && i < handleSize; i++ )
		{
			buffer[i] = (*iHandle)[i];
		}
		buffer[i++] = '\n';
		*nCharsUsed = i;

		output( buffer, i, blueColor, gStdOutWindow );

		HUnlock( iHandle );
	}
	else if ( gActionTaken == kEndOfFileDone )
	{
		*eofFlag = true;
		*nCharsUsed = 0;
	}
	else if ( gActionTaken == kQuitDone )
	{
		short			itemHit;

		itemHit = StopAlert( kQuitAlert, 0 );
		if ( itemHit == kQuitQuit )
		{
			raise(SIGABRT); // does not return to sioDemoRead...
		}
		else
		{
			goto continueLoop;
		}
	}

	CloseDialog( gMyReadDialog );
	gMyReadDialog = 0;
}

// sioDemoWrite  **********************************************************

pascal void sioDemoWrite( SInt16 filenum, char * buffer, SInt32 nChars )
{
	if ( filenum == kSIOStdOutNum )
	{
		output( buffer, nChars, blackColor, gStdOutWindow );
	}
	else if ( filenum == kSIOStdErrNum )
	{
		output( buffer, nChars, redColor, gStdOutWindow );
	}
	else
	{
		DebugStr("\p filenum has bad value" );
	}

	gActionTaken = kNothingDone;
	EventLoop(); // get one event

	if ( gActionTaken == kQuitDone )
	{
		short			itemHit;

		itemHit = StopAlert( kQuitAlert, 0 );
		if ( itemHit == kQuitQuit )
		{
			raise(SIGABRT); // does not return to sioDemoRead...
		}
	}

}

// MenuDispatch  **********************************************************

void MenuDispatch ( SInt32 menuResult )
{
	SInt16 	theMenu = (menuResult >> 16);					// menu selected
	SInt16 	theItem = (menuResult & 0x0000FFFF);       		// item selected
	
	switch (theMenu)
	{
		case kAppleMenu: 
			if ( theItem == kAboutSIODemo )
			{
				(void) Alert( kAboutAlert, 0 );
			}
			else
			{
				Str255	name;
				GetMenuItemText ( GetMenuHandle ( kAppleMenu ), theItem, (StringPtr) name );
				OpenDeskAcc ( (StringPtr) &name );
			}
		break;
		
		case kFileMenu: 
			if ( theItem == kFileQuit )
			{
				gActionTaken = kQuitDone;
			}
		break;
		
		case kEditMenu:
			if ( gMyReadDialog != 0 )
			{
				if ( theItem == kEditCut )
				{
					DialogCut( gMyReadDialog );
				}
				else if ( theItem == kEditCopy )
				{
					DialogCopy( gMyReadDialog );
				}
				else if ( theItem == kEditPaste	)
				{
					DialogPaste( gMyReadDialog );
				}
			}//if
		break;
	}
	HiliteMenu ( 0 );	   		// un-hilite selected menu
	
	return;
}

// DoUpdate  **********************************************************

static void DoUpdate ( WindowRef theWindow )
{
	GrafPtr			savePort;
	CGrafPtr		thePort;

	thePort = GetWindowPort ( theWindow );
	
	GetPort ( &savePort );
	SetPortWindowPort ( theWindow );
	BeginUpdate ( theWindow );					// visRgn temporarily = updateRgn
	EraseRect ( &thePort->portRect );
	
	if ( theWindow == gStdOutWindow->myWindow )
	{
		updateWindowContent( gStdOutWindow );
	}
	
	EndUpdate ( theWindow );					// restore normal visRgn of grafport
	SetPort ( savePort );
	
	return;
}

// DoContentClick  **********************************************************

static void DoContentClick ( WindowRef theWindow, EventRecord* theEvent )
{
#pragma unused (theEvent)

	OSErr		theErr = noErr;
	WindowRef	frontWindow;
	
	// If a movable modal is active, ignore click in an inactive 
	// window, otherwise select it or handle the content click.
	
	frontWindow = FrontWindow ( );
	if ( theWindow != frontWindow )
	{
		if ( frontWindow == gMyReadDialog )
		{
			SysBeep ( 30 );
		}
		else
		{
			SelectWindow ( theWindow );
		}
	}
	return;
}

// DoDragWindow  **********************************************************

static void DoDragWindow ( WindowRef theWindow, EventRecord* theEvent )
{
	WindowRef	frontWindow;

	// If a movable modal is active, ignore click in an inactive 
	// title bar, otherwise let the Window Manager handle it.
	
	frontWindow = FrontWindow ( );
	if ( theWindow != frontWindow && frontWindow == gMyReadDialog )
	{
		SysBeep ( 30 );
	}
	else								
	{
		RgnHandle	theRgn;
		Rect		dragRect;
		
		theRgn = GetGrayRgn ( );
		dragRect = (*theRgn)->rgnBBox;
		DragWindow ( theWindow, theEvent->where, &dragRect );
	}
	
	return;
}

// EventLoop  **********************************************************

void EventLoop ( void )
{
	OSErr			theErr;
	EventRecord		theEvent;
	DialogPtr		whichDialog;

	WaitNextEvent ( everyEvent, &theEvent, 10, nil );	
	
	if ( IsDialogEvent( &theEvent ) )
	{
		if ( theEvent.what == keyDown 
			&& ( theEvent.modifiers & cmdKey ) != 0 )
		{
			char theKey = theEvent.message & charCodeMask;
			MenuDispatch ( MenuKey ( theKey ) );
		}
		else
		{
			short			itemHit;

			if ( DialogSelect( &theEvent, &whichDialog, &itemHit ) != false )
			{
				if ( whichDialog == gMyReadDialog )
				{
					if ( itemHit == kInputWindowAccept )
					{
						gActionTaken = kReadingDone;
					}
					else if ( itemHit == kInputWindowEndOfFile )
					{
						gActionTaken = kEndOfFileDone;
					}
					else if ( itemHit == kInputWindowStop )
					{
						gActionTaken = kQuitDone;
					}
				}
			}
		}
	}
	else
	{
		switch ( theEvent.what )
		{
			case mouseDown: 
				DoMouseDown ( &theEvent );
			break;
			
			case keyDown:
				if ( theEvent.modifiers & cmdKey )
				{
					char theKey = theEvent.message & charCodeMask;
					MenuDispatch ( MenuKey ( theKey ) );
				}
			break;
			
			case activateEvt: 
				// do nothing
			break;
			
			case updateEvt:
				DoUpdate ( (WindowRef) theEvent.message );
			break;
			
			case osEvt:
				if ( (theEvent.message >> 24) & suspendResumeMessage )	// suspend or resume
				{
					if ( (theEvent.message >> 24) & resumeFlag )
					{
						SetCursor ( &qd.arrow );
					}
				}
			break;
			
			case kHighLevelEvent:
				theErr = AEProcessAppleEvent ( &theEvent );
			break;
		}//switch
	}//else

	return;
	
} // EventLoop

// DoMouseDown  **********************************************************

void DoMouseDown ( EventRecord* theEvent )
{
	Point			globalPt = theEvent->where;
	SInt16			windowPart;
	WindowRef		theWindow;
	long			theMenu;
	
	windowPart = FindWindow ( globalPt, &theWindow );
	switch ( windowPart )
	{
		case inMenuBar: 
			theMenu = MenuSelect ( globalPt );
			MenuDispatch ( theMenu );
		break;
		
		case inSysWindow:
			// The SystemClick toolbox routine handles system events
			SystemClick ( theEvent, theWindow );
		break;
		
		case inGoAway:
		case inGrow:
		break;
		
		case inDrag:
			DoDragWindow ( theWindow, theEvent );
		break;

		case inContent:
			DoContentClick ( theWindow, theEvent );
		break;
	}
	
	return;
	
}

// InstallAppleEventHandlers  **********************************************************

OSErr InstallAppleEventHandlers ( void )
{
	OSErr	theErr;

	theErr = AEInstallEventHandler ( kCoreEventClass, kAEOpenApplication, 
		NewAEEventHandlerProc ( HandleOapp ), 0, false );

	if ( theErr )	
		goto CleanupAndBail;

	theErr = AEInstallEventHandler ( kCoreEventClass, kAEOpenDocuments, 
		NewAEEventHandlerProc ( HandleOdoc ), 0, false );

	if ( theErr )	
		goto CleanupAndBail;

	theErr = AEInstallEventHandler ( kCoreEventClass, kAEPrintDocuments, 
		NewAEEventHandlerProc ( HandlePdoc ), 0, false );

	if ( theErr )	
		goto CleanupAndBail;

	theErr = AEInstallEventHandler ( kCoreEventClass, kAEQuitApplication, 
		NewAEEventHandlerProc ( HandleQuit ), 0, false );

	if ( theErr )	
		goto CleanupAndBail;

CleanupAndBail:

	return theErr;
	
}

// HandleOapp  **********************************************************

pascal OSErr HandleOapp ( AEDescList* aevt, AEDescList* reply, long refCon )
{
#pragma unused(aevt, reply, refCon)
	return errAEEventNotHandled;
}

// HandleOdoc  **********************************************************

pascal OSErr HandleOdoc ( AEDescList* aevt, AEDescList* reply, long refCon )
{
#pragma unused(aevt, reply, refCon)
	return errAEEventNotHandled;
}

// HandlePdoc  **********************************************************

pascal OSErr HandlePdoc ( AEDescList* aevt, AEDescList* reply, long refCon )
{
#pragma unused(aevt, reply, refCon)
	return errAEEventNotHandled;
}

// HandleQuit  **********************************************************

pascal OSErr HandleQuit ( AEDescList* aevt, AEDescList* reply, long refCon )
{
#pragma unused(aevt, reply, refCon)

	gActionTaken = kQuitDone;
	
	return noErr;
}

// **********************************************************

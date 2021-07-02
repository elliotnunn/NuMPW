/*
	file RenderingWindow.c
	
	Description:
	This file contains the routines used to manage the windows displayed
	by the HTMLSample application.
	
	HTMLSample is an application illustrating how to use the new
	HTMLRenderingLib services found in Mac OS 9. HTMLRenderingLib
	is Apple's light-weight HTML rendering engine capable of
	displaying HTML files.

	by John Montbriand, 1999.

	Copyright: © 1999 by Apple Computer, Inc.
	all rights reserved.
	
	Disclaimer:
	You may incorporate this sample code into your applications without
	restriction, though the sample code has been provided "AS IS" and the
	responsibility for its operation is 100% yours.  However, what you are
	not permitted to do is to redistribute the source as "DSC Sample Code"
	after having made changes. If you're going to re-distribute the source,
	we require that you make it clear in the source that the code was
	descended from Apple Sample Code, but that you've made changes.
	
	Change History (most recent first):
	10/16/99 created by John Montbriand
*/

#include "RenderingWindow.h"

#include <HTMLRendering.h>
#include <Resources.h>
#include <StdDef.h>
#include <string.h>
#include <Sound.h>

#include "History.h"
#include "CIconButtons.h"
#include "SampleUtils.h"
#include "HTMLSample.h"


	/* resource id numbers for PICT resources used
	for drawing the fill area to the right of the buttons */
enum {
	kLeftBarPicture = 128,
	kCenterBarPicture = 129,
	kRightBarPicture = 130
};

	/* ASCII codes for the arrow keys on the keyboard */
enum {
	kLeftArrowKey = 28,
	kUpArrowKey = 30,
	kDownArrowKey = 29
};

	/* color icon button resources for the default
	navigation buttons. */
enum {
	kBackDefaultButton = 128,
	kHomeDefaultButton = 129,
	kForwardDefaultButton = 130
};

	/* constants used for adding custom buttons. */
enum {
	kExtraButtonsMax = 4,
	kExtraButtonsListID = 128
};

	/* the resource id for the main window's
	WIND resource. */
enum {
	kRenderingWindowID = 128
};




/* RWindowHandle's refer to a data structure created
	for each rendering window to store various state
	variables used for maintaining the window on the
	screen.  Most importantly, for this example, this
	is where a reference to the HTML rendering object
	is stored.  A handle to this structure is stored in
	the rendering window's refcon field. */
	
typedef struct RWindowStruct RWindowVars;
typedef RWindowVars **RWindowHandle;

struct RWindowStruct {
	RWindowHandle prev, next; /* list of open rendering windows */
	WindowPtr rwindow;	/* a pointer to the window */
	HRReference renderer;	/* a reference to the rendering object */
	HistoryDataHandle history;	/* history of visited links for this window */
	Boolean isActive;	/* true when this window is the frontmost window. */
	CIconButtonHandle bback, bhome, bforward;	/* the default buttons */
	short nExtraButtons; /* the number of extra buttons */
	CIconButtonHandle extraButtons[1]; /* handles to the extra buttons */
};


	/* these picture handles are used for drawing the fill area
	to the right of the buttons in the top of the window.  The
	resources these variables reference are read in when
	InitRenderingWindows is called. */
PicHandle gFBLeft = NULL, gFBCenter = NULL, gFBRight = NULL;


	/* a list of open rendering windows. */
RWindowHandle gRWFirst = NULL, gRWLast = NULL;


	/* the global history is used to store 'all' visited links for
	every window.  */
HistoryDataHandle gGlobalHistory = NULL;


	/* gRenderingWindowsOpen is set to true when InitRenderingWindows
		is successful. */
Boolean gRenderingWindowsOpen = false;


/* InitRenderingWindows is called to initialize the environment used by 
	routines defined in this file.  It should be called before any of the 
	other routines defined in this file are called. */
OSStatus InitRenderingWindows(void) {
		/* get the pictures for drawing the fill
		area to the right of the buttons from the
		resource file */
	gFBLeft = GetPicture(kLeftBarPicture);
	if (gFBLeft == NULL) return resNotFound;
	gFBCenter = GetPicture(kCenterBarPicture);
	if (gFBCenter == NULL) return resNotFound;
	gFBRight = GetPicture(kRightBarPicture);
	if (gFBRight == NULL) return resNotFound;
		/* initialize the list of opened windows */
	gRWFirst = gRWLast = NULL;
		/* allocate the global history */
	gGlobalHistory = NewHistory();
	if (gGlobalHistory == NULL) return memFullErr;
		/* set the open flag */
	gRenderingWindowsOpen = true;
		/* done */
	return noErr;
}


/* CloseRenderingWindows closes any open rendering windows and
	deallocates any structures allocated when InitRenderingWindows
	was called. */
OSStatus CloseRenderingWindows(void) {
	if (gRenderingWindowsOpen) {
			/* close any open rendering windows */
		while (gRWFirst != NULL)
			RWCloseWindow((**gRWFirst).rwindow);
			/* release the global history */
		DisposeHistory(gGlobalHistory);
			/* reset the open flag */
		gRenderingWindowsOpen = false;
	}
	return noErr;
}



/* MyHRURLToFSSpecProc is the URL to FSSpec mapping defined for rendering windows.
	it calls the HTMLRenderingLib HRUtilGetFSSpecFromURL routine to map the url
	to a FSSpec and then it checks to see if the file exists and if it's an application.
	If the file does not exist, it re-directs the URL to refer to the error page.  If the
	file exists and it's an application, it launches the application. */
static pascal OSStatus MyHRURLToFSSpecProc(const char *rootURL, const char *linkURL,
			FSSpec *fsspec, URLSourceType urlSourceType, void *refCon) {
	RWindowHandle rwv;
	FSSpec spec;
	OSStatus err;
	FInfo fndrInfo;
		/* set up locals */
	rwv = (RWindowHandle) refCon;
		/* as the rendering library to map the file
		for us. */
	err = HRUtilGetFSSpecFromURL(rootURL, linkURL, &spec);
	if (err != noErr) goto bail;
		/* check to see if the file exists. */
	err = FSpGetFInfo(&spec, &fndrInfo);
	if (err != noErr) goto bail;
		/* if the file is an application, launch it */
	if (fndrInfo.fdType == 'APPL') {
		LaunchParamBlockRec launchpb;
		BlockZero(&launchpb, 0);
		launchpb.launchBlockID = extendedBlock;
		launchpb.launchEPBLength = extendedBlockLen;
		launchpb.launchFileFlags = launchNoFileFlags;
		launchpb.launchControlFlags = launchContinue;
		launchpb.launchAppSpec = &spec;
		err = LaunchApplication(&launchpb);
			/* we return an error so the HTMLRenderingLib
			does not try to draw the file. */
		return fnfErr;
	}
		/* return the FSSpec */
	*fsspec = spec;
	return noErr;
bail:
		/* if an error occured and we're looking for a
		HTML source file, then we re-direct the URL to the
		error file. */
	if (urlSourceType == kHRLookingForHTMLSource) {
		Handle rootURL, errorLink;
			/* the link we have in our resources is application
			relative, so we need to get the application's URL first. */
		if (GetApplicationFolderURL(&rootURL) == noErr) {
				/* and then we pull the application relative link
				from the resource file. */
			errorLink = GetResource(kCStyleStringResourceType, kErrorPageURLString);
			if (errorLink != NULL) {
				HLock(errorLink);
				HLock(rootURL);
					/* after retrieving the URL and the link, we ask the
					rendering library to map them to a FSSpec. */
				if (HRUtilGetFSSpecFromURL(*rootURL, *errorLink, &spec) == noErr) {
						/* then, if the file exists, we return a FSSpec referring
						to the error file. */
					if (FSpGetFInfo(&spec, &fndrInfo) == noErr) {
						*fsspec = spec;
						err = noErr;
					}
				}
				HUnlock(errorLink);
			}
			DisposeHandle(rootURL);
		}
	}
	return err;
}


/* RedrawWindowButtons redraws the three default buttons in the top
	of the window.  the way they are drawn depends on what commands
	are available at the time when this routine is called. here we set the
	origin to 0,0, and the clip region  to the window's clip region.  We
	do so because when this routine is called the origin and clip region 
	may have been left in an unknown state by the rendering library.*/
static void RedrawWindowButtons(RWindowHandle rwv) {
	RgnHandle clipsave;
	Rect r;
		/* set the current grafport to the window's */
	SetPortWindowPort((**rwv).rwindow);
		/* set the origin before drawing.  it may be incorrect when we're called. */
	SetOrigin(0, 0); 
		/* save the clipping region going to links with anchors sets it incorrectly */
	clipsave = NewRgn(); 
	GetClip(clipsave);
	ClipRect(GetPortBounds(GetWindowPort((**rwv).rwindow), &r));
		/* draw the buttons */
	DrawCIconButton((**rwv).bback, (CanGoBack((**rwv).history) ? kCBup : kCBdisabled));
	DrawCIconButton((**rwv).bhome, (CanGoHome((**rwv).history) ? kCBup : kCBdisabled));
	DrawCIconButton((**rwv).bforward, (CanGoForward((**rwv).history) ? kCBup : kCBdisabled));
		/* restore the clipping region */
	SetClip(clipsave);
	DisposeRgn(clipsave);
}


/* MyNewURLProc is called by the rendering library whenever a new HTML file
	is brought into the display.  This routine will be called with the HTML file's
	URL. if addToHistory is true, then the file is added to the history of visited
	links.  */
static pascal OSStatus MyNewURLProc(const char *url, const char *targetFrame, 
								Boolean addToHistory, void *refCon) {
	RWindowHandle rwv;
	Str255 title;
	Point theSize;
		/* set up locals */
	rwv = (RWindowHandle) refCon;
		/* set up the drawing environment */
	SetPortWindowPort((**rwv).rwindow);
	SetOrigin(0, 0);
		/* set the window's title to the new file */
	if (HRGetTitle( (**rwv).renderer, title) == noErr)
		SetWTitle((**rwv).rwindow, title);
	else title[0] = 0;
		/* set the window's standard state rectangle to
		the 'best' size for displaying this page. If the user
		zooms the window, it will go to this size.  */
	if (HRGetRenderedImageSize((**rwv).renderer, &theSize) == noErr)
		SetWindowStandardStateSize((**rwv).rwindow, theSize.h + 16, theSize.v + 32 + 16);
		/* if add to history is true, then we add the URL to both
		the window's history and to the global history. */
	if (addToHistory) {
			/* add to the window's history */
		AddToHistory((**rwv).history, url, title);
			/* add tot the global history. */
		AddToHistory(gGlobalHistory, url, "\p");
			/* redraw the window's controls.  We do
			this here because their visual state depends
			on the state of the window's history. */
		RedrawWindowButtons(rwv);
	}
		/* done */
	return noErr;
}


/* MyHRWasURLVisitedProc is called by the HTML rendering library
	when it would like to determine if a link has been visited.  This
	information is used when drawing links. */
static pascal Boolean MyHRWasURLVisitedProc(const char *url, void *refCon) {
		/* here we return true if the link can be found in the
		global history of all visited links, not just the window's history. */
	return InHistory(gGlobalHistory, url);
}


/* URLContainsAnchor returns true if a URL contains
	an anchor.  i.e. it contains a # character. */
static Boolean URLContainsAnchor(char const *url) {
	char const *cp;
	for (cp = url; *cp != '\0'; cp++)
		if (*cp == '#')
			return true;
	return false;
}


/* RWGotoURL displays HTML file referred to by the url in the
	rendering window.  if addToHistory is true, then the window
	will be added to the window's history list. */
OSStatus RWGotoURL(WindowPtr rWin, char* url, Boolean addToHistory) {
	RWindowHandle rwv;
	OSStatus err;
	Point origin = {0, 0};
	Rect donedrawingbox;
		/* set up locals */
	rwv = (RWindowHandle) GetWRefCon(rWin);

		/* if the URL does not contain an anchor, then scroll
		the HTML view to the origin. */
	if ( ! URLContainsAnchor(url)) {
		err = HRScrollToLocation((**rwv).renderer, &origin);
		if (err != noErr) goto bail;
	}
		/* ask the rendering object to display the HTML page
		referred to by the URL.  note, we pass the addToHistory to
		this routine which in turn passes it to our MyNewURLProc
		routine. */
	err = HRGoToURL((**rwv).renderer, url, addToHistory, false);
	if (err != noErr) goto bail;
	
		/* draw the new page */
	err = HRDraw((**rwv).renderer, NULL);
	if (err != noErr) goto bail;
	
		/* if addToHistory is false, then we would not
		have re-drawn the buttons inside of our MyNewURLProc
		routine so we should draw them here. */
	if ( ! addToHistory)
		RedrawWindowButtons(rwv);
		
		/* we validate the areas that were drawn by this
		routine.  This is to avoid unnecessary redraws when
		the window is first opened.  When the window is opened,
		an update event will be posted for its entire contents.
		These calls prevent the areas we have just drawn from
		being re-drawn when the update event is processed. */
	SetRect(&donedrawingbox, 0, 0, 32*3, 32);
	ValidWindowRect(rWin, &donedrawingbox);
	GetPortBounds(GetWindowPort((**rwv).rwindow), &donedrawingbox);
	donedrawingbox.top += 33;
	ValidWindowRect(rWin, &donedrawingbox);

		/* done */
	return noErr;
bail:
	return err;
}


/* RWGotoAppRelLink displays HTML file referred to by the application
	relative link in the rendering window.  if addToHistory is true, then
	the window will be added to the window's history list. */
OSStatus RWGotoAppRelLink(WindowPtr rWin, char* linkstr, Boolean addToHistory) {

	OSStatus err;
	Handle rootURL, fullURL;
		/* set up our locals */
	rootURL = fullURL = NULL;
		/* get the URL for the application's folder */
	err = GetApplicationFolderURL(&rootURL);
	if (err != noErr) goto bail;
		/* allocate a handle for storing the full URL */
	fullURL = NewHandle(0);
	if (fullURL == NULL) { err = memFullErr; goto bail; }
		/* ask the HTML rendering library to combine
		the url and the link to make a complete URL */
	HLock(rootURL);
	err = HRUtilCreateFullURL(*rootURL, linkstr, fullURL);
	if (err != noErr) goto bail;
	HUnlock(rootURL);
		/* call the RWGotoURL to bring the HTML file into view. */
	MoveHHi(fullURL);
	HLock(fullURL);
	err = RWGotoURL(rWin, *fullURL, addToHistory);
	if (err != noErr) goto bail;
		/* clean up our locals */
	DisposeHandle(rootURL);
	DisposeHandle(fullURL);
		/* done */
	return noErr;
bail:
		/* on error, clean up and return the error. */
	if (rootURL != NULL) DisposeHandle(rootURL);
	if (fullURL != NULL) DisposeHandle(fullURL);
	return err;
}


/* GoBackCommand is called whenever the user chooses
	'go back' by either picking the menu item, clicking on
	the button, or using the left arrow key on the keyboard. */
static void GoBackCommand(RWindowHandle rwv) {
	char** theURL;
		/* get previous the URL from the window's history. */
	if (GoBack((**rwv).history, (Handle*) &theURL) == noErr) {
		MoveHHi((Handle) theURL);
		HLock((Handle) theURL);
			/* call the RWGotoURL routine to bring the
			HTML file referred to by the URL into view. */
		RWGotoURL((**rwv).rwindow, *theURL, false);

		DisposeHandle((Handle) theURL);
	}
}

/* GoHomeCommand is called whenever the user chooses
	'go home' by either picking the menu item, clicking on
	the button, or using the left arrow key on the keyboard. 
	The file opened by this command will be the first file
	displayed in the window when it was opened. */
static void GoHomeCommand(RWindowHandle rwv) {
	char** theURL;
		/* get first the URL from the window's history. */
	if (GoHome((**rwv).history, (Handle*) &theURL) == noErr) {
		MoveHHi((Handle) theURL);
		HLock((Handle) theURL);
			/* call the RWGotoURL routine to bring the
			HTML file referred to by the URL into view. */
		RWGotoURL((**rwv).rwindow, *theURL, false);

		DisposeHandle((Handle) theURL);
	}
}


/* GoForwardCommand is called whenever the user chooses
	'go forward' by either picking the menu item, clicking on
	the button, or using the left arrow key on the keyboard. */
static void GoForwardCommand(RWindowHandle rwv) {
	char** theURL;
		/* get next the URL from the window's history. */
	if (GoForward((**rwv).history, (Handle*) &theURL) == noErr) {
		MoveHHi((Handle) theURL);
		HLock((Handle) theURL);
			/* call the RWGotoURL routine to bring the
			HTML file referred to by the URL into view. */
		RWGotoURL((**rwv).rwindow, *theURL, false);

		DisposeHandle((Handle) theURL);
	}
}


/* RWGoToButtonPage is called whenever the user clicks on
	a custom button.  This routine attempts to load the
	application relative URL stored in the button's string
	data. */
static OSStatus RWGoToButtonPage(WindowPtr rWin, CIconButtonHandle theButton) {
	RWindowHandle rwv;
	OSStatus err;
	Handle linkURL;
		/* set up locals */
	linkURL = NULL;
	rwv = (RWindowHandle) GetWRefCon(rWin);	
		/* get the application relative link from the button */
	err = GetCIconButtonStringData(theButton, &linkURL);
	if (err != noErr) goto bail;
		/* call RWGotoAppRelLink to bring the HTML file
		referred to by the link into the display. */
	MoveHHi(linkURL);
	HLock(linkURL);
	err = RWGotoAppRelLink(rWin, *linkURL, true);
	if (err != noErr) goto bail;
		/* clean up */
	DisposeHandle(linkURL);
		/* done */
	return noErr;
bail:
	if (linkURL != NULL) DisposeHandle(linkURL);
	return err;
}



/* DrawFillBar draws the fill area to the right of the
	button controls in the top of the window. */
static OSStatus DrawFillBar(Rect *bounds) {
	Rect r;
		/* Draw the left side */
	r = *bounds;
	r.right = r.left + 8;
	DrawPicture(gFBLeft, &r);
		/* draw the center */
	r = *bounds;
	r.left += 8;
	r.right -= 8;
	DrawPicture(gFBCenter, &r);
		/* draw the right side */
	r = *bounds;
	r.left = r.right - 8;
	DrawPicture(gFBRight, &r);
		/* done */
	return noErr;
}



/* RWUpdate should be called in response to an update event.
	it calls BeginUpdate and EndUpdate redrawing the window's
	contents as necessary. */
void RWUpdate(WindowPtr rWin) {
	RWindowHandle rwv;
	Rect windowBounds, r;
	RgnHandle windowVisibleRgn;
	short i;
		/* set up locals */
	rwv = (RWindowHandle) GetWRefCon(rWin);
	
		/* set up the drawing environment */
	SetPortWindowPort(rWin);
	SetOrigin(0, 0);
	ClipRect(GetPortBounds(GetWindowPort(rWin), &windowBounds));
	GetPortVisibleRegion(GetWindowPort(rWin), (windowVisibleRgn = NewRgn()));
	BeginUpdate(rWin);
		
		/* draw the button controls and the fill area at the top of the window. */
	SetRect(&r, 32*(3+(**rwv).nExtraButtons), 0, windowBounds.right, 32);
	DrawFillBar(&r);
	RedrawWindowButtons(rwv);
	for (i=0; i<(**rwv).nExtraButtons; i++)
		DrawCIconButton((**rwv).extraButtons[i], kCBup);
		
		/* draw a one pixel border at the bottom of that area */
	MoveTo(0, 32);
	LineTo(windowBounds.right-1, 32);

		/* if the window is not active, then gray out everything we
		just drew.  */
	if ( ! (**rwv).isActive) {
		SetRect(&r, 0, 0, windowBounds.right, 33);
		GrayOutBox(&r);
	}
		
		/* draw the grow icon */
	DrawGrowIconWithoutScrollLines(rWin);
		
		/* draw the HTML image.  we do this last as the time it takes
		to complete may vary depending on the HTML content in the window.
		This way, the window's structure and controls look solid and they
		redraw in a consistent way. */
	HRDraw((**rwv).renderer, windowVisibleRgn);
	
		/* reset the drawing enviroment */
	SetOrigin(0, 0);
	EndUpdate(rWin);
	DisposeRgn(windowVisibleRgn);
}



/* RWActivate should be called in response to activate events.*/
void RWActivate(WindowPtr rWin, Boolean activate) {
	RWindowHandle rwv;
	Rect r, windowBounds;
		/* set up locals */
	rwv = (RWindowHandle) GetWRefCon(rWin);
		/* set the active flag in the window's variables */
	(**rwv).isActive = activate;
		/* call the rendering library to activate or
		deactivate the HTML object */
	if (activate)
		HRActivate((**rwv).renderer);
	else HRDeactivate((**rwv).renderer);
		/* post an update event so the grow icon
		and the controls are re-drawn to reflect
		the new active/inactive state. */
	SetPortWindowPort(rWin);
	GetPortBounds(GetWindowPort(rWin), &windowBounds);
	SetOrigin(0, 0);
	SetRect(&r, 0, 0, windowBounds.right, 33);
	InvalWindowRect(rWin, &r);
		/* ...and the grow box */
	SetRect(&r, 0, 0, 16, 16);
	OffsetRect(&r, windowBounds.right-16, windowBounds.bottom-16);
	InvalWindowRect(rWin, &r);
}


/* RWRecalculateSize should be called whenever the size of a rendering
	window changes.  This routine resizes and redraws the windows
	contents appropriately. */
void RWRecalculateSize(WindowPtr rWin) {
	RWindowHandle rwv;
	Rect r;
		/* set up locals */
	rwv = (RWindowHandle) GetWRefCon(rWin);
		/* set up the drawing environment */
	SetPortWindowPort(rWin);
	SetOrigin(0, 0);
		/* reset the html object's rendering area */
	GetPortBounds(GetWindowPort(rWin), &r);
	SetRect(&r, r.left, r.top+33, r.right+1, r.bottom+1);
	HRSetRenderingRect((**rwv).renderer, &r);
}


/* RWResetGotoMenu should be called before calling MenuKey or MenuSelect.  It
	enables the back, forward, and home menu commands depending on what
	commands are available and it rebuilds the history list at the bottom
	of the go to menu. */
void RWResetGotoMenu(WindowPtr rWin) {
	RWindowHandle rwv;
	MenuHandle goMenu;
		/* set up locals */
	rwv = (RWindowHandle) GetWRefCon(rWin);
	goMenu = GetMenuHandle(mGo);
		/* enable the back command */
	if (CanGoBack((**rwv).history))
		EnableMenuItem(goMenu, iBack);
	else DisableMenuItem(goMenu, iBack);
		/* enable the forward command */
	if (CanGoForward((**rwv).history))
		EnableMenuItem(goMenu, iForward);
	else DisableMenuItem(goMenu, iForward);
		/* enable the home command */
	if (CanGoHome((**rwv).history))
		EnableMenuItem(goMenu, iHome);
	else DisableMenuItem(goMenu, iHome);
		/* remove any items at the bottom of the menu */
	while (CountMenuItems(goMenu) >= iGoSep)
		DeleteMenuItem(goMenu, iGoSep);
		/* append this window's history to the menu */
	AppendHistoryToMenu((**rwv).history, goMenu);
}


/* RWHandleGotoMenu should be called when an item is chosen from the
	go to menu.  item is the number of the item that was chosen. */
void RWHandleGotoMenu(WindowPtr rWin, short item) {
	RWindowHandle rwv;
		/* set up locals */
	rwv = (RWindowHandle) GetWRefCon(rWin);
		/* err, this first part is pretty obvious...*/
	if (item == iBack) {
		GoBackCommand(rwv);
	} else if (item == iForward) {
		GoForwardCommand(rwv);
	} else if (item == iHome) {
		GoHomeCommand(rwv);
	} else if (item > iGoSep) {
		char** theURL;
			/* if one of the history items is chosen from the bottom of
			the menu, then we retrieve that item from the window's
			history and call HRGoToURL to display it. */
		if (GoToMenuItem((**rwv).history, (Handle*) &theURL, item - iGoSep) == noErr) {
			MoveHHi((Handle) theURL);
			HLock((Handle) theURL);
			HRGoToURL((**rwv).renderer, *theURL, false, true);
			SetPortWindowPort(rWin);
			RedrawWindowButtons(rwv);
			DisposeHandle((Handle) theURL);
		}
	}
}


/* RWHandleMouseDown should be called in response to mouse down
	events occuring inside of a rendering window.  This routine responds
	to mouse clicks in the controls at the top of the window. */
void RWHandleMouseDown(WindowPtr rWin, Point where) {
	RWindowHandle rwv;
	Rect r;
		/* set up locals */
	rwv = (RWindowHandle) GetWRefCon(rWin);
	SetPortWindowPort(rWin);
	SetOrigin(0, 0);
	ClipRect(GetPortBounds(GetWindowPort(rWin), &r));

		/* handle clicks in the standard buttons */
	if (TrackCIconButton((**rwv).bback, where)) {
		
		GoBackCommand(rwv);
		
	} else if (TrackCIconButton((**rwv).bhome, where)) {
	
		GoHomeCommand(rwv);
		
	} else if (TrackCIconButton((**rwv).bforward, where)) {
	
		GoForwardCommand(rwv);
		
	} else {
			/* if a user clicks in a custom button, then we
			go to the link specified in it's data */
		short i;
		for (i=0; i<(**rwv).nExtraButtons; i++) {
			if (TrackCIconButton((**rwv).extraButtons[i], where)) {
				OSStatus err;
				err = RWGoToButtonPage(rWin, (**rwv).extraButtons[i]);
				/* report errors here */
				DrawCIconButton((**rwv).extraButtons[i], kCBup);
				break;
			}
			
		}
	}
}


/* RWKeyDown should be called for keydown events when a rendering
	window is the frontmost window.  This routine maps the left, up,
	and right arrow keys to the back, home, and forward commands. */
void RWKeyDown(WindowPtr rWin, char theKey) {
	RWindowHandle rwv;
	Rect r;
	rwv = (RWindowHandle) GetWRefCon(rWin);
	SetPortWindowPort(rWin);
	SetOrigin(0, 0);
	ClipRect(GetPortBounds(GetWindowPort(rWin), &r));
	switch (theKey) {
		case kLeftArrowKey: /* left arrow key */
			if (CanGoBack((**rwv).history)) {
				DrawCIconButton((**rwv).bback, kCBdown);
				GoBackCommand(rwv);
			}
			break;
		case kUpArrowKey: /* up arrow key */
			if (CanGoHome((**rwv).history)) {
				DrawCIconButton((**rwv).bhome, kCBdown);
				GoHomeCommand(rwv);
			}
			break;
		case kDownArrowKey: /* right arrow key */
			if (CanGoForward((**rwv).history)) {
				DrawCIconButton((**rwv).bforward, kCBdown);
				GoForwardCommand(rwv);
			}
			break;
	}
}


/* RWOpen opens a new, empty rendering window.  If successful,
	then *rWindow will contain a pointer to a newly created window. */
OSStatus RWOpen(WindowPtr *rWindow) {
	OSStatus err;
	WindowPtr rwin;
	short i, nExtraButtons;
	HRReference rend; /* the window's rendering object */
	HistoryDataHandle hist; /* the window's history */
	RWindowHandle rwv; /* storage for the window's state variables */
	RBCLRsrcHandle extraButtonIDs; /* extra button id list */
	CIconButtonHandle backButton, homeButton, forwardButton;
	CIconButtonHandle extraButtons[kExtraButtonsMax];
		/* static globals */
	static HRNewURLUPP gNewURLUPP = NULL;
	static HRWasURLVisitedUPP gVisitedURLUPP = NULL;
	static HRURLToFSSpecUPP gTranslateURLUPP = NULL;
	static Point gPosition = { 0, 0};

		/* set up locals */
	rwin = NULL;
	rend = NULL;
	hist = NULL;
	rwv = NULL;
	backButton = homeButton = forwardButton = NULL;
	for (i=0; i<kExtraButtonsMax; i++) extraButtons[i] = NULL;
	nExtraButtons = 0;
	
		/* get the buttons */
	backButton = NewCIconButton(kBackDefaultButton);
	if (backButton == NULL) { err = resNotFound; goto bail; }
	homeButton = NewCIconButton(kHomeDefaultButton);
	if (homeButton == NULL) { err = resNotFound; goto bail; }
	forwardButton = NewCIconButton(kForwardDefaultButton);
	if (forwardButton == NULL) { err = resNotFound; goto bail; }
		/* get the extra buttons */
	extraButtonIDs = (RBCLRsrcHandle) GetResource(kIconButtonIDListType, kExtraButtonsListID);
	if (extraButtonIDs != NULL) {
		nExtraButtons = (**extraButtonIDs).n;
		if (nExtraButtons > kExtraButtonsMax) nExtraButtons = kExtraButtonsMax;
		for (i=0; i < nExtraButtons; i++) {
			extraButtons[i] = NewCIconButton((**extraButtonIDs).ids[i]);
			if (extraButtons[i] == NULL) { err = resNotFound; goto bail; }
			SetCIconButtonPosition(extraButtons[i], 32 * (3 + i), 0);
		}
	}

		/* allocate the window */
	rwin = GetNewCWindow(kRenderingWindowID, NULL, (WindowPtr)(-1));
	if (rwin == NULL) { err = resNotFound; goto bail; }
	SetPortWindowPort(rwin);
		/* position it on the screen */
	MoveWindow(rwin, gPosition.h + 20, gPosition.v + 50, true);
	gPosition.h = (gPosition.h + 10) % 300;
	gPosition.v = (gPosition.v + 32) % 200;
		/* set up the renderer */
	err = HRNewReference(&rend, kHRRendererHTML32Type, GetWindowPort(rwin));
	if (err != noErr) goto bail;
	err = HRSetGrowboxCutout(rend, true);
	if (err != noErr) goto bail;
		/* create the history */
	hist = NewHistory();
	if (hist == NULL) { err = memFullErr; goto bail; }
		/* allocate the storage and store a handle to it in the refcon */
	rwv = (RWindowHandle) NewHandleClear(offsetof(RWindowVars, extraButtons) + sizeof(CIconButtonHandle)*nExtraButtons);
	if (rwv == NULL) { err = memFullErr; goto bail; }
	(**rwv).rwindow = rwin;
	(**rwv).renderer = rend;
	(**rwv).history = hist;
	(**rwv).isActive = false;
	(**rwv).bback = backButton;
	(**rwv).bhome = homeButton;
	(**rwv).bforward = forwardButton;
	(**rwv).nExtraButtons = nExtraButtons;
	for (i=0; i<nExtraButtons; i++)
		(**rwv).extraButtons[i] = extraButtons[i];
	SetWRefCon(rwin, (long) rwv);
		/* set up the new links call back */
	if (gNewURLUPP == NULL) {
		gNewURLUPP = NewHRNewURLUPP(MyNewURLProc);
		if (gNewURLUPP == NULL) { err = memFullErr; goto bail; }
	}
	HRRegisterNewURLUPP( gNewURLUPP, rend, rwv);
		/* set up the visited links call back */
	if (gVisitedURLUPP == NULL) {
		gVisitedURLUPP = NewHRWasURLVisitedUPP(MyHRWasURLVisitedProc);
		if (gVisitedURLUPP == NULL) { err = memFullErr; goto bail; }
	}
	HRRegisterWasURLVisitedUPP( gVisitedURLUPP, rend, rwv);
		/* set up the link translation call back */
	if (gTranslateURLUPP == NULL) {
		gTranslateURLUPP = NewHRURLToFSSpecUPP(MyHRURLToFSSpecProc);
		if (gTranslateURLUPP == NULL) { err = memFullErr; goto bail; }
	}
	HRRegisterURLToFSSpecUPP( gTranslateURLUPP, rend, rwv);
		/* add the window to the list of open windows */
	if (gRWFirst == NULL) {
		gRWFirst = gRWLast = rwv;
	} else {
		(**rwv).next = gRWFirst;
		(**gRWFirst).prev = rwv;
		gRWFirst = rwv;
	}
		/* recalculate the window's coordinates and show it */
	RWRecalculateSize(rwin);
	ShowWindow(rwin);
	*rWindow = rwin;
	return noErr;

bail:
		/* if an error occurs, we recover and return the error code */
	if (hist != NULL) DisposeHistory(hist);
	if (rend != NULL) HRDisposeReference(rend);
	if (rwin != NULL) DisposeWindow(rwin);
	if (rwv != NULL) DisposeHandle((Handle) rwv);
	if (backButton != NULL) DisposeCIconButton(backButton);
	if (homeButton != NULL) DisposeCIconButton(homeButton);
	if (forwardButton != NULL) DisposeCIconButton(forwardButton);
	for (i=0; i < nExtraButtons; i++) {
		if (extraButtons[i] != NULL) DisposeCIconButton(extraButtons[i]);
	}
	return err;
}


/* RWCloseWindow closes the rendering window pointed to by
	rWin.  */
void RWCloseWindow(WindowPtr rWin) {
	RWindowHandle rwv;
	short i;
		/* set up locals */
	rwv = (RWindowHandle) GetWRefCon(rWin);
		/* delete the rendering object */
	HRDisposeReference((**rwv).renderer);
		/* delete the history */
	DisposeHistory((**rwv).history);
		/* deallocate the window */
	DisposeWindow((**rwv).rwindow);
		/* delete the icons */
	DisposeCIconButton((**rwv).bback);
	DisposeCIconButton((**rwv).bhome);
	DisposeCIconButton((**rwv).bforward);
	for (i=0; i < (**rwv).nExtraButtons; i++)
		DisposeCIconButton((**rwv).extraButtons[i]);
		/* remove from the rendering window list */
	if ((**rwv).next != NULL)
		(**(**rwv).next).prev = (**rwv).prev;
	else gRWLast = (*rwv)->prev;
	if ((**rwv).prev != NULL)
		(**(**rwv).prev).next = (**rwv).next;
	else gRWFirst = (**rwv).next;
		/* delete storage */
	DisposeHandle((Handle) rwv);
}


/* IsARenderingWindow returns true if rWin points to a rendering
	window created by RWOpen. You should not call any of the routines
	below for windows that are not rendering windows.  This routine
	provides a convenient way to tell if a windowptr returned by one
	of the toolbox routines is a rendering window. */
Boolean IsARenderingWindow(WindowPtr rWin) {
	RWindowHandle rover;
		/* a null pointer cannot refer to a rendering window */
	if (rWin == NULL) return false;
		/* search the list of windows and see if it's there */
	for (rover = gRWFirst; rover != NULL; rover = (**rover).next)
		if (rWin == (**rover).rwindow)
			return true;
		/* if it's not there, return false. */	
	return false;
}
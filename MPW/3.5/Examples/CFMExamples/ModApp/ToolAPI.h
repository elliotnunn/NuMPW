/*
	File:		ToolAPI.h

	Contains:	Interface to external tools

	Written by:	Richard Clark

	Copyright:	© 1993-1994 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

			8/17/94		bls		Extensive changes for CFM-68K.  Removed support
			 					for emulated 68K apps on PowerPC.  Changed tools
								to export ProcPtrs.  Only go through MixedMode
								when absolutely needed.
								
	To Do:
*/

#ifndef __TOOLAPI__
#define __TOOLAPI__

#ifndef __TYPES__
	#include <Types.h>
#endif

#ifndef __WINDOWS__
	#include <Windows.h>
#endif

#ifndef __EVENTS__
	#include <Events.h>
#endif

#ifndef __MIXEDMODE__
	#include <MixedMode.h>
#endif

// === Pointer definitions for our callback routines
typedef OSErr (*ToolStartupProcPtr)(WindowPtr wp);
typedef void (*ToolShutdownProcPtr)(WindowPtr wp);
typedef void (*ToolMenuAdjustProcPtr)(WindowPtr wp);
typedef void (*ToolMenuDispatchProcPtr)(WindowPtr wp, short menuID, short itemID);
typedef void (*ToolIdleProcPtr)(WindowPtr wp);
typedef void (*ToolUpdateProcPtr)(WindowPtr wp);
typedef void (*ToolWindowClickProcPtr)(WindowPtr wp, EventRecord *theEvent);
typedef void (*ToolWindowKeyProcPtr)(WindowPtr wp, EventRecord *theEvent);
typedef void (*ToolWindowMovedProcPtr)(WindowPtr wp);
typedef void (*ToolWindowResizedProcPtr)(WindowPtr wp);
typedef void (*ToolWindowActivateProcPtr)(WindowPtr wp, Boolean activeFlag);



// === Prototypes for invoking a tool
extern OSErr CallToolStartupProc (ToolStartupProcPtr proc, WindowPtr wp);
extern void  CallToolShutdownProc (ToolShutdownProcPtr proc, WindowPtr wp);
extern void  CallToolMenuAdjustProc (ToolMenuAdjustProcPtr proc, WindowPtr wp);
extern void  CallToolMenuDispatchProc (ToolMenuDispatchProcPtr proc, WindowPtr wp, short menuID, short itemID);
extern void  CallToolIdleProc (ToolIdleProcPtr proc, WindowPtr wp);
extern void  CallToolUpdateProc (ToolUpdateProcPtr proc, WindowPtr wp);
extern void  CallToolWindowClickProc (ToolWindowClickProcPtr proc, WindowPtr wp, EventRecord *theEvent);
extern void  CallToolWindowKeyProc(ToolWindowClickProcPtr proc, WindowPtr wp, EventRecord *theEvent);
extern void  CallToolWindowMovedProc(ToolWindowMovedProcPtr proc, WindowPtr wp);
extern void  CallToolWindowResizedProc(ToolWindowResizedProcPtr proc, WindowPtr wp);
extern void  CallToolWindowActivateProc(ToolWindowActivateProcPtr proc, WindowPtr wp, Boolean activeFlag);


// === The "tool information block"
#ifdef powerc
	#pragma options align=mac68k
#endif
struct ToolInfoBlock {
//	long					    toolMenuID;				// Unique ID assigned to our menu
	ToolShutdownProcPtr			shutdownProc;			// Routine to call when tool is unloaded
	ToolMenuAdjustProcPtr		menuAdjustProc;			// Routine to call to adjust tool's menus
	ToolMenuDispatchProcPtr		menuDispatchProc;		// Routine to call when tool's menu is selected
	ToolIdleProcPtr				toolIdleProc;			// Routine to call when tool's image is idled
	ToolUpdateProcPtr			toolUpdateProc;			// Routine to call when tool's window needs updating
	ToolWindowKeyProcPtr		toolKeyProc;			// Routine to call when tool's window is clicked
	ToolWindowClickProcPtr		toolClickProc;			// Routine to call when tool's window is foremost & typing occurs
	ToolWindowMovedProcPtr		toolWindowMovedProc;	// Routine to call when tool's window has been moved
	ToolWindowResizedProcPtr	toolWindowResizedProc;	// Routine to call when tool's window has been resized
	ToolWindowActivateProcPtr	toolWindowActivateProc;	// Routine to call when tool's window has been activated or deactivated
};
#ifdef powerc
	#pragma options align=reset
#endif

typedef struct ToolInfoBlock ToolInfoBlock, *ToolInfoPtr;

// === Prototypes for our tools
OSErr ToolStartup (WindowPtr wp);
void ToolShutdown (WindowPtr wp);
void ToolMenuAdjust (WindowPtr wp);
void ToolMenuDispatch (WindowPtr wp, short menuID, short itemID);
void ToolIdle (WindowPtr wp);
void ToolUpdate (WindowPtr wp);
void ToolWindowKey(WindowPtr wp, EventRecord *theEvent);
void ToolWindowClick(WindowPtr wp, EventRecord *theEvent);
void ToolWindowMoved(WindowPtr wp);
void ToolWindowResized(WindowPtr wp);
void ToolWindowActivate(WindowPtr wp, Boolean activeFlag);

#endif // __TOOLAPI__
/*
	File:		BaseWindow.h

	Contains:	Base class for a window.

	Version:	Appearance 1.0 SDK

	Copyright:	© 1997-1998 by Apple Computer, Inc., all rights reserved.

	File Ownership:

		DRI:				Edward Voas

		Other Contact:		7 of 9, Borg Collective

		Technology:			OS Technologies Group

	Writers:

		(MAA)	Matt Ackeret
		(edv)	Ed Voas

	Change History (most recent first):

		 <5>	  3/2/98	MAA		Change types.h to MacTypes.h
		 <4>	 1/22/98	MAA		Add DoDragClick
		 <3>	10/28/97	edv		Use RadioGroup control!
		 <2>	 10/8/97	MAA		Add GetMyMenu
		 <1>	 9/11/97	edv		First checked in.
*/

#pragma once

#ifdef __MRC__
#include <MacTypes.h>
#include <Events.h>
#include <Menus.h>
#endif	// __MRC__

enum
{
	kBaseWindowKind		= 2000
};

class BaseWindow
{
	public:
			BaseWindow();
			BaseWindow( SInt16 resID );
		virtual ~BaseWindow();
		
		virtual void		Activate( EventRecord& event );
		virtual void		Deactivate( EventRecord& event );
		virtual void		Update( EventRecord& event );
		virtual void		Draw();
		virtual void		Resize( SInt16 width, SInt16 height );
		virtual void		HandleClick( EventRecord& event );
		virtual void		AdjustMenus();
		virtual void		HandleMenuSelection( SInt16 menuID, SInt16 itemNo );
		virtual void		Idle();
		virtual void		HandleKeyDown( EventRecord& event );
		virtual void 		DoDragClick(EventRecord *event);
	protected:
		virtual MenuHandle	GetMyMenu();
		
		WindowPtr			fWindow;
		CGrafPtr			fPort;
};
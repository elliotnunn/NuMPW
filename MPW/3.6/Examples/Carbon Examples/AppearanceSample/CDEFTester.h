/*
	File:		CDEFTester.h

	Contains:	Code to demonstrate all types of controls.

	Version:    CarbonLib 1.0.2 SDK

	You may incorporate this sample code into your applications without
	restriction, though the sample code has been provided "AS IS" and the
	responsibility for its operation is 100% yours.  However, what you are
	not permitted to do is to redistribute the source as "Apple Sample Code"
	after having made changes. If you're going to re-distribute the source,
	we require that you make it clear in the source that the code was
	descended from Apple Sample Code, but that you've made changes.

	© 1997-2000 by Apple Computer, Inc. All rights reserved.
*/

#ifndef _CDEFTESTER_H
#define _CDEFTESTER_H

#ifdef __MRC__
#include <Controls.h>
#endif	// __MRC__
#include "BaseWindow.h"

enum
{
	kMenuFonts				= 143
};


class CDEFTester : public BaseWindow
{
	public:
			CDEFTester();
			CDEFTester( SInt16 resID );
		virtual ~CDEFTester();
		
		virtual void		Activate( EventRecord& event );
		virtual void		Deactivate( EventRecord& event );
		virtual void		Draw(void);
		virtual void		HandleClick( EventRecord& event );
		virtual void		HandleMenuSelection( SInt16 menuID, SInt16 itemNo );
		virtual void		Idle(void);
		virtual void		HandleKeyDown( EventRecord& event );
		virtual	MenuHandle	GetMyMenu(void);
		
	private:
		void				DisplayPartCode( SInt16 part );
		void				CenterControlInWindow(void);
		void				SetValue(void);
		void				ChangeControlFont(SInt16 menuID, SInt16 itemNo);
		
		static	MenuHandle	fMenu;
		ControlHandle		fControl;
		Rect				fPartRect;
		Boolean 			fIsTab;
		
		ControlHandle		fDisableButton;
		ControlHandle		fSetValueButton;	
};

#endif // _CDEFTESTER_H
/*
	File:		SliderPane.h

	Contains:	Class to drive our example slider pane in MegaDialog.

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

#pragma once

#include <ControlDefinitions.h>
#include "MegaPane.h"
#include "Appearance.h"

class SliderPane : public MegaPane
{
	public:
			SliderPane( DialogPtr dialog, SInt16 items );
		virtual ~SliderPane();
		
		virtual void		ItemHit( SInt16 item );
	
	private:
		static pascal void	SliderFeedbackProc( ControlHandle control, SInt16 part );
		static pascal void	DrawPictureProc( ControlHandle control, SInt16 part );

		static ControlActionUPP	fSliderProc;
		static ControlUserPaneDrawUPP fDrawProc;
		
		ControlHandle		fVerticalSlider1;
		ControlHandle		fVerticalSlider2;
		ControlHandle		fVerticalSlider3;
		ControlHandle		fHorizontalSlider1;
		ControlHandle		fHorizontalSlider2;
		ControlHandle		fHorizontalSlider3;
		ControlHandle		fUserPane;
		
		PicHandle			fPicture;
		SInt16				fPictHeight;
		SInt16				fPictWidth;
};
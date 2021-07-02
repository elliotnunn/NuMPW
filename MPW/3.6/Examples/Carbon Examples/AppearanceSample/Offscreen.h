/*
	File:		Offscreen.h

	Contains:	Class to help with offscreen drawing.

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

#include <QDOffscreen.h>

#define PtoCstr		p2cstr
#define CtoPstr		c2pstr
#define PtoCString	p2cstr
#define CtoPString	c2pstr

#define topLeft(r)	(((Point *) &(r))[0])
#define botRight(r)	(((Point *) &(r))[1])

class Offscreen
{
	public:
			Offscreen();
		virtual ~Offscreen();
	
		virtual void		StartDrawing( const Rect& bounds, Boolean copyDest = false );
		virtual void		EndDrawing();
		virtual void		EndDrawingAndBlend( const RGBColor& opColor );
		
	private:
		GWorldPtr	fWorld;
		CGrafPtr	fSavePort;
		GDHandle	fSaveDevice;
		Rect		fBounds;
};
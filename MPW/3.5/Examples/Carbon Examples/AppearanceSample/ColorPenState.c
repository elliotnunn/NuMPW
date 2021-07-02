/*
	File:		ColorPenState.c

	Contains:	Utility routines to save and restore the graphics state of a grafport.

	Version:    CarbonLib 1.0.2 SDK

	You may incorporate this sample code into your applications without
	restriction, though the sample code has been provided "AS IS" and the
	responsibility for its operation is 100% yours.  However, what you are
	not permitted to do is to redistribute the source as "Apple Sample Code"
	after having made changes. If you're going to re-distribute the source,
	we require that you make it clear in the source that the code was
	descended from Apple Sample Code, but that you've made changes.

	© 1997-1999 by Apple Computer, Inc. All rights reserved.
*/

#include "ColorPenState.h"
#include "ColorUtils.h"
#include "Assertions.h"

//-----------------------------------------------------------------------------------------
//	• GetColorAndPenState
//
//	Gets the current drawing environment and stores the data in state. We copy pen and back
//	pix pats only if they are not type 0 (plain ol expanded black and white patterns).
//-----------------------------------------------------------------------------------------

void
GetColorAndPenState( ColorPenState* state )
{
	GrafPtr			curPort;
	
	GetPort( &curPort );
	
	state->pnPixPat = NewPixPat();
	state->bkPixPat = NewPixPat();

	GetForeColor( &state->foreColor );
	GetBackColor( &state->backColor );
	
	GetPortPenPixPat( curPort, state->pnPixPat );
	GetPortBackPixPat( curPort, state->bkPixPat );
		
	GetPenState( &state->pen );
	state->textMode = GetPortTextMode( curPort );
}

//-----------------------------------------------------------------------------------------
//	• SetColorAndPenState
//
//	Sets the current drawing environment based on the data in state.
//-----------------------------------------------------------------------------------------

void
SetColorAndPenState( ColorPenState* state, Boolean dispose )
{
	GrafPtr		curPort;
	
	GetPort( &curPort );

	SetPenState( &state->pen );

	RGBForeColor( &state->foreColor );
	RGBBackColor( &state->backColor );

	if ( state->pnPixPat )
		SetPortPenPixPat( curPort, state->pnPixPat );
	
	if ( state->bkPixPat )
		SetPortBackPixPat( curPort, state->bkPixPat );

	TextMode( state->textMode );
	
	if ( dispose )
		DisposeColorAndPenState( state );
}

//-----------------------------------------------------------------------------------------
//	• DisposeColorAndPenState
//
//	Blow away any pixpats we might have
//-----------------------------------------------------------------------------------------

void
DisposeColorAndPenState( ColorPenState* state )
{
	if ( state->pnPixPat )
	{
		DisposePixPat( state->pnPixPat );
		state->pnPixPat = nil;
	}

	if ( state->bkPixPat )
	{
		DisposePixPat( state->bkPixPat );
		state->bkPixPat = nil;
	}
}

//-----------------------------------------------------------------------------------------
//	• NormalizeColorAndPen
//
//	Sets up our environment to standard drawing fare.
//-----------------------------------------------------------------------------------------

void
NormalizeColorAndPen()
{
	RGBColor		black, white;
	Pattern			whitePat = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	GrafPtr			curPort;
	
	GetPort( &curPort );
	
	black.red = black.green = black.blue = 0x0000;
	white.red = white.green = white.blue = 0xFFFF;
	
	RGBForeColor( &black );
	RGBBackColor( &white );

	PenNormal();
	BackPat( &whitePat );
	TextMode( srcOr );
}
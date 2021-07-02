/*
	File:		UtilityWindow.cp

	Contains:	Utility window tester class.

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

#ifdef __MRC__
#include <Appearance.h>
#include <ControlDefinitions.h>
#include <MacWindows.h>
#endif	// __MRC__
#include "Assertions.h"
#include "UtilityWindow.h"

#define width( r )		( (r).right - (r).left )
#define height( r )		( (r).bottom - (r).top )

UtilityWindow::UtilityWindow( SInt16 windID ) : BaseWindow( windID )
{				
	Rect		rect;
	
		// Note the use of the new defProc constants here.
		// This eliminates the need to go thru the mapping
		// CDEF for scroll bars, which would normally happen
		// after calling RegisterAppearanceClient.
		
	CalcVertScrollBarRect( rect );
	fVertScrollBar = NewControl( fWindow, &rect, "\p", true, 0, 0, 100, kControlScrollBarProc, 0 );
	
	CalcHorizScrollBarRect( rect );
	fHorizScrollBar = NewControl( fWindow, &rect, "\p", true, 0, 0, 100, kControlScrollBarProc, 0 );	
}

UtilityWindow::UtilityWindow() : BaseWindow( 131 )
{				
	Rect		rect;
	
		// Note the use of the new defProc constants here.
		// This eliminates the need to go thru the mapping
		// CDEF for scroll bars, which would normally happen
		// after calling RegisterAppearanceClient.
		
	CalcVertScrollBarRect( rect );
	fVertScrollBar = NewControl( fWindow, &rect, "\p", true, 0, 0, 100, kControlScrollBarProc, 0 );
	
	CalcHorizScrollBarRect( rect );
	fHorizScrollBar = NewControl( fWindow, &rect, "\p", true, 0, 0, 100, kControlScrollBarProc, 0 );	
}

UtilityWindow::~UtilityWindow()
{
	if ( fVertScrollBar )
		DisposeControl( fVertScrollBar );
	
	if ( fHorizScrollBar )
		DisposeControl( fHorizScrollBar );
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä Activate
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Activates the contents of the window.
//
void
UtilityWindow::Activate( EventRecord& )
{
		// Here we use the new ActivateControl call. We
		// could have still used HiliteControl, but this
		// is much more straightforward. It also works
		// right with and without embedding, so it's a
		// real good idea to start using it.
		
	ActivateControl( fHorizScrollBar );
	ActivateControl( fVertScrollBar );
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä Deactivate
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Deactivates the contents of the window.
//
void
UtilityWindow::Deactivate( EventRecord& )
{
		// Here we use the new DeactivateControl call. We
		// could have still used HiliteControl, but this
		// is much more straightforward. It also works
		// right with and without embedding, so it's a
		// real good idea to start using it.
		
	DeactivateControl( fHorizScrollBar );
	DeactivateControl( fVertScrollBar );
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä Draw
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Draws our window.
//
void
UtilityWindow::Draw()
{
	RgnHandle	region = NewRgn();

	if ( region )
	{
		UpdateControls( fWindow, GetPortVisibleRegion( fPort, region ) );
		DisposeRgn( region );
	}
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä Resize
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Resize our window to the appropriate size specified in width and height. Make sure
//	the scroll bars are repositioned properly.
//
void
UtilityWindow::Resize( SInt16 width, SInt16 height )
{
	Rect		horizRect, vertRect;
	
	InvalidateScrollBars();
	
	BaseWindow::Resize( width, height );
	
	InvalidateScrollBars();
	
	CalcHorizScrollBarRect( horizRect );
	CalcVertScrollBarRect( vertRect );
	
	MoveControl( fHorizScrollBar, horizRect.left, horizRect.top );
	MoveControl( fVertScrollBar, vertRect.left, vertRect.top );
	SizeControl( fHorizScrollBar, width( horizRect ), height( horizRect ) );
	SizeControl( fVertScrollBar, width( vertRect ), height( vertRect ) );
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä HandleClick
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Simple routine to handle scroll bar tracking, even though they don't do anything.
//
void
UtilityWindow::HandleClick( EventRecord& event )
{
	ControlHandle		control;
	SInt16				part;
	Point				localPt;
	ControlActionUPP	actionProc;
	
	SetPort( fPort );
	localPt = event.where;
	GlobalToLocal( &localPt );
	
	part = FindControl( localPt, fWindow, &control );
	switch ( part )
	{
		case kControlUpButtonPart:
		case kControlDownButtonPart:
		case kControlPageUpPart:
		case kControlPageDownPart:
			actionProc = NewControlActionUPP( ScrollBarAction );
			TrackControl( control, localPt, actionProc );
			DisposeControlActionUPP( actionProc );
			break;
		
		case kControlIndicatorPart:
			TrackControl( control, localPt, (ControlActionUPP)-1L );
			break;
	}
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä InvalidateScrollBars
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Invalidates the scroll bar areas.
//
void
UtilityWindow::InvalidateScrollBars()
{
	Rect		tempRect;
	
	SetPort( fPort );
	CalcHorizScrollBarRect( tempRect );
	InvalWindowRect( fWindow, &tempRect );
	EraseRect( &tempRect );
	
	CalcVertScrollBarRect( tempRect );
	InvalWindowRect( fWindow, &tempRect );
	EraseRect( &tempRect );
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä CalcHorizScrollBarRect
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Calculates the position where the horizontal scroll bar should be placed.
//
void
UtilityWindow::CalcHorizScrollBarRect( Rect& rect )
{
	GetPortBounds( fPort, &rect );
	rect.bottom++;
	rect.left = -1;
	rect.top = rect.bottom - 11;
	rect.right -= 9;
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä CalcVertScrollBarRect
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Calculates the position where the vertical scroll bar should be placed.
//
void
UtilityWindow::CalcVertScrollBarRect( Rect& rect )
{
	GetPortBounds( fPort, &rect );
	rect.right++;
	rect.left = rect.right - 11;
	rect.bottom -= 9;
	rect.top = -1;
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä ScrollBarAction
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	A simple callback to give some feedback when clicking the scroll arrows or page
//	up/down areas.
//
pascal void
UtilityWindow::ScrollBarAction( ControlHandle control, SInt16 part )
{
	switch ( part )
	{
		case kControlUpButtonPart:
			if ( GetControlValue( control) > GetControlMinimum( control ) )
				SetControlValue( control, GetControlValue( control ) - 1 );
			break;
		
		case kControlDownButtonPart:
			if ( GetControlValue( control) < GetControlMaximum( control ) )
				SetControlValue( control, GetControlValue( control ) + 1 );
			break;
			
		case kControlPageUpPart:
			if ( GetControlValue( control) > GetControlMinimum( control ) )
				SetControlValue( control, GetControlValue( control ) - 10 );
			break;
			
		case kControlPageDownPart:
			if ( GetControlValue( control) < GetControlMaximum( control ) )
				SetControlValue( control, GetControlValue( control ) + 10 );
			break;
	}			
}

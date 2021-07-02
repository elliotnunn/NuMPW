/*
	File:		BaseDialog.cp

	Contains:	Base class for a dialog.

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
#include <Dialogs.h>
#endif	// __MRC__
#include "BaseDialog.h"

BaseDialog::BaseDialog()
{
	fDialog = NULL;
}

BaseDialog::BaseDialog( SInt16 resID )
{
	fDialog = GetNewDialog( resID, NULL, (WindowPtr)-1L );
	if ( fDialog )
	{
		fWindow = GetDialogWindow( fDialog );
		fPort = GetWindowPort( fWindow );

		SetWindowKind( fWindow, 2000 );
		SetWRefCon( fWindow, (long)this );
	}
	else
	{
		// •••check for errors and do something real
	   ExitToShell();
	}
}

BaseDialog::~BaseDialog()
{
	if ( fWindow )
	{
		DisposeDialog( fDialog );
		fWindow = nil;
	}
}

void
BaseDialog::Update( EventRecord& event )
{
	DialogPtr		dialog;
	SInt16			itemHit;

	SetWindowKind( fWindow, dialogKind );
	DialogSelect( &event, &dialog, &itemHit );
	SetWindowKind( fWindow, 2000 );
}

void
BaseDialog::Activate( EventRecord& event )
{
	DialogPtr		dialog;
	SInt16			itemHit;
	
	SetWindowKind( fWindow, dialogKind );
	DialogSelect( &event, &dialog, &itemHit );
	SetWindowKind( fWindow, 2000 );
}

void
BaseDialog::Deactivate( EventRecord& event )
{
	DialogPtr		dialog;
	SInt16			itemHit;
	
	SetWindowKind( fWindow, dialogKind );
	DialogSelect( &event, &dialog, &itemHit );
	SetWindowKind( fWindow, 2000 );
}

void
BaseDialog::HandleClick( EventRecord& event )
{
	DialogRef		dialog;
	SInt16			itemHit;
	
	if ( DialogSelect( &event, &dialog, &itemHit ) )
	{
		HandleItemHit( itemHit );
	}
}

void
BaseDialog::HandleKeyDown( EventRecord& event )
{
	DialogPtr		dialog;
	SInt16			itemHit;
	
	SetWindowKind( fWindow, dialogKind );
	DialogSelect( &event, &dialog, &itemHit );
	SetWindowKind( fWindow, 2000 );
}


void
BaseDialog::HandleItemHit( short /*itemHit*/ )
{
}
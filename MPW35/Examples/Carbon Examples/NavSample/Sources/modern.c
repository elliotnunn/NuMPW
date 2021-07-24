/*	File:		modern.c	Contains:	Example Navigation Services code using the modern API	Version:	1.4	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.				("Apple") in consideration of your agreement to the following terms, and your				use, installation, modification or redistribution of this Apple software				constitutes acceptance of these terms.  If you do not agree with these terms,				please do not use, install, modify or redistribute this Apple software.				In consideration of your agreement to abide by the following terms, and subject				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s				copyrights in this original Apple software (the "Apple Software"), to use,				reproduce, modify and redistribute the Apple Software, with or without				modifications, in source and/or binary forms; provided that if you redistribute				the Apple Software in its entirety and without modifications, you must retain				this notice and the following text and disclaimers in all such redistributions of				the Apple Software.  Neither the name, trademarks, service marks or logos of				Apple Computer, Inc. may be used to endorse or promote products derived from the				Apple Software without specific prior written permission from Apple.  Except as				expressly stated in this notice, no other rights or licenses, express or implied,				are granted by Apple herein, including but not limited to any patent rights that				may be infringed by your derivative works or by other works in which the Apple				Software may be incorporated.				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN				COMBINATION WITH YOUR PRODUCTS.				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.	Copyright � 2000-2001 Apple Computer, Inc., All Rights Reserved*/	#ifndef Common_Defs#include "Common.h"#endif#ifndef __myFILE__#include "file.h"	// for DoModernSaveCopy()#endif#include "modern.h"	extern NavDialogRef			gAppNavDialog;extern NavEventUPP 			gEventProc;			// the app's event callbackextern NavObjectFilterUPP	gModernFilterProc;extern NavObjectFilterUPP	gFilterProc;		// the app's nav object filter callbackextern NavObjectFilterUPP 	gModernCustomShowFilterProc;extern NavEventUPP			gModernCustomShowEventProc;extern Boolean				gNavServicesExists;#if TARGET_API_MAC_CARBON// *****************************************************************************// *// * 	ModernCleanUp( )// *// *****************************************************************************void ModernCleanUp( ){    if ( gAppNavDialog != NULL )    	NavDialogDispose( gAppNavDialog );    gAppNavDialog = NULL;}// *****************************************************************************// *// * 	modernFilterProc( )// *// *****************************************************************************pascal Boolean modernFilterProc( AEDesc* theItem, void* info, NavCallBackUserData callBackUD, NavFilterModes filterMode ){#pragma unused( theItem, callBackUD, filterMode )	Boolean 				display = true;	NavFileOrFolderInfo* 	theInfo = (NavFileOrFolderInfo*)info;		if ( theInfo != NULL )		if ( !theInfo->isFolder )		{					if ( theInfo->fileAndFolder.fileInfo.finderInfo.fdType != kFileType &&					theInfo->fileAndFolder.fileInfo.finderInfo.fdType != kFileTypePICT )				display = false;		}						return display;}// *****************************************************************************// *// * 	modernEventProc( )// *// *****************************************************************************pascal void modernEventProc( NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void* callBackUD ){	Document* 	theDocument = (Document*)callBackUD;	OSStatus 	err = noErr;		        		switch( callBackSelector )	{		case kNavCBUserAction:		{			NavReplyRecord 	reply;			NavUserAction 	userAction = 0;						if ((err = NavDialogGetReply( callBackParms->context, &reply )) == noErr )			{				userAction = NavDialogGetUserAction( callBackParms->context );								switch( userAction )	 			{	  				case kNavUserActionSaveAs:	  				{	  					if ( theDocument != NULL )		  					err = DoModernSaveCopy( theDocument );	  					break;	  				}	  					  				case kNavUserActionOpen:	  				{        				        if ( gAppNavDialog != NULL )				        {					           	long count = 0;							short index;							err = AECountItems( &reply.selection, &count );							for ( index = 1; index <= count; index++ )							{								AEKeyword 	keyWord;								AEDesc		theDesc;								if (( err = AEGetNthDesc( &reply.selection, index, typeWildCard, &keyWord, &theDesc )) == noErr )								{									FSSpec 	fileSpec;									FInfo	fileInfo;																		if (( err = GetFSSpecInfo( &theDesc, &fileSpec )) == noErr )										if (( err = FSpGetFInfo( &fileSpec, &fileInfo )) == noErr )										{											// decide if the doc we are opening is a 'PICT' or 'TEXT':											if ( fileInfo.fdType == kFileType )												(void)DoOpenFile( &fileSpec, false );											else												if ( fileInfo.fdType == kFileTypePICT )													(void)DoOpenFile( &fileSpec, true );								        }							        AEDisposeDesc( &theDesc );							    }							}						}	  					break;	  				}	  					  				case kNavUserActionChoose:	  				{        				        if ( gAppNavDialog != NULL )				        {					           	long count = 0;							short index;							err = AECountItems( &reply.selection, &count );							for ( index = 1; index <= count; index++ )							{								AEKeyword 	keyWord;								DescType 	typeCode;								Size 		actualSize = 0;								FSRef		fileRef;								FSSpec 		fileSpec;																// for Mac OS X, the returned selection is FSRef-based,								// if this yields a coercion error, then get the selection as FSSpec-based:								if (( err = AEGetNthPtr( &reply.selection, index, typeFSRef, &keyWord, &typeCode, &fileRef, sizeof( FSRef ), &actualSize )) == errAECoercionFail )								{									err = AEGetNthPtr( &reply.selection, index, typeFSS, &keyWord, &typeCode, &fileSpec, sizeof( FSRef ), &actualSize );								}							}				        }	  					break;	  				}	  						  				case kNavUserActionCancel:	  					//..	  					break;	  						  				case kNavUserActionNewFolder:	                	//..	                	break;	            }	          	  			err = NavDisposeReply( &reply );            }            break;		}					case kNavCBTerminate:		{			if ( theDocument != NULL )            {            	// this Nav dialog belongs to a certain document:            	NavDialogDispose( theDocument->fNavigationDialog );            	theDocument->fNavigationDialog = NULL;            }            else           		ModernCleanUp( );        }    }}//==============================================================================#pragma mark -#pragma mark	� Modern Basic Examples �//==============================================================================// *****************************************************************************// *// * 	DoCreateConfirmPanel( )// *// *	This example shows how to use the modern API equivalent of NavAskSaveChanges().// *	The alert is window modal or a sheet (if running on Mac OS X).// *// *****************************************************************************void DoCreateConfirmPanel( Document* theDocument ){    OSStatus 					err = noErr;    NavDialogCreationOptions	dialogOptions;    NavAskSaveChangesAction		action = kNavSaveChangesClosingDocument;    Str255						pStr;        if ((err = NavGetDefaultDialogCreationOptions( &dialogOptions )) == noErr )    {                         Str255 windTitle;		GetWTitle( theDocument->theWindow, windTitle );        		dialogOptions.saveFileName = CFStringCreateWithPascalString( NULL, windTitle, CFStringGetSystemEncoding( ) );              GetIndString( pStr, rAppStringsID, sApplicationName );        dialogOptions.clientName = CFStringCreateWithPascalString( NULL, pStr, CFStringGetSystemEncoding( ) );                                                 dialogOptions.parentWindow = theDocument->theWindow;        dialogOptions.modality = kWindowModalityWindowModal;      	        if ((err = NavCreateAskSaveChangesDialog( 	&dialogOptions,                                                    action,                                                    theDocument->fConfirmEventProc,                                                    (void*)theDocument,                                                    &theDocument->fNavigationDialog )) == noErr)            if ( theDocument->fNavigationDialog != NULL )            {                if ((err = NavDialogRun( theDocument->fNavigationDialog )) != noErr )                {                    NavDialogDispose( theDocument->fNavigationDialog );                    theDocument->fNavigationDialog = NULL;                }            }              		if ( dialogOptions.clientName != NULL )    		CFRelease( dialogOptions.clientName );    	if ( dialogOptions.saveFileName != NULL )    		CFRelease( dialogOptions.saveFileName );    }}// *****************************************************************************// *// * 	DoCreateOpenPanel( )// *// *	This example shows how to use the modern API equivalent of NavGetFile.// *	The dialog is modeless (if running on Mac OS X), does not use a// *	NavTypeList, custom previews or user context.// *// *****************************************************************************void DoCreateOpenPanel( ){    OSStatus 					err = noErr;    NavDialogCreationOptions	dialogOptions;        if (( err = NavGetDefaultDialogCreationOptions( &dialogOptions )) == noErr )    {       	Str255 pStr;               GetIndString( pStr, rAppStringsID, sApplicationName );        dialogOptions.clientName = CFStringCreateWithPascalString( NULL, pStr, CFStringGetSystemEncoding( ) );                dialogOptions.modality = kWindowModalityNone;	// make it modeless                if ((err = NavCreateGetFileDialog( 	&dialogOptions,                                        	NULL,		// no NavTypeList                                        	gEventProc,                                       	 	NULL,		// no custom previews                                       		gModernFilterProc,                                        	NULL,		// no document context                                        	&gAppNavDialog )) == noErr)		{   			if (( err = NavDialogRun( gAppNavDialog )) != noErr)    			ModernCleanUp( );	  	}	        		if ( dialogOptions.clientName != NULL )    		CFRelease( dialogOptions.clientName );    }}// *****************************************************************************// *// * 	DoCreateSavePanel( )// *// *	This example shows how to use the modern API equivalent of NavPutFile().// *	The dialog is window modal or a sheet (if running on Mac OS X).// *// *****************************************************************************void DoCreateSavePanel( Document* theDocument ){    OSStatus 					err = noErr;	NavDialogCreationOptions	dialogOptions;    if (( err = NavGetDefaultDialogCreationOptions( &dialogOptions )) == noErr )    {		Str255 pStr;				CopyWindowTitleAsCFString( theDocument->theWindow, &dialogOptions.saveFileName );			GetIndString( pStr, rAppStringsID, sApplicationName );        dialogOptions.clientName = CFStringCreateWithPascalString( NULL, pStr, CFStringGetSystemEncoding( ) );                dialogOptions.parentWindow = theDocument->theWindow;        dialogOptions.modality = kWindowModalityWindowModal;	// make the dialog modal to our parent doc                       if ((err = NavCreatePutFileDialog( 	&dialogOptions,									        kFileType,									        kFileCreator,									        gEventProc,									        (void*)theDocument,									        &theDocument->fNavigationDialog )) == noErr)     	{     		if ( theDocument->fNavigationDialog != NULL )            {                 if (( err = NavDialogRun( theDocument->fNavigationDialog )) != noErr )                {                	NavDialogDispose( theDocument->fNavigationDialog );                    theDocument->fNavigationDialog = NULL;                }            }    	}       	     	if ( dialogOptions.saveFileName != NULL )    		CFRelease( dialogOptions.saveFileName );    	if ( dialogOptions.clientName != NULL )    		CFRelease( dialogOptions.clientName );    }}// *****************************************************************************// *// * 	DoCreateChoosePanel( )// *// *	This example shows how to use the modern API equivalent of NavChooseObject().// *// *****************************************************************************void DoCreateChoosePanel( ){    OSStatus 					err = noErr;    NavDialogCreationOptions	dialogOptions;        if ((err = NavGetDefaultDialogCreationOptions( &dialogOptions )) == noErr)    {      	Str255 pStr;               GetIndString( pStr, rAppStringsID, sChooseObject );        dialogOptions.message = CFStringCreateWithPascalString( NULL, pStr, CFStringGetSystemEncoding( ) );        GetIndString( pStr, rAppStringsID, sApplicationName );        dialogOptions.clientName = CFStringCreateWithPascalString( NULL, pStr, CFStringGetSystemEncoding( ) );                        dialogOptions.modality = kWindowModalityAppModal;	// make the dialog modal to this app                 if ((err = NavCreateChooseObjectDialog( &dialogOptions,										        gEventProc,										        NULL,	// no filtering,                                            	NULL, 	// no custom previews										      	NULL,	// no document context										        &gAppNavDialog )) == noErr)      	{     		if ((err = NavDialogRun( gAppNavDialog )) != noErr)               	ModernCleanUp( );     	}     	 		if ( dialogOptions.message != NULL )    		CFRelease( dialogOptions.message );		if ( dialogOptions.clientName != NULL )    		CFRelease( dialogOptions.clientName );	    }}// *****************************************************************************// *// * 	DoCreateNewFolderPanel( )// *// *	This example shows how to use the modern API equivalent of NavNewFolder().// *// *****************************************************************************void DoCreateNewFolderPanel( ){    OSStatus 					err = noErr;    NavDialogCreationOptions	dialogOptions;          if ((err = NavGetDefaultDialogCreationOptions( &dialogOptions )) == noErr)    {	   	Str255 pStr;	   		   	GetIndString( pStr, rAppStringsID, sCreateFolder );	    dialogOptions.message = CFStringCreateWithPascalString( NULL, pStr, CFStringGetSystemEncoding( ) );	    	    GetIndString( pStr, rAppStringsID, sApplicationName );		dialogOptions.clientName = CFStringCreateWithPascalString( NULL, pStr, CFStringGetSystemEncoding( ) );	                    	    dialogOptions.modality = kWindowModalityAppModal;	// make the dialog modal to this app	    	    if ((err = NavCreateNewFolderDialog(	&dialogOptions,											    gEventProc,											   	NULL,		// no document context											    &gAppNavDialog )) == noErr)	    {	       	if (( err = NavDialogRun( gAppNavDialog )) != noErr)         		ModernCleanUp( );	    }	      		if ( dialogOptions.message != NULL )			CFRelease( dialogOptions.message );		if ( dialogOptions.clientName != NULL )			CFRelease( dialogOptions.clientName );	}}//==============================================================================#pragma mark	� Modern Customize Show Popup Example �//==============================================================================// =============================================================================// custom menu item indices:#define kModernCustomType1 	1#define kModernCustomType2 	2#define kModernCustomType3 	3OSType 	gModernCurrentType;		// keep track of our current popup selectionOSStatus ModernExtendTypePopup( NavDialogCreationOptions* creationOptions );// *****************************************************************************// *// * 	customShowEventProc( )// *// *****************************************************************************pascal Boolean customShowFilterProc( AEDesc* theItem, void* info, NavCallBackUserData callBackUD, NavFilterModes filterMode ){#pragma unused( theItem, callBackUD, filterMode )	Boolean 				display = true;	NavFileOrFolderInfo* 	theInfo = (NavFileOrFolderInfo*)info;		if ( theInfo != NULL )	{		// use 'gModernCurrentType' to guide you on what should be filtered:	}						return display;}// *****************************************************************************// *// * 	customShowEventProc( )// *// *****************************************************************************pascal void customShowEventProc( NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void* callBackUD ){#pragma unused( callBackUD )	OSStatus err = noErr;		        		switch( callBackSelector )	{		case kNavCBUserAction:		{			NavReplyRecord 	reply;			NavUserAction 	userAction = 0;						if ((err = NavDialogGetReply( callBackParms->context, &reply )) == noErr )			{				userAction = NavDialogGetUserAction( callBackParms->context );								switch( userAction )	 			{	  				case kNavUserActionOpen:	  				{        						// handle the open operation here�	  					break;	  				}	            }	          	  			err = NavDisposeReply( &reply );            }            break;		}					case kNavCBPopupMenuSelect:		{			if ( callBackParms != NULL )			{				NavMenuItemSpec menuItemSpec = *(NavMenuItemSpec*)callBackParms->eventData.eventDataParms.param;								menuItemSpec.menuType++;							if ( menuItemSpec.menuType == kModernCustomType1 )					gModernCurrentType = menuItemSpec.menuType;							else					if ( menuItemSpec.menuType == kModernCustomType2 )						gModernCurrentType = kModernCustomType2;					else						if ( menuItemSpec.menuType == kModernCustomType3 )							gModernCurrentType = kModernCustomType3;						else							gModernCurrentType = 0;	// no custom menu selected				}			break;		}				case kNavCBTerminate:		{       		ModernCleanUp( );       		break;        }    }}// *****************************************************************************// *// * 	ModernExtendTypePopup( )// *// *****************************************************************************OSStatus ModernExtendTypePopup( NavDialogCreationOptions* creationOptions )		{    OSStatus 			theErr = noErr;	Str255 				testItemName; 	CFStringRef			itemStr = NULL;	CFArrayCallBacks 	callBacks = kCFTypeArrayCallBacks;		creationOptions->popupExtension = CFArrayCreateMutable( kCFAllocatorDefault, 3, &callBacks );     if ( creationOptions->popupExtension != NULL )    {	    GetIndString( testItemName, rAppStringsID, sCustomMenuTitle1 );	    itemStr = CFStringCreateWithPascalString( kCFAllocatorDefault, testItemName, CFStringGetSystemEncoding( ) );	    CFArrayInsertValueAtIndex( (CFMutableArrayRef)creationOptions->popupExtension, 0, (void*)itemStr );		CFRelease( itemStr );	    	    GetIndString( testItemName, rAppStringsID, sCustomMenuTitle2 );	    itemStr = CFStringCreateWithPascalString( kCFAllocatorDefault, testItemName, CFStringGetSystemEncoding( ) );	    CFArrayInsertValueAtIndex( (CFMutableArrayRef)creationOptions->popupExtension, 1, (void*)itemStr );		CFRelease( itemStr );	    	    GetIndString( testItemName, rAppStringsID, sCustomMenuTitle3 );	    itemStr = CFStringCreateWithPascalString( kCFAllocatorDefault, testItemName, CFStringGetSystemEncoding( ) );	    CFArrayInsertValueAtIndex( (CFMutableArrayRef)creationOptions->popupExtension, 2, (void*)itemStr );		CFRelease( itemStr );   	}     return theErr;}// *****************************************************************************// *// * 	DoCreateCustomShowPanel( )// *// *****************************************************************************void DoCreateCustomShowPanel( ){    OSStatus 					err = noErr;    NavDialogCreationOptions	creationOptions;        if ((err = NavGetDefaultDialogCreationOptions( &creationOptions )) == noErr)    {	   	if (( err = ModernExtendTypePopup( &creationOptions )) == noErr )	   	{		   	Str255 pStr;		   			   	GetIndString( pStr, rAppStringsID, sCustomShowPrompt );		    creationOptions.message = CFStringCreateWithPascalString( NULL, pStr, CFStringGetSystemEncoding( ) );		    		    GetIndString( pStr, rAppStringsID, sApplicationName );			creationOptions.clientName = CFStringCreateWithPascalString( NULL, pStr, CFStringGetSystemEncoding( ) );						    creationOptions.modality = kWindowModalityAppModal;	// make the dialog modal to this app		    			if ((err = NavCreateGetFileDialog( 	&creationOptions,	                                        	NULL,				// no NavTypeList	                                        	gModernCustomShowEventProc,	                                       	 	NULL,				// no custom previews	                                       		gModernCustomShowFilterProc,	                                        	NULL,				// no document context	                                        	&gAppNavDialog )) == noErr)		    {	  			if (( err = NavDialogRun( gAppNavDialog )) != noErr)	    			ModernCleanUp( );		    }		      			if ( creationOptions.message != NULL )				CFRelease( creationOptions.message );			if ( creationOptions.clientName != NULL )				CFRelease( creationOptions.clientName );			if ( creationOptions.popupExtension != NULL )				CFRelease( creationOptions.popupExtension );		}	}}//==============================================================================#pragma mark -#pragma mark	� Customize with CarbonEvents Example �//==============================================================================NavDialogRef	gCarbonEventCustomDialog;Handle 			gCarbonEventDitlList;short 			gCarbonEventLastTryWidth;short 			gCarbonEventLastTryHeight;NavEventUPP 	gCarbonEventProc;EventHandlerUPP	gEventHandler;EventHandlerRef	gEventHandlerRef;#define kNavSampleControlSignature 'smpl'	// our signature we mark with our controls for CarbonEvents	void SetDialogControlID( DialogRef dialog, short itemNum, short id );Boolean HandleControlEvent( EventRef inEvent );pascal OSStatus EventWindowHandler( EventHandlerCallRef inHandlerRef, EventRef inEvent, void* userData );OSStatus SetupEventHandler( WindowRef window );OSStatus HandleCarbonEventStart( NavCBRecPtr callBackParms );void HandleCarbonCustomizeEvent( NavCBRecPtr callBackParms );pascal void modernCarbonEventProc( NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void* callBackUD );// *****************************************************************************// *// * 	SetDialogControlID( )// *// *****************************************************************************void SetDialogControlID( DialogRef dialog, short itemNum, short id ){	ControlRef 	control;	ControlID 	theID;		theID.signature = kNavSampleControlSignature;	theID.id = id;		GetDialogItemAsControl( dialog, itemNum, &control );	SetControlID( control, &theID );  }										// *****************************************************************************// *// *	HandleCarbonCustomizeEvent( )	// *// *****************************************************************************void HandleCarbonCustomizeEvent( NavCBRecPtr callBackParms ){								DialogTHndl DLOG;	short		neededHeight, neededWidth;	UInt32 		version = NavLibraryVersion( );		// here are the desired dimensions for our custom area:	if ( version >= 0x02008000 )		DLOG = (DialogTHndl)GetResource( 'DLOG', kControlListID );	else		DLOG = (DialogTHndl)GetResource( 'DLOG', kControlListIDInset );			neededHeight = callBackParms->customRect.top + ((**DLOG).boundsRect.bottom - (**DLOG).boundsRect.top);	neededWidth = callBackParms->customRect.left + ((**DLOG).boundsRect.right - (**DLOG).boundsRect.left);		// check to see if this is the first round of negotiations:	if ( callBackParms->customRect.right == 0 && callBackParms->customRect.bottom == 0 )	{		// it is, so tell NavServices what dimensions we want:		callBackParms->customRect.right = neededWidth;		callBackParms->customRect.bottom = neededHeight;	}	else	{		// we are in the middle of negotiating:		if ( gCarbonEventLastTryWidth != callBackParms->customRect.right )			if ( callBackParms->customRect.right < neededWidth )	// is NavServices width too small for us?				callBackParms->customRect.right = neededWidth;		if ( gCarbonEventLastTryHeight != callBackParms->customRect.bottom )	// is NavServices height too small for us?			if ( callBackParms->customRect.bottom < neededHeight )				callBackParms->customRect.bottom = neededHeight;	}		// remember our last size so the next time we can re-negotiate:	gCarbonEventLastTryWidth = callBackParms->customRect.right;	gCarbonEventLastTryHeight = callBackParms->customRect.bottom;}// *****************************************************************************// *// * 	HandleControlEvent( )// *// *****************************************************************************Boolean HandleControlEvent( EventRef inEvent ){	Boolean		result = false;	OSStatus 	theErr = noErr;				  	ControlRef 	control;		if (( theErr = GetEventParameter( inEvent,										  kEventParamDirectObject,										  typeControlRef,										  NULL,										  sizeof( ControlRef ),										  NULL,										  &control )) == noErr)                                                                                                                         	{		WindowRef window = NULL;		window = GetControlOwner( control );		if ( window != NULL )		{			ControlID theID = {0};			GetControlID( control, &theID );			if ( theID.id > 0 )			{			   	// make sure its our own controls (and not Nav's):			   	if ( theID.signature == kNavSampleControlSignature )				{					UInt16 firstItem = 0;					if (( theErr = NavCustomControl( gCarbonEventCustomDialog, kNavCtlGetFirstControlID, &firstItem )) == noErr )						result = HandleCustomControls( window, gCarbonEventCustomDialog, control, firstItem, theID.id );				}			}		}	}	    return result;}// *****************************************************************************// *// * 	EventWindowHandler( )// *// *****************************************************************************pascal OSStatus EventWindowHandler( EventHandlerCallRef inHandlerRef, EventRef inEvent, void* userData ){#pragma unused( inHandlerRef, userData )	Boolean 	eventProcessed = false;	OSStatus 	theErr = noErr;		switch( GetEventClass( inEvent ) )	{		case kEventClassKeyboard:		{			switch( GetEventKind( inEvent ) )			{				case kEventRawKeyDown:				{					UInt8 	charCode = 0;					UInt32 	modifierKeys = 0;					theErr = GetEventParameter( inEvent,												kEventParamKeyModifiers,												typeUInt32,												NULL,												sizeof( UInt32 ),												NULL,												&modifierKeys );							theErr = GetEventParameter( inEvent,												kEventParamKeyMacCharCodes,												typeChar,												NULL,												sizeof( charCode ),												NULL,												&charCode );																	//.. handle your raw key events here:				}				break;			}		}		break;				case kEventClassWindow:		{			WindowRef window = NULL;			if (( theErr = GetEventParameter( inEvent,											  kEventParamDirectObject,											  typeWindowRef,											  NULL,											  sizeof( WindowRef ),											  NULL,											  &window )) == noErr)			{										  				switch( GetEventKind( inEvent ) )				{					case kEventWindowActivated:					{						//.. handle window activate						break;					}	    					    			case kEventWindowDeactivated:	    			{	    				//.. handle window deactivate	    				break;	    			}						case kEventWindowDrawContent:					{										Rect bounds;						if ((theErr = GetWindowBounds( window, kWindowUpdateRgn, &bounds )) == noErr )						{							// check if the update region is valid:							if ( bounds.right > 0 || bounds.bottom > 0 )							{								//.. handle any updating here:							}						}					}					break;				}			}		}		break;				case kEventClassMouse:		{				switch( GetEventKind( inEvent ) )			{								case kEventMouseDown:				{					//.. handle your raw mousdown events here:					break;				}			}		}		break;						case kEventClassControl:		{			switch( GetEventKind( inEvent ) )			{  				case kEventControlHit:				{					eventProcessed = HandleControlEvent( inEvent );					break;				}			}		}		break;					}		if ( eventProcessed )		return noErr;	else		return eventNotHandledErr;}// *****************************************************************************// *// * 	SetupEventHandler( )// *// *****************************************************************************OSStatus SetupEventHandler( WindowRef window ){	OSErr 			theErr = noErr;	EventTargetRef 	targ;	EventTypeSpec 	windowEvents[6];		// install the window event handler to handle most of our Carbon events:	gEventHandler = NewEventHandlerUPP( EventWindowHandler );		targ = GetWindowEventTarget( window );	// note: calling InstallStandardEventHandler( targ ); is not necessary, Nav does it for you!		windowEvents[0].eventClass = kEventClassControl; 	windowEvents[0].eventKind = kEventControlHit;  	  	windowEvents[1].eventClass = kEventClassMouse; 	windowEvents[1].eventKind = kEventMouseDown;    	windowEvents[2].eventClass = kEventClassKeyboard; 	windowEvents[2].eventKind = kEventRawKeyDown;      	windowEvents[3].eventClass = kEventClassWindow; 	windowEvents[3].eventKind = kEventWindowDrawContent;      	windowEvents[4].eventClass = kEventClassWindow; 	windowEvents[4].eventKind = kEventWindowActivated;   	windowEvents[5].eventClass = kEventClassWindow; 	windowEvents[5].eventKind = kEventWindowDeactivated; 	theErr = InstallEventHandler( targ, gEventHandler, 9, windowEvents, NULL, &gEventHandlerRef );	return theErr;}// *****************************************************************************// *// *	HandleCarbonEventStart( )	// *// *****************************************************************************OSStatus HandleCarbonEventStart( NavCBRecPtr callBackParms ){	OSErr 	theErr = noErr;	UInt32 	version = NavLibraryVersion( );		// here are the desired dimensions for our custom area:	if ( version >= 0x02008000 )		gCarbonEventDitlList = GetResource( 'DITL', kControlListID );	else		gCarbonEventDitlList = GetResource( 'DITL', kControlListIDInset );			if ( gCarbonEventDitlList != NULL && ResError( ) == noErr )	{		if ((theErr = NavCustomControl( callBackParms->context, kNavCtlAddControlList, gCarbonEventDitlList )) == noErr)		{			UInt16 firstItem = 0;						// ask NavServices for our first control ID:			if ((theErr = NavCustomControl( callBackParms->context, kNavCtlGetFirstControlID, &firstItem )) == noErr)				{				ControlRef itemH;								// set the command popup selection:				if ( firstItem + kPopupCommand > 0 )				{						MenuHandle 	popupMenu = NULL;					Size 		outSize = 0;										GetDialogItemAsControl( GetDialogFromWindow( callBackParms->window ), firstItem + kPopupCommand, &itemH );					GetControlData( itemH, kControlEntireControl, kControlPopupButtonMenuHandleTag, sizeof( MenuHandle ), (Ptr)&popupMenu, &outSize );					if ( popupMenu != NULL )					{						InsertMenu( popupMenu, -1 );						SetControlValue( itemH, 1 );					}				}								SetupEventHandler( callBackParms->window );								// tag each custom control for CarbonEvents with our own control item constants:				SetDialogControlID( GetDialogFromWindow( callBackParms->window ),									firstItem + kPopupCommand,									kPopupCommand );				SetDialogControlID( GetDialogFromWindow( callBackParms->window ),									firstItem + kAllowDismissCheck,									kAllowDismissCheck );				SetDialogControlID( GetDialogFromWindow( callBackParms->window ),									firstItem + kPictItem,									kPictItem );				SetDialogControlID( GetDialogFromWindow( callBackParms->window ),									firstItem + kRadioOneItem,									kRadioOneItem );				SetDialogControlID( GetDialogFromWindow( callBackParms->window ),									firstItem + kRadioTwoItem,									kRadioTwoItem );				SetDialogControlID( GetDialogFromWindow( callBackParms->window ),									firstItem + kButtonItem,									kButtonItem );													GetDialogItemAsControl( GetDialogFromWindow( callBackParms->window ), firstItem + kRadioOneItem, &itemH );				SetControlValue( itemH, true );								RightJustifyPict( callBackParms );			}		}	}			return theErr;}// *****************************************************************************// *// * 	modernCarbonEventProc( )// *// *****************************************************************************pascal void modernCarbonEventProc( NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void* callBackUD ){#pragma unused( callBackUD )	OSStatus err = noErr;		        		switch( callBackSelector )	{		case kNavCBStart:		{			HandleCarbonEventStart( callBackParms );			break;		}				case kNavCBCustomize:		{			HandleCarbonCustomizeEvent( callBackParms );			break;		}					case kNavCBAdjustRect:		{			RightJustifyPict( callBackParms );			break;		}				case kNavCBUserAction:		{			NavReplyRecord 	reply;			NavUserAction 	userAction = 0;						if ((err = NavDialogGetReply( callBackParms->context, &reply )) == noErr )			{				userAction = NavDialogGetUserAction( callBackParms->context );								switch( userAction )	 			{	  				case kNavUserActionOpen:	  					//..	  					break;    	            }	          	  			err = NavDisposeReply( &reply );            }            break;		}					case kNavCBTerminate:		{       		if ( gCarbonEventDitlList != NULL )				ReleaseResource( gCarbonEventDitlList );							if ( gEventHandlerRef != NULL )				RemoveEventHandler( gEventHandlerRef );			if ( gEventHandler != NULL )				DisposeEventHandlerUPP( gEventHandler );						if ( gCarbonEventCustomDialog != NULL )       		{       			NavDialogDispose( gCarbonEventCustomDialog );       			gCarbonEventCustomDialog = NULL;       		}       		       		if ( gCarbonEventProc != NULL )       			DisposeNavEventUPP( gCarbonEventProc );        }    }}// *****************************************************************************// *// * 	DoCreateCustomCarbonEventPanel( )// *// *	This example shows how to use the modern API equivalent of NavGetFile.// *	It demonstrates the use of CarbonEvents when customizing the interface.// *// *****************************************************************************void DoCreateCustomCarbonEventPanel( ){    OSStatus 					err = noErr;    NavDialogCreationOptions	dialogOptions;       if (( err = NavGetDefaultDialogCreationOptions( &dialogOptions )) == noErr )    {        Str255 pStr;        		dialogOptions.optionFlags += kNavDontUseCustomFrame;		dialogOptions.optionFlags -= kNavAllowPreviews;		dialogOptions.modality = kWindowModalityNone;		         gCarbonEventProc = NewNavEventUPP( modernCarbonEventProc );                GetIndString( pStr, rAppStringsID, sApplicationName );        dialogOptions.clientName = CFStringCreateWithPascalString( NULL, pStr, GetApplicationTextEncoding( ) );                GetIndString( pStr, rAppStringsID, sCustomCarbonEventPrompt );        dialogOptions.message = CFStringCreateWithPascalString( NULL, pStr, GetApplicationTextEncoding( ) );                if ((err = NavCreateGetFileDialog( 	&dialogOptions,                                        	NULL,                                        	gCarbonEventProc,                                       	 	NULL,                                       		NULL,                                        	NULL,                                        	&gCarbonEventCustomDialog )) == noErr)		{			if ( gCarbonEventCustomDialog != NULL )			{              	if (( err = NavDialogRun( gCarbonEventCustomDialog )) != noErr)            	{                    NavDialogDispose( gCarbonEventCustomDialog );                    gCarbonEventCustomDialog = NULL;                }	        }	  	}	        		if ( dialogOptions.clientName != NULL )    		CFRelease( dialogOptions.clientName );    	if ( dialogOptions.message != NULL )    		CFRelease( dialogOptions.message );    }}#endif
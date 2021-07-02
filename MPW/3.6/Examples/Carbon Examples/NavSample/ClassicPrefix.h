/*
	File:		NavSampleHeader.h

	Contains:	Precompiled header file for NavSample.

	Version:	1.3

	Copyright:	Copyright © 2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define TARGET_API_MAC_CARBON				0
#define TARGET_API_MAC_OS8					1	// we are not building for Carbon
#define ACCESSOR_CALLS_ARE_FUNCTIONS 		1	// preserve old #define-style accessors so we can use:
												// 		GetPortBounds
												// 		GetQDGlobalScreenBits
												// 		GetDialogFromWindow
												// 		SetListSelectionFlags
												// 		GetListVerticalScrollBar
												// 		GetPortVisibleRegion... (you get the picture)

#include <Aliases.h>
#include <AppleEvents.h>
#include <CodeFragments.h>
#include <Components.h>
#include <Controls.h>
#include <ControlDefinitions.h>
#include <Devices.h>
#include <Dialogs.h>
#include <DiskInit.h>
#include <Drag.h>
#include <Errors.h>
#include <Events.h>
#include <Files.h>
#include <Finder.h>
#include <FixMath.h>
#include <Folders.h>
#include <Fonts.h>
#include <Gestalt.h>
#include <Icons.h>
#include <LowMem.h>
#include <MacMemory.h>
#include <Menus.h>
#include <Navigation.h>
#include <OSUtils.h>
#include <Packages.h>
#include <Printing.h>
#include <QDOffscreen.h>
#include <QuickDraw.h>
#include <Resources.h>
#include <Scrap.h>
#include <Script.h>
#include <SegLoad.h>
#include <Sound.h>
#include <StandardFile.h>
#include <TextEdit.h>
#include <TextUtils.h>
#include <ToolUtils.h>
#include <Translation.h>
#include <MacTypes.h>
#include <MacWindows.h>
#include <fp.h>
#include <Processes.h>
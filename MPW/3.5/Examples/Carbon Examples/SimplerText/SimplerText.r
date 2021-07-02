/*
	File:		SimplerText.r
	
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

	Copyright © 2001 Apple Computer, Inc., All Rights Reserved
*/

#include "Types.r"

data 'carb' (0) {
	$"00"
};

resource 'MENU' (128)
{
	128,
	textMenuProc,
	0xFFFFFFFF,
	enabled,
	apple,
	{
		"About SimplerText", noIcon, noKey, noMark, plain,
	}
};

resource 'xmnu' (128, purgeable)
{
	versionZero
	{
		{
			dataItem { 'abou', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph }
		}
	};
};

resource 'MENU' (129)
{
	129,
	textMenuProc,
	0xFFFFFFFF,
	enabled,
	"File",
	{
		"New", noIcon, "N", noMark, plain,
		"Open...", noIcon, "O", noMark, plain,
		"Close", noIcon, "W", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Save", noIcon, "S", noMark, plain,
		"Save As...", noIcon, "S", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'xmnu' (129, purgeable)
{
	versionZero
	{
		{
			dataItem { 'new ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'open', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'clos', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			skipItem {},
			dataItem { 'save', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'svas', kMenuShiftModifier, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			skipItem {},
			dataItem { 'quit', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph }
		}
	};
};

resource 'MENU' (130)
{
	130,
	textMenuProc,
	0xFFFFFFFF,
	enabled,
	"Edit",
	{
		"Undo", noIcon, "Z", noMark, plain,
		"Redo", noIcon, "Z", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Cut", noIcon, "X", noMark, plain,
		"Copy", noIcon, "C", noMark, plain,
		"Paste", noIcon, "V", noMark, plain,
		"Clear", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Select All", noIcon, "A", noMark, plain
	}
};

resource 'xmnu' (130, purgeable)
{
	versionZero
	{
		{
			dataItem { 'undo', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'redo', kMenuShiftModifier, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			skipItem {},
			dataItem { 'cut ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'copy', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'past', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'clea', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			skipItem {},
			dataItem { 'sall', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph }
		}
	};
};

resource 'MENU' (131)
{
	131,
	textMenuProc,
	0xFFFFFFFF,
	enabled,
	"Font",
	{
	}
};


resource 'MBAR' (128)
{
	{ 128, 129, 130, 131 }
};
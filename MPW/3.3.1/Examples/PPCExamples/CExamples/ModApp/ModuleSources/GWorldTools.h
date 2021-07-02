// File: GWorldTools.h

#ifndef __QUICKDRAW__
	#include <Quickdraw.h>
#endif

#ifndef __WINDOWS__
	#include <Windows.h>
#endif

#ifndef __QDOFFSCREEN__
	#include <QDOffscreen.h>
#endif

enum {
	// Custom result codes for CopyBufferToWindow
	kPixelsPurged = 1
	};


Rect GetGlobalBounds (WindowPtr wp);
OSErr AllocateBuffer (WindowPtr wp, Rect bounds, GWorldPtr *buffer, Boolean disposeOld);
OSErr LockBuffer (GWorldPtr buffer, GWorldFlags *oldPixState);
OSErr UnlockBuffer (GWorldPtr buffer, GWorldFlags oldPixState);
OSErr UpdateBuffer (WindowPtr wp, GWorldPtr* buffer);
OSErr DisposeBuffer (GWorldPtr* buffer);
OSErr CopyBufferToWindow (WindowPtr wp, GWorldPtr buffer);

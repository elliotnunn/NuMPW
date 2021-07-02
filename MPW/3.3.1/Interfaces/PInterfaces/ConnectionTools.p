{
	File:		ConnectionTools.p

	Copyright:	© 1983-1993 by Apple Computer, Inc.
				All rights reserved.

	Version:	System 7.1 for ETO #11
	Created:	Tuesday, March 30, 1993 18:00

}

{$IFC UNDEFINED UsingIncludes}
{$SETC UsingIncludes := 0}
{$ENDC}

{$IFC NOT UsingIncludes}
 UNIT ConnectionTools;
 INTERFACE
{$ENDC}

{$IFC UNDEFINED UsingConnectionTools}
{$SETC UsingConnectionTools := 1}

{$I+}
{$SETC ConnectionToolsIncludes := UsingIncludes}
{$SETC UsingIncludes := 1}
{$IFC UNDEFINED UsingDialogs}
{$I $$Shell(PInterfaces)Dialogs.p}
{$ENDC}
{$IFC UNDEFINED UsingConnections}
{$I $$Shell(PInterfaces)Connections.p}
{$ENDC}
{$SETC UsingIncludes := ConnectionToolsIncludes}

CONST

{ messages for DefProc }
cmInitMsg = 0;
cmDisposeMsg = 1;
cmSuspendMsg = 2;
cmResumeMsg = 3;
cmMenuMsg = 4;
cmEventMsg = 5;
cmActivateMsg = 6;
cmDeactivateMsg = 7;

cmIdleMsg = 50;
cmResetMsg = 51;
cmAbortMsg = 52;

cmReadMsg = 100;
cmWriteMsg = 101;
cmStatusMsg = 102;
cmListenMsg = 103;
cmAcceptMsg = 104;
cmCloseMsg = 105;
cmOpenMsg = 106;
cmBreakMsg = 107;
cmIOKillMsg = 108;
cmEnvironsMsg = 109;

{ new connection tool messages for ctb 1.1 }
cmNewIOPBMsg = 110;
cmDisposeIOPBMsg = 111;
cmGetErrorStringMsg = 112;
cmPBReadMsg = 113;
cmPBWriteMsg = 114;
cmPBIOKillMsg = 115;

{    messages for validate DefProc    }
cmValidateMsg = 0;
cmDefaultMsg = 1;

{    messages for Setup DefProc    }
cmSpreflightMsg = 0;
cmSsetupMsg = 1;
cmSitemMsg = 2;
cmSfilterMsg = 3;
cmScleanupMsg = 4;

{    messages for scripting defProc    }
cmMgetMsg = 0;
cmMsetMsg = 1;

{    messages for localization defProc    }
cmL2English = 0;
cmL2Intl = 1;

{    private data constants }
cdefType = 'cdef';	{ main connection definition procedure    }
cvalType = 'cval';	{ validation definition procedure    }
csetType = 'cset';	{ connection setup definition procedure    }
clocType = 'cloc';	{ connection configuration localization defProc    }
cscrType = 'cscr';	{ connection scripting defProc interfaces    }

cbndType = 'cbnd';	{ bundle type for connection    }
cverType = 'vers';

TYPE
CMDataBufferPtr = ^CMDataBuffer;
CMDataBuffer = RECORD
 thePtr: Ptr;
 count: LONGINT;
 channel: CMChannel;
 flags: CMFlags;
 END;

CMCompletorPtr = ^CMCompletorRecord;
CMCompletorRecord = RECORD
 async: BOOLEAN;
 completionRoutine: ProcPtr;
 END;

{ Private Data Structure }

CMSetupPtr = ^CMSetupStruct;
CMSetupStruct = RECORD
 theDialog: DialogPtr;
 count: INTEGER;
 theConfig: Ptr;
 procID: INTEGER;	{ procID of the tool }
 END;



{$ENDC} { UsingConnectionTools }

{$IFC NOT UsingIncludes}
 END.
{$ENDC}

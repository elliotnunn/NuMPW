/* *   ShLibExample.r * *	resource file for sample C++ program * *	Copyright � 1994, Apple Computer, Inc.  All rights reserved. */#include "Types.r"#include "CodeFragmentTypes.r"resource 'cfrg' (0) {   {      kPowerPC,				/* Target machine's Architecture. */      kFullLib,				/* This is not an update. */	  kNoVersionNum,		/* Current version. */	  kNoVersionNum,		/* Definition version. */	  kDefaultStackSize,	/* Stack size of application. */	  kNoAppSubFolder,		/* Not used here.  Can be the resource-id of an 'alis'	                           resource.  Used to provide additional location							   to search for libraries. */	  kIsApp,				/* This is an application (not a lib or drop-in). */	  kOnDiskFlat,          /* This code fragment is on disk, in the data fork. */	  kZeroOffset,		    /* Offset of code into data fork. */	  kWholeFork,           /* Code takes up all of data fork (can give a size). */	  "ShLibExample",       /* Name of application. */     'm68k',				/* Target machine's Architecture. */      kFullLib,				/* This is not an update. */	  kNoVersionNum,		/* Current version. */	  kNoVersionNum,		/* Definition version. */	  kDefaultStackSize,	/* Stack size of application. */	  kNoAppSubFolder,		/* Not used here.  Can be the resource-id of an 'alis'	                           resource.  Used to provide additional location							   to search for libraries. */	  kIsApp,				/* This is an application (not a lib or drop-in). */	  kOnDiskSegmented,     /* This code fragment is on disk, in the resource fork. */	  'rseg',		   		/* code is type 'rseg' */	  0,           			/* Size is computed */	  "ShLibExample"        /* Name of application. */   }};resource 'vers' (1) {	0x2,	0x0,	development,	0xd,	verUS,	"2.0d1",	"Version 2.0d1.  Copyright � 1994, Apple Computer, Inc."};
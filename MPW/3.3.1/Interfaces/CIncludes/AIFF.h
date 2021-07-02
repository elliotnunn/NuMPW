/*
	File:		AIFF.h

	Copyright:	© 1984-1993 by Apple Computer, Inc., all rights reserved.

	WARNING
	This file was auto generated by the interfacer tool. Modifications
	must be made to the master file.

*/

#ifndef __AIFF__
#define __AIFF__

#ifndef __TYPES__
#include <Types.h>
/*	#include <ConditionalMacros.h>								*/
/*	#include <MixedMode.h>										*/
/*		#include <Traps.h>										*/
#endif

#define AIFFID 'AIFF'

#define AIFCID 'AIFC'

#define FormatVersionID 'FVER'

#define CommonID 'COMM'

#define FORMID 'FORM'

#define SoundDataID 'SSND'

#define MarkerID 'MARK'

#define InstrumentID 'INST'

#define MIDIDataID 'MIDI'

#define AudioRecordingID 'AESD'

#define ApplicationSpecificID 'APPL'

#define CommentID 'COMT'

#define NameID 'NAME'

#define AuthorID 'AUTH'

#define CopyrightID '(c) '

#define AnnotationID 'ANNO'

enum  {
	NoLooping					= 0,
	ForwardLooping				= 1,
	ForwardBackwardLooping		= 2,
/* AIFF-C Versions */
	AIFCVersion1				= 0xA2805140
};

typedef unsigned long ID;

typedef short MarkerIdType;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct ChunkHeader {
	ID							ckID;
	long						ckSize;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif


/* Compression Names */

#define NoneName "\pnot compressed"

#define ACE2to1Name "\pACE 2-to-1"

#define ACE8to3Name "\pACE 8-to-3"

#define MACE3to1Name "\pMACE 3-to-1"

#define MACE6to1Name "\pMACE 6-to-1"


/* Compression Types */

#define NoneType 'NONE'

#define ACE2Type 'ACE2'

#define ACE8Type 'ACE8'

#define MACE3Type 'MAC3'

#define MACE6Type 'MAC6'

typedef struct ChunkHeader ChunkHeader;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct ContainerChunk {
	ID							ckID;
	long						ckSize;
	ID							formType;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct ContainerChunk ContainerChunk;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct FormatVersionChunk {
	ID							ckID;
	long						ckSize;
	unsigned long				timestamp;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct FormatVersionChunk FormatVersionChunk;

typedef FormatVersionChunk *FormatVersionChunkPtr;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct CommonChunk {
	ID							ckID;
	long						ckSize;
	short						numChannels;
	unsigned long				numSampleFrames;
	short						sampleSize;
	extended80					sampleRate;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct CommonChunk CommonChunk;

typedef CommonChunk *CommonChunkPtr;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct ExtCommonChunk {
	ID							ckID;
	long						ckSize;
	short						numChannels;
	unsigned long				numSampleFrames;
	short						sampleSize;
	extended80					sampleRate;
	ID							compressionType;
	char						compressionName[1];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct ExtCommonChunk ExtCommonChunk;

typedef ExtCommonChunk *ExtCommonChunkPtr;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct SoundDataChunk {
	ID							ckID;
	long						ckSize;
	unsigned long				offset;
	unsigned long				blockSize;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct SoundDataChunk SoundDataChunk;

typedef SoundDataChunk *SoundDataChunkPtr;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct Marker {
	MarkerIdType				id;
	unsigned long				position;
	Str255						markerName;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct Marker Marker;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct MarkerChunk {
	ID							ckID;
	long						ckSize;
	unsigned short				numMarkers;
	Marker						Markers[1];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct MarkerChunk MarkerChunk;

typedef MarkerChunk *MarkerChunkPtr;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct AIFFLoop {
	short						playMode;
	MarkerIdType				beginLoop;
	MarkerIdType				endLoop;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct AIFFLoop AIFFLoop;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct InstrumentChunk {
	ID							ckID;
	long						ckSize;
	char						baseFrequency;
	char						detune;
	char						lowFrequency;
	char						highFrequency;
	char						lowVelocity;
	char						highVelocity;
	short						gain;
	AIFFLoop					sustainLoop;
	AIFFLoop					releaseLoop;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct InstrumentChunk InstrumentChunk;

typedef InstrumentChunk *InstrumentChunkPtr;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct MIDIDataChunk {
	ID							ckID;
	long						ckSize;
	unsigned char				MIDIdata[1];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct MIDIDataChunk MIDIDataChunk;

typedef MIDIDataChunk *MIDIDataChunkPtr;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct AudioRecordingChunk {
	ID							ckID;
	long						ckSize;
	unsigned char				AESChannelStatus[24];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct AudioRecordingChunk AudioRecordingChunk;

typedef AudioRecordingChunk *AudioRecordingChunkPtr;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct ApplicationSpecificChunk {
	ID							ckID;
	long						ckSize;
	OSType						applicationSignature;
	char						data[1];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct ApplicationSpecificChunk ApplicationSpecificChunk;

typedef ApplicationSpecificChunk *ApplicationSpecificChunkPtr;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct Comment {
	unsigned long				timeStamp;
	MarkerIdType				marker;
	unsigned short				count;
	char						text[1];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct Comment Comment;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct CommentsChunk {
	ID							ckID;
	long						ckSize;
	unsigned short				numComments;
	Comment						comments[1];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct CommentsChunk CommentsChunk;

typedef CommentsChunk *CommentsChunkPtr;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TextChunk {
	ID							ckID;
	long						ckSize;
	char						text[1];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TextChunk TextChunk;

typedef TextChunk *TextChunkPtr;

#endif

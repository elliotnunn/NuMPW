/*
     File:       DirectoryCopy.h
 
     Contains:   A robust, general purpose directory copy routine.
 
     Version:    Technology: MoreFiles
                 Release:    1.5.1
 
     Copyright:  © 1992-2001 by Apple Computer, Inc., all rights reserved.
 
     Bugs?:      For bug reports, consult the following page on
                 the World Wide Web:
 
                     http://developer.apple.com/bugreporter/
 
*/

/*
    You may incorporate this sample code into your applications without
    restriction, though the sample code has been provided "AS IS" and the
    responsibility for its operation is 100% yours.  However, what you are
    not permitted to do is to redistribute the source as "DSC Sample Code"
    after having made changes. If you're going to re-distribute the source,
    we require that you make it clear in the source that the code was
    descended from Apple Sample Code, but that you've made changes.
*/

#ifndef __DIRECTORYCOPY__
#define __DIRECTORYCOPY__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif

#ifndef __FILES__
#include <Files.h>
#endif

#include "Optimization.h"


#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_IMPORT
#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
    #pragma pack(2)
#endif

/*****************************************************************************/

enum {
  getNextItemOp                 = 1,    /* couldn't access items in this directory - no access privileges */
  copyDirCommentOp              = 2,    /* couldn't copy directory's Finder comment */
  copyDirAccessPrivsOp          = 3,    /* couldn't copy directory's AFP access privileges */
  copyDirFMAttributesOp         = 4,    /* couldn't copy directory's File Manager attributes */
  dirCreateOp                   = 5,    /* couldn't create destination directory */
  fileCopyOp                    = 6     /* couldn't copy file */
};


/*****************************************************************************/

typedef CALLBACK_API( Boolean , CopyErrProcPtr )(OSErr error, short failedOperation, short srcVRefNum, long srcDirID, ConstStr255Param srcName, short dstVRefNum, long dstDirID, ConstStr255Param dstName);
typedef STACK_UPP_TYPE(CopyErrProcPtr)                          CopyErrUPP;
EXTERN_API_C( CopyErrUPP )
NewCopyErrUPP(CopyErrProcPtr userRoutine);
#if !OPAQUE_UPP_TYPES
  enum { uppCopyErrProcInfo = 0x003EFA90 };  /* pascal 1_byte Func(2_bytes, 2_bytes, 2_bytes, 4_bytes, 4_bytes, 2_bytes, 4_bytes, 4_bytes) */
  #ifdef __cplusplus
    inline CopyErrUPP NewCopyErrUPP(CopyErrProcPtr userRoutine) { return (CopyErrUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCopyErrProcInfo, GetCurrentArchitecture()); }
  #else
    #define NewCopyErrUPP(userRoutine) (CopyErrUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCopyErrProcInfo, GetCurrentArchitecture())
  #endif
#endif

EXTERN_API_C( void )
DisposeCopyErrUPP(CopyErrUPP userUPP);
#if !OPAQUE_UPP_TYPES
  #ifdef __cplusplus
    inline void DisposeCopyErrUPP(CopyErrUPP userUPP) { DisposeRoutineDescriptor((UniversalProcPtr)userUPP); }
  #else
    #define DisposeCopyErrUPP(userUPP) DisposeRoutineDescriptor(userUPP)
  #endif
#endif

EXTERN_API_C( Boolean )
InvokeCopyErrUPP(
  OSErr             error,
  short             failedOperation,
  short             srcVRefNum,
  long              srcDirID,
  ConstStr255Param  srcName,
  short             dstVRefNum,
  long              dstDirID,
  ConstStr255Param  dstName,
  CopyErrUPP        userUPP);
#if !OPAQUE_UPP_TYPES
  #ifdef __cplusplus
    inline Boolean InvokeCopyErrUPP(OSErr error, short failedOperation, short srcVRefNum, long srcDirID, ConstStr255Param srcName, short dstVRefNum, long dstDirID, ConstStr255Param dstName, CopyErrUPP userUPP) { return (Boolean)CALL_EIGHT_PARAMETER_UPP(userUPP, uppCopyErrProcInfo, error, failedOperation, srcVRefNum, srcDirID, srcName, dstVRefNum, dstDirID, dstName); }
  #else
    #define InvokeCopyErrUPP(error, failedOperation, srcVRefNum, srcDirID, srcName, dstVRefNum, dstDirID, dstName, userUPP) (Boolean)CALL_EIGHT_PARAMETER_UPP((userUPP), uppCopyErrProcInfo, (error), (failedOperation), (srcVRefNum), (srcDirID), (srcName), (dstVRefNum), (dstDirID), (dstName))
  #endif
#endif

#if CALL_NOT_IN_CARBON || OLDROUTINENAMES
    /* support for pre-Carbon UPP routines: New...Proc and Call...Proc */
    #define NewCopyErrProc(userRoutine)                         NewCopyErrUPP(userRoutine)
    #define CallCopyErrProc(userRoutine, error, failedOperation, srcVRefNum, srcDirID, srcName, dstVRefNum, dstDirID, dstName) InvokeCopyErrUPP(error, failedOperation, srcVRefNum, srcDirID, srcName, dstVRefNum, dstDirID, dstName, userRoutine)
#endif /* CALL_NOT_IN_CARBON */

/*
    This is the prototype for the CopyErrProc function DirectoryCopy
    calls if an error condition is detected sometime during the copy.  If
    CopyErrProc returns false, then DirectoryCopy attempts to continue with
    the directory copy operation.  If CopyErrProc returns true, then
    DirectoryCopy stops the directory copy operation.

    error           input:  The error result code that caused CopyErrProc to
                            be called.
    failedOperation input:  The operation that returned an error to
                            DirectoryCopy.
    srcVRefNum      input:  Source volume specification.
    srcDirID        input:  Source directory ID.
    srcName         input:  Source file or directory name, or nil if
                            srcDirID specifies the directory.
    dstVRefNum      input:  Destination volume specification.
    dstDirID        input:  Destination directory ID.
    dstName         input:  Destination file or directory name, or nil if
                            dstDirID specifies the directory.

    __________
    
    Also see:   FilteredDirectoryCopy, FSpFilteredDirectoryCopy, DirectoryCopy, FSpDirectoryCopy
*/

/*****************************************************************************/

typedef CALLBACK_API( Boolean , CopyFilterProcPtr )(const CInfoPBRec * cpbPtr);
typedef STACK_UPP_TYPE(CopyFilterProcPtr)                       CopyFilterUPP;
EXTERN_API_C( CopyFilterUPP )
NewCopyFilterUPP(CopyFilterProcPtr userRoutine);
#if !OPAQUE_UPP_TYPES
  enum { uppCopyFilterProcInfo = 0x000000D0 };  /* pascal 1_byte Func(4_bytes) */
  #ifdef __cplusplus
    inline CopyFilterUPP NewCopyFilterUPP(CopyFilterProcPtr userRoutine) { return (CopyFilterUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCopyFilterProcInfo, GetCurrentArchitecture()); }
  #else
    #define NewCopyFilterUPP(userRoutine) (CopyFilterUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCopyFilterProcInfo, GetCurrentArchitecture())
  #endif
#endif

EXTERN_API_C( void )
DisposeCopyFilterUPP(CopyFilterUPP userUPP);
#if !OPAQUE_UPP_TYPES
  #ifdef __cplusplus
    inline void DisposeCopyFilterUPP(CopyFilterUPP userUPP) { DisposeRoutineDescriptor((UniversalProcPtr)userUPP); }
  #else
    #define DisposeCopyFilterUPP(userUPP) DisposeRoutineDescriptor(userUPP)
  #endif
#endif

EXTERN_API_C( Boolean )
InvokeCopyFilterUPP(
  const CInfoPBRec *  cpbPtr,
  CopyFilterUPP       userUPP);
#if !OPAQUE_UPP_TYPES
  #ifdef __cplusplus
    inline Boolean InvokeCopyFilterUPP(const CInfoPBRec * cpbPtr, CopyFilterUPP userUPP) { return (Boolean)CALL_ONE_PARAMETER_UPP(userUPP, uppCopyFilterProcInfo, cpbPtr); }
  #else
    #define InvokeCopyFilterUPP(cpbPtr, userUPP) (Boolean)CALL_ONE_PARAMETER_UPP((userUPP), uppCopyFilterProcInfo, (cpbPtr))
  #endif
#endif

#if CALL_NOT_IN_CARBON || OLDROUTINENAMES
    /* support for pre-Carbon UPP routines: New...Proc and Call...Proc */
    #define NewCopyFilterProc(userRoutine)                      NewCopyFilterUPP(userRoutine)
    #define CallCopyFilterProc(userRoutine, cpbPtr)             InvokeCopyFilterUPP(cpbPtr, userRoutine)
#endif /* CALL_NOT_IN_CARBON */

/*
    This is the prototype for the CopyFilterProc function called by
    FilteredDirectoryCopy and GetLevelSize. If true is returned,
    the file/folder is included in the copy, otherwise it is excluded.
    
    pb  input:  Points to the CInfoPBRec for the item under consideration.

    __________
    
    Also see:   FilteredDirectoryCopy, FSpFilteredDirectoryCopy
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FilteredDirectoryCopy(
  short               srcVRefNum,
  long                srcDirID,
  ConstStr255Param    srcName,
  short               dstVRefNum,
  long                dstDirID,
  ConstStr255Param    dstName,
  ConstStr255Param    copyName,
  void *              copyBufferPtr,
  long                copyBufferSize,
  Boolean             preflight,
  CopyErrProcPtr      copyErrHandler,
  CopyFilterProcPtr   copyFilterProc);


/*
    The FilteredDirectoryCopy function makes a copy of a directory
    structure in a new location. If copyBufferPtr <> NIL, it points to
    a buffer of copyBufferSize that is used to copy files data. The
    larger the supplied buffer, the faster the copy. If
    copyBufferPtr = NIL, then this routine allocates a buffer in the
    application heap. If you pass a copy buffer to this routine, make
    its size a multiple of 512 ($200) bytes for optimum performance.
    
    The optional copyFilterProc parameter lets a routine you define
    decide what files or directories are copied to the destination.
    
    FilteredDirectoryCopy normally creates a new directory *in* the
    specified destination directory and copies the source directory's
    content into the new directory. However, if root parent directory
    (fsRtParID) is passed as the dstDirID parameter and NULL is
    passed as the dstName parameter, DirectoryCopy renames the
    destination volume to the source directory's name (truncating
    if the name is longer than 27 characters) and copies the source
    directory's content into the destination volume's root directory.
    This special case is supported by FilteredDirectoryCopy, but
    not by FSpFilteredDirectoryCopy since with FSpFilteredDirectoryCopy,
    the dstName parameter can not be NULL.
    
    srcVRefNum      input:  Source volume specification.
    srcDirID        input:  Source directory ID.
    srcName         input:  Source directory name, or nil if
                            srcDirID specifies the directory.
    dstVRefNum      input:  Destination volume specification.
    dstDirID        input:  Destination directory ID.
    dstName         input:  Destination directory name, or nil if
                            dstDirID specifies the directory.
    copyName        input:  Points to the new directory name if the directory
                            is to be renamed or nil if the directory isn't to
                            be renamed.
    copyBufferPtr   input:  Points to a buffer of copyBufferSize that
                            is used the i/o buffer for the copy or
                            nil if you want DirectoryCopy to allocate its
                            own buffer in the application heap.
    copyBufferSize  input:  The size of the buffer pointed to
                            by copyBufferPtr.
    preflight       input:  If true, DirectoryCopy makes sure there are
                            enough allocation blocks on the destination
                            volume to hold the directory's files before
                            starting the copy.
    copyErrHandler  input:  A pointer to the routine you want called if an
                            error condition is detected during the copy, or
                            nil if you don't want to handle error conditions.
                            If you don't handle error conditions, the first
                            error will cause the copy to quit and
                            DirectoryCopy will return the error.
                            Error handling is recommended...
    copyFilterProc  input:  A pointer to the filter routine you want called
                            for each item in the source directory, or NULL
                            if you don't want to filter.
    
    Result Codes
        noErr               0       No error
        readErr             –19     Driver does not respond to read requests
        writErr             –20     Driver does not respond to write requests
        badUnitErr          –21     Driver reference number does not
                                    match unit table
        unitEmptyErr        –22     Driver reference number specifies a
                                    nil handle in unit table
        abortErr            –27     Request aborted by KillIO
        notOpenErr          –28     Driver not open
        dskFulErr           -34     Destination volume is full
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        tmfoErr             -42     Too many files open
        fnfErr              -43     Source file not found, or destination
                                    directory does not exist
        wPrErr              -44     Volume locked by hardware
        fLckdErr            -45     File is locked
        vLckdErr            -46     Destination volume is read-only
        fBsyErr             -47     The source or destination file could
                                    not be opened with the correct access
                                    modes
        dupFNErr            -48     Destination file already exists
        opWrErr             -49     File already open for writing
        paramErr            -50     No default volume or function not
                                    supported by volume
        permErr             -54     File is already open and cannot be opened using specified deny modes
        memFullErr          -108    Copy buffer could not be allocated
        dirNFErr            -120    Directory not found or incomplete pathname
        wrgVolTypErr        -123    Function not supported by volume
        afpAccessDenied     -5000   User does not have the correct access
        afpDenyConflict     -5006   The source or destination file could
                                    not be opened with the correct access
                                    modes
        afpObjectTypeErr    -5025   Source is a directory, directory not found
                                    or incomplete pathname
    
    __________
    
    Also see:   CopyErrProcPtr, CopyFilterProcPtr, FSpFilteredDirectoryCopy,
                DirectoryCopy, FSpDirectoryCopy, FileCopy, FSpFileCopy
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpFilteredDirectoryCopy(
  const FSSpec *      srcSpec,
  const FSSpec *      dstSpec,
  ConstStr255Param    copyName,
  void *              copyBufferPtr,
  long                copyBufferSize,
  Boolean             preflight,
  CopyErrProcPtr      copyErrHandler,
  CopyFilterProcPtr   copyFilterProc);


/*
    The FSpFilteredDirectoryCopy function makes a copy of a directory
    structure in a new location. If copyBufferPtr <> NIL, it points to
    a buffer of copyBufferSize that is used to copy files data. The
    larger the supplied buffer, the faster the copy. If
    copyBufferPtr = NIL, then this routine allocates a buffer in the
    application heap. If you pass a copy buffer to this routine, make
    its size a multiple of 512 ($200) bytes for optimum performance.
    
    The optional copyFilterProc parameter lets a routine you define
    decide what files or directories are copied to the destination.
    
    srcSpec         input:  An FSSpec record specifying the directory to copy.
    dstSpec         input:  An FSSpec record specifying destination directory
                            of the copy.
    copyName        input:  Points to the new directory name if the directory
                            is to be renamed or nil if the directory isn't to
                            be renamed.
    copyBufferPtr   input:  Points to a buffer of copyBufferSize that
                            is used the i/o buffer for the copy or
                            nil if you want DirectoryCopy to allocate its
                            own buffer in the application heap.
    copyBufferSize  input:  The size of the buffer pointed to
                            by copyBufferPtr.
    preflight       input:  If true, FSpDirectoryCopy makes sure there are
                            enough allocation blocks on the destination
                            volume to hold the directory's files before
                            starting the copy.
    copyErrHandler  input:  A pointer to the routine you want called if an
                            error condition is detected during the copy, or
                            nil if you don't want to handle error conditions.
                            If you don't handle error conditions, the first
                            error will cause the copy to quit and
                            DirectoryCopy will return the error.
                            Error handling is recommended...
    copyFilterProc  input:  A pointer to the filter routine you want called
                            for each item in the source directory, or NULL
                            if you don't want to filter.
    
    Result Codes
        noErr               0       No error
        readErr             –19     Driver does not respond to read requests
        writErr             –20     Driver does not respond to write requests
        badUnitErr          –21     Driver reference number does not
                                    match unit table
        unitEmptyErr        –22     Driver reference number specifies a
                                    nil handle in unit table
        abortErr            –27     Request aborted by KillIO
        notOpenErr          –28     Driver not open
        dskFulErr           -34     Destination volume is full
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        tmfoErr             -42     Too many files open
        fnfErr              -43     Source file not found, or destination
                                    directory does not exist
        wPrErr              -44     Volume locked by hardware
        fLckdErr            -45     File is locked
        vLckdErr            -46     Destination volume is read-only
        fBsyErr             -47     The source or destination file could
                                    not be opened with the correct access
                                    modes
        dupFNErr            -48     Destination file already exists
        opWrErr             -49     File already open for writing
        paramErr            -50     No default volume or function not
                                    supported by volume
        permErr             -54     File is already open and cannot be opened using specified deny modes
        memFullErr          -108    Copy buffer could not be allocated
        dirNFErr            -120    Directory not found or incomplete pathname
        wrgVolTypErr        -123    Function not supported by volume
        afpAccessDenied     -5000   User does not have the correct access
        afpDenyConflict     -5006   The source or destination file could
                                    not be opened with the correct access
                                    modes
        afpObjectTypeErr    -5025   Source is a directory, directory not found
                                    or incomplete pathname
    
    __________
    
    Also see:   CopyErrProcPtr, CopyFilterProcPtr, FilteredDirectoryCopy,
                DirectoryCopy, FSpDirectoryCopy, FileCopy, FSpFileCopy
*/

/*****************************************************************************/

EXTERN_API( OSErr )
DirectoryCopy(
  short              srcVRefNum,
  long               srcDirID,
  ConstStr255Param   srcName,
  short              dstVRefNum,
  long               dstDirID,
  ConstStr255Param   dstName,
  ConstStr255Param   copyName,
  void *             copyBufferPtr,
  long               copyBufferSize,
  Boolean            preflight,
  CopyErrProcPtr     copyErrHandler);


/*
    The DirectoryCopy function makes a copy of a directory structure in a
    new location. If copyBufferPtr <> NIL, it points to a buffer of
    copyBufferSize that is used to copy files data.  The larger the
    supplied buffer, the faster the copy.  If copyBufferPtr = NIL, then this
    routine allocates a buffer in the application heap. If you pass a
    copy buffer to this routine, make its size a multiple of 512
    ($200) bytes for optimum performance.
    
    DirectoryCopy normally creates a new directory *in* the specified
    destination directory and copies the source directory's content into
    the new directory. However, if root parent directory (fsRtParID)
    is passed as the dstDirID parameter and NULL is passed as the
    dstName parameter, DirectoryCopy renames the destination volume to
    the source directory's name (truncating if the name is longer than
    27 characters) and copies the source directory's content into the
    destination volume's root directory. This special case is supported
    by DirectoryCopy, but not by FSpDirectoryCopy since with
    FSpDirectoryCopy, the dstName parameter can not be NULL.
    
    srcVRefNum      input:  Source volume specification.
    srcDirID        input:  Source directory ID.
    srcName         input:  Source directory name, or nil if
                            srcDirID specifies the directory.
    dstVRefNum      input:  Destination volume specification.
    dstDirID        input:  Destination directory ID.
    dstName         input:  Destination directory name, or nil if
                            dstDirID specifies the directory.
    copyName        input:  Points to the new directory name if the directory
                            is to be renamed or nil if the directory isn't to
                            be renamed.
    copyBufferPtr   input:  Points to a buffer of copyBufferSize that
                            is used the i/o buffer for the copy or
                            nil if you want DirectoryCopy to allocate its
                            own buffer in the application heap.
    copyBufferSize  input:  The size of the buffer pointed to
                            by copyBufferPtr.
    preflight       input:  If true, DirectoryCopy makes sure there are
                            enough allocation blocks on the destination
                            volume to hold the directory's files before
                            starting the copy.
    copyErrHandler  input:  A pointer to the routine you want called if an
                            error condition is detected during the copy, or
                            nil if you don't want to handle error conditions.
                            If you don't handle error conditions, the first
                            error will cause the copy to quit and
                            DirectoryCopy will return the error.
                            Error handling is recommended...
    
    Result Codes
        noErr               0       No error
        readErr             –19     Driver does not respond to read requests
        writErr             –20     Driver does not respond to write requests
        badUnitErr          –21     Driver reference number does not
                                    match unit table
        unitEmptyErr        –22     Driver reference number specifies a
                                    nil handle in unit table
        abortErr            –27     Request aborted by KillIO
        notOpenErr          –28     Driver not open
        dskFulErr           -34     Destination volume is full
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        tmfoErr             -42     Too many files open
        fnfErr              -43     Source file not found, or destination
                                    directory does not exist
        wPrErr              -44     Volume locked by hardware
        fLckdErr            -45     File is locked
        vLckdErr            -46     Destination volume is read-only
        fBsyErr             -47     The source or destination file could
                                    not be opened with the correct access
                                    modes
        dupFNErr            -48     Destination file already exists
        opWrErr             -49     File already open for writing
        paramErr            -50     No default volume or function not
                                    supported by volume
        permErr             -54     File is already open and cannot be opened using specified deny modes
        memFullErr          -108    Copy buffer could not be allocated
        dirNFErr            -120    Directory not found or incomplete pathname
        wrgVolTypErr        -123    Function not supported by volume
        afpAccessDenied     -5000   User does not have the correct access
        afpDenyConflict     -5006   The source or destination file could
                                    not be opened with the correct access
                                    modes
        afpObjectTypeErr    -5025   Source is a directory, directory not found
                                    or incomplete pathname
    
    __________
    
    Also see:   CopyErrProcPtr, FSpDirectoryCopy, FilteredDirectoryCopy,
                FSpFilteredDirectoryCopy, FileCopy, FSpFileCopy
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpDirectoryCopy(
  const FSSpec *     srcSpec,
  const FSSpec *     dstSpec,
  ConstStr255Param   copyName,
  void *             copyBufferPtr,
  long               copyBufferSize,
  Boolean            preflight,
  CopyErrProcPtr     copyErrHandler);


/*
    The FSpDirectoryCopy function makes a copy of a directory structure in a
    new location. If copyBufferPtr <> NIL, it points to a buffer of
    copyBufferSize that is used to copy files data.  The larger the
    supplied buffer, the faster the copy.  If copyBufferPtr = NIL, then this
    routine allocates a buffer in the application heap. If you pass a
    copy buffer to this routine, make its size a multiple of 512
    ($200) bytes for optimum performance.
    
    srcSpec         input:  An FSSpec record specifying the directory to copy.
    dstSpec         input:  An FSSpec record specifying destination directory
                            of the copy.
    copyName        input:  Points to the new directory name if the directory
                            is to be renamed or nil if the directory isn't to
                            be renamed.
    copyBufferPtr   input:  Points to a buffer of copyBufferSize that
                            is used the i/o buffer for the copy or
                            nil if you want DirectoryCopy to allocate its
                            own buffer in the application heap.
    copyBufferSize  input:  The size of the buffer pointed to
                            by copyBufferPtr.
    preflight       input:  If true, FSpDirectoryCopy makes sure there are
                            enough allocation blocks on the destination
                            volume to hold the directory's files before
                            starting the copy.
    copyErrHandler  input:  A pointer to the routine you want called if an
                            error condition is detected during the copy, or
                            nil if you don't want to handle error conditions.
                            If you don't handle error conditions, the first
                            error will cause the copy to quit and
                            DirectoryCopy will return the error.
                            Error handling is recommended...
    
    Result Codes
        noErr               0       No error
        readErr             –19     Driver does not respond to read requests
        writErr             –20     Driver does not respond to write requests
        badUnitErr          –21     Driver reference number does not
                                    match unit table
        unitEmptyErr        –22     Driver reference number specifies a
                                    nil handle in unit table
        abortErr            –27     Request aborted by KillIO
        notOpenErr          –28     Driver not open
        dskFulErr           -34     Destination volume is full
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        tmfoErr             -42     Too many files open
        fnfErr              -43     Source file not found, or destination
                                    directory does not exist
        wPrErr              -44     Volume locked by hardware
        fLckdErr            -45     File is locked
        vLckdErr            -46     Destination volume is read-only
        fBsyErr             -47     The source or destination file could
                                    not be opened with the correct access
                                    modes
        dupFNErr            -48     Destination file already exists
        opWrErr             -49     File already open for writing
        paramErr            -50     No default volume or function not
                                    supported by volume
        permErr             -54     File is already open and cannot be opened using specified deny modes
        memFullErr          -108    Copy buffer could not be allocated
        dirNFErr            -120    Directory not found or incomplete pathname
        wrgVolTypErr        -123    Function not supported by volume
        afpAccessDenied     -5000   User does not have the correct access
        afpDenyConflict     -5006   The source or destination file could
                                    not be opened with the correct access
                                    modes
        afpObjectTypeErr    -5025   Source is a directory, directory not found
                                    or incomplete pathname
    
    __________
    
    Also see:   CopyErrProcPtr, DirectoryCopy, FilteredDirectoryCopy,
                FSpFilteredDirectoryCopy, FileCopy, FSpFileCopy
*/

/*****************************************************************************/

#include "OptimizationEnd.h"

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
    #pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DIRECTORYCOPY__ */

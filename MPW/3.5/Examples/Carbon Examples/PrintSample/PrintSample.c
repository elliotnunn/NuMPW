/*
	File:		PrintSample.c

	Contains:	This sample code is the Carbon equivalent of the classic print loop
				documented in Tech Note 1092 "A Print Loop That Cares ...".  This code
				illustrates the use of functions defined in PMApplication.h instead of
				Printing.h.
	
	Change History:
		17 Oct 00	pd	Tested with several classic print drivers 
					(incl. LaserWriter 8, several HP DeskJet and Epson Stylus drivers)
		17 Oct 99	pd	Updated for Carbon 1.1 SDK to use session APIs
		13 Sep 99	pd	Updated for Carbon 1.0 SDK
		11 Apr 99	dk	Original version for Developer Preview 1

	Version:    1.0b1, CarbonLib 1.1 SDK
	
	Technology:	Carbon Printing for Mac OS 8, 9 & X

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

	© 1998-2001 by Apple Computer, Inc. All rights reserved.
*/


#define TARGET_API_MAC_CARBON 1

#ifndef TARGET_API_MAC_OSX
#define TARGET_API_MAC_OSX 0
#define USE_UMBRELLA_HEADERS 0
#endif

#if TARGET_API_MAC_OSX
#if USE_UMBRELLA_HEADERS
	#include <Carbon/Carbon.h>
#else
	#include <Print/PMApplication.h>
#endif
#else
	#include <PMApplication.h>
#endif

//	Define INCLUDE_POSTSCRIPT to exercise the LaserWriter 8 compatibility path
//	on Mac OS X (see IncludePostScriptInSpoolFile and DrawPage functions).
//#define	INCLUDE_POSTSCRIPT 1


/*------------------------------------------------------------------------------
	Globals
------------------------------------------------------------------------------*/
static	Handle	gflatPageFormat = NULL;		// used in FlattenAndSavePageFormat


/*------------------------------------------------------------------------------
	Prototypes
------------------------------------------------------------------------------*/
int			main(void);
OSStatus 	DoPageSetupDialog(PMPrintSession printSession, PMPageFormat* pageFormat);
OSStatus 	DoPrintDialog(PMPrintSession printSession, PMPageFormat pageFormat,
				PMPrintSettings* printSettings);
void		DoPrintLoop(PMPrintSession printSession, PMPageFormat pageFormat,
				PMPrintSettings printSettings);
OSStatus 	FlattenAndSavePageFormat(PMPageFormat pageFormat);
OSStatus 	LoadAndUnflattenPageFormat(PMPageFormat* pageFormat);
OSStatus	DetermineNumberOfPagesInDoc(PMPageFormat pageFormat, UInt32* numPages);
void 		DrawPage(PMPrintSession printSession, UInt32 pageNumber, Boolean addPostScript);
Boolean		IncludePostScriptInSpoolFile(PMPrintSession printSession);
void 		PostPrintingErrors(OSStatus status);


/*------------------------------------------------------------------------------
    Function:	main
	
    Parameters:
        <none>
	
    Description:
        Uses PMCreateSession/PMRelease instead of PMBegin/PMEnd.  Note that the two
        printing objects, PMPageSetup and PMPrintSettings are valid outside the
        printing session.  This was not the case with PMBegin/PMEnd in the previous
        version of this sample code.  Note also that no nesting of printing sessions
        is allowed for Carbon applications running under MacOS 8 or 9.
        In this sample code, we show where an old print record could be read in and
        converted.  This is followed by displaying the Page Setup dialog, then the
        Print dialog, and finally we print the pages in the document.
	
------------------------------------------------------------------------------*/
int main(void)
{
    OSStatus		status = noErr;
    PMPageFormat	pageFormat = kPMNoPageFormat;
    PMPrintSettings	printSettings = kPMNoPrintSettings;
    PMPrintSession	printSession;
    Handle			oldPrintRecord = NULL;	// place holder for old print record

    //	Standard Toolbox initialization.
    InitCursor();
		
    //	Initialize the printing manager and create a printing session.
    status = PMCreateSession(&printSession);
    if (status != noErr) return 0;	// pointless to continue if PMCreateSession fails
    
    //	If your application has an old print record, it can be converted into new
    //	PMPageFormat and PMPrintSettings objects.  In this sample code, we skip this
    //	step.
    if (oldPrintRecord != NULL)
        status = PMSessionConvertOldPrintRecord(printSession, oldPrintRecord,
                    &printSettings, &pageFormat);
                                                    
    //	Display the Page Setup dialog.
    if (status == noErr)
        status = DoPageSetupDialog(printSession, &pageFormat);
                            
    //	Display the Print dialog.
    if (status == noErr)
        status = DoPrintDialog(printSession, pageFormat, &printSettings);
    
    //	Execute the print loop.
    if (status == noErr)
        DoPrintLoop(printSession, pageFormat, printSettings);
                    
    //	Release the PageFormat and PrintSettings objects.  PMRelease decrements the
    //	ref count of the allocated objects.  We let the Printing Manager decide when
    //	to release the allocated memory.
    if (pageFormat != kPMNoPageFormat)
        (void)PMRelease(pageFormat);
    if (printSettings != kPMNoPrintSettings)
        (void)PMRelease(printSettings);
    
    //	Terminate the current printing session. 
    (void)PMRelease(printSession);
    
    return 0;
    
}	// main



/*------------------------------------------------------------------------------

    Function:	DoPageSetupDialog
    
    Parameters:
        printSession	-	current printing session
        pageFormat		-	a PageFormat object addr
    
    Description:
        If the caller passes in an empty PageFormat object, DoPageSetupDialog
        creates a new one, otherwise it validates the one provided by the caller.
        It then invokes the Page Setup dialog and checks for Cancel. Finally it
        flattens the PageFormat object so it can be saved with the document.
        Note that the PageFormat object is modified by this function.
	
------------------------------------------------------------------------------*/
OSStatus 	DoPageSetupDialog(PMPrintSession printSession, PMPageFormat* pageFormat)
{
	OSStatus	status;
	Boolean		accepted;
	
	//	Set up a valid PageFormat object.
	if (*pageFormat == kPMNoPageFormat)
            {
            status = PMCreatePageFormat(pageFormat);
		
            //	Note that PMPageFormat is not session-specific, but calling
            //	PMSessionDefaultPageFormat assigns values specific to the printer
            //	associated with the current printing session.
            if ((status == noErr) && (*pageFormat != kPMNoPageFormat))
                status = PMSessionDefaultPageFormat(printSession, *pageFormat);
            }
	else
            status = PMSessionValidatePageFormat(printSession, *pageFormat, kPMDontWantBoolean);

	//	Display the Page Setup dialog.	
	if (status == noErr)
            {
            status = PMSessionPageSetupDialog(printSession, *pageFormat, &accepted);
            if (!accepted)
                status = kPMCancel;		// user clicked Cancel button
            }	
				
	//	If the user did not cancel, flatten and save the PageFormat object
	//	with our document.
	if (status == noErr)
            status = FlattenAndSavePageFormat(*pageFormat);

	return status;
	
}	//	DoPageSetupDialog



/*------------------------------------------------------------------------------
	Function:	DoPrintDialog
		
	Parameters:
		printSession	-	current printing session
		pageFormat		-	a PageFormat object addr
		printSettings	-	a PrintSettings object addr
			
	Description:
		If the caller passes an empty PrintSettings object, DoPrintDialog creates
		a new one, otherwise it validates the one provided by the caller.
		It then invokes the Print dialog and checks for Cancel.
		Note that the PrintSettings object is modified by this function.
		
------------------------------------------------------------------------------*/
OSStatus 	DoPrintDialog(PMPrintSession printSession, PMPageFormat pageFormat,
				PMPrintSettings* printSettings)
{
	OSStatus	status;
	Boolean		accepted;
	UInt32		realNumberOfPagesinDoc;
	
	//	In this sample code the caller provides a valid PageFormat reference but in
	//	your application you may want to load and unflatten the PageFormat object
	//	that was saved at PageSetup time.  See LoadAndUnflattenPageFormat below.
	
	//	Set up a valid PrintSettings object.
	if (*printSettings == kPMNoPrintSettings)
            {
            status = PMCreatePrintSettings(printSettings);	

            //	Note that PMPrintSettings is not session-specific, but calling
            //	PMSessionDefaultPrintSettings assigns values specific to the printer
            //	associated with the current printing session.
            if ((status == noErr) && (*printSettings != kPMNoPrintSettings))
                status = PMSessionDefaultPrintSettings(printSession, *printSettings);
            }
	else
            status = PMSessionValidatePrintSettings(printSession, *printSettings, kPMDontWantBoolean);
	
	//	Before displaying the Print dialog, we calculate the number of pages in the
	//	document.  On Mac OS X this is useful because we can prime the Print dialog
	//	with the actual page range of the document and prevent the user from entering
	//	out-of-range numbers.  This is not possible on Mac OS 8 and 9 because the driver,
	//	not the printing manager, controls the page range fields in the Print dialog.

	//	Calculate the number of pages required to print the entire document.
	if (status == noErr)
			status = DetermineNumberOfPagesInDoc(pageFormat, &realNumberOfPagesinDoc);

	//	Set a valid page range before displaying the Print dialog
	if (status == noErr)
            status = PMSetPageRange(*printSettings, 1, realNumberOfPagesinDoc);

	//	Display the Print dialog.
	if (status == noErr)
            {
            status = PMSessionPrintDialog(printSession, *printSettings, pageFormat, &accepted);
            if (!accepted)
                status = kPMCancel;		// user clicked Cancel button
            }
		
	return status;
	
}	//	DoPrintDialog


/*------------------------------------------------------------------------------
	Function:
		DoPrintLoop
	
	Parameters:
		printSession	-	current printing session
		pageFormat		-	a PageFormat object addr
		printSettings	-	a PrintSettings object addr
	
	Description:
		DoPrintLoop calculates which pages to print and executes the print
		loop, calling DrawPage for each page.
				
------------------------------------------------------------------------------*/
void	DoPrintLoop(PMPrintSession printSession, PMPageFormat pageFormat,
            PMPrintSettings printSettings)
{
	OSStatus	status,
                printError;
	Boolean		addPostScript = false;
	GrafPtr		currPort, printingPort;
    UInt32		realNumberOfPagesinDoc,
                pageNumber,
                firstPage,
                lastPage;
	CFStringRef	jobName = CFSTR("Carbon Printing Sample");

	//	Since this sample code doesn't have a window, give the spool file a name.
	status = PMSetJobNameCFString(printSettings, jobName);

	//	Get the user's Print dialog selection for first and last pages to print.
	if (status == noErr)
            {
            status = PMGetFirstPage(printSettings, &firstPage);
            if (status == noErr)
                status = PMGetLastPage(printSettings, &lastPage);
            }

	//	Check that the selected page range does not exceed the actual number of
	//	pages in the document.
	if (status == noErr)
            {
			status = DetermineNumberOfPagesInDoc(pageFormat, &realNumberOfPagesinDoc);
			if (realNumberOfPagesinDoc < lastPage)
                	lastPage = realNumberOfPagesinDoc;
            }

	//	Before executing the print loop, tell the Carbon Printing Manager which pages
	//	will be spooled so that the progress dialog can reflect an accurate page count.
	//	This is recommended on Mac OS X.  On Mac OS 8 and 9, we have no control over
	//	what the printer driver displays.
	
	if (status == noErr)
		status = PMSetFirstPage(printSettings, firstPage, false);
	if (status == noErr)
		status = PMSetLastPage(printSettings, lastPage, false);
    
#if	INCLUDE_POSTSCRIPT
    //	Check if we can add PostScript to the spool file
    if (status == noErr)
    	addPostScript = IncludePostScriptInSpoolFile(printSession);
#endif
	
	//	Note, we don't have to worry about the number of copies.  The printing
	//	manager handles this.  So we just iterate through the document from the
	//	first page to be printed, to the last.
	if (status == noErr)
            {
            //	Establish a graphics context for drawing the document's pages.
            status = PMSessionBeginDocument(printSession, printSettings, pageFormat);
            if (status == noErr)
				{
                //	Print the selected range of pages in the document.		
                pageNumber = firstPage;
                while ((pageNumber <= lastPage) && (PMSessionError(printSession) == noErr))
                    {
                    //	Note, we don't have to deal with the classic Printing Manager's
                    //	128-page boundary limit.
				
                    //	Set up a page for printing.  Under the classic Printing Manager, applications
                    //	could provide a page rect different from the one in the print record to achieve
                    //	scaling. This is no longer recommended and on Mac OS X, the PageRect argument
                    //	is ignored.
                    status = PMSessionBeginPage(printSession, pageFormat, NULL);
                    if (status != noErr)
                        break;
                    
					//	Save the current QD grafport.
					GetPort(&currPort);
                    
                    //	Get the current graphics context, in this case a Quickdraw grafPort,
                    //	for drawing the page.
                    status = PMSessionGetGraphicsContext(printSession, kPMGraphicsContextQuickdraw,
                    	(void**) &printingPort);
                    if (status == noErr) {
                    
	                    //	Set the printing port before drawing the page.
	                    SetPort((GrafPtr) printingPort);
					
 	                   //	Draw the page.
                	    DrawPage(printSession, pageNumber, addPostScript);

						//	Restore the QD grafport.
						SetPort(currPort);
						}
				
                    //	Close the page.
                    status = PMSessionEndPage(printSession);
                    if (status != noErr)
                        break;
					
                    //	And loop.
                        pageNumber++;
                    }
			
                // Close the printing port.  This dismisses the progress dialog on Mac OS X.
                (void)PMSessionEndDocument(printSession);
		}
            }
		
	//	Only report a printing error once we have completed the print loop. This
	//	ensures that every PMBeginXXX call is followed by a matching PMEndXXX
	//	call, so the Printing Manager can release all temporary memory and close
	//	properly.
	printError = PMSessionError(printSession);
	if (printError != noErr && printError != kPMCancel)
            PostPrintingErrors(printError);
		
}	//	DoPrintLoop



/*------------------------------------------------------------------------------
	Function:
		FlattenAndSavePageFormat
	
	Parameters:
		pageFormat	-	a PageFormat object
	
	Description:
		Flattens a PageFormat object so it can be saved with the document.
		Assumes caller passes a validated PageFormat object.
		
------------------------------------------------------------------------------*/
OSStatus FlattenAndSavePageFormat(PMPageFormat pageFormat)
{
    OSStatus	status;
    Handle	flatFormatHandle = NULL;
	
    //	Flatten the PageFormat object to memory.
    status = PMFlattenPageFormat(pageFormat, &flatFormatHandle);
	
    //	Write the PageFormat data to file.
    //	In this sample code we simply copy it to a global.	
    gflatPageFormat = flatFormatHandle;

    return status;
}	//	FlattenAndSavePageFormat



/*------------------------------------------------------------------------------
    Function:	LoadAndUnflattenPageFormat
	
    Parameters:
        pageFormat	- PageFormat object read from document file
	
    Description:
        Gets flattened PageFormat data from the document and returns a PageFormat
        object.
        The function is not called in this sample code but your application
        will need to retrieve PageFormat data saved with documents.
		
------------------------------------------------------------------------------*/
OSStatus	LoadAndUnflattenPageFormat(PMPageFormat* pageFormat)
{
    OSStatus	status;
    Handle	flatFormatHandle = NULL;

    //	Read the PageFormat flattened data from file.
    //	In this sample code we simply copy it from a global.
    flatFormatHandle = gflatPageFormat;

    //	Convert the PageFormat flattened data into a PageFormat object.
    status = PMUnflattenPageFormat(flatFormatHandle, pageFormat);
	
    return status;
}	//	LoadAndUnflattenPageFormat



/*------------------------------------------------------------------------------
    Function:	DetermineNumberOfPagesInDoc
	
    Parameters:
    	pageFormat	- a PageFormat object addr
		numPages	- on return, the size of the document in pages
			
    Description:
    	Calculates the number of pages needed to print the entire document.
		
------------------------------------------------------------------------------*/
OSStatus	DetermineNumberOfPagesInDoc(PMPageFormat pageFormat, UInt32* numPages)
{
    OSStatus	status;
    PMRect		pageRect;

	//	PMGetAdjustedPageRect returns the page size taking into account rotation,
	//	resolution and scaling settings.
	status = PMGetAdjustedPageRect(pageFormat, &pageRect);

	//	In this sample code we simply return a hard coded number.  In your application,
	//	you will need to figure out how many page rects are needed to image the
	//	current document.
	*numPages = 1;

    return status;
    
}	//	DetermineNumberOfPagesinDoc



/*------------------------------------------------------------------------------
    Function:	DrawPage
	
    Parameters:
        printSession	- current printing session
        pageNumber		- the logical page number in the document
        addPostScript	- flag to enable PostScript code
	
    Description:
        Draws the contents of a single page.  If addPostScript is true, DrawPage
        adds PostScript code into the spool file.  See the Printing chapter in
        Inside Macintosh, Imaging with QuickDraw, for details about PostScript
        PicComments.
		
------------------------------------------------------------------------------*/
void DrawPage(PMPrintSession printSession, UInt32 pageNumber, Boolean addPostScript)
{
#pragma unused (pageNumber)	
    //	In this sample code we do some very simple text drawing.    
    MoveTo(72,72);
    TextFont(kFontIDHelvetica);
    TextSize(12);
	DrawString("\psome QuickDraw text");
	
    //	Conditionally insert PostScript into the spool file.
    if (addPostScript)
    	{
	    OSStatus	status;
	    
	    // The following PostScript code handles the transformation to QuickDraw's coordinate system
	    // assuming a letter size page.
	    Str255		postScriptStr1 = "\p0 792 translate 1 -1 scale \r";
	    
	    // Set the current PostScript font to Times and draw some more text.
	    Str255		postScriptStr2 = "\p/Times-Roman findfont 12 scalefont setfont \r";
	    Str255		postScriptStr3 = "\p( and some PostScript text) show\r";
       	    
	    status = PMSessionPostScriptBegin(printSession);
   	    if (status == noErr)
			{
        	(void) PMSessionPostScriptData(printSession, (char *)&postScriptStr1[1], postScriptStr1[0]);
			(void) PMSessionPostScriptData(printSession, (char *)&postScriptStr2[1], postScriptStr2[0]);
			(void) PMSessionPostScriptData(printSession, (char *)&postScriptStr3[1], postScriptStr3[0]);
        	(void) PMSessionPostScriptEnd(printSession);
        	}
    	}   
		
}	//	DrawPage



/*------------------------------------------------------------------------------
    Function:	IncludePostScriptInSpoolFile
	
    Parameters:
        printSession	- current printing session
	
    Description:
    	Check if current printer driver supports embedding of PostScript in the spool file, and
		if it does, instruct the Carbon Printing Manager to generate a PICT w/ PS spool file.
                		
------------------------------------------------------------------------------*/
Boolean IncludePostScriptInSpoolFile(PMPrintSession printSession)
{
    Boolean	includePostScript = false;
    OSStatus	status;
    CFArrayRef	supportedFormats = NULL;
    SInt32	i, numSupportedFormats;

    // Get the list of spool file formats supported by the current driver.
    // PMSessionGetDocumentFormatGeneration returns the list of formats which can be generated
    // by the spooler (client-side) AND converted by the despooler (server-side).
    // PMSessionGetDocumentFormatSupported only returns the list of formats which can be converted
    // by the despooler.  The third argument to PMSessionGetDocumentFormatSupported specifies the
    // maximum number of formats to be returned in the list; zero gets every one.
    
    status = PMSessionGetDocumentFormatGeneration(printSession, &supportedFormats);
    if (status == noErr)
        {
        // Check if PICT w/ PS is in the list of supported formats.
        numSupportedFormats = CFArrayGetCount(supportedFormats);
        
        for (i=0; i<numSupportedFormats; i++)
            {
            if ( CFStringCompare(CFArrayGetValueAtIndex(supportedFormats, i),
                kPMDocumentFormatPICTPS, kCFCompareCaseInsensitive) == kCFCompareEqualTo )
                {
                // PICT w/ PS is supported, so tell the Printing Mgr to generate a PICT w/ PS spool file
                
				// Build an array of graphics contexts containing just one type, Quickdraw,
				// meaning that we will be using a QD port to image our pages in the print loop.
				CFStringRef	strings[1];
				CFArrayRef	arrayOfGraphicsContexts;
				
				strings[0] = kPMGraphicsContextQuickdraw;
				arrayOfGraphicsContexts = CFArrayCreate(CFAllocatorGetDefault(),
					(const void **)strings, 1, &kCFTypeArrayCallBacks);
										
				if (arrayOfGraphicsContexts != NULL)
					{
					// Request a PICT w/ PS spool file
					status = PMSessionSetDocumentFormatGeneration(printSession, kPMDocumentFormatPICTPS, 
						arrayOfGraphicsContexts, NULL);
					
					if (status == noErr)
						 includePostScript = true;	// Enable use of PS PicComments in DrawPage.

					// Deallocate the array used for the list of graphics contexts.
					CFRelease(arrayOfGraphicsContexts);
					}
				
				break;
				}
            }
                    
		// Deallocate the array used for the list of supported spool file formats.
        CFRelease(supportedFormats);
	}
            
    return includePostScript;
}	//	IncludePostScriptInSpoolFile



/*------------------------------------------------------------------------------
    Function:	PostPrintingErrors
	
    Parameters:
        status	-	error code
	
    Description:
        This is where we could post an alert to report any problem reported
        by the Printing Manager.
		
------------------------------------------------------------------------------*/
void 	PostPrintingErrors(OSStatus status)
{
#pragma unused (status)	
}	//	PostPrintingErrors


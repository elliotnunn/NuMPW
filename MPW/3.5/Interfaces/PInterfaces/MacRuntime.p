{
Created: Saturday, April 16, 1994 2:44:46 PM
    MacRuntime.p
    Pascal Interface to the Macintosh Libraries

        Copyright Apple Computer, Inc. 1994
        All rights reserved
}


{$IFC UNDEFINED UsingIncludes}
{$SETC UsingIncludes := 0}
{$ENDC}

{$IFC NOT UsingIncludes}
    UNIT MacRuntime;
    INTERFACE
{$ENDC}

{$IFC UNDEFINED UsingMacRuntime AND UNDEFINED __MACRUNTIME__}
{$SETC UsingMacRuntime := 1}
{$SETC __MACRUNTIME__ := 1}

{$I+}
{$SETC MacRuntimeIncludes := UsingIncludes}
{$SETC UsingIncludes := 1}
{$IFC UNDEFINED UsingPasLibIntf}
{$I $$Shell(PInterfaces)PasLibIntf.p}
{$ENDC}

{$IFC UNDEFINED __CONDITIONALMACROS__}
{$I ConditionalMacros.p}
{$ENDC}

{$SETC UsingIncludes := MacRuntimeIncludes}

{$ALIGN MAC68K}

{
 Miscellaneous Operations
}

{$IFC NOT UNDEFINED __CFM68K__}
	{$IFC NOT UNDEFINED UsingSharedLibs}
		{$PUSH}
		{$LibExport+}
	{$ENDC}
{$ENDC}

	FUNCTION IEStandAlone: BOOLEAN;
	FUNCTION TrapAvailable(trap: INTEGER): BOOLEAN;

{$IFC NOT UNDEFINED __CFM68K__}
	{$IFC NOT UNDEFINED UsingSharedLibs}
		{$POP}
	{$ENDC}
{$ENDC}

{$ALIGN RESET}

{$ENDC}    { UsingIntEnv }

{$IFC NOT UsingIncludes}
    END.
{$ENDC}

#
#   c_sample.make
#
#	MPW makefile for c_sample C program
#
#	Copyright © 1994, Apple Computer, Inc.  All rights reserved.
#
AppName  = c_sample
Creator = '????'
Type	= 'APPL'

SymOpt  = off   # Set to "on" to build a debuggable version.
PPCCOpt = off	# Set to off, speed, or local

Source_Folder = :

68K_Object_Folder = :68KObjects:

PPC_Object_Folder = :PPCObjects:

{68K_Object_Folder} ƒ {Source_Folder}

{PPC_Object_Folder} ƒ {Source_Folder}

Sources = c_sample1.c ∂
		  c_sample2.c
		  
Headers = c_sample.h

Resources = {AppName}.r

PPC_Objects = 	{PPC_Object_Folder}c_sample1.c.ppc.o ∂
				{PPC_Object_Folder}c_sample2.c.ppc.o
				
68K_Objects = 	{68K_Object_Folder}c_sample1.c.68k.o ∂
				{68K_Object_Folder}c_sample2.c.68k.o
				
PPCC_Options = 	-sym {SymOpt} -opt {PPCCOpt}

C_Options =  -sym {SymOpt}  # Use universal headers for 68K build.

PPC_Libraries =	"{SharedLibraries}"InterfaceLib	∂
				"{SharedLibraries}"StdCLib		∂
				"{PPCLibraries}"StdCRuntime.o	∂
				"{PPCLibraries}"PPCCRuntime.o
				
68K_Libraries = "{Libraries}"MacRuntime.o   ∂
 				"{Libraries}"Interface.o 
										
Makefile =		{AppName}.make

C =				SC
PPCC = 			MrC

#------------------------------------------------------------------------------
		
.c.ppc.o	ƒ	.c {Makefile}        # Whenever the Makefile changes, all .o's get built.
	{PPCC} {PPCC_Options} {default}.c -o {Targ}
	
.c.68k.o	ƒ	.c {Makefile}        # Whenever the Makefile changes, all .o's get built.
	{C} {C_Options} {default}.c -o {Targ}
	
.a.68k.o	ƒ	.a {Makefile}         # Whenever the Makefile changes, all .o's get built.
	{Asm} {default}.a -o {Targ}
	
#------------------------------------------------------------------------------				

{AppName}	ƒ {AppName}.ppc {AppName}.68K
	
	# Duplicate the PowerPC code into the fat binary package.
	
	duplicate -y {AppName}.ppc {Targ}
	
	# rez in 'CODE' resources from 68K version.
	
	rez -d FAT {AppName}.r -a -o {Targ}
	
{AppName}.ppc ƒƒ {PPC_Objects}
	PPCLink	{PPC_Objects}			∂
		{PPC_Libraries}				∂
		-sym {SymOpt}				∂
		-o {Targ}
	rez -append -o {Targ} -d PowerPC {Resources}

{AppName}.68K ƒƒ {68K_Objects} {Resources}
	Link -d -t {Type} -c {Creator} ∂
		{68K_Objects} 			∂
		{68K_Libraries}		    ∂
		# -sym {SymOpt}			∂
		-o {Targ}
	rez -append -o {Targ} {Resources}
	

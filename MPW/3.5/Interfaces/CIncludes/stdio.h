/************************************************************

    stdio.h
    Standard input and output.

    Copyright Apple Computer,Inc.  1995-2001
    All rights reserved

************************************************************/


#ifndef __STDIO__
#define __STDIO__

/*
 * Include common declarations 
 */

#include <NullDef.h>
#include <SizeTDef.h>
#include <SeekDefs.h>
#include <VaListTDef.h>


/*
 *  The basic data structure for a stream is the FILE type.
 */

#ifdef powerc
#pragma options align=power
#endif
struct FILE {
    int             _cnt;
    unsigned char   *_ptr;
    unsigned char   *_base;
    unsigned char   *_end;
    unsigned short  _size;
    unsigned short  _flag;
    unsigned short  _file;
};
#ifdef powerc
#pragma options align=reset
#endif

typedef struct FILE FILE;


/*
 *  The type to uniquely specify the position in a file, returned by fgetpos().
 */

typedef long fpos_t;


/*
 *  Values for various bits of a FILE's _flag field.
 *  The values _IOFBF, _IOLBF, and _IONBF can be used as the
 *  third argument to setvbuf();
 */

#define _IOFBF      0x00    /* File is fully buffered       */
#define _IOREAD     0x01    /* File is open for Reading     */
#define _IOWRT      0x02    /* File is open for Writing    */
#define _IONBF      0x04    /* File I/O is unbuffered       */
#define _IOMYBUF    0x08    /* Buffer allocated by stdio    */
#define _IOEOF      0x10    /* End of file reached          */
#define _IOERR      0x20    /* I/O error has occurred       */
#define _IOLBF      0x40    /* File is line buffered        */
#define _IORW       0x80    /* File is open for Read/Write  */
#define _IOSYNC    0x100    /* Flush output on Read         */
#define _IOBINARY  0x200    /* For backward compatibility   */
#define _IOBACK   0x4000    /* For backward compatibility   */


/*
 *  The default buffer sizes for a fully buffered or line buffered file.
 */

#define BUFSIZ    1024
#define _LBFSIZ    254


/*
 *  The normal end-of-file indicator.
 */

#define EOF       (-1)


/*
 *  FOPEN_MAX is the minimum number of files that a program is guaranteed to be able
 *  to have open simultaneously (including the pre-opened stdin, stdout, and stderr).
 *  The numbers are listed in Inside Macintosh, page IV-178, as:
 *  64K ROM, 128K Macintosh     12 files
 *  64K ROM, 512K Macintosh     40 files
 *  128K ROM                    40 files per volume
 *
 *  FILENAME_MAX is the maximum length of a file name, including a trailing zero byte.
 */

#define FOPEN_MAX      12
#define FILENAME_MAX   32


/*
 *  L_tmpnam is the size of char array long enough to hold a temporary file name
 *  generated by tmpnam(), including the trailing null byte.  The name is in the
 *  form tmp.AAAXXXXXX, where AAA is a sequence of lower case letters ("aaa", "baa",
 *  ... "zzz" on successive calls), and XXXXXX is a lower case letter followed by a sequence
 *  of digits, all determined at runtime.
 */

#define L_tmpnam       14


/*
 *  TMP_MAX is the number of distinct file names that tmpnam() can generate.
 */

#define TMP_MAX     17576


/*
 *  The standard predefined streams: error, input, and output
 */

#define stdin        (&_iob[0])
#define stdout       (&_iob[1])
#define stderr       (&_iob[2])


#ifdef __cplusplus
extern "C" {
#endif

#if defined (__powerc) || defined (powerc) || defined (__CFM68K__)
    #pragma import on
#endif

/*
 * Operations on Files
 */

extern int remove (const char *filename);
extern int rename (const char *oldname, const char *newname);
extern FILE *tmpfile (void);
extern char *tmpnam (char *s);


/*
 * File Access Functions
 */

extern int fclose (FILE *stream);
extern int fflush (FILE *stream);
extern FILE *fopen (const char *filename, const char *mode);
extern FILE *freopen (const char *filename, const char *mode, FILE *stream);
extern void setbuf (FILE *stream, char *buf);
extern int setvbuf (FILE *stream, char *buf, int mode, size_t size);


/*
 * Formatted Input/Output Functions
 */

extern int fprintf (FILE *stream, const char *format, ...);
extern int fscanf (FILE *stream, const char *format, ...);
extern int printf (const char *format, ...);
extern int scanf (const char *format, ...);
extern int sprintf (char *s, const char *format, ...);
extern int sscanf (const char *s, const char *format, ...);
extern int vfprintf (FILE *stream, const char *format, va_list arg);
extern int vprintf (const char *format, va_list arg);
extern int vsprintf (char *s, const char *format, va_list arg);


/*
 * Character Input/Output Functions
 */

extern int fgetc (FILE *stream);
extern char *fgets (char *s, int n, FILE *stream);
extern int fputc (int c, FILE *stream);
extern int fputs (const char *s, FILE *stream);
extern int getc (FILE *stream);
extern int getchar (void);
extern char *gets (char *s);
extern int putc (int c, FILE *stream);
extern int putchar (int c);
extern int puts (const char *s);
extern int ungetc (int c, FILE *stream);

/*
 * WARNING!!
 *
 * These macros evaluate their arguments more than once.
 * Be sure that evaluation of the "s" argument has no side effects.
 *
 * For example, using "getc(mychar++)" would cause "mychar" to be
 * incremented twice.
 *
 * To avoid this, either assign "mychar" to a temporary, or put the
 * function name in paranthesis, so that the macro is not envoked:
 *    "(getc)(mychar++)"
 *
 */

#define getchar() (getc(stdin))
#define putchar(c) (putc((c), stdout))

/*
	MP-savvy functions.
	
	These functions permit locking a file for a task's
	use, and writing or reading the file without incurring
	additional locking/unlocking overhead.
	
*/

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && ( defined( __powerc ) || defined( powerc ) )
	extern int getchar_unlocked( void );
	extern int getc_unlocked( FILE* stream );
	extern int putchar_unlocked( int c );
	extern int putc_unlocked( int c, FILE* stream );
	extern void flockfile( FILE* file );
	extern int ftrylockfile( FILE* file );
	extern void funlockfile( FILE* file );
#endif

/*
 * Direct Input/Output Functions
 */

extern size_t fread (void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t fwrite (const void *ptr, size_t size, size_t nmemb, FILE *stream);


/*
 * File Positioning Functions
 */

extern int fgetpos (FILE *stream, fpos_t *pos);
extern int fseek (FILE *stream, long int offset, int whence);
extern int fsetpos (FILE *stream, const fpos_t *pos);
extern long int ftell (FILE *stream);
extern void rewind (FILE *stream);


/*
 * Error Handling Functions
 */

extern void clearerr (FILE *stream);
extern int feof (FILE *stream);
extern int ferror (FILE *stream);
extern void perror (const char *s);

/*
 * Internal structures exposed by previous macro definitions.
 */

extern FILE _iob[];    /* Array of FILE control blocks. */
#define _NFILE 40      /* Size of _iob. */

extern int _filbuf(FILE *);
extern int _flsbuf(unsigned char, FILE *);


/*
 *  Non-ANSI extensions
 *
 * The prefered mechanism for enabling these is by defining __useAppleExts__.  
 * In the absence of this symbol, the __STDC__ symbol is used to enable or
 * disable these extentions.
 */

#if defined (__useAppleExts__) || \
     (defined (applec) && ! defined (__STDC__)) || \
     (defined (__PPCC__) && __STDC__ == 0)

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && ( defined( __powerc ) || defined( powerc ) )
extern int fileno( FILE *iop );
#else
#define fileno(p)  (p)->_file
#endif

extern FILE *fdopen(int fildes, const char *mode);
extern void fsetfileinfo (const char *filename, unsigned long newcreator, unsigned long newtype);
extern int getw(FILE *stream);
extern int putw(int w, FILE *stream);

#endif

#if defined (__powerc) || defined (powerc) || defined (__CFM68K__)
    #pragma import off
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __STDIO__ */
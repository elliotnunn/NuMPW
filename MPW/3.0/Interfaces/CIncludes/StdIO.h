/*
 *	StdIO.h -- Standard C I/O Package
 *	Modified for use with Macintosh C
 *	Apple Computer, Inc.  1985-1988
 *
 *	Copyright American Telephone & Telegraph
 *	Used with permission, Apple Computer Inc. (1985)
 *	All rights reserved.
 */

#ifndef __STDIO__
#define __STDIO__

#ifndef __STDDEF__
#include <StdDef.h>
#endif __STDDEF__

#ifndef __STDARG__
#include <StdArg.h>
#endif __STDARG__


/*
 *	The basic data structure for a stream is the FILE.
 */

typedef struct {
	int 			_cnt;
	unsigned char	*_ptr;
	unsigned char	*_base;
	unsigned char	*_end;
	unsigned short	_size;
	unsigned short	_flag;
	unsigned short	_file;
} FILE;


/*
 *	fpos_t is a type that can express any position in a file.  A file's
 *	end-of-file marker has type fpos_t.
 */

typedef long fpos_t;


/*
 *	These macros give the meanings of bits in a FILE's _flag.  setvbuf() takes
 *	one of _IOFBF, _IOLBF, or _IONBF as its third argument.
 */

#define _IOFBF		0			/* Pseudo-flag, default buffering style */
#define _IOREAD 	(1<<0)		/* Current mode is for reading */
#define _IOWRT		(1<<1)		/* Current mode is for writing */
#define _IONBF		(1<<2)		/* no buffering */
#define _IOMYBUF	(1<<3)		/* buffer was allocated by stdio */
#define _IOEOF		(1<<4)
#define _IOERR		(1<<5)
#define _IOLBF		(1<<6)		/* fflush(iop) when a \n is written */
#define _IORW		(1<<7)		/* Enable read/write access */
#define _IOSYNC		(1<<8)		/* Input triggers fflush() to output fp's */
#define _IOBINARY	(1<<9)		/* Binary stream */


/*
 *	Default file buffer sizes used by setbuf() and setvbuf().
 */

#define BUFSIZ	1024			/* default file buffer size */
#define _LBFSIZ  100			/* Line buffer size */


/*
 *	The standard end-of-file indicator.
 */

#define EOF		(-1)


/*
 *	L_tmpnam is the size of char array long enough to hold a temporary file name
 *	generated by tmpnam(), including the trailing null byte.  The name is in the
 *	form tmp.AAAXXXXXX, where AAA is a sequence of lower case letters ("aaa", "baa",
 *	... "zzz" on successive calls), and XXXXXX is a lower case letter followed by a sequence
 *	of digits, all determined at runtime.
 *	TMP_MAX is the number of distinct file names that tmpnam() can generate.
 */

#define L_tmpnam	14
#define TMP_MAX		17576


/*
 *	The minimum number of files that a program is guaranteed to be able to have
 *	open simultaneously (including the pre-opened stdin, stdout, and stderr).
 *	The numbers are listed in Inside Macintosh, page IV-178, as:
 *	64K ROM, 128K Macintosh		12 files
 *	64K ROM, 512K Macintosh		40 files
 *	128K ROM					40 files per volume
 */

#define FOPEN_MAX	12


/*
 *	Maximum length of a file name, including a trailing zero byte.
 */

#define FILENAME_MAX	32


/*
 *	For use by fseek():
 */

#define SEEK_CUR	1
#define SEEK_END	2
#define SEEK_SET	0


/*
 *	The standard predefined streams.
 */

#define stdin		(&_iob[0])
#define stdout		(&_iob[1])
#define stderr		(&_iob[2])


#ifdef __safe_link
extern "C" {
#endif

/*
 *	Operations on files
 */

int remove (const char *);
int rename (const char *, const char *);
FILE *tmpfile (void);
char *tmpnam (char *);


/*
 *	File access functions
 */

int fclose (FILE *);
int fflush (FILE *);
FILE *fopen (const char *, const char *);
FILE *freopen (const char *, const char *, FILE *);
void setbuf (FILE *, char *);
int setvbuf (FILE *, char *, int, size_t);


/*
 *	Formatted input/output functions
 */

int fprintf (FILE *, const char *, ...);
int fscanf (FILE *, const char *, ...);
int printf (const char *, ...);
int scanf (const char *, ...);
int sprintf (char *, const char *, ...);
int sscanf (const char *, const char *, ...);
int vfprintf (FILE *, const char *, va_list);
int vprintf (const char *, va_list);
int vsprintf (char *, const char *, va_list);


/*
 *	Character input/output functions and macros
 */

int fgetc (FILE *);
char *fgets (char *, int, FILE *);
int fputc (int, FILE *);
int fputs (const char *, FILE *);
int getc (FILE *stream);
int getchar (void);
char *gets (char *);
int putc (int c, FILE *stream);
int putchar (int c);
int puts (const char *);
int ungetc (int, FILE *);

#define getc(p) 	(--(p)->_cnt >= 0 ? (int) *(p)->_ptr++ : _filbuf(p))
#define getchar()	getc(stdin)
#define putc(x, p)	(--(p)->_cnt >= 0 ? \
						((int) (*(p)->_ptr++ = (unsigned char) (x))) : \
						_flsbuf((unsigned char) (x), (p)))
#define putchar(x)	putc((x), stdout)


/*
 *	Direct input/output functions
 */

size_t fread (void *, size_t, size_t, FILE *);
size_t fwrite (const void *, size_t, size_t, FILE *);


/*
 *	File positioning functions
 */

int fgetpos (FILE *, fpos_t *);
int fseek (FILE *, long int, int);
int fsetpos (FILE *, const fpos_t *);
long int ftell (FILE *);
void rewind (FILE *);


/*
 *	Error-handling functions and macros
 */

void clearerr (FILE *stream);
int feof (FILE *stream);
int ferror (FILE *stream);
void perror (const char *s);

#define clearerr(p) ((void)((p)->_flag &= ~(_IOERR | _IOEOF)))
#define feof(p) 	((p)->_flag & _IOEOF)
#define ferror(p)	((p)->_flag & _IOERR)


/*
 *	Non-ANSI extensions, and things used internally:
 */

#define _NFILE	  20
extern FILE _iob[_NFILE];
#define _bufend(p)	(p)->_end
#define _bufsiz(p)	(p)->_size
#define fileno(p)	(p)->_file
FILE *fdopen (int, char *);
int _filbuf (FILE *);
int _flsbuf (int, FILE *);
int getw (FILE *);
int putw (int, FILE *);

#ifdef __safe_link
}
#endif


#endif __STDIO__
/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/*$Header: /home/kbs/jutta/src/gsm/gsm-1.0/inc/RCS/config.h,v 1.1 1992/10/28 00:11:08 jutta Exp $*/

#ifndef	CONFIG_H
#define	CONFIG_H

#define	SIGHANDLER_T	void		/* signal handlers are void	*/
/*#define HAS_SYSV_SIGNAL	1		/* sigs not blocked/reset?	*/

#define	HAS_STDLIB_H	1		/* /usr/include/stdlib.h	*/
#define	HAS_LIMITS_H	1		/* /usr/include/limits.h	*/
#define	HAS_FCNTL_H	1		/* /usr/include/fcntl.h		*/
#define	HAS_ERRNO_DECL	1		/* errno.h declares errno	*/

#define	HAS_FSTAT 	1		/* fstat syscall		*/
/*#define	HAS_FCHMOD 	1		/* fchmod syscall		*/
/*#define	HAS_FCHOWN 	1		/* fchown syscall		*/

#define	HAS_STRING_H 	1		/* /usr/include/string.h 	*/
/*efine	HAS_STRINGS_H	1		/* /usr/include/strings.h 	*/

#define	HAS_UTIME	1		/* POSIX utime(path, times)	*/
/*#define	HAS_UTIMES	1		/* use utimes()	syscall instead	*/
#define	HAS_UTIME_H	1		/* UTIME header file		*/
#define	HAS_UTIMBUF	1		/* struct utimbuf		*/
/*#define	HAS_UTIMEUSEC   1		/* microseconds in utimbuf?	*/

#endif	/* CONFIG_H */

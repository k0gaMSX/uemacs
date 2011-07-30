
/*	TERMIO.C
 *
 * The functions in this file negotiate with the operating system for
 * characters, and write characters in a barely buffered fashion on the display.
 * All operating systems.
 *
 *	modified by Petri Kutvonen
 */


#ifndef POSIX

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#include	<signal.h>
#include	<termio.h>
#include	<fcntl.h>
int kbdflgs;			/* saved keyboard fd flags      */
int kbdpoll;			/* in O_NDELAY mode                     */
int kbdqp;			/* there is a char in kbdq      */
char kbdq;			/* char we've already read      */
struct termio otermio;		/* original terminal characteristics */
struct termio ntermio;		/* charactoristics to use inside */
#if	XONXOFF
#define XXMASK	0016000
#endif


extern int rtfrmshell();	/* return from suspended shell */
#define TBUFSIZ 128
char tobuf[TBUFSIZ];		/* terminal output buffer */

/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw. On CPM it is a no-op.
 */
void ttopen(void)
{
	ioctl(0, TCGETA, &otermio);	/* save old settings */
	ntermio.c_iflag = 0;	/* setup new settings */
#if	XONXOFF
	ntermio.c_iflag = otermio.c_iflag & XXMASK;	/* save XON/XOFF P.K. */
#endif
	ntermio.c_oflag = 0;
	ntermio.c_cflag = otermio.c_cflag;
	ntermio.c_lflag = 0;
	ntermio.c_line = otermio.c_line;
	ntermio.c_cc[VMIN] = 1;
	ntermio.c_cc[VTIME] = 0;
	ioctl(0, TCSETAW, &ntermio);	/* and activate them */
	kbdflgs = fcntl(0, F_GETFL, 0);
	kbdpoll = FALSE;

	/* provide a smaller terminal output buffer so that
	   the type ahead detection works better (more often) */
	setvbuf(stdout, &tobuf[0], _IOFBF, TBUFSIZ);
	signal(SIGTSTP, SIG_DFL);	/* set signals so that we can */
	signal(SIGCONT, rtfrmshell);	/* suspend & restart emacs */
	TTflush();

	/* on all screens we are not sure of the initial position
	   of the cursor                                        */
	ttrow = 999;
	ttcol = 999;
}

/*
 * This function gets called just before we go back home to the command
 * interpreter. On VMS it puts the terminal back in a reasonable state.
 * Another no-operation on CPM.
 */
void ttclose(void)
{

	ioctl(0, TCSETAW, &otermio);	/* restore terminal settings */
	fcntl(0, F_SETFL, kbdflgs);
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */
void ttputc(c)
{
	fputc(c, stdout);
}

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
int ttflush(void)
{
/*
 * Add some terminal output success checking, sometimes an orphaned
 * process may be left looping on SunOS 4.1.
 *
 * How to recover here, or is it best just to exit and lose
 * everything?
 *
 * jph, 8-Oct-1993
 */
	int status;

	status = fflush(stdout);

	if (status != 0 && errno != EAGAIN) {
		exit(EXIT_FAILURE);
	}
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all. Very simple on CPM, because the system can do exactly what you want.
 */
ttgetc()
{
	if (kbdqp)
		kbdqp = FALSE;
	else {
		if (kbdpoll && fcntl(0, F_SETFL, kbdflgs) < 0)
			return FALSE;
		kbdpoll = FALSE;
		while (read(0, &kbdq, 1) != 1);
	}
	return kbdq & 255;
}

#if	TYPEAH
/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/

typahead()
{
	if (!kbdqp) {
		if (!kbdpoll && fcntl(0, F_SETFL, kbdflgs | O_NDELAY) < 0)
			return FALSE;
		kbdpoll = 1;
		kbdqp = (1 == read(0, &kbdq, 1));
	}
	return kbdqp;
}
#endif

#endif				/* not POSIX */

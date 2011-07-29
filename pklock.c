/*	PKLOCK.C
 *
 *	locking routines as modified by Petri Kutvonen
 */

#include "estruct.h"
#include "edef.h"
#include "efunc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define MAXLOCK 512
#define MAXNAME 128


/**********************
 *
 * if successful, returns NULL
 * if file locked, returns username of person locking the file
 * if other error, returns "LOCK ERROR: explanation"
 *
 *********************/
char *dolock(char *fname)
{
	int fd, n;
	static char lname[MAXLOCK], locker[MAXNAME + 1];
	int mask;
	struct stat sbuf;

	strcat(strcpy(lname, fname), ".lock~");

	/* check that we are not being cheated, qname must point to     */
	/* a regular file - even this code leaves a small window of     */
	/* vulnerability but it is rather hard to exploit it            */

#if defined(S_IFLNK)
	if (lstat(lname, &sbuf) == 0)
#else
	if (stat(lname, &sbuf) == 0)
#endif
#if defined(S_ISREG)
		if (!S_ISREG(sbuf.st_mode))
#else
		if (!(((sbuf.st_mode) & 070000) == 0))	/* SysV R2 */
#endif
			return "LOCK ERROR: not a regular file";

	mask = umask(0);
	fd = open(lname, O_RDWR | O_CREAT, 0666);
	umask(mask);
	if (fd < 0) {
		if (errno == EACCES)
			return NULL;
#ifdef EROFS
		if (errno == EROFS)
			return NULL;
#endif
		return "LOCK ERROR: cannot access lock file";
	}
	if ((n = read(fd, locker, MAXNAME)) < 1) {
		lseek(fd, 0, SEEK_SET);
/*		strcpy(locker, getlogin()); */
		cuserid(locker);
		strcat(locker + strlen(locker), "@");
		gethostname(locker + strlen(locker), 64);
		write(fd, locker, strlen(locker));
		close(fd);
		return NULL;
	}
	locker[n > MAXNAME ? MAXNAME : n] = 0;
	return locker;
}


/*********************
 *
 * undolock -- unlock the file fname
 *
 * if successful, returns NULL
 * if other error, returns "LOCK ERROR: explanation"
 *
 *********************/

char *undolock(char *fname)
{
	static char lname[MAXLOCK];

	strcat(strcpy(lname, fname), ".lock~");
	if (unlink(lname) != 0) {
		if (errno == EACCES || errno == ENOENT)
			return NULL;
#ifdef EROFS
		if (errno == EROFS)
			return NULL;
#endif
		return "LOCK ERROR: cannot remove lock file";
	}
	return NULL;
}

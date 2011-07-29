/*	EPATH.H
 *
 *	This file contains certain info needed to locate the
 *	initialization (etc) files on a system dependent basis
 *
 *	modified by Petri Kutvonen
 */
#ifndef EPATH_H_
#define EPATH_H_

/*	possible names and paths of help files under different OSs	*/
static char *pathname[] = {
	".emacsrc", "emacs.hlp",
  #if	PKCODE
	    "/usr/global/lib/", "/usr/local/bin/", "/usr/local/lib/",
  #endif
        "/usr/local/", "/usr/lib/",

        ""};

#endif  /* EPATH_H_ */

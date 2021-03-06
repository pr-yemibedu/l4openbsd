/*	$OpenBSD: fsdbutil.c,v 1.14 2009/10/27 23:59:33 deraadt Exp $	*/
/*	$NetBSD: fsdbutil.c,v 1.5 1996/09/28 19:30:37 christos Exp $	*/

/*-
 * Copyright (c) 1996 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by John T. Kohl.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include "fsdb.h"
#include "fsck.h"

char **
crack(char *line, int *argc)
{
	static char *argv[8];
	int i;
	char *p, *val;

	for (p = line, i = 0; p != NULL && i < 8; i++) {
		while ((val = strsep(&p, " \t\n")) != NULL && *val == '\0')
			/**/;
		if (val)
			argv[i] = val;
		else
			break;
	}
	*argc = i;
	return argv;
}

int
argcount(struct cmdtable *cmdp, int argc, char *argv[])
{
	if (cmdp->minargc == cmdp->maxargc)
		warnx("command `%s' takes %u arguments", cmdp->cmd, cmdp->minargc-1);
	else
		warnx("command `%s' takes from %u to %u arguments",
		    cmdp->cmd, cmdp->minargc-1, cmdp->maxargc-1);
	warnx("usage: %s: %s", cmdp->cmd, cmdp->helptxt);
	return 1;
}

void
printstat(const char *cp, ino_t inum, union dinode *dp)
{
	struct group *grp;
	struct passwd *pw;
	time_t t;
	char *p;

	printf("%s: ", cp);
	switch (DIP(dp, di_mode) & IFMT) {
	case IFDIR:
		puts("directory");
		break;
	case IFREG:
		puts("regular file");
		break;
	case IFBLK:
		printf("block special (%d,%d)",
		    (int)major(DIP(dp, di_rdev)), (int)minor(DIP(dp, di_rdev)));
		break;
	case IFCHR:
		printf("character special (%d,%d)",
		    (int)major(DIP(dp, di_rdev)), (int)minor(DIP(dp, di_rdev)));
		break;
	case IFLNK:
		fputs("symlink",stdout);
		if (DIP(dp, di_size) > 0 &&
		    DIP(dp, di_size) < sblock.fs_maxsymlinklen &&
		    DIP(dp, di_blocks) == 0) {
			char *p = sblock.fs_magic == FS_UFS1_MAGIC ?
			    (char *)dp->dp1.di_shortlink :
			    (char *)dp->dp2.di_shortlink;
			printf(" to `%.*s'\n", (int)DIP(dp, di_size), p);
		} else
			putchar('\n');
		break;
	case IFSOCK:
		puts("socket");
		break;
	case IFIFO:
		puts("fifo");
		break;
	}

	printf("I=%u MODE=%o SIZE=%llu", inum, DIP(dp, di_mode), DIP(dp, di_size));
	t = DIP(dp, di_mtime);
	p = ctime(&t);
	printf("\n\tMTIME=%15.15s %4.4s [%d nsec]", &p[4], &p[20],
	    DIP(dp, di_mtimensec));
	t = DIP(dp, di_ctime);
	p = ctime(&t);
	printf("\n\tCTIME=%15.15s %4.4s [%d nsec]", &p[4], &p[20],
	    DIP(dp, di_ctimensec));
	t = DIP(dp, di_atime);
	p = ctime(&t);
	printf("\n\tATIME=%15.15s %4.4s [%d nsec]\n", &p[4], &p[20],
	    DIP(dp, di_atimensec));

	if ((pw = getpwuid(DIP(dp, di_uid))))
		printf("OWNER=%s ", pw->pw_name);
	else
		printf("OWNUID=%u ", DIP(dp, di_uid));
	if ((grp = getgrgid(DIP(dp, di_gid))))
		printf("GRP=%s ", grp->gr_name);
	else
		printf("GID=%u ", DIP(dp, di_gid));

	printf("LINKCNT=%hd FLAGS=%#x BLKCNT=%x GEN=%x\n", DIP(dp, di_nlink),
	    DIP(dp, di_flags), (unsigned)DIP(dp, di_blocks), DIP(dp, di_gen));
}

int
checkactive(void)
{
	if (!curinode) {
		warnx("no current inode");
		return 0;
	}
	return 1;
}

int
checkactivedir(void)
{
	if (!curinode) {
		warnx("no current inode");
		return 0;
	}
	if ((DIP(curinode, di_mode) & IFMT) != IFDIR) {
		warnx("inode %d not a directory", curinum);
		return 0;
	}
	return 1;
}

int
printactive(void)
{
	if (!checkactive())
		return 1;
	switch (DIP(curinode, di_mode) & IFMT) {
	case IFDIR:
	case IFREG:
	case IFBLK:
	case IFCHR:
	case IFLNK:
	case IFSOCK:
	case IFIFO:
		printstat("current inode", curinum, curinode);
		break;
	case 0:
		printf("current inode %d: unallocated inode\n", curinum);
		break;
	default:
		printf("current inode %d: screwy itype 0%o (mode 0%o)?\n",
		    curinum, DIP(curinode, di_mode) & IFMT,
		    DIP(curinode, di_mode));
		break;
	}
	return 0;
}

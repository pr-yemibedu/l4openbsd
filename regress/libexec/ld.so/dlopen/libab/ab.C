/*
 * Public Domain 2003 Dale Rahn
 *
 * $OpenBSD: ab.C,v 1.4 2005/09/19 18:39:37 deraadt Exp $
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "ab.h"

char strbuf[512];

extern "C" {
char *libname = "libab";
};

extern "C" char *
lib_entry()
{
	strlcpy(strbuf, libname, sizeof strbuf);
	strlcat(strbuf, ":", sizeof strbuf);
	strlcat(strbuf, "ab", sizeof strbuf);
	return strbuf;
	std::cout << "called into ab " << libname << " libname " << "\n";
}

BB::BB(char *str)
{
	_name = str;
}

BB::~BB()
{
}
BB ab("local");

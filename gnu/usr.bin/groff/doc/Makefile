#Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
#     Written by James Clark (jjc@jclark.com)
#
#This file is part of groff.
#
#groff is free software; you can redistribute it and/or modify it under
#the terms of the GNU General Public License as published by the Free
#Software Foundation; either version 2, or (at your option) any later
#version.
#
#groff is distributed in the hope that it will be useful, but WITHOUT ANY
#WARRANTY; without even the implied warranty of MERCHANTABILITY or
#FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
#for more details.
#
#You should have received a copy of the GNU General Public License along
#with groff; see the file COPYING.  If not, write to the Free Software
#Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

FFLAG=-F..
TROFF=../troff/troff -M../tmac $(FFLAG)
GROPS=../grops/grops $(FFLAG)
DOCS=meref.ps meintro.ps pic.ps
MEMACROS=../macros/tmac.e
SOELIM=../soelim/soelim

all: $(DOCS)

.SUFFIXES: .tr .me .ms .ps .dit

.dit.ps:
	$(GROPS) $< >$@

.me.dit:
	$(SOELIM) $< \
	| sed -e "s;@VERSION@;`cat ../VERSION`;" \
	| $(TROFF) -Tps $(FFLAG) -me  >$@

.tr.dit:
	$(TROFF) -Tps $< >$@

meref.ps: meref.dit
meintro.ps: meintro.dit

# This production must use -p -e -t so pic/eqn processing is done
pic.ps: pic.ms
	sed -e "s;@VERSION@;`cat ../VERSION`;" $< \
	| ../groff/groff -p -e -t -Tps $(FFLAG) -ms  >$@

pic.html: pic.ms
	sed -e "s;@VERSION@;`cat ../VERSION`;" $< \
	| ../groff/groff -p -e -t -Thtml $(FFLAG) -ms -mhtml >$@

install:

clean:
	-rm -f *.ps *.html *.png *.gif *.dit core
	-rm -f *.aux *.cp *.cps *.cv *.cn *.dvi *.fn *.fns *.ky *.kys \
               *.log *.op *.pg *.pgs *.ps *.toc *.tp *.tps *.tr *.vr *.vrs

distclean: clean

realclean: distclean

extraclean: clean
	-rm -f core *~ \#* junk temp grot

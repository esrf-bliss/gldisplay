############################################################################
# This file is part of gldisplay, a submodule of LImA project the
# Library for Image Acquisition
#
# Copyright (C) : 2009-2011
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################
.PHONY: src test clean

all:	src test

src:
	$(MAKE) -C src

test:
	$(MAKE) -C test

config:	sip.clean sip/Makefile
	
sip:	sip/Makefile sip/gldisplay.so

sip/Makefile:
	test -n "$$QTDIR" || \
		(echo "Must define the QTDIR variable (e.g. /usr/share/qt4)" &&\
		 false)
	cd sip && python configure.py

sip/gldisplay.so:
	$(MAKE) -C sip

clean:	sip.clean
	$(MAKE) -C src clean
	$(MAKE) -C test clean

sip.clean:
	cd sip && python clean.py


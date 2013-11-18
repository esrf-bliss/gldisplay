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
INCL		= ../include
TEST		= ../test

COMMON		= ../../../common
CONTROL		= ../../../control
HARDWARE	= ../../../hardware
SPS		= ../../Sps
PROCLIB		= ../../Processlib
PROCLIBCORE	= $(PROCLIB)/core

ifdef QTDIR3
QTDIR		= $(QTDIR3)
endif

ifndef QTDIR
$(error QTDIR environment variable must be defined)
endif

MOC             = $(QTDIR)/bin/moc

CFLAGS		= -Wall -g -I$(INCL) -fPIC -pthread

IMGFLAGS	= $(CFLAGS) -I$(QTDIR)/include -DQT_THREAD_SUPPORT -O3 
IMGLDFLAGS	= -L$(QTDIR)/lib -lqt-mt -lGL

LIMAFLAGS	= -I$(COMMON)/include -I$(CONTROL)/include \
		  -I$(HARDWARE)/include -I$(PROCLIBCORE)/include \
		  -I$(SPS)/Include

# targets
all:				$(TEST)/image_test $(TEST)/gldisplay_test

GLDisplay.o:			GLDisplay.cpp $(INCL)/GLDisplay.h
	$(CXX) -c $(IMGFLAGS) $(LIMAFLAGS) -o $@ $<

# The GLDisplay library
$(TEST)/gldisplay_test:		$(TEST)/gldisplay_test.o GLDisplay.o \
				$(SPS)/Src/sps.o image.o
	$(CXX) -o $@ $+ $(IMGLDFLAGS)

$(TEST)/gldisplay_test.o:	$(TEST)/gldisplay_test.cpp \
				$(INCL)/GLDisplay.h
	$(CXX) -c $(IMGFLAGS) $(LIMAFLAGS) -o $@ $<

# The image library
$(TEST)/image_test:		$(TEST)/image_test.o image.o 
	$(CXX) -o $@ $+ $(IMGLDFLAGS)

$(TEST)/image_test.o:		$(TEST)/image_test.c $(INCL)/imageapi.h
	$(CC) -c $(IMGFLAGS) -o $@ $<

image.o:		image_aux.o moc_image.o
	$(LD) -r -o $@ $+

image_aux.o:		image.cpp $(INCL)/image.h $(INCL)/imageapi.h \
			$(INCL)/autoobj.h $(INCL)/prectime.h
	$(CXX) -c $(IMGFLAGS) -o $@ $<

moc_image.o:		moc_image.cpp
	$(CXX) -c $(IMGFLAGS) -o $@ $<

moc_image.cpp:		$(INCL)/image.h
	$(MOC) $< -o $@


# cleanup
clean:
	rm -f *.o moc_* $(TEST)/*.o $(TEST)/image_test $(TEST)/gldisplay_test

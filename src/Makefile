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

LIMA		= ../../..
COMMON		= $(LIMA)/common
CONTROL		= $(LIMA)/control
HARDWARE	= $(LIMA)/hardware
SPS		= $(LIMA)/third-party/Sps
PROCLIB		= $(LIMA)/third-party/Processlib
PROCLIBCORE	= $(PROCLIB)/core
SIMU		= $(LIMA)/camera/simulator

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
		  -I$(SPS)/Include -I$(SIMU)/include -DWITH_SPS_IMAGE
LIMALDFLAGS	= -L$(LIMA)/build -L$(PROCLIB)/build \
		  -llimasimulator -llimacore -lprocesslib

GLDISPLAY_OBJS	= GLDisplay.o CtGLDisplay.o $(SPS)/Src/sps.o image.o

TEST_PROGS	= $(TEST)/image_test \
		  $(TEST)/gldisplay_test \
		  $(TEST)/simu_gldisplay_test

# targets
all:				$(TEST_PROGS)

$(TEST)/simu_gldisplay_test:	$(TEST)/simu_gldisplay_test.o $(GLDISPLAY_OBJS)
	$(CXX) -o $@ $+ $(IMGLDFLAGS) $(LIMALDFLAGS)

$(TEST)/simu_gldisplay_test.o:	$(TEST)/simu_gldisplay_test.cpp \
				$(INCL)/GLDisplay.h
	$(CXX) -c $(IMGFLAGS) $(LIMAFLAGS) -o $@ $<

$(TEST)/gldisplay_test:		$(TEST)/gldisplay_test.o $(GLDISPLAY_OBJS)
	$(CXX) -o $@ $+ $(IMGLDFLAGS) $(LIMALDFLAGS)

$(TEST)/gldisplay_test.o:	$(TEST)/gldisplay_test.cpp \
				$(INCL)/GLDisplay.h
	$(CXX) -c $(IMGFLAGS) $(LIMAFLAGS) -o $@ $<

# The GLDisplay library
GLDisplay.o:			GLDisplay.cpp $(INCL)/GLDisplay.h
	$(CXX) -c $(IMGFLAGS) $(LIMAFLAGS) -o $@ $<

CtGLDisplay.o:			CtGLDisplay.cpp $(INCL)/CtGLDisplay.h \
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
	rm -f *.o moc_* $(TEST)/*.o $(TEST_PROGS)

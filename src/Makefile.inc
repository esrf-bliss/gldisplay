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
SRC		= ../src
BUILD		= ../build
TEST		= ../test

VERSION		= ../VERSION

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
LIMALDFLAGS	= -L$(LIMA)/build -L$(PROCLIB)/build
LIMALDLIBS	= -llimacore -lprocesslib
SIMULDLIBS	= -llimasimulator 

GLDISPLAY_LIBFULLVER	= $(shell cat $(VERSION))
GLDISPLAY_LIBMINORVER	= $(shell echo $(GLDISPLAY_LIBFULLVER) | cut -f1-2 -d.)
GLDISPLAY_OBJS		= $(SRC)/CtGLDisplay.o $(SRC)/GLDisplay.o \
			  $(SRC)/image.o
GLDISPLAY_LIBLINK	= $(BUILD)/libgldisplay.so
GLDISPLAY_LIBMINORLINK	= $(GLDISPLAY_LIBLINK).$(GLDISPLAY_LIBMINORVER)
GLDISPLAY_LIB		= $(GLDISPLAY_LIBLINK).$(GLDISPLAY_LIBFULLVER)
GLDISPLAY_LIBLINKNAME	= $(shell basename $(GLDISPLAY_LIBMINORLINK))
GLDISPLAY_LIBLDFLAGS	= -fPIC -Wl,-soname,$(GLDISPLAY_LIBLINKNAME)

GLDISPLAYLDFLAGS	= -L$(BUILD)
GLDISPLAYLDLIBS		= -lgldisplay

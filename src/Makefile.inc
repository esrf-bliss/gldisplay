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

include $(LIMA_DIR)/config.inc
include $(LIMA_DIR)/control/control.inc

INCL		= ../include
SRC		= ../src
BUILD		= ../build
TEST		= ../test

VERSION		= ../VERSION

ifndef QTDIR
$(error QTDIR environment variable must be defined)
endif

MOC             = $(QTDIR)/bin/moc

CPPFLAGS	= -I$(INCL)
CXXFLAGS	= $(CT_CXXFLAGS)

QTDEFINES       = -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE -DQT_NO_DEBUG \
		  -DQT_OPENGL_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED

QTINCLD		= $(QTDIR)/include
QTINCLS		= -I$(QTINCLD) -I$(QTINCLD)/QtCore -I$(QTINCLD)/QtGui \
		  -I$(QTINCLD)/QtOpenGL
QTFLAGS		= -pipe -g -O2 -Wall -W -D_REENTRANT \
		   $(QTINCLS) $(QTDEFINES)
QTLDFLAGS	= -Wl,-O1 -L$(QTDIR)/lib -lQtCore -lQtGui -lQtOpenGL -lGL

IMGFLAGS	= $(CPPFLAGS) $(CXXFLAGS) $(QTFLAGS)
IMGLDFLAGS	= $(QTLDFLAGS)

LIMAFLAGS	= $(CT_CPPFLAGS) $(CT_CXXFLAGS) \
                  -I$(LIMA_DIR)/camera/simulator/include
LIMADEPLIBS     = $(CT_DEPLIBS) $(LIMA_DIR)/build,limacore
LIMALDFLAGS	= $(CT_LDFLAGS) $(call lib-depflags,$(LIMADEPLIBS))
LIMALDLIBS	= $(CT_LDLIBS) $(call lib-deplibs,$(LIMADEPLIBS))
SIMULDLIBS	= -llimasimulator 

GLDISPLAY_LIBFULLVER	= $(shell cat $(VERSION))
GLDISPLAY_LIBMINORVER	= $(shell echo $(GLDISPLAY_LIBFULLVER) | cut -f1-2 -d.)
GLDISPLAY_OBJS		= $(SRC)/CtGLDisplay.o $(SRC)/GLDisplay.o \
			  $(SRC)/SimplePipe.o $(SRC)/image.o
GLDISPLAY_LIBLINK	= $(BUILD)/libgldisplay.so
GLDISPLAY_LIBMINORLINK	= $(GLDISPLAY_LIBLINK).$(GLDISPLAY_LIBMINORVER)
GLDISPLAY_LIB		= $(GLDISPLAY_LIBLINK).$(GLDISPLAY_LIBFULLVER)
GLDISPLAY_LIBLINKNAME	= $(shell basename $(GLDISPLAY_LIBMINORLINK))
GLDISPLAY_LIBLDFLAGS	= -fPIC -Wl,-soname,$(GLDISPLAY_LIBLINKNAME) \
			  $(QTLDFLAGS)

GLDISPLAYLDFLAGS	= -L$(BUILD)
GLDISPLAYLDLIBS		= -lgldisplay

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

include ../src/Makefile.inc

# targets
all:				$(GLDISPLAY_LIB)

$(GLDISPLAY_LIB):		$(GLDISPLAY_OBJS)
	mkdir -p $(BUILD)
	$(CXX) -shared -o $@ $(GLDISPLAY_LIBLDFLAGS) $+
	rm -f $(GLDISPLAY_LIBMINORLINK) $(GLDISPLAY_LIBLINK)
	ln -s `basename $(GLDISPLAY_LIB)` $(GLDISPLAY_LIBMINORLINK)
	ln -s `basename $(GLDISPLAY_LIBMINORLINK)` $(GLDISPLAY_LIBLINK)

# The GLDisplay library
$(SRC)/GLDisplay.o:		$(SRC)/GLDisplay.cpp $(INCL)/GLDisplay.h
	$(CXX) -c $(IMGFLAGS) $(LIMAFLAGS) -o $@ $<

$(SRC)/CtGLDisplay.o:		$(SRC)/CtGLDisplay.cpp $(INCL)/CtGLDisplay.h \
				$(INCL)/GLDisplay.h
	$(CXX) -c $(IMGFLAGS) $(LIMAFLAGS) -o $@ $<

# The image library
$(SRC)/image.o:			$(SRC)/image_aux.o $(SRC)/moc_image.o
	$(LD) -r -o $@ $+

$(SRC)/image_aux.o:		$(SRC)/image.cpp $(INCL)/image.h \
				$(INCL)/imageapi.h $(INCL)/autoobj.h \
				$(INCL)/prectime.h
	$(CXX) -c $(IMGFLAGS) -o $@ $<

$(SRC)/moc_image.o:		$(SRC)/moc_image.cpp
	$(CXX) -c $(IMGFLAGS) -o $@ $<

$(SRC)/moc_image.cpp:		$(INCL)/image.h
	$(MOC) $< -o $@

# cleanup
clean:
	rm -f $(SRC)/*.o $(SRC)/moc_* $(BUILD)/libgldisplay.so*

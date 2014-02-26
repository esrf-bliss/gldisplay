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

TEST_PROGS	= $(TEST)/image_test \
		  $(TEST)/gldisplay_test

# targets
all:				$(TEST_PROGS)

$(TEST)/gldisplay_test:		$(TEST)/gldisplay_test.o $(GLDISPLAY_LIB)
	$(CXX) -o $@ $< $(IMGLDFLAGS) $(LIMALDFLAGS) $(GLDISPLAYLDFLAGS) \
			$(LIMALDLIBS) $(GLDISPLAYLDLIBS)

$(TEST)/gldisplay_test.o:	$(TEST)/gldisplay_test.cpp \
				$(INCL)/GLDisplay.h
	$(CXX) -c $(IMGFLAGS) $(LIMAFLAGS) -o $@ $<

# The image library
$(TEST)/image_test:		$(TEST)/image_test.o $(SRC)/image.o 
	$(CXX) -o $@ $+ $(IMGLDFLAGS)

$(TEST)/image_test.o:		$(TEST)/image_test.c $(INCL)/imageapi.h
	$(CC) -c $(IMGFLAGS) -o $@ $<

# cleanup
clean:
	rm -f $(TEST)/*.o $(TEST_PROGS)

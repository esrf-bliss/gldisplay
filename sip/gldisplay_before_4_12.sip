//###########################################################################
// This file is part of gldisplay, a submodule of LImA project the
// Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################
%Module gldisplay 0

%ModuleHeaderCode

extern "C" { void gldisplay_import_array();}

#define PYLIST_2_ARGC_ARGV_HEAD(a0, argc, argv, ok) \
	int argc = PyList_Size(a0); \
	char **argv = NULL; \
	ok = true; \
	if (argc > 0) { \
		argv = new char*[argc]; \
		for (int i = 0; (i < argc) && ok; ++i) { \
			PyObject *s = PyList_GET_ITEM(a0, i); \
			ok = PyString_Check(s); \
			if (ok) \
				argv[i] = PyString_AsString(s); \
			else \
				sipBadCallableArg(0, a0); \
		} \
	}

#define PYLIST_2_ARGC_ARGV_TAIL(argv) \
	delete [] argv;

%End

%PostInitialisationCode
gldisplay_import_array();
%End

class GLDisplay
{
%TypeHeaderCode
#include <GLDisplay.h>
%End

 public:
	GLDisplay(SIP_PYLIST)[(int argc, char **argv)];
%MethodCode
	bool ok;
	PYLIST_2_ARGC_ARGV_HEAD(a0, argc, argv, ok)
        if (ok) {
		Py_BEGIN_ALLOW_THREADS
		try {
			sipCpp = new GLDisplay(argc, argv);
		} catch (...) {
			PYLIST_2_ARGC_ARGV_TAIL(argv)
			throw;
		}
		Py_END_ALLOW_THREADS
        }
	PYLIST_2_ARGC_ARGV_TAIL(argv)
%End

	~GLDisplay();

	void createWindow(std::string caption);
	bool isClosed();

	void setBuffer(void *buffer_ptr, int width, int height, int depth);
	void updateBuffer();

	void refresh();
};

%Import ../../../sip/core/limacore_tmp.sip

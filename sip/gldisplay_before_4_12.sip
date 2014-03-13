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

#define PYLIST_2_ARGC_ARGV_HEAD(ai, argc, argv, ok) \
	int argc = PyList_Size(ai); \
	char **argv = NULL; \
	ok = true; \
	if (argc > 0) { \
		argv = new char*[argc]; \
		for (int i = 0; (i < argc) && ok; ++i) { \
			PyObject *s = PyList_GET_ITEM(ai, i); \
			ok = PyString_Check(s); \
			if (ok) \
				argv[i] = PyString_AsString(s); \
			else \
				sipBadCallableArg(0, ai); \
		} \
	}

#define PYLIST_2_ARGC_ARGV_TAIL(argv) \
	delete [] argv;

#define GLDISPLAY_CONSTRUCTOR_ARGC_ARGV(ai, glklass, glklass_args) \
	bool ok; \
	PYLIST_2_ARGC_ARGV_HEAD(ai, argc, argv, ok) \
        if (ok) { \
		Py_BEGIN_ALLOW_THREADS \
		try { \
			sipCpp = new glklass glklass_args; \
		} catch (...) { \
			Py_BLOCK_THREADS \
			PYLIST_2_ARGC_ARGV_TAIL(argv) \
			throw; \
		} \
		Py_END_ALLOW_THREADS \
        } \
	PYLIST_2_ARGC_ARGV_TAIL(argv)

#define GLDISPLAY_CONSTRUCTOR_ARGC_ARGV_A0(glklass) \
	GLDISPLAY_CONSTRUCTOR_ARGC_ARGV(a0, glklass, (argc,argv) )

#define GLDISPLAY_CONSTRUCTOR_ARGC_ARGV_A1(glklass) \
	GLDISPLAY_CONSTRUCTOR_ARGC_ARGV(a1, glklass, (a0,argc,argv) )

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
	GLDISPLAY_CONSTRUCTOR_ARGC_ARGV_A0(GLDisplay)
%End

	~GLDisplay();

	void createWindow(std::string caption);
	bool isClosed();

	void setBuffer(char *buffer_ptr /KeepReference/,
		       int width, int height, int depth);
	void updateBuffer();

	void setTestImage(bool active);

	void refresh();

	void getRates(float *update /Out/, float *refresh /Out/);
	void getNorm(unsigned long *minval /Out/, unsigned long *maxval /Out/,
		     int *autorange /Out/);
	void setNorm(unsigned long minval, unsigned long maxval,
		     int autorange);
};


class SPSGLDisplayBase
{
%TypeHeaderCode
#include <GLDisplay.h>
%End

 public:
	SPSGLDisplayBase(SIP_PYLIST)[(int argc, char **argv)];
%MethodCode
	GLDISPLAY_CONSTRUCTOR_ARGC_ARGV_A0(sipSPSGLDisplayBase)
%End
	virtual ~SPSGLDisplayBase();

	void setSpecArray(std::string spec_name, std::string array_name);
	void getSpecArray(std::string& spec_name /Out/,
			  std::string& array_name /Out/);

	void setCaption(std::string caption);

	virtual void createWindow() = 0;
	virtual bool isClosed() = 0;

	virtual void setTestImage(bool active) = 0;

	virtual void refresh() = 0;

	virtual void getRates(float *update /Out/, float *refresh /Out/) = 0;
	virtual void getNorm(unsigned long *minval /Out/,
			     unsigned long *maxval /Out/,
			     int *autorange /Out/) = 0;
	virtual void setNorm(unsigned long minval, unsigned long maxval,
			     int autorange) = 0;

 protected:
	bool checkSpecArray();
	void releaseBuffer();
};


class LocalSPSGLDisplay : SPSGLDisplayBase
{
%TypeHeaderCode
#include <GLDisplay.h>
%End

 public:
	LocalSPSGLDisplay(SIP_PYLIST)[(int argc, char **argv)];
%MethodCode
	GLDISPLAY_CONSTRUCTOR_ARGC_ARGV_A0(sipLocalSPSGLDisplay)
%End
	virtual ~LocalSPSGLDisplay();

	virtual void createWindow();
	virtual bool isClosed();

	virtual void setTestImage(bool active);

	virtual void refresh();

	virtual void getRates(float *update /Out/, float *refresh /Out/);
	virtual void getNorm(unsigned long *minval /Out/,
			     unsigned long *maxval /Out/,
			     int *autorange /Out/);
	virtual void setNorm(unsigned long minval, unsigned long maxval,
			     int autorange);

};

class GLForkCallback
{
%TypeHeaderCode
#include <GLDisplay.h>
%End

 public:
	GLForkCallback();
	virtual ~GLForkCallback();

 protected:
	virtual void execInForked() = 0;
};

class GLForkable
{
%TypeHeaderCode
#include <GLDisplay.h>
%End

 public:
	GLForkable();
	~GLForkable();

	void addForkCallback(GLForkCallback *fork_cb);
	void removeForkCallback(GLForkCallback *fork_cb);

	void execInForked();
};

class ForkedSPSGLDisplay : SPSGLDisplayBase, GLForkable
{
%TypeHeaderCode
#include <GLDisplay.h>
%End

 public:
	ForkedSPSGLDisplay(SIP_PYLIST)[(int argc, char **argv)];
%MethodCode
	GLDISPLAY_CONSTRUCTOR_ARGC_ARGV_A0(sipForkedSPSGLDisplay)
%End
	virtual ~ForkedSPSGLDisplay();

	virtual void createWindow();
	virtual bool isClosed();

	virtual void setTestImage(bool active);

	virtual void refresh();

	virtual void getRates(float *update /Out/, float *refresh /Out/);
	virtual void getNorm(unsigned long *minval /Out/,
			     unsigned long *maxval /Out/,
			     int *autorange /Out/);
	virtual void setNorm(unsigned long minval, unsigned long maxval,
			     int autorange);

	void setRefreshTime(float refresh_time);

};

class CtGLDisplay
{
%TypeHeaderCode
#include <CtGLDisplay.h>
using namespace lima;
%End

 public:
	CtGLDisplay(CtControl *ct_control);
	virtual ~CtGLDisplay();

	virtual void createWindow() = 0;
	virtual bool isClosed() = 0;
	virtual void closeWindow() = 0;
	virtual void refresh() = 0;
	virtual void setTestImage(bool active) = 0;

	virtual void getRates(float *update /Out/,
			      float *refresh /Out/) = 0;
	virtual void getNorm(unsigned long *minval /Out/,
			     unsigned long *maxval /Out/,
			     int *autorange /Out/) = 0;
	virtual void setNorm(unsigned long minval, unsigned long maxval,
			     int autorange) = 0;
};


class CtSPSGLDisplay : CtGLDisplay, GLForkable
{
%TypeHeaderCode
#include <CtGLDisplay.h>
using namespace lima;
%End
 public:
	CtSPSGLDisplay(CtControl *ct_control, SIP_PYLIST)
		       [(CtControl *ct_control,
			 int argc = 0, char **argv = NULL)];
%MethodCode
	GLDISPLAY_CONSTRUCTOR_ARGC_ARGV_A1(sipCtSPSGLDisplay)
%End
 public:
	virtual ~CtSPSGLDisplay();

	void setSpecArray(std::string spec_name, std::string array_name);
	void getSpecArray(std::string& spec_name /Out/,
			  std::string& array_name /Out/);

	virtual void createWindow();
	virtual bool isClosed();
	virtual void closeWindow();
	virtual void refresh();
	virtual void setTestImage(bool active);

	void setRefreshTime(float refresh_time);

	virtual void getRates(float *update /Out/, float *refresh /Out/);
	virtual void getNorm(unsigned long *minval /Out/,
			     unsigned long *maxval /Out/,
			     int *autorange /Out/);
	virtual void setNorm(unsigned long minval, unsigned long maxval,
			     int autorange);

};

%Import ../../../sip/core/limacore_tmp.sip

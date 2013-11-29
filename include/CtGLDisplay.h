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
#ifndef __CTGLDISPLAY_H__
#define __CTGLDISPLAY_H__

#include "GLDisplay.h"
#include "CtControl.h"

class CtGLDisplay
{
 public:
	CtGLDisplay(lima::CtControl *ct_control);
	virtual ~CtGLDisplay();

	virtual void createWindow() = 0;
	virtual bool isClosed() = 0;
	virtual void closeWindow() = 0;
	virtual void refresh() = 0;
	virtual void setTestImage(bool active) = 0;

	virtual void getRates(float *update, float *refresh) = 0;
	virtual void getNorm(unsigned long *minval, unsigned long *maxval,
			     int *autorange) = 0;
	virtual void setNorm(unsigned long minval, unsigned long maxval,
			     int autorange) = 0;

 protected:
	lima::CtControl *m_ct_control;
};


class CtSPSGLDisplay : public CtGLDisplay
{
 public:
	CtSPSGLDisplay(lima::CtControl *ct_control,
		       int argc = 0, char **argv = NULL);
	virtual ~CtSPSGLDisplay();

	void setSpecArray(std::string spec_name, std::string array_name);
	void getSpecArray(std::string& spec_name, std::string& array_name);

	virtual void createWindow();
	virtual bool isClosed();
	virtual void closeWindow();
	virtual void refresh();
	virtual void setTestImage(bool active);

	void setRefreshTime(float refresh_time);

	virtual void getRates(float *update, float *refresh);
	virtual void getNorm(unsigned long *minval, unsigned long *maxval,
			     int *autorange);
	virtual void setNorm(unsigned long minval, unsigned long maxval,
			     int autorange);

 private:
	static const double DefaultRefreshTime;

	static void processlibForkCleanup(void *data);

	SPSGLDisplayBase *m_sps_gl_display;
	bool m_use_forked;
};

#endif // __CTGLDISPLAY_H__

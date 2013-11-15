//###########################################################################
// This file is part of ProcessLib, a submodule of LImA project the
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
#ifndef __GLDISPLAY_H__
#define __GLDISPLAY_H__

#include <sstream>
#include "SinkTask.h"
#include "imageapi.h"
#include "Debug.h"
#include "CtControl.h"

using namespace lima;

namespace Tasks
{

class GLDisplay;

class GLDisplayTask : public SinkTaskBase
{
	DEB_CLASS_NAMESPC(DebModControl, "GLDisplayTask", "Tasks");
 public:
	GLDisplayTask(GLDisplay *display) : m_display(display) {}
	GLDisplayTask(const GLDisplayTask& o) : m_display(o.m_display) {}

	virtual void process(Data&);
	
 private:
	GLDisplay *m_display;
};

class DLL_EXPORT GLDisplay 
{
	DEB_CLASS_NAMESPC(DebModControl, "GLDisplay", "Tasks");
 public:
	GLDisplay(CtControl *ct);
	~GLDisplay();
	
	void setActive(bool active, int run_level);
	void prepare();
	
 private:
	friend class GLDisplayTask;

	static bool image_inited;
	image_t m_image;
	CtControl *m_ct;
	SoftOpInstance m_softopinst;
};

}

#endif // __GLDISPLAY_H__

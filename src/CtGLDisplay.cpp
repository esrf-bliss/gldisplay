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

#include "CtGLDisplay.h"
#include "CtSpsImage.h"

using namespace lima;
using namespace std;

#include "PoolThreadMgr.h"

CtGLDisplay::CtGLDisplay(CtControl *ct_control)
	: m_ct_control(ct_control), m_refresh_time(10e-3)
{
}

CtGLDisplay::~CtGLDisplay()
{
}

void CtGLDisplay::setRefreshTime(double refresh_time)
{
	m_refresh_time = refresh_time;
}

void CtGLDisplay::getRefreshTime(double& refresh_time)
{
	refresh_time = m_refresh_time;
}


CtSPSGLDisplay::CtSPSGLDisplay(CtControl *ct_control, int argc, char **argv)
	: CtGLDisplay(ct_control)
{
	m_sps_gl_display = new SPSGLDisplay(argc, argv);
}

CtSPSGLDisplay::~CtSPSGLDisplay()
{
	closeWindow();
}

void CtSPSGLDisplay::closeWindow()
{
	if (m_sps_gl_display)
		delete m_sps_gl_display;
	m_sps_gl_display = NULL;
}

void CtSPSGLDisplay::createWindow()
{
	m_sps_gl_display->createForkedWindow(m_refresh_time, 
						     processlibForkCleanup, 
						     NULL);
}

bool CtSPSGLDisplay::isClosed()
{
	return m_sps_gl_display->isClosed();
}

void CtSPSGLDisplay::refresh()
{
	return m_sps_gl_display->refresh();
}

void CtSPSGLDisplay::processlibForkCleanup(void *data)
{
	PoolThreadMgr::get().setThreadWaitOnQuit(false);
}

void CtSPSGLDisplay::setSpecArray(string spec_name, string array_name)
{
	CtSpsImage *ct_display = m_ct_control->display();
	ct_display->setNames(spec_name, array_name);
	ct_display->setActive(true);
	m_sps_gl_display->setSpecArray(spec_name, array_name);
}

void CtSPSGLDisplay::getSpecArray(string& spec_name, string& array_name)
{
	m_sps_gl_display->getSpecArray(spec_name, array_name);
}

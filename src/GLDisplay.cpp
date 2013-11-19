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
#include "GLDisplay.h"

#include "image.h"
#include "sps.h"

#include <iostream>
#include <sstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

//-------------------------------------------------------------
// GLDisplay
//-------------------------------------------------------------

ImageLib *GLDisplay::m_image_lib = NULL;

GLDisplay::GLDisplay(int argc, char **argv)
{
	if (!m_image_lib)
		m_image_lib = new ImageLib(argc, argv);

	m_image_window = NULL;
	m_window_closed = false;
}

GLDisplay::~GLDisplay()
{
	if (m_image_window)
		delete m_image_window;
}

void GLDisplay::createWindow(string caption)
{
	m_image_window = m_image_lib->createImage(caption.c_str());
	m_window_closed = false;
	m_image_window->setCloseCB(windowClosedCB, this);
}

ImageWindow *GLDisplay::getImageWindow()
{
	if (!m_image_window)
		throw exception();
	return m_image_window;
}

bool GLDisplay::isClosed()
{
	return m_window_closed;
}

void GLDisplay::windowClosedCB(void *cb_data)
{
	GLDisplay *sps_gl_display = static_cast<GLDisplay *>(cb_data);
	sps_gl_display->m_window_closed = true;
	sps_gl_display->m_image_window = NULL;
}

void GLDisplay::setBuffer(void *buffer_ptr, int width, int height, int depth)
{
	getImageWindow()->setBuffer(buffer_ptr, width, height, depth);
}

void GLDisplay::updateBuffer()
{
	getImageWindow()->update(false);
}

void GLDisplay::refresh()
{
	m_image_lib->poll();
}


//-------------------------------------------------------------
// SPSGLDisplay
//-------------------------------------------------------------

const int SPSGLDisplay::SPS_TypeDepth[SPS_NrTypes] = {
	0,	// SPS_DOUBLE
	0,	// SPS_FLOAT
	4,	// SPS_INT
	4,	// SPS_UINT
	2,	// SPS_SHORT
	2,	// SPS_USHORT
	1,	// SPS_CHAR
	1,	// SPS_UCHAR
	0,	// SPS_STRING
	4,	// SPS_LONG
	4,	// SPS_ULONG
};

const double SPSGLDisplay::ParentCheckTime = 0.5;

SPSGLDisplay::SPSGLDisplay(int argc, char **argv)
{
	m_gldisplay = new GLDisplay(argc, argv);
	m_buffer_ptr = NULL;
	m_parent_pid = m_child_pid = 0;
	m_child_ended = false;
}

SPSGLDisplay::~SPSGLDisplay()
{
	if (isForkedParent()) {
		kill(m_child_ended, SIGTERM);
	} else {
		delete m_gldisplay;
		releaseBuffer();
	}
}

void SPSGLDisplay::setSpecArray(string spec_name, string array_name)
{
	m_spec_name = spec_name;
	m_array_name = array_name;
	ostringstream os;
	os << m_array_name << "@" << m_spec_name;
	m_caption = os.str();
}

void SPSGLDisplay::getSpecArray(string& spec_name, string& array_name)
{
	spec_name = m_spec_name;
	array_name = m_array_name;
}

void SPSGLDisplay::setCaption(string caption)
{
	m_caption = caption;
}

void SPSGLDisplay::createWindow()
{
	m_gldisplay->createWindow(m_caption);
}

void SPSGLDisplay::createForkedWindow(double refresh_time,
				      ForkCleanup *fork_cleanup,
				      void *cleanup_data)
{
	m_refresh_time = refresh_time;
	m_child_ended = false;
	m_child_pid = fork();
	if (m_child_pid == 0) {
		m_parent_pid = getppid();
		signal(SIGINT, SIG_IGN);
		if (fork_cleanup)
			fork_cleanup(cleanup_data);

		m_gldisplay->createWindow(m_caption);
		runChild();
		exit(0);
	}
}

void SPSGLDisplay::runChild()
{
	int sleep_usec = int(m_refresh_time * 1e6 + 0.1);

	while (!isClosed() && checkParentAlive()) {
		refresh();
		usleep(sleep_usec);
	}
}

bool SPSGLDisplay::checkParentAlive()
{
	static struct timeval t0 = {0,0};
	struct timeval t;

	gettimeofday(&t, NULL);
	double elapsed = ((t.tv_sec - t0.tv_sec) + 
			  (t.tv_usec - t0.tv_usec) * 1e-6);
	if (elapsed < ParentCheckTime)
		return true;

	t0 = t;
	return (kill(m_parent_pid, 0) == 0);
}

bool SPSGLDisplay::isForkedParent()
{
	return (m_child_pid != 0);
}

void SPSGLDisplay::releaseBuffer()
{
	if (m_buffer_ptr) {
		char *spec_name = const_cast<char *>(m_spec_name.c_str());
		char *array_name = const_cast<char *>(m_array_name.c_str());
		SPS_FreeDataCopy(spec_name, array_name);
	}

	m_buffer_ptr = NULL;
	m_width = m_height = m_depth = 0;
}

bool SPSGLDisplay::checkSpecArray()
{
	char *spec_name = const_cast<char *>(m_spec_name.c_str());
	char *array_name = const_cast<char *>(m_array_name.c_str());

	int rows, cols, type;
	if (SPS_GetArrayInfo(spec_name, array_name, &rows, &cols, &type,
			     NULL)) {
		releaseBuffer();
		return false;
	}

	void *prev_buffer_ptr = m_buffer_ptr;

	int depth = SPS_TypeDepth[type];
	bool size_changed = ((cols != m_width) || (rows != m_height) || 
			     (depth != m_depth));
	bool need_update = size_changed || SPS_IsUpdated(spec_name, 
							 array_name);
	if (need_update)
		m_buffer_ptr = SPS_GetDataCopy(spec_name, array_name, type,
					       &rows, &cols);
	if (size_changed) {
		SPS_IsUpdated(spec_name, array_name); 	// force update sync
		m_width = cols;
		m_height = rows;
		m_depth = depth;
	}

	if (size_changed || (m_buffer_ptr != prev_buffer_ptr))
		m_gldisplay->setBuffer(m_buffer_ptr, m_width, m_height, 
				       m_depth);

	return need_update;
}


bool SPSGLDisplay::isClosed()
{
	if (!isForkedParent())
		return m_gldisplay->isClosed();

	if (!m_child_ended)
		m_child_ended = (waitpid(m_child_pid, NULL, WNOHANG) != 0);

	return m_child_ended;
}

void SPSGLDisplay::refresh()
{
	if (isForkedParent())
		return;

	if (checkSpecArray())
		m_gldisplay->updateBuffer();
	m_gldisplay->refresh();
}


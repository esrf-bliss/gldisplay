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
#ifndef __GLDISPLAY_H__
#define __GLDISPLAY_H__

#include <string>
#include <vector>
#include "sps.h"

class ImageWindow;
class ImageLib;

class GLDisplay
{
 public:
	GLDisplay(int argc, char **argv);
	~GLDisplay();

	void createWindow(std::string caption);
	bool isClosed();

	void setBuffer(void *buffer_ptr, int width, int height, int depth);
	void updateBuffer();

	void refresh();

 private:
	ImageWindow *getImageWindow();
	static void windowClosedCB(void *cb_data);

	ImageWindow *m_image_window;
	bool m_window_closed;

	static ImageLib *m_image_lib;
};


class SPSGLDisplay
{
 public:
	typedef void ForkCleanup(void *cleanup_data);

	SPSGLDisplay(int argc, char **argv);
	~SPSGLDisplay();

	void setSpecArray(std::string spec_name, std::string array_name);

	void setCaption(std::string caption);
	void createWindow();
	void createForkedWindow(double refresh_time,
				ForkCleanup *fork_cleanup = NULL,
				void *cleanup_data = NULL);
	bool isClosed();

	void refresh();

 private:
	bool checkSpecArray();
	void releaseBuffer();

	bool isForkedParent();
	void runChild();
	bool checkParentAlive();

	std::string m_spec_name;
	std::string m_array_name;
	void *m_buffer_ptr;
	int m_width;
	int m_height;
	int m_depth;
	GLDisplay *m_gldisplay;
	std::string m_caption;

	int m_parent_pid;
	int m_child_pid;
	bool m_child_ended;
	double m_refresh_time;

	static const double ParentCheckTime;

	enum {
		SPS_NrTypes = 11,
	};
	static const int SPS_TypeDepth[SPS_NrTypes];
};

#endif // __GLDISPLAY_H__

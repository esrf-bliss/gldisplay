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

#include "SimplePipe.h"
#include "AutoObj.h"

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

	void setTestImage(bool active);

	void refresh();

	void getRates(float *update, float *refresh);
	void getNorm(unsigned long *minval, unsigned long *maxval,
		     int *autorange);
	void setNorm(unsigned long minval, unsigned long maxval,
		     int autorange);

	static void Sleep(float sleep_time);

 private:
	ImageWindow *getImageWindow();
	static void windowClosedCB(void *cb_data);

	ImageWindow *m_image_window;
	bool m_window_closed;

	static ImageLib *m_image_lib;
};


class SPSGLDisplayBase
{
 public:
	SPSGLDisplayBase(int argc, char **argv);
	virtual ~SPSGLDisplayBase();

	void setSpecArray(std::string spec_name, std::string array_name);
	void getSpecArray(std::string& spec_name, std::string& array_name);

	void setCaption(std::string caption);

	virtual void createWindow() = 0;
	virtual bool isClosed() = 0;

	virtual void setTestImage(bool active) = 0;

	virtual void refresh() = 0;

	virtual void getRates(float *update, float *refresh) = 0;
	virtual void getNorm(unsigned long *minval, unsigned long *maxval,
			     int *autorange) = 0;
	virtual void setNorm(unsigned long minval, unsigned long maxval,
			     int autorange) = 0;

 protected:
	bool checkSpecArray();
	void releaseBuffer();

	std::string m_spec_name;
	std::string m_array_name;
	void *m_buffer_ptr;
	int m_width;
	int m_height;
	int m_depth;
	GLDisplay *m_gldisplay;
	std::string m_caption;

	enum {
		SPS_NrTypes = 11,
	};
	static const int SPS_TypeDepth[SPS_NrTypes];
};

class LocalSPSGLDisplay : public SPSGLDisplayBase
{
 public:
	LocalSPSGLDisplay(int argc, char **argv);
	virtual ~LocalSPSGLDisplay();

	virtual void createWindow();
	virtual bool isClosed();

	virtual void setTestImage(bool active);

	virtual void refresh();

	virtual void getRates(float *update, float *refresh);
	virtual void getNorm(unsigned long *minval, unsigned long *maxval,
			     int *autorange);
	virtual void setNorm(unsigned long minval, unsigned long maxval,
			     int autorange);

 private:
};

class GLForkable;

class GLForkCallback
{
 public:
	GLForkCallback();
	virtual ~GLForkCallback();

 protected:
	virtual void execInForked() = 0;

 private:
	friend class GLForkable;
	GLForkable *m_forkable;
};

class GLForkable
{
 public:
	GLForkable();
	~GLForkable();

	void addForkCallback(GLForkCallback *fork_cb);
	void removeForkCallback(GLForkCallback *fork_cb);

	void execInForked();

 private:
	typedef std::vector<GLForkCallback *> ForkCbList;
	ForkCbList m_fork_cb_list;
};

class ForkedSPSGLDisplay : public SPSGLDisplayBase, public GLForkable
{
 public:
	typedef void ForkCleanup(void *cleanup_data);

	ForkedSPSGLDisplay(int argc, char **argv);
	virtual ~ForkedSPSGLDisplay();

	virtual void createWindow();
	virtual bool isClosed();

	virtual void setTestImage(bool active);

	virtual void refresh();

	virtual void getRates(float *update, float *refresh);
	virtual void getNorm(unsigned long *minval, unsigned long *maxval,
			     int *autorange);
	virtual void setNorm(unsigned long minval, unsigned long maxval,
			     int autorange);

	void setRefreshTime(float refresh_time);

 private:
	void runChild();
	bool checkParentAlive();

	std::string sendChildCmd(std::string cmd);
	std::string checkParentCmd();
	bool processParentCmd(std::string cmd_str);

	int m_parent_pid;
	int m_child_pid;
	lima::AutoPtr<Pipe> m_cmd_pipe;
	lima::AutoPtr<Pipe> m_res_pipe;
	bool m_child_ended;
	float m_refresh_time;

	static const float ParentCheckTime;

	enum {
		CmdQuit,
		CmdTestImage,
		CmdGetRates,
		CmdGetNorm,
		CmdSetNorm,
		CmdSetRefreshTime,
		NrCmd,
	};
	static const std::string CmdList[NrCmd];
};

#endif // __GLDISPLAY_H__

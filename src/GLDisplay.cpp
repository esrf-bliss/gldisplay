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
#include <unistd.h>

using namespace std;

//-------------------------------------------------------------
// Debug helper
//-------------------------------------------------------------

class nullstreambuff : public streambuf
{};

class nullstream : public ostream
{
public:
	nullstream() : ostream(new nullstreambuff) {}
} cnull;


#ifdef DEBUG
#define debug cout
#else
#define debug cnull
#endif

//-------------------------------------------------------------
// GLDisplay
//-------------------------------------------------------------

inline void Sleep(float sleep_time)
{
	GLDisplay::Sleep(sleep_time);
}


//-------------------------------------------------------------
// GLDisplay
//-------------------------------------------------------------

ImageLib *GLDisplay::m_image_lib = NULL;

void GLDisplay::Sleep(float sleep_time)
{
	int sleep_usec = int(sleep_time * 1e6 + 0.1);
	usleep(sleep_usec);
}

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

void GLDisplay::setTestImage(bool active)
{
	m_image_lib->setTestImage(getImageWindow(), active);
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

void GLDisplay::getRates(float *update, float *refresh)
{
	getImageWindow()->getRates(update, refresh);
}

void GLDisplay::getNorm(unsigned long *minval, unsigned long *maxval,
			int *autorange)
{
	getImageWindow()->getNorm(minval, maxval, autorange);
}

void GLDisplay::setNorm(unsigned long minval, unsigned long maxval,
			int autorange)
{
	getImageWindow()->setNorm(minval, maxval, autorange);
}


//-------------------------------------------------------------
// SPSGLDisplayBase
//-------------------------------------------------------------

const int SPSGLDisplayBase::SPS_TypeDepth[SPS_NrTypes] = {
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

SPSGLDisplayBase::SPSGLDisplayBase(int argc, char **argv)
{
	m_gldisplay = new GLDisplay(argc, argv);
	m_buffer_ptr = NULL;
}

SPSGLDisplayBase::~SPSGLDisplayBase()
{
	delete m_gldisplay;
	releaseBuffer();
}

void SPSGLDisplayBase::setSpecArray(string spec_name, string array_name)
{
	m_spec_name = spec_name;
	m_array_name = array_name;
	ostringstream os;
	os << m_array_name << "@" << m_spec_name;
	m_caption = os.str();
}

void SPSGLDisplayBase::getSpecArray(string& spec_name, string& array_name)
{
	spec_name = m_spec_name;
	array_name = m_array_name;
}

void SPSGLDisplayBase::setCaption(string caption)
{
	m_caption = caption;
}

void SPSGLDisplayBase::releaseBuffer()
{
	if (m_buffer_ptr) {
		char *spec_name = const_cast<char *>(m_spec_name.c_str());
		char *array_name = const_cast<char *>(m_array_name.c_str());
		SPS_FreeDataCopy(spec_name, array_name);
	}

	m_buffer_ptr = NULL;
	m_width = m_height = m_depth = 0;
}

bool SPSGLDisplayBase::checkSpecArray()
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


//-------------------------------------------------------------
// LocalSPSGLDisplay
//-------------------------------------------------------------

LocalSPSGLDisplay::LocalSPSGLDisplay(int argc, char **argv)
	: SPSGLDisplayBase(argc, argv)
{
}

LocalSPSGLDisplay::~LocalSPSGLDisplay()
{
}

void LocalSPSGLDisplay::createWindow()
{
	m_gldisplay->createWindow(m_caption);
}

void LocalSPSGLDisplay::setTestImage(bool active)
{
	m_gldisplay->setTestImage(active);
}

bool LocalSPSGLDisplay::isClosed()
{
	return m_gldisplay->isClosed();
}

void LocalSPSGLDisplay::refresh()
{
	if (checkSpecArray())
		m_gldisplay->updateBuffer();
	m_gldisplay->refresh();
}

void LocalSPSGLDisplay::getRates(float *update, float *refresh)
{
	m_gldisplay->getRates(update, refresh);
}

void LocalSPSGLDisplay::getNorm(unsigned long *minval, unsigned long *maxval,
				int *autorange)
{
	m_gldisplay->getNorm(minval, maxval, autorange);
}

void LocalSPSGLDisplay::setNorm(unsigned long minval, unsigned long maxval,
				int autorange)
{
	m_gldisplay->setNorm(minval, maxval, autorange);
}


//-------------------------------------------------------------
// ForkedSPSGLDisplay
//-------------------------------------------------------------

const float ForkedSPSGLDisplay::ParentCheckTime = 0.5;

const string ForkedSPSGLDisplay::CmdList[NrCmd] = {
	"quit",
	"testimage",
	"getrates",
	"getnorm",
	"setnorm",
	"setrefreshtime",
};

ForkedSPSGLDisplay::ForkedSPSGLDisplay(int argc, char **argv)
	: SPSGLDisplayBase(argc, argv)
{
	m_parent_pid = m_child_pid = 0;
	m_child_ended = false;
	m_fork_cleanup = NULL;
	m_cleanup_data = NULL;
}

ForkedSPSGLDisplay::~ForkedSPSGLDisplay()
{
	if (!isClosed()) {
		sendChildCmd(CmdList[CmdQuit]);
		while (!isClosed())
			Sleep(m_refresh_time);
		debug << "Child quited" << endl;
	}
}

void ForkedSPSGLDisplay::setForkCleanup(ForkCleanup *fork_cleanup,
					void *cleanup_data)
{
	m_fork_cleanup = fork_cleanup;
	m_cleanup_data = cleanup_data;
}

void ForkedSPSGLDisplay::createWindow()
{
	m_cmd_pipe = new Pipe();
	m_res_pipe = new Pipe();

	m_child_ended = false;
	m_child_pid = fork();
	if (m_child_pid == 0) {
		m_cmd_pipe->close(Pipe::WriteFd);
		m_res_pipe->close(Pipe::ReadFd);

		m_parent_pid = getppid();
		signal(SIGINT, SIG_IGN);
		if (m_fork_cleanup)
			m_fork_cleanup(m_cleanup_data);

		runChild();
	} else {
		m_cmd_pipe->close(Pipe::ReadFd);
		m_res_pipe->close(Pipe::WriteFd);
	}
}

void ForkedSPSGLDisplay::runChild()
{
	m_gldisplay->createWindow(m_caption);
	while (!m_gldisplay->isClosed() && checkParentAlive()) {
		if (checkSpecArray())
			m_gldisplay->updateBuffer();
		m_gldisplay->refresh();

		string cmd;
		try {
			cmd = checkParentCmd();
		} catch (...) {
			continue;
		}
		if (cmd.size() && processParentCmd(cmd)) {
			debug << "Quiting child" << endl;
			break;
		}

		Sleep(m_refresh_time);
	}

	releaseBuffer();
	delete m_gldisplay;

	_exit(0);
}

bool ForkedSPSGLDisplay::checkParentAlive()
{
	static Rate check_rate(1 / ParentCheckTime);
	if (!check_rate.isTime())
		return true;

	debug << "Checking parent" << endl;
	return (kill(m_parent_pid, 0) == 0);
}

string ForkedSPSGLDisplay::sendChildCmd(string cmd)
{
	cmd.append("\n");
	debug << "Sending: " << cmd;
	m_cmd_pipe->write(cmd);

	string ans;
	ans = m_res_pipe->readLine(1024, "\n");

	debug << "Replied: " << ans;
	int sep_spc = (ans.size() > 3) ? 1 : 0;
	string ok_str = string("OK") + (sep_spc ? " " : "");
	int ok_len = ok_str.size();
	if (ans.substr(0, ok_len) != ok_str) {
		cerr << "Invalid ans: '" << ans << "'" << endl;
		throw exception();
	}

	return ans.substr(ok_len, ans.size() - (ok_len + 1));
}

string ForkedSPSGLDisplay::checkParentCmd()
{
	string cmd = m_cmd_pipe->readLine(1024, "\n", 0);
	if (!cmd.size())
		return cmd;

	string::iterator it = cmd.end() - 1;
	bool bad_cmd = (*it != '\n');
	if (!bad_cmd) {
		cmd.erase(it);
		bad_cmd = (cmd.find('\n') != cmd.npos);
	}
	if (bad_cmd) {
		cerr << "Invalid cmd: '" << cmd << "'" << endl;
		throw exception();
	}

	debug << "Received: " << cmd << endl;
	return cmd;
}

bool ForkedSPSGLDisplay::processParentCmd(string cmd_str)
{
	istringstream is(cmd_str);
	string main_cmd, *cmd_ptr = const_cast<string *>(CmdList);
	is >> main_cmd;
	int cmd;
	for (cmd = 0; cmd < NrCmd; ++cmd, ++cmd_ptr)
		if (main_cmd == *cmd_ptr)
			break;

	string ans = "OK";
	bool quit = false;
	if (cmd == NrCmd) {
		cerr << "Unknown command: " << cmd_str << endl;
		ans = "ERROR";
	} if (cmd == CmdQuit) {
		quit = true;
	} else if (cmd == CmdTestImage) {
		int active;
		is >> active;
		m_gldisplay->setTestImage(active);
	} else if (cmd == CmdGetRates) {
		float update, refresh;
		m_gldisplay->getRates(&update, &refresh);
		ostringstream os;
		os << ans << " " << update << " " << refresh;
		ans = os.str();
	} else if (cmd == CmdGetNorm) {
		unsigned long minval, maxval;
		int autorange;
		m_gldisplay->getNorm(&minval, &maxval, &autorange);
		ostringstream os;
		os << ans << " " << minval << " " << maxval << " "
		   << autorange;
		ans = os.str();
	} else if (cmd == CmdSetNorm) {
		unsigned long minval, maxval;
		int autorange;
		is >> minval >> maxval >> autorange;
		m_gldisplay->setNorm(minval, maxval, autorange);
	} else if (cmd == CmdSetRefreshTime) {
		is >> m_refresh_time;
	}

	ans.append("\n");
	debug << "Answering: " << ans;
	m_res_pipe->write(ans);

	return quit;
}

void ForkedSPSGLDisplay::setTestImage(bool active)
{
	ostringstream os;
	os << CmdList[CmdTestImage] << " " << int(active);
	sendChildCmd(os.str());
}

bool ForkedSPSGLDisplay::isClosed()
{
	if (!m_child_ended)
		m_child_ended = (waitpid(m_child_pid, NULL, WNOHANG) != 0);

	return m_child_ended;
}

void ForkedSPSGLDisplay::refresh()
{
	return;
}

void ForkedSPSGLDisplay::getRates(float *update, float *refresh)
{
	string ans = sendChildCmd(CmdList[CmdGetRates]);
	istringstream is(ans);
	is >> *update >> *refresh;
}

void ForkedSPSGLDisplay::getNorm(unsigned long *minval, unsigned long *maxval,
				 int *autorange)
{
	string ans = sendChildCmd(CmdList[CmdGetNorm]);
	cout << "ans: '" << ans << "'" << endl;
	istringstream is(ans);
	is >> *minval >> *maxval >> *autorange;
}

void ForkedSPSGLDisplay::setNorm(unsigned long minval, unsigned long maxval,
				 int autorange)
{
	ostringstream os;
	os << CmdList[CmdSetNorm] << " " << minval << " " << maxval
	   << " " << autorange;
	sendChildCmd(os.str());
}

void ForkedSPSGLDisplay::setRefreshTime(float refresh_time)
{
	ostringstream os;
	os << CmdList[CmdSetRefreshTime] << " " << refresh_time;
	sendChildCmd(os.str());
}


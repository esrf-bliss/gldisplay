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
#include "SimulatorInterface.h"
#include "PoolThreadMgr.h"
#include "CtControl.h"
#include "CtAcquisition.h"
#include "CtSpsImage.h"
#include "MiscUtils.h"

#include "GLDisplay.h"
#include <iostream>

using namespace lima;
using namespace std;

Simulator::GaussPeak peak_list[] = {
	Simulator::GaussPeak(924, 512, 100, 100),
	Simulator::GaussPeak(757, 512,  30,  80),
	Simulator::GaussPeak(562, 512,  50, 120),
	Simulator::GaussPeak(652, 512,  10, 130),
};
std::vector<Simulator::GaussPeak> peaks(C_LIST_ITERS(peak_list));

double peak_angle_list[] = {
	180,
	110,
	20,
	305,
};
std::vector<double> peak_angles(C_LIST_ITERS(peak_angle_list));

void fork_cleanup(void *data)
{
	PoolThreadMgr::get().setThreadWaitOnQuit(false);
}

int main(int argc, char *argv[])
{
	double exp_time = 0.1;
	int nb_frames = 180;
	double refresh_time = 0.01;

	char *spec_name = "GLDisplayTest";
	char *array_name = "Simulator";

	argc--, argv++;
	SPSGLDisplay sps_gl_display(argc, argv);
	sps_gl_display.setSpecArray(spec_name, array_name);
	sps_gl_display.createForkedWindow(refresh_time, fork_cleanup, NULL);

	Simulator::Camera simu;
	Simulator::FrameBuilder *fb = simu.getFrameBuilder();
	fb->setPeaks(peaks);
	fb->setPeakAngles(peak_angles);
	fb->setRotationSpeed(360 / nb_frames);

	Simulator::Interface simu_hw(simu);
	CtControl *ct = new CtControl(&simu_hw);

	CtAcquisition *ct_acq = ct->acquisition();
	CtSpsImage *ct_display = ct->display();

	ct_acq->setAcqExpoTime(exp_time);
	ct_acq->setAcqNbFrames(nb_frames);
	ct_display->setNames(spec_name, array_name);
	ct_display->setActive(true);

	ct->prepareAcq();
	ct->startAcq();

	while (!sps_gl_display.isClosed()) {
		sps_gl_display.refresh();
		Sleep(refresh_time);
	}

	CtControl::Status ct_status;
	ct->getStatus(ct_status);

	if (ct_status.AcquisitionStatus == AcqRunning) {
		ct->stopAcq();
		while (ct_status.AcquisitionStatus == AcqRunning) {
			Sleep(refresh_time);
			ct->getStatus(ct_status);
		}
	}

	return 0;
}

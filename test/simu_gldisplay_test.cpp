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
#include "CtControl.h"
#include "CtAcquisition.h"
#include "MiscUtils.h"
#include "prectime.h"
#include "CtGLDisplay.h"
#include <iostream>
#include <iomanip>

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

const double TestAlternatePeriod = 5.0;

int main(int argc, char *argv[])
{
	double exp_time = 0.1;
	int nb_frames = 180;
	double refresh_time = 0.01;

	bool alternate_test_image = false;

	char *spec_name = "GLDisplayTest";
	char *array_name = "Simulator";

	if ((argc > 1) && (string(argv[1]) == "--alternate-test"))
		alternate_test_image = true;

	Simulator::Camera simu;
	Simulator::FrameBuilder *simu_fb = simu.getFrameBuilder();
	simu_fb->setPeaks(peaks);
	simu_fb->setPeakAngles(peak_angles);
	simu_fb->setRotationSpeed(360 / nb_frames);

	Simulator::Interface simu_hw(simu);
	CtControl *ct_control = new CtControl(&simu_hw);
	CtAcquisition *ct_acq = ct_control->acquisition();

	ct_acq->setAcqExpoTime(exp_time);
	ct_acq->setAcqNbFrames(nb_frames);

	CtSPSGLDisplay *ct_gl_display = new CtSPSGLDisplay(ct_control, 
							   argc, argv);
	ct_gl_display->setSpecArray(spec_name, array_name);
	ct_gl_display->createWindow();

	ct_control->prepareAcq();
	ct_control->startAcq();

	int test_image = 0;
	Rate test_change_rate(1 / TestAlternatePeriod);
	Rate rates_refresh_rate(1);

	while (!ct_gl_display->isClosed()) {
		ct_gl_display->refresh();
		Sleep(refresh_time);

		if (alternate_test_image && test_change_rate.isTime()) {
			test_image ^= 1;
			cout << "Setting test image to " << test_image << endl;
			ct_gl_display->setTestImage(test_image);
		}

		if (rates_refresh_rate.isTime()) {
			float update, refresh;
			ct_gl_display->getRates(&update, &refresh);
			cout << fixed << setprecision(1)
			     << "update: " << update << ", "
			     << "refresh: " << refresh << endl;
		}
	}

	CtControl::Status ct_status;
	ct_control->getStatus(ct_status);

	if (ct_status.AcquisitionStatus == AcqRunning) {
		ct_control->stopAcq();
		while (ct_status.AcquisitionStatus == AcqRunning) {
			Sleep(refresh_time);
			ct_control->getStatus(ct_status);
		}
	}

	delete ct_gl_display;
	delete ct_control;

	return 0;
}

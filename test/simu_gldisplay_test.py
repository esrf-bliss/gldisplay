############################################################################
# This file is part of gldisplay, a submodule of LImA project the
# Library for Image Acquisition
#
# Copyright (C) : 2009-2011
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http:#www.gnu.org/licenses/>.
############################################################################

from Lima import Core
from Lima import Simulator
from Lima import GLDisplay

import sys
import time

peaks = [
	Simulator.GaussPeak(924, 512, 100, 100),
	Simulator.GaussPeak(757, 512,  30,  80),
	Simulator.GaussPeak(562, 512,  50, 120),
	Simulator.GaussPeak(652, 512,  10, 130),
]

peak_angles = [
	180.0,
	110.0,
	20.0,
	305.0,
]

TestAlternatePeriod = 5.0
refresh_time = 0.01
alternate_test_image = False
int_trig_mult = False
acq_stopped = False

class ImageStatusCallback(Core.CtControl.ImageStatusCallback):

	def __init__(self, simu, ct_control):
		super(ImageStatusCallback, self).__init__()
		self.simu = simu
		self.ct_control = ct_control
		self.last_acq_frame = -1

	def imageStatusChanged(self, status):
		last_acq_frame = status.LastImageAcquired
		if last_acq_frame == self.last_acq_frame:
			return
		self.last_acq_frame = last_acq_frame
		ct_acq = self.ct_control.acquisition()
		last_frame = ct_acq.getAcqNbFrames() - 1
		if self.last_acq_frame == last_frame:
			return
		if acq_stopped:
			return
		ct_status = self.ct_control.getStatus()
		if ct_status.AcquisitionStatus != Core.AcqRunning:
			return
		Ready = Core.HwInterface.StatusType.Ready
		while self.simu.getStatus() != Ready:
			time.sleep(0.01)
		self.ct_control.startAcq()

def refresh_loop(ct_gl_display):
	test_image = 0
	test_change_t0 = time.time()
	rates_refresh_t0 = time.time()

	while not ct_gl_display.isClosed():
		ct_gl_display.refresh()
		time.sleep(refresh_time)

		dt = time.time() - test_change_t0
		if alternate_test_image and (dt > TestAlternatePeriod):
			test_image ^= 1
			print 'Setting test image to %s' % test_image
			ct_gl_display.setTestImage(test_image)
			test_change_t0 = time.time()

		dt = time.time() - rates_refresh_t0
		if dt > 1:
			update, refresh = ct_gl_display.getRates()
			print 'update: %.1f, refresh: %.1f' % (update, refresh)
			rates_refresh_t0 = time.time()

def main(argv):
	global alternate_test_image, int_trig_mult, acq_stopped

	exp_time = 0.1
	nb_frames = 180

	spec_name = 'GLDisplayTest'
	array_name = 'Simulator'

	for arg in argv[1:]:
		if arg == '--alternate-test':
			alternate_test_image = True
		if arg == '--int-trig-mult':
			int_trig_mult = True

	trig_mode = Core.IntTrigMult if int_trig_mult else Core.IntTrig

	simu = Simulator.Camera()
	simu_fb = simu.getFrameBuilder()
	simu_fb.setPeaks(peaks)
	simu_fb.setPeakAngles(peak_angles)
	simu_fb.setRotationSpeed(360 / nb_frames)
	simu_fb.setGrowFactor(0)

	simu_hw = Simulator.Interface(simu)
	ct_control = Core.CtControl(simu_hw)
	ct_acq = ct_control.acquisition()

	ct_acq.setAcqExpoTime(exp_time)
	ct_acq.setAcqNbFrames(nb_frames)
	ct_acq.setTriggerMode(trig_mode)

	ct_gl_display = GLDisplay.CtSPSGLDisplay(ct_control, argv)
	ct_gl_display.setSpecArray(spec_name, array_name)
	ct_gl_display.createWindow()

	if trig_mode == Core.IntTrigMult:
		cb = ImageStatusCallback(simu, ct_control)
		ct_control.registerImageStatusCallback(cb)

	ct_control.prepareAcq()
	ct_control.startAcq()

	try:
		refresh_loop(ct_gl_display)
	except KeyboardInterrupt:
		pass

	ct_status = ct_control.getStatus()

	if ct_status.AcquisitionStatus == Core.AcqRunning:
		acq_stopped = True
		ct_control.stopAcq()
		while ct_status.AcquisitionStatus == Core.AcqRunning:
			time.sleep(refresh_time)
			ct_status = ct_control.getStatus()

	del ct_gl_display
	del ct_control

	return 0


if __name__ == '__main__':
	main(sys.argv)

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

# Very simple program that starts the Simulator with the following:
#
# + Simulator default config
#   + A single fixed peak
#   + Linear intensity grow factor on every frame of the acquisition
# + Lima Control Layer (core) default parameters:
#   + No software processing (Bin, RoI, Automatic Saving, etc.)
# + Overwritten acquisition parameters:
#   + Nb. of frames: 10
#   + Exposure time: 0.1 s
# + CtSPSGLDisplay activates SPS in Control Layer:
#   + Spec/array name: GLDisplayTest/Simulator
#   + Starts the display in forked process (SIGCHLD signal ignored)
#   + The display performs auto-normalisation every second
#
# The acquired (processed) number of frames is updated on the screen

import time
import signal
from Lima import Core, Simulator, GLDisplay

simu = Simulator.Camera()
hw_inter = Simulator.Interface(simu)
ct_control = Core.CtControl(hw_inter)

gldisplay = GLDisplay.CtSPSGLDisplay(ct_control, [])
gldisplay.setSpecArray('GLDisplayTest', 'Simulator')
gldisplay.createWindow()
signal.signal(signal.SIGCHLD, signal.SIG_IGN)

ct_acq = ct_control.acquisition()
ct_acq.setAcqNbFrames(10)
ct_acq.setAcqExpoTime(0.1)

ct_control.prepareAcq()
ct_control.startAcq()
ct_status = ct_control.getStatus
while ct_status().AcquisitionStatus == Core.AcqRunning:
	time.sleep(0.2)
	print 'Frame: %d' % ct_status().ImageCounters.LastImageReady

nframes = ct_status().ImageCounters.LastImageReady + 1
print 'Finished: %d' % nframes

print 'Press any key to quit'
raw_input()


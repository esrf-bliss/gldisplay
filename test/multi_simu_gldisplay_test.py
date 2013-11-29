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

from Lima import GLDisplay

import os
import sys
import time
import struct
from subprocess import Popen

# Run a SPS-based Simulator display
this_prg = sys.argv[0]
this_dir = os.path.dirname(this_prg)
test_prg = os.path.join(this_dir, 'simu_gldisplay_test.py')
test_opts = ['--alternate-test']
p = Popen(['python', test_prg] + test_opts)
time.sleep(2)


# The previous program uses this SPS array
spec_name = 'GLDisplayTest'
array_name = 'Simulator'

# Simple direct, local window (needs refresh polling)
# Display the test image with 16-bit (sHort) data
width, height, spack = 1024, 1024, 'H'

gldisplay = GLDisplay.GLDisplay([])
depth = struct.calcsize(spack)
gldisplay.createWindow('Basic Test')
s = struct.pack(spack, 0) * (width * height)
gldisplay.setBuffer(s, width, height, depth)
gldisplay.setTestImage(1)
gldisplay.refresh()

# SPS local viewer window (needs refresh polling)
# Attach to the launched SPS-based Simulator display array
localspsgldisplay = GLDisplay.LocalSPSGLDisplay([])
localspsgldisplay.setSpecArray(spec_name, array_name)
localspsgldisplay.setCaption('Local %s@%s' % (spec_name, array_name))
localspsgldisplay.createWindow()

# SPS forked viewer window (does not need refresh polling)
# Attach to the same launched SPS-based Simulator display array
forkedspsgldisplay = GLDisplay.ForkedSPSGLDisplay([])
forkedspsgldisplay.setSpecArray(spec_name, array_name)
forkedspsgldisplay.setCaption('Forked %s@%s' % (spec_name, array_name))
forkedspsgldisplay.createWindow()

try:
	while True:
		gldisplay.refresh()
		localspsgldisplay.refresh()
		time.sleep(0.1)
except:
	pass

del forkedspsgldisplay
del localspsgldisplay
del gldisplay
del p
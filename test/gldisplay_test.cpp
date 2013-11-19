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
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;

string prog_name;

void usage()
{
	cout << "Usage: " << prog_name << " [options] spec_name array_name"
	     << endl;
	cout << endl;
	cout << "  Options:" << endl;
	cout << "      -f   Use forked GL display" << endl;

	exit(1);
}

int main(int argc, char *argv[])
{
	prog_name = argv[0];
	argc--, argv++;

	SPSGLDisplay sps_gl_display(argc, argv);

	// search options
	double refresh_time = 0.01;
	bool use_fork = false;
	while ((argc > 0) && ((*argv)[0] == '-')) {
		if (!strcmp(*argv, "-f"))
			use_fork = true;
		argc--, argv++;
	}
	if (argc < 2)
		usage();

	char *spec_name = *argv++;
	char *array_name = *argv++;
	argc -= 2;

	sps_gl_display.setSpecArray(spec_name, array_name);
	if (use_fork)
		sps_gl_display.createForkedWindow(refresh_time);
	else
		sps_gl_display.createWindow();

	while (!sps_gl_display.isClosed()) {
		sps_gl_display.refresh();
		usleep(int(refresh_time * 1e6 + 0.1));
	}

	return 0;
}

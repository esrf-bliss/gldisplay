#include "GLDisplay.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *prog_name = NULL;

void usage()
{
	printf("Usage: %s [options] spec_name array_name\n", prog_name);
	printf("\n");
	printf("  Options:\n");
	printf("      -f   Use forked GL display\n");

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

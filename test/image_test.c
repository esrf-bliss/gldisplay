#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "imageapi.h"

void end_prog(void *data)
{
	int *end_ptr = (int *) data;
	*end_ptr = 1;
}

int main(int argc, char *argv[])
{
	int end, sleep_usec, xsize, ysize, depth, len;
	unsigned short *buffer;
	image_t img;
	struct timeval t0, t1;
	float elapsed;

	xsize = ysize = 512;
	depth = sizeof(*buffer);
	len = xsize * ysize * depth;
	buffer = (unsigned short *) malloc(len);
	if (buffer == NULL) {
		fprintf(stderr, "Error allocating buffer: %d bytes\n", len);
		exit(1);
	}

	image_init(argc, argv);

	image_create(&img, "Test Image");
	image_set_buffer(img, buffer, xsize, ysize, depth);
	image_set_test(img);

	end = 0;
	image_close_cb(img, end_prog, &end);

	sleep_usec = 10000;
	gettimeofday(&t0, NULL);

	while (!end) {
		image_update(img);
		image_poll();

		usleep(sleep_usec);

		gettimeofday(&t1, NULL);
		elapsed = ((t1.tv_sec  - t0.tv_sec) +
			   (t1.tv_usec - t0.tv_usec) * 1e-6);
		if (elapsed >= 0.2) {
			float update, refresh;
			image_get_rates(img, &update, &refresh);
			printf("update: %.1f, refresh: %.1f\n", 
				update, refresh);
			t0 = t1;
		}
	}
		     
	free(buffer);

	return 0;
}


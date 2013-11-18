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
#ifndef __IMAGEAPI_H
#define __IMAGEAPI_H


typedef void *image_t;
#define IMAGE_INVALID	NULL

#ifdef __cplusplus
extern "C" {
#endif

int image_init(int argc, char **argv);
void image_exit(void);

/* warning, for the moment only 16-bit buffers supported */
int image_create(image_t *img_ptr, char *caption);
int image_set_buffer(image_t img, void *buffer, int width, int height, 
		     int depth);

int image_close_cb(image_t img, void (*close_cb)(void *data), void *cb_data);
void image_destroy(image_t img);
void image_update(image_t img);
int image_set_test(image_t img);

void image_get_rates(image_t img, float *update, float *refresh);
void image_get_norm(image_t img, unsigned long *min_val, 
		    unsigned long *max_val, int *auto_range);
void image_set_norm(image_t img, unsigned long min_val, 
		    unsigned long max_val, int auto_range);

int image_poll(void);



#ifdef __cplusplus
}
#endif


#endif /* __IMAGEAPI_H */

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
#define GL_GLEXT_PROTOTYPES

#include <QCloseEvent>
#include <QX11Info>
#include "image.h"
#include "imageapi.h"
#include <GL/glx.h>
#include <GL/glext.h>

#include <math.h>
#include <iostream>
#include <cstdio>
using namespace std;


/********************************************************************
 * ImageContext
 ********************************************************************/

ImageContext::ImageContext(const QGLFormat& format, QPaintDevice *device)
	: QGLContext(format, device)
{
}

void *ImageContext::chooseVisual()
{
	int visualclass = TrueColor;
	QString env = getenv("IMAGE_VISUAL");
	if (env == "DirectColor")
		visualclass = DirectColor;

	// this should allocate XVisualInfo(s)
	XVisualInfo *visual = (XVisualInfo *) QGLContext::chooseVisual();

	Display *display = QX11Info::display();
	if (display == NULL)
		return visual;

	int screen = DefaultScreen(display);

	XVisualInfo vi;
	if (!visual) {
		// get X11 alloc. XVisualInfo: can be XFree'd by ~QGLContext
		vi.screen = screen;
		int nv;
		visual = XGetVisualInfo(display, VisualScreenMask, &vi, &nv);
	}
	if (!visual)
		// OK, last resort: alloc using malloc (better than new?)
		visual = (XVisualInfo *) malloc(sizeof(XVisualInfo));
	if (!visual) {
		cerr << "Error: cannot obtain XVisualInfo memory!" << endl;
		return NULL;
	}

	if (XMatchVisualInfo(display, screen, 24, visualclass, &vi) != 0) {
		*visual = vi;
		if (Image::debug) {
			cout << "Using visual 0x" << hex << vi.visualid 
			     << dec << endl;
		}
	}

	return visual;
}


/********************************************************************
 * Image
 ********************************************************************/

int Image::debug = 0;

Image::Image() 
{ 
	setBuffer(NULL, 0, 0, 0); 
}


Image::Image(void *ptr, unsigned width, unsigned height, unsigned depth) 
{ 
	setBuffer(ptr, width, height, depth); 
}


void Image::setBuffer(void *ptr, unsigned width, unsigned height, 
		      unsigned depth)
{
	buff.init(ptr, depth);
	w = width;
	h = height;
}

void Image::setTestImage(bool active)
{
	ImagePixelPtr p = buff;
	
	for (unsigned int i = 0; i < h; ++i) {
		for (unsigned int j = 0; j < w; ++j) {
			float val = 0;
			if (active)
				val = float(i + j) * maxVal() / (w + h);
			p.store(unsigned(val));
			++p;
		}
	}
}


/********************************************************************
 * ImageWidget
 ********************************************************************/

QString ImageWidget::colormapName(ColormapType cmap)
{
	switch (cmap) {
	case Grayscale:   return "Grayscale";
	case Temperature: return "Temperature";
	default:	  break;
	}
		
	return "Unknown";
}

ImageWidget::ColormapType ImageWidget::colormapType(QString name)
{
	ColormapType cmap;

	for (cmap = Grayscale; cmap < Unknown; cmap = ColormapType(cmap + 1))
		if (colormapName(cmap) == name)
			break;
	return cmap;
}

ImageWidget::ImageWidget(QWidget *parent, ColormapType cmap)
	: QGLWidget(parent)
{
	ImageContext *context;
	context = new ImageContext(QGLFormat(QGL::DoubleBuffer), this);
	setContext(context);

	b_type = GL_UNSIGNED_BYTE;
	x = y = 0;
	factor = 0;
	min_val = max_val = 0;
	auto_range = 1;

	must_resize = 0;
	must_normalize = 0;
	normalize_rate = 1.0;

	colormap = cmap;	
	mapsize = 256;

	draw_mode = GL_LUMINANCE;
	QString env = getenv("IMAGE_DRAW_MODE");
	if (env == "ColorIndex")
		draw_mode = GL_COLOR_INDEX;

	x11nrcolors = 0;

	test_active = 0;
	env = getenv("IMAGE_TEST_PATTERN");
	if ((env == "1") || (env == "Yes"))
		test_active = 1;
}

ImageWidget::~ImageWidget()
{
	if (x11nrcolors > 0) {
		XFreeColormap(QX11Info::display(), x11colormap);
		delete [] x11cmap;
	}
	free(testimage.ptr().vPtr());
}

void ImageWidget::setColormap(ColormapType cmap)
{
	colormap = cmap;

	const GLint nrtempcolors = 4;
	GLfloat tempcolors[nrtempcolors][4] = {
		{0.0, 0.0, 1.0, 1.0},	// blue
		{0.0, 1.0, 0.0, 1.0},	// green
		{1.0, 1.0, 0.0, 1.0},	// yellow
		{1.0, 0.0, 0.0, 1.0},	// red
	};
	float map[mapsize][4];
	for (int i = 0; i < mapsize; i++) {
		for (int j = 0; j < 4; j++) {
			if (colormap == Grayscale) {
				map[i][j] = float(i) / mapsize;
			} else {
				int colorsize = mapsize / (nrtempcolors - 1);
				int base = i / colorsize;
				float factor, val;
				factor = float(i % colorsize) / colorsize;
				val = tempcolors[base + 1][j] * factor;
				val += tempcolors[base][j] * (1 - factor);
				map[i][j] = val;
			}
		}
	}

	if (0 && checkX11Colormap(map, mapsize)) {
		setX11Colormap(map, mapsize);
		return;
	}

	if (draw_mode == GL_LUMINANCE) {
		int ok = checkColorTableColormap(map, mapsize);
		if (!ok) {
			draw_mode = GL_COLOR_INDEX;
			cerr << "GL_ARB_imaging not found. "
			     << "Using GL_COLOR_INDEX draw mode." << endl;
		}
	}

	if (draw_mode == GL_LUMINANCE) {
		if (colormap != Grayscale)
			setColorTableColormap(map, mapsize);
	} else {
		setPixelMapColormap(map, mapsize);
	}
}
	
int ImageWidget::checkColorTableColormap(float map[][4], int size)
{
	char *extensions = (char *) glGetString(GL_EXTENSIONS);
	char *ext = "GL_ARB_imaging";
	char *ptr = strstr(extensions, ext);
	if (!ptr)
		return 0;

	ptr += strlen(ext);
	if (*ptr && (*ptr != ' '))
		return 0;

	int tab = GL_PROXY_COLOR_TABLE;
	glColorTable(tab, GL_RGBA, mapsize, GL_RGBA, GL_FLOAT, map);
	GLint v = 0;
	glGetColorTableParameteriv(tab, GL_COLOR_TABLE_WIDTH, &v);

	return (v == mapsize);
}

void ImageWidget::setColorTableColormap(float map[][4], int size)
{
	glColorTable(GL_COLOR_TABLE, GL_RGBA, size, GL_RGBA, GL_FLOAT, map);
	glEnable(GL_COLOR_TABLE);
}

void ImageWidget::setPixelMapColormap(float map[][4], int size)
{
	GLfloat cimap[4][size];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < size; j++)
			cimap[i][j] = map[j][i];

	glPixelMapfv(GL_PIXEL_MAP_I_TO_R, size, cimap[0]);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_G, size, cimap[1]);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_B, size, cimap[2]);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_A, size, cimap[3]);
}

int ImageWidget::checkX11Colormap(float map[][4], int size)
{
	Display *display = QX11Info::display();
	Window win = winId();
	XWindowAttributes wa;
	if (XGetWindowAttributes(display, win, &wa) == 0)
		return 0;

	Visual *visual = wa.visual;
	VisualID visualid = XVisualIDFromVisual(visual);
	if (Image::debug)
		cout << "visualid=0x" << hex << visualid << dec << endl;

	XVisualInfo match, *vi;
	match.visualid = visualid;
	int nrvi;
	vi = XGetVisualInfo(display, VisualIDMask, &match, &nrvi);

	int ok = 0;
	if (nrvi != 1) {
		cerr << "Could not get visual info" << endl;
	} else if (vi->c_class != DirectColor) {
		cerr << "Visual 0x" << hex << vi->visualid << dec 
		     << " is not of DirectColor class" << endl;
	} else
		ok = 1;

	if (nrvi > 0)
		XFree(vi);
	return ok;
}

void ImageWidget::setX11Colormap(float map[][4], int size)
{
	Display *display = QX11Info::display();
	Visual *visual = (Visual *) x11Info().visual();
	Window win = (Window) winId();

	if (x11nrcolors > 0) {
		XFreeColormap(display, x11colormap);
		delete [] x11cmap;
		x11nrcolors = 0;
	}

	x11colormap = XCreateColormap(display, win, visual, AllocAll);

	XWindowAttributes wa;
	XGetWindowAttributes(display, win, &wa);

	if (Image::debug) {
		cout << "Window attributes:" << endl;
		cout << "x=" << wa.x << " y=" << wa.y 
		     << " width=" << wa.width << " height: " << wa.height 
		     << endl;
		cout << "border_width=" << wa.border_width 
		     << " depth: " << wa.depth << endl;
		cout << "visual=" << (void *) wa.visual 
		     << " (visual=" << (void *) visual << ")" << endl;
		cout << "map_installed=" << wa.map_installed << endl;
		cout << "colormap=" << (void *) wa.colormap 
		     << " (colormap=" << (void *) x11colormap << ")" << endl;
	}

	x11cmap = new unsigned long[size];
	unsigned long rmask, gmask, bmask;
	rmask = gmask = bmask = 0;
//	if (XAllocColorPlanes(display, x11colormap, 1, x11cmap, size, 8, 8, 8, 
//			      &rmask, &gmask, &bmask) == 0)
//		return;

	if (Image::debug) {
		cout << hex << "rmask=" << rmask << " gmask=" << gmask 
		     << " bmask=" << bmask << dec << endl;
	}

	unsigned short max = 65535;

	for (int i = 0; i < size; i++) {
		XColor color;
		color.pixel = i;
		color.red   = (unsigned short) (map[i][0] * max);
		color.green = (unsigned short) (map[i][1] * max);
		color.blue  = (unsigned short) (map[i][2] * max);
		color.flags = DoRed | DoGreen | DoBlue;
		XStoreColor(display, x11colormap, &color);
	}

	XSetWindowColormap(display, win, x11colormap);
	XInstallColormap(display, x11colormap);
	XGetWindowAttributes(display, win, &wa);

	if (Image::debug) {
		cout << "map_installed=" << wa.map_installed << endl;
		cout << "colormap=" << (void *) wa.colormap 
		     << " (colormap=" << (void *) x11colormap << ")" << endl;
	}

	x11nrcolors = size;
}

void ImageWidget::initializeGL()
{
	if (Image::debug)
		cout << "In initializeGL" << endl;

	setColormap(colormap);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ZERO);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_DITHER);
        glDisable(GL_FOG);
        glDisable(GL_LIGHTING);
        glDisable(GL_LOGIC_OP);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_TEXTURE_1D);
        glDisable(GL_TEXTURE_2D);
}

void ImageWidget::resizeGL(int width, int height)
{
	w_width = width;
	w_height = height;

	calcResize();
}

void ImageWidget::calcResize()
{
	glViewport(0, 0, w_width, w_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w_width, 0, w_height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	
	Image& image = getActiveImage();
	if (!image.isValid())
		return;

	int b_width = image.width();
	int b_height = image.height();
	GLfloat xfactor = float(w_width) / b_width;
	GLfloat yfactor = float(w_height) / b_height;
	factor = min(xfactor, yfactor);

	x = int((w_width - b_width * factor) / 2);
	y = int((w_height + b_height * factor) / 2);
	y = min(max(0, y), w_height - 1);

	glRasterPos2i(x, y);
	glPixelZoom(factor, -factor);
	if (Image::debug)
		cout << "calcResize: x=" << x << ", y=" << y << ", "
		     << "factor=" << factor << endl;
}

void ImageWidget::paintGL()
{
        glClear(GL_COLOR_BUFFER_BIT);

	Image& image = getActiveImage();
	if (image.isValid()) {
		if (must_resize)
			calcResize();
		must_resize = 0;

		normalize(must_normalize);
		must_normalize = 0;

		glDrawPixels(image.width(), image.height(), draw_mode, b_type, 
			     image.ptr().vPtr());
	}

	glFlush();
}


int ImageWidget::setBuffer(void *ptr, int width, int height, int depth,
			   bool do_update)
{
	if (!ptr && !width && !height && !depth) {
		if (realimage.isValid()) {
			realimage.setBuffer(NULL, 0, 0, 0);
			reallocTestImage();
			if (w_width && w_height)
				updateImage(false);
		}
		return 0;
	}

	if (!ptr || !width || !height)
		return -1;

	switch (depth) {
	case 1:	b_type = GL_UNSIGNED_BYTE; break;
	case 2: b_type = GL_UNSIGNED_SHORT; break;
	case 4: b_type = GL_UNSIGNED_INT; break;
	default:
		return -1;
	}

	bool first_time = !realimage.isValid();
	bool size_changed = ((width  != int(realimage.width())) || 
			     (height != int(realimage.height())));

	realimage.setBuffer(ptr, width, height, depth);
	if (test_active && (first_time || size_changed))
		reallocTestImage();

	if (!w_width || !w_height) {
		if (Image::debug) 
			cout << "Window has no width/height" << endl;
		return 0;
	}

	if (size_changed)
		must_resize = 1;

	if (first_time || do_update)
		updateImage(first_time);

	return 0;
}

void ImageWidget::reallocTestImage()
{
	free(testimage.ptr().vPtr());
	testimage.setBuffer(NULL, 0, 0, 0);
	if (!realimage.isValid())
		return;

	void *test_buffer = malloc(realimage.size());
	if (!test_buffer)
		throw exception();
	testimage.setBuffer(test_buffer, realimage.width(), 
			    realimage.height(), realimage.depth());
	testimage.setTestImage(test_active);
}

void ImageWidget::updateImage(bool force_norm)
{
	if (force_norm)
		must_normalize = 1;
	updateGL();
}

Image& ImageWidget::getActiveImage()
{
	return *(test_active ? &testimage : &realimage);
}

void ImageWidget::normalize(bool force)
{
	Image& image = getActiveImage();
	ImagePixelPtr ptr = image.ptr();
	int i, len = image.nrPixels();

	if (len == 0)
		return;

	if (!normalize_rate.isTime() && !force)
		return;

	GLuint abs_max_val = image.maxVal();
	if (auto_range) {
		max_val = 0;
		min_val = abs_max_val;

		for (i = 0; i < len; ++i, ++ptr) {
			unsigned val = *ptr;
			if (val > max_val)
				max_val = val;
			if (val < min_val)
				min_val = val;
		}
	}

	float fmin_val = float(min_val) / abs_max_val;
	float fmax_val = float(max_val) / abs_max_val;
	float scale = 1;
	if (fmin_val != fmax_val)
		scale = 1.0 / (fmax_val - fmin_val);
	float foffset = -fmin_val * scale;

	/* for luminance */
	glPixelTransferf(GL_RED_SCALE,    scale);
	glPixelTransferf(GL_RED_BIAS,     foffset);
	glPixelTransferf(GL_GREEN_SCALE,  scale);
	glPixelTransferf(GL_GREEN_BIAS,   foffset);
	glPixelTransferf(GL_BLUE_SCALE,   scale);
	glPixelTransferf(GL_BLUE_BIAS,    foffset);

	scale *= float(mapsize) / abs_max_val;
	for (i = -16; i < 16; i++)
		if (int(scale / pow(2.0, i)) == 1)
			break;
	int offset = -int(min_val * pow(2.0, i));

	/* for color index */
	glPixelTransferi(GL_INDEX_SHIFT,  i);
	glPixelTransferi(GL_INDEX_OFFSET, offset);

}

void ImageWidget::setTestImage(bool active)
{
	bool was_active = test_active;
	test_active = active;
	if (realimage.isValid() && (active != was_active)) {
		reallocTestImage();
		updateImage(true);
	}
}

void ImageWidget::getNorm(unsigned long *minval, unsigned long *maxval, 
			  int *autorange)
{
	if (minval)
		*minval = min_val;
	if (maxval)
		*maxval = max_val;
	if (autorange)
		*autorange = auto_range;
}

void ImageWidget::setNorm(unsigned long minval, unsigned long maxval, 
			  int autorange)
{
	min_val = minval;
	max_val = maxval;
	auto_range = autorange;
	updateImage(true);
}


/********************************************************************
 * ImageWindow
 ********************************************************************/

float ImageWindow::DefaultMaxRefreshRate = 25;

ImageWindow::ImageWindow(QString caption)
	: QMainWindow()
{
	relaxed = false;
	calc_rate = 1.0;
	update_rate.setUpdateRate(&calc_rate);
	refresh_rate.setUpdateRate(&calc_rate);
	max_refresh_rate = NULL;
	if (DefaultMaxRefreshRate > 0)
		max_refresh_rate = new Rate(DefaultMaxRefreshRate);

	buffer_event = NULL;

	close_cb = NULL;
	close_cb_data = NULL;

	if (caption.isEmpty())
		caption = "Buffer";
	setWindowTitle(caption);

	setAttribute(Qt::WA_DeleteOnClose);

	image = new ImageWidget();
	setCentralWidget(image);

	startTimer(0);
	show();
}

ImageWindow::~ImageWindow()
{
	if (buffer_event != NULL)
		delete buffer_event;
	if (max_refresh_rate != NULL)
		delete max_refresh_rate;
}

int ImageWindow::setBuffer(void *buffer, int width, int height, int depth)
{ 
	SetBufferEvent *event;
	event = new SetBufferEvent(buffer, width, height, depth);

	Lock lock(buffer_mutex);

	if (buffer_event != NULL)
		delete buffer_event;
	buffer_event = event;

	update(false);

	return 0;
}

SetBufferEvent *ImageWindow::checkBufferEvent()
{
	Lock lock(buffer_mutex);

	SetBufferEvent *event = buffer_event;
	buffer_event = NULL;
	
	return event;
}

void ImageWindow::update(bool just_update)
{
	update_rate.update();
	if (relaxed && !just_update) {
		UpdateEvent *event = new UpdateEvent();
		QApplication::postEvent(this, event);
		relaxed = false;
	}
}

void ImageWindow::closeEvent(QCloseEvent *event)
{
	emit closed();
	if (close_cb)
		close_cb(close_cb_data);

	event->accept();
}	

void ImageWindow::setCloseCB(closeCB cb, void *cb_data)
{
	close_cb = cb;
	close_cb_data = cb_data;
}

void ImageWindow::setColormap(ImageWidget::ColormapType colormap)
{
	image->setColormap(colormap);
	image->update();
}
	
void ImageWindow::getRates(float *update, float *refresh)
{
	if (update)
		*update = update_rate.get();
	if (refresh)
		*refresh = refresh_rate.get();
}

void ImageWindow::getNorm(unsigned long *minval, unsigned long *maxval, 
			  int *autorange)
{
	image->getNorm(minval, maxval, autorange);
}

void ImageWindow::setNorm(unsigned long minval, unsigned long maxval, 
			  int autorange)
{
	image->setNorm(minval, maxval, autorange);
}

int ImageWindow::startTimer(int msec)
{
	timer_id = QMainWindow::startTimer(msec);
	return timer_id;
}

void ImageWindow::timerEvent(QTimerEvent *event)
{
	killTimer(timer_id);
	if (!max_refresh_rate || max_refresh_rate->isTime()) {
		relaxed = true;
		return;
	}

	float msec = min(max_refresh_rate->remainingTime() * 1e3, 1.0);
	startTimer(int(msec));
}

void ImageWindow::customEvent(QEvent *event)
{ 
	SetBufferEvent *bevent = checkBufferEvent();
	if (bevent != NULL) {
		SetBufferEvent::BufferData* buff_data = bevent->bufferData();
		realSetBuffer(buff_data->buffer, buff_data->width, 
			      buff_data->height, buff_data->depth);
		delete bevent;
	}

	switch (event->type()) {
	case ImageEvent::Update:
		realUpdate();
		startTimer(0);
		break;
	default:
		break;
	}
}

void ImageWindow::realUpdate()
{ 
	image->updateImage();
	refresh_rate.update();
}

int ImageWindow::realSetBuffer(void *buffer, int width, int height, 
			       int depth)
{
	return image->setBuffer(buffer, width, height, depth, false); 
}


/********************************************************************
 * ImageApplication
 ********************************************************************/

ImageApplication *ImageApplication::createApplication(int& argc, char **argv)
{
	int visualclass = TrueColor;
	char *disp_name = NULL;

	for (int i = 0; i < argc; i++) {
		if ((QString(argv[i]) == "-display") && (i < argc - 1))
			disp_name = argv[i + 1];
		if ((QString(argv[i]) == "-visual") && (i < argc - 1) &&
		    QString(argv[i + 1]) == "DirectColor")
			visualclass = DirectColor;
	}

	disp_name = XDisplayName(disp_name);
	int disp_nr;
	if ((sscanf(disp_name, "localhost:%d", &disp_nr) == 1) && 
	    (disp_nr >= 10)) {
		// Display 10+ on 127.0.0.1 is probably virtual:
		// force indirect rendering to avoid problem on Mesa-GLX client
		// library not finding a direct rendering GLX visual
		setenv("LIBGL_ALWAYS_INDIRECT", "yes", 0);
	}

	Display *display = XOpenDisplay(disp_name);
	if (display == NULL)
		return NULL;

	int screen = DefaultScreen(display);

	XVisualInfo vi;
	Visual *visual = NULL;
	if (XMatchVisualInfo(display, screen, 24, visualclass, &vi) != 0) {
		visual = vi.visual;
		char *visualname;
		switch (vi.c_class) {
		case DirectColor: visualname = "DirectColor"; break;
		case TrueColor:   visualname = "TrueColor";   break;
		case PseudoColor: visualname = "PseudoColor"; break;
		case GrayScale:   visualname = "GrayScale";   break;
		case StaticColor: visualname = "StaticColor"; break;
		default: visualname = "Unknown";
		};

		if (Image::debug) {
			cout << "Default visual 0x" << hex << vi.visualid
			     << " class " << dec << visualname << endl;
		}
	}

	ImageApplication *app;
	app = new ImageApplication(display, argc, argv, (Qt::HANDLE) visual);

	return app; 
}

void ImageApplication::destroyApplication(ImageApplication *app)
{
	Display *xdisplay = app->display;
	delete app;
	if (xdisplay)
		XCloseDisplay(xdisplay);
}

ImageApplication::ImageApplication(Display *xdisplay, int& argc, char **argv,
				   Qt::HANDLE visual, Qt::HANDLE xcolormap)
	: QApplication(xdisplay, argc, argv, visual, xcolormap)
{
	display = xdisplay;
	colormap = ImageWidget::Grayscale;
	QString env = getenv("IMAGE_COLORMAP");
	colormap = ImageWidget::colormapType(env);
	
	for (int i = 0; i < argc; i++) {
		if ((QString(argv[i]) == "-colormap") && (i < argc - 1))
			colormap = ImageWidget::colormapType(argv[i + 1]);
	}

	if (colormap == ImageWidget::Unknown)
		colormap = ImageWidget::Grayscale;
}

ImageApplication::~ImageApplication()
{
}

int ImageApplication::poll()
{
	processEvents();
	return 0;
}

ImageWindow *ImageApplication::createImage(QString caption)
{
	ImageWindow *win;

	win = new ImageWindow(caption);
	win->setColormap(colormap);

	while (!win->isRelaxed())
		processEvents();

	return win;
}

int ImageApplication::setImageBuffer(ImageWindow *win, void *buffer,
				     int width, int height, int depth)
{
	win->setBuffer(buffer, width, height, depth);
	return 0;
}

void ImageApplication::updateImage(ImageWindow *win, bool just_update)
{
	win->update(just_update);
}

void ImageApplication::destroyImage(ImageWindow *win)
{
	win->close();
	delete win;
}

int ImageApplication::setImageCloseCB(ImageWindow *win,
				      ImageWindow::closeCB cb, void *data)
{
	win->setCloseCB(cb, data);
	return 0;
}

void ImageApplication::getImageRates(ImageWindow *win, 
				     float *update, float *refresh)
{
	win->getRates(update, refresh);
}

void ImageApplication::getImageNorm(ImageWindow *win, unsigned long *minval, 
				    unsigned long *maxval, int *autorange)
{
	win->getNorm(minval, maxval, autorange);
}

void ImageApplication::setImageNorm(ImageWindow *win, unsigned long minval, 
				    unsigned long maxval, int autorange)
{
	win->setNorm(minval, maxval, autorange);
}

void ImageApplication::setTestImage(ImageWindow *win, bool active)
{
	win->imageWidget()->setTestImage(active);
}


/********************************************************************
 * Argv
 ********************************************************************/

Argv::Argv(int argc, char **argv)
{
	// Qt assumes arguments are persistent: make a static copy
	str_list = new StringPtr[argc];
	argv_ptr = new char *[argc + 1];

	for (int i = 0; i < argc; i++, argv++) {
		int len = strlen(*argv) + 1;
		StringPtr p = new char[len];
		strcpy(p, *argv);
		argv_ptr[i] = p;
		str_list[i] = p;
	}
	argv_ptr[argc] = NULL;
	argv_count = argc;

	if (Image::debug) {
		cout << "ImageApplication: copy cmdline args:";
		for (argv = argv_ptr; *argv; argv++)
			cout << " " << *argv;
		cout << endl;
	}
}


/********************************************************************
 * Image Lib
 ********************************************************************/

ImageLib::ImageLib(int argc, char **argv)
{
	Lock lock = getLock();

	char *str = getenv("IMAGE_DEBUG");
	if (str)
		Image::debug = atoi(str);

	app = NULL;
	main_thread = pthread_self();
	img_op_requested = OpNone;
	img_op_win = NULL;

	argv_copy = new Argv(argc, argv);
	argv = argv_copy->getArgv();

	bool create_app = true;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--init_on_first_image") == 0)
			create_app = false;
	}
	if (create_app)
		createApplication();
}

ImageLib::~ImageLib()
{
	Lock lock = getLock();
	destroyApplication();
}

void ImageLib::createApplication()
{
	int& argc = argv_copy->getArgc();
	char **argv = argv_copy->getArgv();
	app = ImageApplication::createApplication(argc, argv);
}

void ImageLib::destroyApplication()
{
	if (appCreated())
		ImageApplication::destroyApplication(app);
}

int ImageLib::poll()
{
	if (!inMainThread())
		return -1;

	Lock lock = getLock();

	switch (img_op_requested) {
	case OpNone:
		break;
	case OpInit:
		createApplication();
		goto OpDone;
	case OpCleanup:
		break;
	case OpImgCreate:
		img_op_win = app->createImage(img_op_caption);
		goto OpDone;
	case OpImgNorm:
		app->setImageNorm(img_op_win, img_op_norm[0], 
				  img_op_norm[1], img_op_norm[2]);
		goto OpDone;
	case OpImgDestroy:
		app->destroyImage(img_op_win);
	OpDone:
		img_op_requested = OpNone;
		img_op_done.wakeOne();
	}

	if (!appCreated())
		return 0;

	return app->poll();
}

void ImageLib::checkApplication()
{
	{
		Lock lock = getLock();
		if (appCreated())
			return;
	}

	if (inMainThread()) {
		Lock lock = getLock();
		createApplication();
		return;
	} 

	Lock img_lock = imageLock();
	{
		Lock lock = getLock();
		img_op_requested = OpInit;
		img_op_done.wait(&main_mutex);
	}
}

ImageWindow *ImageLib::createImage(QString caption)
{
	checkApplication();

	if (inMainThread()) {
		Lock lock = getLock();
		return app->createImage(caption);
	} 

	ImageWindow *win;
	Lock img_lock = imageLock();
	{
		Lock lock = getLock();
		img_op_requested = OpImgCreate;
		img_op_caption = caption;
		img_op_done.wait(&main_mutex);
		win = img_op_win;
	}

	return win;
}

void ImageLib::destroyImage(ImageWindow *win)
{
	if (inMainThread()) {
		Lock lock = getLock();
		app->destroyImage(win);
		return;
	}

	Lock img_lock = imageLock();
	{
		Lock lock = getLock();
		img_op_requested = OpImgDestroy;
		img_op_win = win;
		img_op_done.wait(&main_mutex);
	}
}

int ImageLib::setImageBuffer(ImageWindow *win, void *buffer,
			     int width, int height, int depth)
{
	app->setImageBuffer(win, buffer, width, height, depth);
	return 0;
}

void ImageLib::setTestImage(ImageWindow *win, int active)
{
	Lock lock = getLock();
	app->setTestImage(win, active);
}

int ImageLib::setImageCloseCB(ImageWindow *win,
			      ImageWindow::closeCB cb, void *data)
{
	Lock lock = getLock();
	return app->setImageCloseCB(win, cb, data);
}

void ImageLib::updateImage(ImageWindow *win)
{
	Lock lock = tryLock();
	return app->updateImage(win, !lock.locked());
}

void ImageLib::getImageRates(ImageWindow *win, float *update, float *refresh)
{
	Lock lock = getLock();
	app->getImageRates(win, update, refresh);
}

void ImageLib::getImageNorm(ImageWindow *win, unsigned long *minval, 
			    unsigned long *maxval, int *autorange)
{
	Lock lock = getLock();
	app->getImageNorm(win, minval, maxval, autorange);
}

void ImageLib::setImageNorm(ImageWindow *win, unsigned long minval, 
			    unsigned long maxval, int autorange)
{
	if (inMainThread()) {
		Lock lock = getLock();
		app->setImageNorm(win, minval, maxval, autorange);
		return;
	} 

	Lock img_lock = imageLock();
	{
		Lock lock = getLock();
		img_op_requested = OpImgNorm;
		img_op_win = win;
		img_op_norm[0] = minval;
		img_op_norm[1] = maxval;
		img_op_norm[2] = autorange;
		img_op_done.wait(&main_mutex);
	}
}


/********************************************************************
 * Image API
 ********************************************************************/

static ImageLib *img_lib = NULL;

static inline 
ImageWindow *imageWindow(image_t img)
{
	return (ImageWindow *) img;
}

static inline
image_t imageT(ImageWindow *w)
{
	return (image_t) w;
}

int image_init(int argc, char **argv)
{
	try {
		img_lib = new ImageLib(argc, argv);
	} catch (...) {
		return -1;
	}

	image_poll();
	return 0;
}

void image_exit()
{
	delete img_lib;
	img_lib = NULL;
}

int image_create(image_t *img_ptr, char *caption)
{
	ImageWindow *w = img_lib->createImage(caption);
	*img_ptr = imageT(w);
	return 0;
}

int image_set_buffer(image_t img, void *buffer, int width, int height, 
		     int depth)
{
	return img_lib->setImageBuffer(imageWindow(img), buffer, 
				       width, height, depth);
}

int image_set_test(image_t img, int active)
{
	img_lib->setTestImage(imageWindow(img), active);
	return 0;
}

int image_close_cb(image_t img, void (*close_cb)(void *data), void *cb_data)
{
	return img_lib->setImageCloseCB(imageWindow(img), close_cb, cb_data);
}

void image_destroy(image_t img)
{
	img_lib->destroyImage(imageWindow(img));
}

void image_update(image_t img)
{
	img_lib->updateImage(imageWindow(img));
}

void image_get_rates(image_t img, float *update, float *refresh)
{
	img_lib->getImageRates(imageWindow(img), update, refresh);
}

void image_get_norm(image_t img, unsigned long *min_val, 
		    unsigned long *max_val, int *auto_range)
{
	img_lib->getImageNorm(imageWindow(img), min_val, max_val, auto_range);
}

void image_set_norm(image_t img, unsigned long min_val, 
		    unsigned long max_val, int auto_range)
{
	img_lib->setImageNorm(imageWindow(img), min_val, max_val, auto_range);
}

int image_poll()
{
	return img_lib->poll();
}


#ifndef __IMAGE_H
#define __IMAGE_H

#include <qobject.h>
#include <qmainwindow.h>
#include <qapplication.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <qgl.h>
#include <X11/Xutil.h>
#include <pthread.h>

#include "prectime.h"
#include "autoobj.h"


typedef AutoLock<QMutex> Lock;


/********************************************************************
 * ImageContext
 ********************************************************************/

class ImageContext : public QGLContext
{
public:
	ImageContext(const QGLFormat& format, QPaintDevice *device);

protected:
	void *chooseVisual();
};


/********************************************************************
 * ImageDataPtr
 ********************************************************************/

class ImagePixelPtr
{
public:
	union Ptr {
		unsigned char  *c;
		unsigned short *s;
		unsigned int   *i;
		unsigned long  *l;
		void           *v;
	};

	ImagePixelPtr() { init(NULL, 0); }
	ImagePixelPtr(void *p, unsigned dpth) { init(p, dpth); }
	template <class T>
	ImagePixelPtr(T *p) { init(p, sizeof(T)); }

	unsigned operator *()
	{
		switch (d) {
		case 1: return *ptr.c;
		case 2: return *ptr.s;
		case 4: return *ptr.i;
		}
		return 0;
	}

	unsigned store(unsigned val)
	{
		switch (d) {
		case 1: return *ptr.c = val;
		case 2: return *ptr.s = val;
		case 4: return *ptr.i = val;
		}
		return 0;
	}

	ImagePixelPtr& operator++()
	{ ptr.c += d; return *this; }

	unsigned char  *cPtr()
	{ return ptr.c; }
	unsigned short *sPtr()
	{ return ptr.s; }
	unsigned int   *iPtr()
	{ return ptr.i; }
	void           *vPtr()
	{ return ptr.v; }

	void init(void *p, unsigned dpth)
	{ ptr.v = p; d = dpth; }

	unsigned long depth()
	{ return d; }

private:
	Ptr ptr;
	unsigned long d;
};


/********************************************************************
 * Image
 ********************************************************************/

class Image : public QObject
{
public:
	Image();
	Image(void *ptr, unsigned width, unsigned height, unsigned depth);
	
	bool isValid()
	{ return buff.vPtr() && buff.depth() && w && h; }

	void setBuffer(void *ptr, unsigned width, unsigned height, 
		       unsigned depth);
	void setTestImage();

	unsigned maxVal()
	{ return (1ULL << (8 * buff.depth())) - 1; }

	ImagePixelPtr ptr()
	{ return buff; }
	unsigned width()
	{ return w; }
	unsigned height()
	{ return h; }
	unsigned nrPixels()
	{ return w * h; }

	static int debug;

private:
	ImagePixelPtr buff;
	unsigned w, h;
};


/********************************************************************
 * ImageWidget
 ********************************************************************/

class ImageWidget : public QGLWidget
{
	Q_OBJECT

public:
	enum ColormapType {
		Grayscale,
		Temperature,
		Unknown,
	};

	ImageWidget(QWidget *parent, const char *name = NULL, 
	      ColormapType cmap = Grayscale);
	~ImageWidget();

	int setBuffer(void *buffer, int width, int height, int depth,
		      bool do_update = true);

	static QString colormapName(ColormapType cmap);
	static ColormapType colormapType(QString name);

public slots:
	void setTestImage();
	void setColormap(ColormapType cmap);
	void updateImage(bool force_norm = false);
	void normalize(bool force = false); 
	void getNorm(unsigned long *minval, unsigned long *maxval, 
		     int *autorange);
	void setNorm(unsigned long minval, unsigned long maxval, 
		     int autorange);

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

	void calcResize();

	int  checkColorTableColormap(float map[][4], int size);
	void setColorTableColormap(float map[][4], int size);
	void setPixelMapColormap(float map[][4], int size);
	int  checkX11Colormap(float map[][4], int size);
	void setX11Colormap(float map[][4], int size);

private:
	Image image;
	GLboolean testimage;
	GLenum b_type;
	GLsizei w_width, w_height;
	GLint x, y;
	GLfloat factor;
	GLuint min_val, max_val;
	GLint auto_range;
	Rate normalize_rate;
	ColormapType colormap;
	GLint mapsize;
	int x11nrcolors;
	Colormap x11colormap;
	unsigned long *x11cmap;
	GLint draw_mode;
	GLboolean must_normalize;
	GLboolean must_resize;
};


/********************************************************************
 * ImageEvent / SetBufferEvent / UpdateEvent
 ********************************************************************/

class ImageEvent : public QCustomEvent
{
public:
	enum { SetBuffer = User + 1, Update };

	ImageEvent(int type) : 
		QCustomEvent(type) {}
};

class SetBufferEvent : public ImageEvent
{
public:
	struct BufferData {
		void *buffer;
		int width, height, depth;
		BufferData(void *b, int w, int h, int d) :
			buffer(b), width(w), height(h), depth(d) {}
	};

	SetBufferEvent(void *b, int w, int h, int d) :
		ImageEvent(SetBuffer), buff_data(b, w, h, d) 
	{	setData(&buff_data);	}

private:
	BufferData buff_data;
};

class UpdateEvent : public ImageEvent
{
 public:
	UpdateEvent() : ImageEvent(Update) {}
};


/********************************************************************
 * ImageWindow
 ********************************************************************/

class ImageWindow : public QMainWindow
{
	Q_OBJECT
public:
	typedef void (*closeCB)(void *data);

	static float DefaultMaxRefreshRate;

public:
	ImageWindow(const char *caption = NULL);
	~ImageWindow();

	int setBuffer(void *buffer, int width, int height, int depth);
	void setColormap(ImageWidget::ColormapType colormap);
	
	void closeEvent(QCloseEvent *event);
	void setCloseCB(closeCB cb, void *cb_data);

	void getRates(float *update, float *refresh);
	void getNorm(unsigned long *minval, unsigned long *maxval, 
		     int *autorange);
	void setNorm(unsigned long minval, unsigned long maxval, 
		     int autorange);

	ImageWidget *imageWidget()
	{ return image; }

	bool isRelaxed()
	{ return relaxed; }

public slots:
	void update(bool just_update);

signals:
	void closed();

protected:
	void timerEvent(QTimerEvent *event);
	void customEvent(QCustomEvent *event);

	void realUpdate();
	int realSetBuffer(void *buffer, int width, int height, 
			  int depth);

	SetBufferEvent *checkBufferEvent();

private:
	ImageWidget *image;

	volatile bool relaxed;
	Rate update_rate, refresh_rate, calc_rate, *max_refresh_rate;

	QMutex buffer_mutex;
	SetBufferEvent *buffer_event;
		
	closeCB close_cb;
	void *close_cb_data;
};


/********************************************************************
 * ImageApplication
 ********************************************************************/

class ImageApplication : public QApplication
{
	Q_OBJECT

public:
	ImageApplication(Display *display, int argc, char **argv, 
			 HANDLE visual = 0, HANDLE xcolormap = 0);
	~ImageApplication();

	static ImageApplication *createApplication(int argc = 0, 
						   char **argv = NULL);
	static void destroyApplication(ImageApplication *app);

	ImageWindow *createImage(QString caption);
	int setImageBuffer(ImageWindow *win, void *buffer, 
			   int width, int height, int depth);
	void destroyImage(ImageWindow *win);
	int setImageCloseCB(ImageWindow *win,
			    ImageWindow::closeCB cb, void *data);

	void updateImage(ImageWindow *win, bool just_update);
	void setTestImage(ImageWindow *win);
	int poll();

	void getImageRates(ImageWindow *win, float *update, float *refresh);
	void getImageNorm(ImageWindow *win, unsigned long *minval, 
			  unsigned long *maxval, int *autorange);
	void setImageNorm(ImageWindow *win, unsigned long minval, 
			  unsigned long maxval, int autorange);

private:
	ImageWidget::ColormapType colormap;
};


/********************************************************************
 * Argv
 ********************************************************************/

class Argv
{
public:
	typedef AutoPtr<char, true> StringPtr;
	typedef AutoPtr<StringPtr, true> StringList;
	typedef AutoPtr<char *, true> CStringList;

	Argv(int argc, char **argv);

	char **getArgv() const
	{ return argv_ptr; }

	int getArgc() const
	{ return argv_count; }

private:
	int argv_count;
	CStringList argv_ptr;
	StringList str_list;
};


/********************************************************************
 * ImageLib
 ********************************************************************/

class ImageLib
{
public:
	ImageLib(int argc, char **argv);
	~ImageLib();

	void createApplication();
	void destroyApplication();

	ImageWindow *createImage(QString caption);
	void destroyImage(ImageWindow *win);

	int poll();

	int setImageBuffer(ImageWindow *win, void *buffer, 
			   int width, int height, int depth);
	void setTestImage(ImageWindow *win);
	int setImageCloseCB(ImageWindow *win,
			    ImageWindow::closeCB cb, void *data);
	void updateImage(ImageWindow *win);

	void getImageRates(ImageWindow *win, float *update, float *refresh);
	void getImageNorm(ImageWindow *win, unsigned long *minval, 
			  unsigned long *maxval, int *autorange);
	void setImageNorm(ImageWindow *win, unsigned long minval, 
			  unsigned long maxval, int autorange);

protected:
	enum ImageOp { 
		OpNone, OpInit, OpCleanup, 
		OpImgCreate, OpImgDestroy, OpImgNorm,
	};

	Lock getLock()
	{ return Lock(main_mutex); }

	Lock tryLock()
	{ return Lock(main_mutex, Lock::TryLocked); }

	Lock imageLock()
	{ return Lock(img_op_mutex); }

	bool inMainThread() 
	{ return pthread_equal(pthread_self(), main_thread); }

	bool appCreated()
	{ return (app != NULL); }

	void checkApplication();

private:
	Argv *argv_copy;
	QMutex main_mutex;
	pthread_t main_thread;
	QMutex img_op_mutex;
	ImageOp img_op_requested;
	QString img_op_caption;
	ImageWindow *img_op_win;
	unsigned long img_op_norm[3];
	QWaitCondition img_op_done;
	ImageApplication *app;
};


#endif /* __IMAGE_H */

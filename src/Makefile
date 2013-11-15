INCL		= ../include
TEST		= ../test

COMMON		= ../../../common
CONTROL		= ../../../control
HARDWARE	= ../../../hardware
PROCLIB		= ../../Processlib
PROCLIBCORE	= $(PROCLIB)/core

ifdef QTDIR3
QTDIR		= $(QTDIR3)
endif

ifndef QTDIR
$(error QTDIR environment variable must be defined)
endif

MOC             = $(QTDIR)/bin/moc

CFLAGS		= -Wall -g -I$(INCL) -fPIC -pthread

IMGFLAGS	= $(CFLAGS) -I$(QTDIR)/include -DQT_THREAD_SUPPORT -O3 
IMGLDFLAGS	= -L$(QTDIR)/lib -lqt-mt -lGL

LIMAFLAGS	= -I$(COMMON)/include -I$(CONTROL)/include \
		  -I$(HARDWARE)/include -I$(PROCLIBCORE)/include

# targets
all:			 GLDisplay.o $(TEST)/image_test

GLDisplay.o:	GLDisplay.cpp $(INCL)/GLDisplay.h
	$(CXX) -c $(IMGFLAGS) $(LIMAFLAGS) -o $@ $<
 
# The image library
$(TEST)/image_test:		$(TEST)/image_test.o image.o 
	$(CXX) -o $@ $+ $(IMGLDFLAGS)

$(TEST)/image_test.o:		$(TEST)/image_test.c $(INCL)/imageapi.h
	$(CC) -c $(IMGFLAGS) -o $@ $<

image.o:		image_aux.o moc_image.o
	$(LD) -r -o $@ $+

image_aux.o:		image.cpp $(INCL)/image.h $(INCL)/imageapi.h \
			$(INCL)/autoobj.h $(INCL)/prectime.h
	$(CXX) -c $(IMGFLAGS) -o $@ $<

moc_image.o:		moc_image.cpp
	$(CXX) -c $(IMGFLAGS) -o $@ $<

moc_image.cpp:		$(INCL)/image.h
	$(MOC) $< -o $@


# cleanup
clean:
	rm -f *.o moc_* $(TEST)/*.o $(TEST)/image_test

#include "GLDisplay.h"
#include "Exceptions.h"
#include "SoftOpId.h"

using namespace Tasks;


void GLDisplayTask::process(Data& data)
{
	image_t& image = m_display->m_image;
	image_set_buffer(image, data.data(), 
			 data.dimensions[0], data.dimensions[1], data.depth());
	image_update(image);
}



bool GLDisplay::image_inited = false;

GLDisplay::GLDisplay(CtControl *ct)
	: m_ct(ct)
{
	DEB_CONSTRUCTOR();

	if (!image_inited) {
		char *argv[] = {NULL};
		int argc = sizeof(argv) / sizeof(argv[0]);
		int ret = image_init(argc, argv);
		if (ret < 0)
			THROW_CTL_ERROR(Error) << "Cannot init image API";
		image_inited = true;
	}

	int ret = image_create(&m_image, "Lima Live");
	if (ret < 0)
		THROW_CTL_ERROR(Error) << "Cannot create an image";
}

GLDisplay::~GLDisplay()
{
	DEB_DESTRUCTOR();

	image_destroy(m_image);
}

void GLDisplay::setActive(bool active, int run_level)
{
	SoftOpExternalMgr* ext_op = m_ct->externalOperation();
	ext_op->addOp(USER_SINK_TASK, "LiveDisplay", run_level, m_softopinst);
	m_softopinst.m_opt->set
		      
}

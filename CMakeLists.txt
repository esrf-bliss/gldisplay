############################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2017
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# Contact: lima@esrf.fr
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
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################

find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)

qt4_generate_moc("${CMAKE_CURRENT_SOURCE_DIR}/include/image.h" "${CMAKE_BINARY_DIR}/third-party/gldisplay/src/moc_image.cpp")

add_definitions(-D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE -DQT_NO_DEBUG 
				-DQT_OPENGL_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
				-pipe -g -O2 -Wall -W -D_REENTRANT -O1)
set(gldisplay_src src/CtGLDisplay.cpp src/GLDisplay.cpp
	src/image.cpp
	${CMAKE_BINARY_DIR}/third-party/gldisplay/src/moc_image.cpp)

file(STRINGS "VERSION" gldisplay_vers)
add_library(gldisplay SHARED ${gldisplay_src})

limatools_set_library_soversion(gldisplay "VERSION")

target_include_directories(gldisplay PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(gldisplay PUBLIC "${CMAKE_SOURCE_DIR}/third-party/Sps/Include")


target_link_libraries(gldisplay  Qt4::QtGui Qt4::QtOpenGL)
find_library(GL_LIB GL)
find_library(X11_LIB X11)

target_link_libraries(gldisplay limacore)
target_link_libraries(gldisplay ${GL_LIB})
target_link_libraries(gldisplay ${X11_LIB})

include(GNUInstallDirs)
install(TARGETS gldisplay LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(LIMA_ENABLE_PYTHON)
	set(NAME "gldisplay")
	set(IMPORTS 
"${IMPORTS}
%Import limacore.sip")

	if(SIP_VERSION_STR VERSION_LESS "4.12")
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sip/gldisplay_before_4_12.sip sip/${NAME}.sip)
	else()
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sip/gldisplay.sip sip/${NAME}.sip)
	endif()
	set(SIP_CONCAT_PARTS 1)
	set(SIP_INCLUDES ${SIP_INCLUDES}
		"${CMAKE_SOURCE_DIR}/third-party/Processlib/sip"
		"${CMAKE_BINARY_DIR}/sip/core"
		"${CMAKE_SOURCE_DIR}/third-party/Processlib/tasks/sip"
		"${CMAKE_SOURCE_DIR}/common/sip"
		"${CMAKE_SOURCE_DIR}/hardware/sip"
		"${CMAKE_SOURCE_DIR}/control/sip"
		"${CMAKE_SOURCE_DIR}/control/software_operation/sip"
		"${CMAKE_CURRENT_SOURCE_DIR}/sip")
	add_sip_python_module(${NAME} ${CMAKE_CURRENT_BINARY_DIR}/sip/${NAME}.sip)
	target_include_directories(python_module_${NAME} PRIVATE
							  ${PYTHON_INCLUDE_DIRS}
							  "${CMAKE_SOURCE_DIR}/sip"
							  "${CMAKE_SOURCE_DIR}/sip/core")
	target_link_libraries(python_module_${NAME} ${NAME})
	install(FILES python/__init__.py DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/Lima/GLDisplay")

endif()

if(LIMA_ENABLE_TESTS)
	add_subdirectory(test)
endif()

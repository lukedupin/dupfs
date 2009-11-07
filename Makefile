#############################################################################
# Makefile for building: bin/dupfs
# Generated by qmake (2.01a) (Qt 4.5.0) on: Fri Nov 6 20:39:23 2009
# Project:  dupfs.pro
# Template: app
# Command: /usr/bin/qmake -unix -o Makefile dupfs.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -D_FILE_OFFSET_BITS=64 -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -pipe -O2 -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I/usr/share/qt4/mkspecs/linux-g++ -I. -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtNetwork -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I. -Imoc -I.
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = $(SUBLIBS)  -L/usr/lib -I/usr/include/fuse -pthread -lfuse -lrt -ldl -lulockmgr -lpthread -lQtGui -lQtNetwork -lQtCore
AR            = ar cqs
RANLIB        = 
QMAKE         = /usr/bin/qmake
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -sf
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = objects/

####### Files

SOURCES       = main.cpp \
		window.cpp \
		orm_light/orm_light.cpp \
		fuse_tracker.cpp \
		fuse_cpp_interface.cpp \
		boot_system.cpp moc/moc_window.cpp \
		moc/moc_fuse_tracker.cpp \
		moc/moc_boot_system.cpp \
		qrc_systray.cpp
OBJECTS       = objects/main.o \
		objects/window.o \
		objects/orm_light.o \
		objects/fuse_tracker.o \
		objects/fuse_cpp_interface.o \
		objects/boot_system.o \
		objects/moc_window.o \
		objects/moc_fuse_tracker.o \
		objects/moc_boot_system.o \
		objects/qrc_systray.o
DIST          = /usr/share/qt4/mkspecs/common/g++.conf \
		/usr/share/qt4/mkspecs/common/unix.conf \
		/usr/share/qt4/mkspecs/common/linux.conf \
		/usr/share/qt4/mkspecs/qconfig.pri \
		/usr/share/qt4/mkspecs/features/qt_functions.prf \
		/usr/share/qt4/mkspecs/features/qt_config.prf \
		/usr/share/qt4/mkspecs/features/exclusive_builds.prf \
		/usr/share/qt4/mkspecs/features/default_pre.prf \
		/usr/share/qt4/mkspecs/features/release.prf \
		/usr/share/qt4/mkspecs/features/default_post.prf \
		/usr/share/qt4/mkspecs/features/warn_on.prf \
		/usr/share/qt4/mkspecs/features/unix/thread.prf \
		/usr/share/qt4/mkspecs/features/qt.prf \
		/usr/share/qt4/mkspecs/features/moc.prf \
		/usr/share/qt4/mkspecs/features/resources.prf \
		/usr/share/qt4/mkspecs/features/uic.prf \
		/usr/share/qt4/mkspecs/features/yacc.prf \
		/usr/share/qt4/mkspecs/features/lex.prf \
		dupfs.pro
QMAKE_TARGET  = dupfs
DESTDIR       = bin/
TARGET        = bin/dupfs

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	@$(CHK_DIR_EXISTS) bin/ || $(MKDIR) bin/ 
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

Makefile: dupfs.pro  /usr/share/qt4/mkspecs/linux-g++/qmake.conf /usr/share/qt4/mkspecs/common/g++.conf \
		/usr/share/qt4/mkspecs/common/unix.conf \
		/usr/share/qt4/mkspecs/common/linux.conf \
		/usr/share/qt4/mkspecs/qconfig.pri \
		/usr/share/qt4/mkspecs/features/qt_functions.prf \
		/usr/share/qt4/mkspecs/features/qt_config.prf \
		/usr/share/qt4/mkspecs/features/exclusive_builds.prf \
		/usr/share/qt4/mkspecs/features/default_pre.prf \
		/usr/share/qt4/mkspecs/features/release.prf \
		/usr/share/qt4/mkspecs/features/default_post.prf \
		/usr/share/qt4/mkspecs/features/warn_on.prf \
		/usr/share/qt4/mkspecs/features/unix/thread.prf \
		/usr/share/qt4/mkspecs/features/qt.prf \
		/usr/share/qt4/mkspecs/features/moc.prf \
		/usr/share/qt4/mkspecs/features/resources.prf \
		/usr/share/qt4/mkspecs/features/uic.prf \
		/usr/share/qt4/mkspecs/features/yacc.prf \
		/usr/share/qt4/mkspecs/features/lex.prf \
		/usr/lib/libQtGui.prl \
		/usr/lib/libQtNetwork.prl \
		/usr/lib/libQtCore.prl
	$(QMAKE) -unix -o Makefile dupfs.pro
/usr/share/qt4/mkspecs/common/g++.conf:
/usr/share/qt4/mkspecs/common/unix.conf:
/usr/share/qt4/mkspecs/common/linux.conf:
/usr/share/qt4/mkspecs/qconfig.pri:
/usr/share/qt4/mkspecs/features/qt_functions.prf:
/usr/share/qt4/mkspecs/features/qt_config.prf:
/usr/share/qt4/mkspecs/features/exclusive_builds.prf:
/usr/share/qt4/mkspecs/features/default_pre.prf:
/usr/share/qt4/mkspecs/features/release.prf:
/usr/share/qt4/mkspecs/features/default_post.prf:
/usr/share/qt4/mkspecs/features/warn_on.prf:
/usr/share/qt4/mkspecs/features/unix/thread.prf:
/usr/share/qt4/mkspecs/features/qt.prf:
/usr/share/qt4/mkspecs/features/moc.prf:
/usr/share/qt4/mkspecs/features/resources.prf:
/usr/share/qt4/mkspecs/features/uic.prf:
/usr/share/qt4/mkspecs/features/yacc.prf:
/usr/share/qt4/mkspecs/features/lex.prf:
/usr/lib/libQtGui.prl:
/usr/lib/libQtNetwork.prl:
/usr/lib/libQtCore.prl:
qmake:  FORCE
	@$(QMAKE) -unix -o Makefile dupfs.pro

dist: 
	@$(CHK_DIR_EXISTS) objects/dupfs1.0.0 || $(MKDIR) objects/dupfs1.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) objects/dupfs1.0.0/ && $(COPY_FILE) --parents window.h orm_light/orm_light.h fuse_tracker.h d_widget.h fuse_cpp_interface.h boot_system.h objects/dupfs1.0.0/ && $(COPY_FILE) --parents systray.qrc objects/dupfs1.0.0/ && $(COPY_FILE) --parents main.cpp window.cpp orm_light/orm_light.cpp fuse_tracker.cpp fuse_cpp_interface.cpp boot_system.cpp objects/dupfs1.0.0/ && (cd `dirname objects/dupfs1.0.0` && $(TAR) dupfs1.0.0.tar dupfs1.0.0 && $(COMPRESS) dupfs1.0.0.tar) && $(MOVE) `dirname objects/dupfs1.0.0`/dupfs1.0.0.tar.gz . && $(DEL_FILE) -r objects/dupfs1.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compiler_moc_header_make_all: moc/moc_window.cpp moc/moc_fuse_tracker.cpp moc/moc_boot_system.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc/moc_window.cpp moc/moc_fuse_tracker.cpp moc/moc_boot_system.cpp
moc/moc_window.cpp: fuse_tracker.h \
		orm_light/orm_light.h \
		fuse_cpp_interface.h \
		window.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) window.h -o moc/moc_window.cpp

moc/moc_fuse_tracker.cpp: orm_light/orm_light.h \
		fuse_cpp_interface.h \
		fuse_tracker.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) fuse_tracker.h -o moc/moc_fuse_tracker.cpp

moc/moc_boot_system.cpp: window.h \
		fuse_tracker.h \
		orm_light/orm_light.h \
		fuse_cpp_interface.h \
		boot_system.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) boot_system.h -o moc/moc_boot_system.cpp

compiler_rcc_make_all: qrc_systray.cpp
compiler_rcc_clean:
	-$(DEL_FILE) qrc_systray.cpp
qrc_systray.cpp: systray.qrc \
		images/green_clock.png \
		images/black_clock.png \
		images/black_spin_clock_4.png \
		images/black_spin_clock_7.png \
		images/green_spin_clock_3.png \
		images/red_spin_clock_5.png \
		images/black_spin_clock_5.png \
		images/green_spin_clock_6.png \
		images/red_spin_clock_2.png \
		images/blue_spin_clock_5.png \
		images/red_clock.png \
		images/black_spin_clock_2.png \
		images/green_spin_clock_0.png \
		images/blue_spin_clock_7.png \
		images/green_spin_clock_5.png \
		images/red_spin_clock_3.png \
		images/blue_spin_clock_0.png \
		images/black_spin_clock_0.png \
		images/black_spin_clock_3.png \
		images/blue_spin_clock_2.png \
		images/on_the_clock.xcf \
		images/red_spin_clock_0.png \
		images/black_spin_clock_1.png \
		images/blue_clock.png \
		images/green_spin_clock_2.png \
		images/red_spin_clock_6.png \
		images/green_spin_clock_7.png \
		images/red_spin_clock_1.png \
		images/green_spin_clock_1.png \
		images/blue_spin_clock_4.png \
		images/red_spin_clock_7.png \
		images/blue_spin_clock_1.png \
		images/on_the_clock.png \
		images/blue_spin_clock_6.png \
		images/black_spin_clock_6.png \
		images/green_spin_clock_4.png \
		images/blue_spin_clock_3.png \
		images/red_spin_clock_4.png
	/usr/bin/rcc -name systray systray.qrc -o qrc_systray.cpp

compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_uic_make_all:
compiler_uic_clean:
compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: compiler_moc_header_clean compiler_rcc_clean 

####### Compile

objects/main.o: main.cpp boot_system.h \
		window.h \
		fuse_tracker.h \
		orm_light/orm_light.h \
		fuse_cpp_interface.h \
		version.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o objects/main.o main.cpp

objects/window.o: window.cpp window.h \
		fuse_tracker.h \
		orm_light/orm_light.h \
		fuse_cpp_interface.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o objects/window.o window.cpp

objects/orm_light.o: orm_light/orm_light.cpp orm_light/orm_light.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o objects/orm_light.o orm_light/orm_light.cpp

objects/fuse_tracker.o: fuse_tracker.cpp fuse_tracker.h \
		orm_light/orm_light.h \
		fuse_cpp_interface.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o objects/fuse_tracker.o fuse_tracker.cpp

objects/fuse_cpp_interface.o: fuse_cpp_interface.cpp fuse_cpp_interface.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o objects/fuse_cpp_interface.o fuse_cpp_interface.cpp

objects/boot_system.o: boot_system.cpp boot_system.h \
		window.h \
		fuse_tracker.h \
		orm_light/orm_light.h \
		fuse_cpp_interface.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o objects/boot_system.o boot_system.cpp

objects/moc_window.o: moc/moc_window.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o objects/moc_window.o moc/moc_window.cpp

objects/moc_fuse_tracker.o: moc/moc_fuse_tracker.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o objects/moc_fuse_tracker.o moc/moc_fuse_tracker.cpp

objects/moc_boot_system.o: moc/moc_boot_system.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o objects/moc_boot_system.o moc/moc_boot_system.cpp

objects/qrc_systray.o: qrc_systray.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o objects/qrc_systray.o qrc_systray.cpp

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:


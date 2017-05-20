#
#   CS224 - Path
#

# Environment Variables
# G3D_PATH - absolute path to G3D Library

G3D_PATH = $$(G3D_PATH)

isEmpty(G3D_PATH) {
    # default sunlab path
    G3D_PATH = /contrib/projects/g3d10/G3D10
}

# convert relative paths and sanitize
# G3D_PATH = $$absolute_path($${G3D_PATH})
# G3D_PATH = $$system_path($${G3D_PATH})

message("G3D Path : " $${G3D_PATH})

CONFIG -= qt
QT -= core gui opengl

TARGET = path
TEMPLATE = app

SOURCES += \
    app.cpp \
    world.cpp \
    main.cpp \
    threadpool.cpp \
    pathtracer.cpp \
    dofCam.cpp \
    SkyCube.cpp

HEADERS += \
    app.h \
    world.h \
    threadpool.h \
    pathtracer.h \
    medium.h \
    dofCam.h \
    SkyCube.h

DEFINES += G3D_PATH=\\\"$${G3D_PATH}\\\"
INCLUDEPATH += $${G3D_PATH}/build/include
               $${G3D_PATH}/tbb/include

LIBS += \
    -L$${G3D_PATH}/build/lib \
    -lGLG3D \
    -lG3D \
    -lassimp \
    -lglfw \
    -lXrandr \
    -lGLU \
    -lX11 \
    -lfreeimage \
    -lzip \
    -lz \
    -lGL \
    -lpthread \
    -lXi \
    -lXxf86vm \
    -lrt \
    -lenet \
    -ltbb \
    -lglew \
    -lXcursor

macx {
    LIBS = \
        -L$${G3D_PATH}/build/lib \
        -L/usr/X11/lib \
        -framework OpenGL \
        -framework Cocoa \
        -framework IOKit \
        -framework AGL \
        -lpthread \
        -lswscale.2 \
        -lGLG3D \
        -lassimp \
        -lglfw \
        -lavformat.54 \
        -lavcodec.54 \
        -lavutil.51 \
        -lG3D \
        -lfreeimage \
        -lzip \
        -lz \
        -lenet \
        -ltbb \
        -lglew
        # might be needed -lXcursor

    INCLUDEPATH += /usr/X11/include

    # CLANG!
    QMAKE_CXXFLAGS += -stdlib=libstdc++
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
}

#  hide object files and misc
OBJECTS_DIR = $${OUT_PWD}/.obj
MOC_DIR = $${OUT_PWD}/.moc
RCC_DIR = $${OUT_PWD}/.rcc
UI_DIR = $${OUT_PWD}/.ui

QMAKE_CXXFLAGS += -std=c++14 -msse4.1

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -fno-strict-aliasing
QMAKE_CXXFLAGS_WARN_ON -= -Wall
QMAKE_CXXFLAGS_WARN_ON += -Waddress -Warray-bounds -Wc++0x-compat -Wchar-subscripts -Wformat\
                          -Wmain -Wmissing-braces -Wparentheses -Wreorder -Wreturn-type \
                          -Wsequence-point -Wsign-compare -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch \
                          -Wtrigraphs -Wuninitialized -Wunused-label -Wunused-variable \
                          -Wvolatile-register-var -Wno-extra

OTHER_FILES +=

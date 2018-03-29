#-------------------------------------------------
#
# Project created by QtCreator 2018-03-29T19:31:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SportAnalysis
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mythread.cpp \
    mythreadcoordinate.cpp \
    readfilethread.cpp \
    replaymdthread.cpp \
    replaythread.cpp \
    winsockmattransmissionclient.cpp \
    quanju.cpp

HEADERS  += mainwindow.h \
    mythread.h \
    mythreadcoordinate.h \
    readfilethread.h \
    replaymdthread.h \
    replaythread.h \
    winsockmattransmissionclient.h \
    quanju.h

FORMS    += mainwindow.ui
INCLUDEPATH+= F:\soft\opencv/build\include\opencv2\
              F:\soft\opencv\build\include\opencv\
              F:\soft\opencv\build\include

LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_calib3d2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_contrib2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_core2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_features2d2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_flann2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_gpu2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_highgui2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_imgproc2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_legacy2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_ml2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_nonfree2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_objdetect2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_ocl2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_photo2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_stitching2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_superres2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_ts2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_video2413d.lib
LIBS+=F:\soft\opencv\build\x64\vc12\lib\opencv_videostab2413d.lib


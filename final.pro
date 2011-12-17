QT += core \
    gui \
    opengl
TARGET = final
TEMPLATE = app
INCLUDEPATH += lab \
    lib \
    math \
    support
DEPENDPATH += lab \
    lib \
    math \
    support
HEADERS += lab/glwidget.h \
    lib/targa.h \
    lib/glm.h \
    math/vector.h \
    support/resourceloader.h \
    support/mainwindow.h \
    support/camera.h \
    lib/targa.h \
    rgbe/rgbe.h
SOURCES += lab/glwidget.cpp \
    lib/targa.cpp \
    lib/glm.cpp \
    support/resourceloader.cpp \
    support/mainwindow.cpp \
    support/main.cpp \
    support/camera.cpp \
    rgbe/rgbe.cpp
FORMS += mainwindow.ui \
    support/mainwindow.ui
OTHER_FILES += shaders/refract.vert \
    shaders/refract.frag \
    shaders/reflect.vert \
    shaders/reflect.frag \
    shaders/brightpass.frag \
    shaders/blur.frag \
    shaders/tonemap.frag \
    shaders/basic.frag \
    shaders/basic.vert \
    shaders/shadow.frag \
    shaders/shadow.vert \
    shaders/bilat.frag \
    shaders/bilat_high.frag \
    shaders/tester.frag \
    shaders/color.frag \
    shaders/combine.frag
RESOURCES += 

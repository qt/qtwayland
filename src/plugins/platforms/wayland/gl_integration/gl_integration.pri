isEmpty(QT_WAYLAND_GL_CONFIG):QT_WAYLAND_GL_CONFIG = $$(QT_WAYLAND_GL_CONFIG)
contains(QT_CONFIG, opengl):!equals(QT_WAYLAND_GL_CONFIG, nogl) {

    DEFINES += QT_WAYLAND_GL_SUPPORT

    HEADERS += \
        $$PWD/qwaylandglintegration.h

    SOURCES += \
        $$PWD/qwaylandglintegration.cpp

    equals(QT_WAYLAND_GL_CONFIG, brcm_egl) {
        CONFIG -= config_wayland_egl config_xcomposite
    }

    equals(QT_WAYLAND_GL_CONFIG, xcomposite) {
        CONFIG -= config_wayland_egl config_brcm_egl
    }

    config_wayland_egl {
        include ($$PWD/wayland_egl/wayland_egl.pri)
        QT_WAYLAND_GL_INTEGRATION = wayland_egl
    }else:config_brcm_egl {
        include ($$PWD/brcm_egl/brcm_egl.pri)
        QT_WAYLAND_GL_INTEGRATION = brcm_egl
    }else:config_xcomposite {
        config_egl {
            include ($$PWD/xcomposite_egl/xcomposite_egl.pri)
            QT_WAYLAND_GL_INTEGRATION = xcomposite_egl
        }else:config_glx {
            include ($$PWD/xcomposite_glx/xcomposite_glx.pri)
            QT_WAYLAND_GL_INTEGRATION = xcomposite_glx
        }
    }else:mac {
        include ($$PWD/readback_cgl/readback_cgl.pri)
            QT_WAYLAND_GL_INTEGRATION = readback_cgl
    }else {
        config_egl {
            include ($$PWD/readback_egl/readback_egl.pri)
            QT_WAYLAND_GL_INTEGRATION = readback_egl
        }else:config_glx {
            include ($$PWD/readback_glx/readback_glx.pri)
            QT_WAYLAND_GL_INTEGRATION = readback_glx
        }
    }

    system(echo "Qt Wayland plugin configured with openGL integration: $$QT_WAYLAND_GL_INTEGRATION")

}


isEmpty(QT_WAYLAND_GL_CONFIG):QT_WAYLAND_GL_CONFIG = $$(QT_WAYLAND_GL_CONFIG)

!isEqual(QT_WAYLAND_GL_CONFIG,nogl) {
    HEADERS += \
        $$PWD/graphicshardwareintegration.h

    SOURCES += \
        $$PWD/graphicshardwareintegration.cpp

    DEFINES += QT_COMPOSITOR_WAYLAND_GL

    isEqual(QT_WAYLAND_GL_CONFIG, custom) {
        QT_WAYLAND_GL_INTEGRATION = $$QT_WAYLAND_GL_CONFIG
    } else {
        equals(QT_WAYLAND_GL_CONFIG, brcm_egl) {
            CONFIG -= config_wayland_egl config_xcomposite
        }

        equals(QT_WAYLAND_GL_CONFIG, xcomposite) {
            CONFIG -= config_wayland_egl config_brcm_egl
        }

        config_wayland_egl {
            include (wayland_egl/wayland_egl.pri)
            QT_WAYLAND_GL_INTEGRATION = wayland_egl
        }else:config_brcm_egl {
            include (brcm_egl/brcm_egl.pri)
            QT_WAYLAND_GL_INTEGRATION = brcm_egl
        }else:config_xcomposite{
            config_egl{
                include (xcomposite_egl/xcomposite_egl.pri)
                QT_WAYLAND_GL_INTEGRATION = xcomposite_egl
            }else:config_glx{
                include (xcomposite_glx/xcomposite_glx.pri)
                QT_WAYLAND_GL_INTEGRATION = xcomposite_glx
            }
        }
    }
    system(echo "Qt-Compositor configured with openGL integration: $$QT_WAYLAND_GL_INTEGRATION")
} else {
    system(echo "Qt-Compositor configured as raster only compositor")
}


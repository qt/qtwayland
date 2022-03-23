%modules = ( # path to module name map
    "QtWaylandGlobal" => "$basedir/src/global",
    "QtWaylandCompositor" => "$basedir/src/compositor",
    "QtWaylandClient" => "$basedir/src/client",
    "QtWaylandEglClientHwIntegration" => "$basedir/src/hardwareintegration/client/wayland-egl",
    "QtWaylandEglCompositorHwIntegration" => "$basedir/src/hardwareintegration/compositor/wayland-egl",
    "QtWlShellIntegration" => "$basedir/src/plugins/shellintegration/wl-shell",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
);
%deprecatedheaders = (
    "QtWaylandClient" =>  {
        "qwaylandclientexport.h" => "QtWaylandClient/qtwaylandclientglobal.h"
    },
    "QtWaylandCompositor" =>  {
        "qwaylandexport.h" => "QtWaylandCompositor/qtwaylandcompositorglobal.h"
    }
);
%classnames = (
    "qwaylandquickextension.h" => "QWaylandQuickExtension",
);
%inject_headers = (
    "$basedir/src/client" => [
        "^qwayland-hardware-integration.h",
        "^qwayland-pointer-gestures-unstable-v1.h",
        "^qwayland-qt-windowmanager.h",
        "^qwayland-qt-key-unstable-v1.h" ,
        "^qwayland-server-buffer-extension.h",
        "^qwayland-surface-extension.h",
        "^qwayland-tablet-unstable-v2.h",
        "^qwayland-text-input-unstable-v1.h",
        "^qwayland-text-input-unstable-v2.h",
        "^qwayland-text-input-unstable-v4-wip.h",
        "^qwayland-qt-text-input-method-unstable-v1.h",
        "^qwayland-touch-extension.h",
        "^qwayland-viewporter.h",
        "^qwayland-wayland.h",
        "^qwayland-wp-primary-selection-unstable-v1.h",
        "^qwayland-fractional-scale-v1.h",
        "^qwayland-xdg-output-unstable-v1.h",
        "^wayland-hardware-integration-client-protocol.h",
        "^wayland-pointer-gestures-unstable-v1-client-protocol.h",
        "^wayland-qt-windowmanager-client-protocol.h",
        "^wayland-qt-key-unstable-v1-client-protocol.h",
        "^wayland-server-buffer-extension-client-protocol.h",
        "^wayland-surface-extension-client-protocol.h",
        "^wayland-tablet-unstable-v2-client-protocol.h",
        "^wayland-text-input-unstable-v1-client-protocol.h",
        "^wayland-text-input-unstable-v2-client-protocol.h",
        "^wayland-text-input-unstable-v4-wip-client-protocol.h",
        "^wayland-qt-text-input-method-unstable-v1-client-protocol.h",
        "^wayland-touch-extension-client-protocol.h",
        "^wayland-viewporter-client-protocol.h",
        "^wayland-wayland-client-protocol.h",
        "^wayland-fractional-scale-v1-client-protocol.h",
        "^wayland-wp-primary-selection-unstable-v1-client-protocol.h",
        "^wayland-xdg-output-unstable-v1-client-protocol.h",
    ],
    "$basedir/src/plugins/shellintegration/xdg-shell" => [
        "^qwayland-xdg-shell.h",
        "^qwayland-xdg-decoration-unstable-v1.h",
        "^wayland-xdg-shell-client-protocol.h",
        "^wayland-xdg-decoration-unstable-v1-client-protocol.h",
    ],
    "$basedir/src/compositor" => [
        "^qwayland-server-wayland.h",
        "^qwayland-server-hardware-integration.h",
        "^qwayland-server-idle-inhibit-unstable-v1.h",
        "^qwayland-server-ivi-application.h",
        "^qwayland-server-qt-windowmanager.h",
        "^qwayland-server-qt-key-unstable-v1.h",
        "^qwayland-server-qt-texture-sharing-unstable-v1.h",
        "^qwayland-server-scaler.h",
        "^qwayland-server-server-buffer-extension.h",
        "^qwayland-server-text-input-unstable-v2.h",
        "^qwayland-server-text-input-unstable-v4-wip.h",
        "^qwayland-server-qt-text-input-method-unstable-v1.h",
        "^qwayland-server-touch-extension.h",
        "^qwayland-server-viewporter.h",
        "^qwayland-server-xdg-decoration-unstable-v1.h",
        "^qwayland-server-xdg-output-unstable-v1.h",
        "^qwayland-server-xdg-shell.h",
        "^qwayland-server-presentation-time.h",
        "^wayland-hardware-integration-server-protocol.h",
        "^wayland-idle-inhibit-unstable-v1-server-protocol.h",
        "^wayland-ivi-application-server-protocol.h",
        "^wayland-qt-windowmanager-server-protocol.h",
        "^wayland-qt-key-unstable-v1-server-protocol.h",
        "^wayland-qt-texture-sharing-unstable-v1-server-protocol.h",
        "^wayland-scaler-server-protocol.h",
        "^wayland-server-buffer-extension-server-protocol.h",
        "^wayland-text-input-unstable-v2-server-protocol.h",
        "^wayland-text-input-unstable-v4-wip-server-protocol.h",
        "^wayland-qt-text-input-method-unstable-v1-server-protocol.h",
        "^wayland-viewporter-server-protocol.h",
        "^wayland-touch-extension-server-protocol.h",
        "^wayland-wayland-server-protocol.h",
        "^wayland-xdg-decoration-unstable-v1-server-protocol.h",
        "^wayland-xdg-output-unstable-v1-server-protocol.h",
        "^wayland-xdg-shell-server-protocol.h",
        "^qwayland-server-qt-shell-unstable-v1.h",
        "^wayland-qt-shell-unstable-v1-server-protocol.h",
        "^wayland-presentation-time-server-protocol.h",
    ],
    "$basedir/src/plugins/hardwareintegration/compositor/linux-dmabuf-unstable-v1" => [
        "^qwayland-server-linux-dmabuf-unstable-v1.h",
        "^wayland-linux-dmabuf-unstable-v1-server-protocol.h",
    ],
);
@private_headers = ( qr/^qwayland-.*\.h/, qr/^wayland-.*-protocol\.h/ );

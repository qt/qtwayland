// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQTSHELL_H
#define QWAYLANDQTSHELL_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandResource>
#include <QtCore/QSize>

#include <QtWaylandCompositor/QWaylandShellSurface>
#include <QtWaylandCompositor/qwaylandquickchildren.h>

struct wl_resource;
struct wl_interface;

QT_BEGIN_NAMESPACE

class QWaylandQtShellPrivate;
class QWaylandQtShellSurface;
class QWaylandQtShellChrome;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQtShell : public QWaylandCompositorExtensionTemplate<QWaylandQtShell>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQtShell)

public:
    QWaylandQtShell();
    QWaylandQtShell(QWaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

    void registerChrome(QWaylandQtShellChrome *chrome);
    void unregisterChrome(QWaylandQtShellChrome *chrome);

private Q_SLOTS:
    void chromeActivated();
    void chromeDeactivated();

Q_SIGNALS:
    void qtShellSurfaceRequested(QWaylandSurface *surface, const QWaylandResource &resource);
    void qtShellSurfaceCreated(QWaylandQtShellSurface *qtShellSurface);

private:
    bool moveChromeToFront(QWaylandQtShellChrome *chrome);
};


class QWaylandQtShellSurfacePrivate;
class QWaylandSurfaceRole;
class QWaylandResource;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQtShellSurface : public QWaylandShellSurfaceTemplate<QWaylandQtShellSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQtShellSurface)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandQtShellSurface)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(uint windowFlags READ windowFlags NOTIFY windowFlagsChanged)
    Q_PROPERTY(uint windowState READ windowState NOTIFY windowStateChanged)
    Q_PROPERTY(QString windowTitle READ windowTitle READ windowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(QRect windowGeometry READ windowGeometry NOTIFY windowGeometryChanged)
    Q_PROPERTY(QPoint windowPosition READ windowPosition WRITE setWindowPosition NOTIFY windowGeometryChanged)
    Q_PROPERTY(bool positionAutomatic READ positionAutomatic NOTIFY positionAutomaticChanged)
    Q_PROPERTY(QSize minimumSize READ minimumSize NOTIFY minimumSizeChanged)
    Q_PROPERTY(QSize maximumSize READ maximumSize NOTIFY maximumSizeChanged)
    Q_PROPERTY(int frameMarginLeft READ frameMarginLeft WRITE setFrameMarginLeft NOTIFY frameMarginChanged)
    Q_PROPERTY(int frameMarginRight READ frameMarginRight WRITE setFrameMarginRight NOTIFY frameMarginChanged)
    Q_PROPERTY(int frameMarginTop READ frameMarginTop WRITE setFrameMarginTop NOTIFY frameMarginChanged)
    Q_PROPERTY(int frameMarginBottom READ frameMarginBottom WRITE setFrameMarginBottom NOTIFY frameMarginChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(CapabilityFlags capabilities READ capabilities WRITE setCapabilities NOTIFY capabilitiesChanged)
    Q_MOC_INCLUDE("qwaylandsurface.h")

public:
    // Matches the "capabilities" enum in the protocol xml
    enum CapabilityFlag {
        InteractiveMove = 1,
        InteractiveResize = 2
    };
    Q_DECLARE_FLAGS(CapabilityFlags, CapabilityFlag)
    Q_ENUM(CapabilityFlag)

    QWaylandQtShellSurface();
    QWaylandQtShellSurface(QWaylandQtShell *application, QWaylandSurface *surface, const QWaylandResource &resource);

    void initialize(QWaylandQtShell *qtShell, QWaylandSurface *surface,
                    const QWaylandResource &resource);

    QWaylandSurface *surface() const;

    static const wl_interface *interface();
    static QByteArray interfaceName();
    static QWaylandSurfaceRole *role();
    static QWaylandQtShellSurface *fromResource(::wl_resource *resource);

    QRect windowGeometry() const;

    void setWindowPosition(const QPoint &position);
    QPoint windowPosition() const;

    Q_INVOKABLE void requestWindowGeometry(uint windowState, const QRect &windowGeometry);

    QSize minimumSize() const;
    QSize maximumSize() const;

    void setFrameMargins(const QMargins &margins);

    int frameMarginLeft() const;
    void setFrameMarginLeft(int left);

    int frameMarginRight() const;
    void setFrameMarginRight(int right);

    int frameMarginTop() const;
    void setFrameMarginTop(int top);

    int frameMarginBottom() const;
    void setFrameMarginBottom(int bottom);

    bool positionAutomatic() const;

    bool active() const;
    void setActive(bool active);

    QString windowTitle() const;

    uint windowFlags() const;

    Q_INVOKABLE void sendClose();

    uint windowState() const;
    void setWindowState(uint windowState);
#if QT_CONFIG(wayland_compositor_quick)
    QWaylandQuickShellIntegration *createIntegration(QWaylandQuickShellSurfaceItem *item) override;
#endif

    CapabilityFlags capabilities() const;
    void setCapabilities(CapabilityFlags capabilities);

Q_SIGNALS:
    void surfaceChanged();
    void windowFlagsChanged();
    void windowStateChanged();
    void windowGeometryChanged();
    void minimumSizeChanged();
    void maximumSizeChanged();
    void positionAutomaticChanged();
    void startMove();
    void startResize(Qt::Edges edges);
    void windowTitleChanged();
    void frameMarginChanged();
    void raiseRequested();
    void lowerRequested();
    void activeChanged();
    void capabilitiesChanged();

private Q_SLOTS:
    void surfaceCommitted();

private:
    friend class QWaylandQtShellChrome;

    void initialize() override;

    QWaylandQtShell *shell() const;
};

QT_END_NAMESPACE

#endif // QWAYLANDQTSHELL_H

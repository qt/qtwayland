/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "waylandcompositor.h"

#include "waylandsurface.h"

#include <QApplication>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>

#include <QDebug>

// This has no GL support, meaning only readback-based GL clients will work.
// Others, e.g. xcomposite-glx, will typically crash the client as there is no
// corresponding integration on the compositor side.

class QSurfaceWidget : public QWidget
{
    Q_OBJECT
public:
    QSurfaceWidget(WaylandSurface *surface, QWidget *parent = 0) : QWidget(parent), m_surface(surface) {
	setMouseTracking(true);

	connect(m_surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));
    }

private slots:
    void surfaceDamaged(const QRect &rect) {

        // NB! This is dangerous. There may be no paintEvent() called, for
        // example if the mdi subwindow is completely covered by another one.
        // And in that case there is no frameFinished() -> the client may block
        // in its waitForFrameSync() for a long time...

	update(rect);
    }

protected:
    void paintEvent(QPaintEvent *) {
        QPainter p(this);

	if (m_surface->type() == WaylandSurface::Shm)
	    p.drawImage(0, 0, m_surface->image());
	else
	    qWarning() << "Can't draw surface of type" << m_surface->type();

        m_surface->frameFinished();
    }

    void mousePressEvent(QMouseEvent *e) {
	m_surface->sendMousePressEvent(e->pos(), e->button());
    }

    void mouseMoveEvent(QMouseEvent *e) {
	m_surface->sendMouseMoveEvent(e->pos());
    }

    void mouseReleaseEvent(QMouseEvent *e) {
	m_surface->sendMouseReleaseEvent(e->pos(), e->button());
    }

    void keyPressEvent(QKeyEvent *e) {
        m_surface->sendKeyPressEvent(e->nativeScanCode());
    }

    void keyReleaseEvent(QKeyEvent *e) {
        m_surface->sendKeyReleaseEvent(e->nativeScanCode());
    }

private:
    WaylandSurface *m_surface;
};

class QSurfaceSubWindow : public QMdiSubWindow
{
    Q_OBJECT
public:
    QSurfaceSubWindow(WaylandSurface *surface, QWidget *parent = 0) : QMdiSubWindow(parent), m_surface(surface) {
	QSurfaceWidget *widget = new QSurfaceWidget(surface, this);

	setContentsMargins(0, 0, 0, 0);
	setMouseTracking(true);
	setWidget(widget);

        connect(m_surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
        connect(m_surface, SIGNAL(mapped(const QSize &)), this, SLOT(surfaceMapped(const QSize &)));
        connect(m_surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));
    }

private slots:
    void surfaceDestroyed(QObject *) {
	hide();
	deleteLater();
    }

    void surfaceMapped(const QSize &size) {
        m_surface->setInputFocus();
        widget()->setMinimumSize(size);
        resize(sizeHint());
        show();
    }

    void surfaceDamaged(const QRect &) {
    }

protected:
    void focusInEvent(QFocusEvent * e) {
	QMdiSubWindow::focusInEvent(e);
	widget()->setFocus();
	m_surface->setInputFocus();
    }

    void resizeEvent(QResizeEvent *e) {
	QMdiSubWindow::resizeEvent(e);
	m_surface->setGeometry(geometry());
    }

    void moveEvent(QMoveEvent *e) {
	QMdiSubWindow::moveEvent(e);
	m_surface->setGeometry(geometry());
    }


protected:
    WaylandSurface *m_surface;
};

class QWidgetCompositorMdi : public QMainWindow, public WaylandCompositor
{
    Q_OBJECT
public:
    QWidgetCompositorMdi() : WaylandCompositor(this->windowHandle()) {
	m_mdiArea = new QMdiArea(this);

	setCentralWidget(m_mdiArea);
    }

protected:
    void surfaceCreated(WaylandSurface *surface) {
	QMdiSubWindow *window = new QSurfaceSubWindow(surface);

	window->setAttribute(Qt::WA_DeleteOnClose);
	m_mdiArea->addSubWindow(window);
    }

private:
    QMdiArea *m_mdiArea;
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidgetCompositorMdi compositor;

    compositor.resize(800, 600);
    compositor.show();

    return app.exec();

}

#include "main.moc"

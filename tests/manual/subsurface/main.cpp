// Copyright (C) 2015 LG Electronics Inc, author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlFileSelector>
#include <QQmlContext>
#include <QQuickView>

#include "shmwindow.h"

class Filter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool sync READ getSync NOTIFY syncChanged)

public:
    Filter()
    {
        sync = false;
    }

    bool eventFilter(QObject *object, QEvent *event)
    {
        Q_UNUSED(object);
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Space) {
                toggleSync(quick);
                toggleSync(shm);
            }
        }
        return false;
    }

    void toggleSync(QWindow *w)
    {
    }

    bool getSync() const
    {
        return sync;
    }

Q_SIGNALS:
    void syncChanged();

public:
    QWindow *quick;
    QWindow *shm;
    bool sync;
};

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QQuickView view;
    view.connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));
    view.setResizeMode(QQuickView::SizeRootObjectToView);

    Filter f;
    view.rootContext()->setContextProperty("syncStatus", &f);
    view.installEventFilter(&f);

    view.setSource(QUrl("qrc:/main.qml"));
    view.show();

    QQuickView child(&view);
    child.connect(child.engine(), SIGNAL(quit()), &app, SLOT(quit()));
    child.setSource(QUrl("qrc:/child.qml"));
    child.setResizeMode(QQuickView::SizeRootObjectToView);
    child.setGeometry(QRect(150, 70, 100, 100));
    child.show();

    ShmWindow shm(&view);
    shm.setGeometry(QRect(30, 30, 50, 50));
    shm.show();

    f.quick = &child;
    f.shm = &shm;

    return app.exec();
}

#include "main.moc"


// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CUSTOMEXTENSION_H
#define CUSTOMEXTENSION_H

#include <QtWaylandClient/QWaylandClientExtension>
#include <QtGui/QWindow>
#include <QtQml/QQmlEngine>

#include "qwayland-custom.h"

QT_BEGIN_NAMESPACE

class CustomExtensionObject;

//! [CustomExtension]
class CustomExtension : public QWaylandClientExtensionTemplate<CustomExtension>
        , public QtWayland::qt_example_extension
//! [CustomExtension]
{
    Q_OBJECT
    QML_ELEMENT
public:
    CustomExtension();
    Q_INVOKABLE void registerWindow(QWindow *window);

    CustomExtensionObject *createCustomObject(const QString &color, const QString &text);

public slots:
    void sendBounce(QWindow *window, uint ms);
    void sendSpin(QWindow *window, uint ms);

signals:
    void eventReceived(const QString &text, uint value);
    void fontSize(QWindow *window, uint pixelSize);
    void showDecorations(bool);

private slots:
    void handleExtensionActive();

private:
    void example_extension_close(wl_surface *surface) override;
    void example_extension_set_font_size(wl_surface *surface, uint32_t pixel_size) override;
    void example_extension_set_window_decoration(uint32_t state) override;

    bool eventFilter(QObject *object, QEvent *event) override;

    QWindow *windowForSurface(struct ::wl_surface *);
    void sendWindowRegistration(QWindow *);

    QList<QWindow *> m_windows;
    bool m_activated = false;
};

class CustomExtensionObject : public QWaylandClientExtensionTemplate<CustomExtensionObject>
        , public QtWayland::qt_example_local_object
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    CustomExtensionObject(struct ::qt_example_local_object *wl_object, const QString &text);

    QString text() const
    {
        return m_text;
    }

protected:
    void example_local_object_clicked() override;

public slots:
    void setText(const QString &text);

signals:
    void textChanged(const QString &text);
    void clicked();

private:
    QString m_text;
};



QT_END_NAMESPACE

#endif // CUSTOMEXTENSION_H

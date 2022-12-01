// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDKEYMAP_H
#define QWAYLANDKEYMAP_H

#include <QtCore/QObject>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qtwaylandqmlinclude.h>
#if QT_CONFIG(wayland_compositor_quick)
#include <QtWaylandCompositor/qwaylandquickchildren.h>
#endif

QT_BEGIN_NAMESPACE

class QWaylandKeymapPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandKeymap : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandKeymap)
#if QT_CONFIG(wayland_compositor_quick)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandKeymap)
#endif
    Q_PROPERTY(QString layout READ layout WRITE setLayout NOTIFY layoutChanged)
    Q_PROPERTY(QString variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(QString options READ options WRITE setOptions NOTIFY optionsChanged)
    Q_PROPERTY(QString rules READ rules WRITE setRules NOTIFY rulesChanged)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    QML_NAMED_ELEMENT(WaylandKeymap)
    QML_ADDED_IN_VERSION(1, 0)
public:
    QWaylandKeymap(const QString &layout = QString(), const QString &variant = QString(), const QString &options = QString(),
                   const QString &model = QString(), const QString &rules = QString(), QObject *parent = nullptr);

    QString layout() const;
    void setLayout(const QString &layout);
    QString variant() const;
    void setVariant(const QString &variant);
    QString options() const;
    void setOptions(const QString &options);
    QString rules() const;
    void setRules(const QString &rules);
    QString model() const;
    void setModel(const QString &model);

Q_SIGNALS:
    void layoutChanged();
    void variantChanged();
    void optionsChanged();
    void rulesChanged();
    void modelChanged();
};

QT_END_NAMESPACE

#endif //QWAYLANDKEYMAP_H

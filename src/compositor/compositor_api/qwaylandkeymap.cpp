/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

#include "qwaylandkeymap.h"
#include "qwaylandkeymap_p.h"

QT_BEGIN_NAMESPACE

QWaylandKeymap::QWaylandKeymap(const QString &layout, const QString &variant, const QString &options, const QString &model, const QString &rules, QObject *parent)
    : QObject(*new QWaylandKeymapPrivate(layout, variant, options, model, rules), parent)
{
}

QString QWaylandKeymap::layout() const {
    Q_D(const QWaylandKeymap);
    return d->m_layout;
}

void QWaylandKeymap::setLayout(const QString &layout)
{
    Q_D(QWaylandKeymap);
    if (d->m_layout == layout)
        return;
    d->m_layout = layout;
    emit layoutChanged();
}

QString QWaylandKeymap::variant() const
{
    Q_D(const QWaylandKeymap);
    return d->m_variant;
}

void QWaylandKeymap::setVariant(const QString &variant)
{
    Q_D(QWaylandKeymap);
    if (d->m_variant == variant)
        return;
    d->m_variant = variant;
    emit variantChanged();
}

QString QWaylandKeymap::options() const {
    Q_D(const QWaylandKeymap);
    return d->m_options;
}

void QWaylandKeymap::setOptions(const QString &options)
{
    Q_D(QWaylandKeymap);
    if (d->m_options == options)
        return;
    d->m_options = options;
    emit optionsChanged();
}

QString QWaylandKeymap::rules() const {
    Q_D(const QWaylandKeymap);
    return d->m_rules;
}

void QWaylandKeymap::setRules(const QString &rules)
{
    Q_D(QWaylandKeymap);
    if (d->m_rules == rules)
        return;
    d->m_rules = rules;
    emit rulesChanged();
}

QString QWaylandKeymap::model() const {
    Q_D(const QWaylandKeymap);
    return d->m_model;
}

void QWaylandKeymap::setModel(const QString &model)
{
    Q_D(QWaylandKeymap);
    if (d->m_model == model)
        return;
    d->m_model = model;
    emit modelChanged();
}

QWaylandKeymapPrivate::QWaylandKeymapPrivate(const QString &layout, const QString &variant,
                                             const QString &options, const QString &model,
                                             const QString &rules)
    : m_layout(layout)
    , m_variant(variant)
    , m_options(options)
    , m_rules(rules)
    , m_model(model)
{
}

QT_END_NAMESPACE

#include "moc_qwaylandkeymap.cpp"

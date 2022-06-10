// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandoutputmode.h"
#include "qwaylandoutputmode_p.h"

/*!
   \class QWaylandOutputMode
   \inmodule QtWaylandCompositor
   \since 5.8
   \brief The QWaylandOutputMode class holds the resolution and refresh rate of an output.

   QWaylandOutputMode holds the resolution and refresh rate of an output.
   Resolution is expressed in pixels and refresh rate is measured in mHz.

   \sa QWaylandOutput
*/

QWaylandOutputMode::QWaylandOutputMode()
    : d(new QWaylandOutputModePrivate)
{
}

QWaylandOutputMode::QWaylandOutputMode(const QSize &size, int refreshRate)
    : d(new QWaylandOutputModePrivate)
{
    d->size = size;
    d->refreshRate = refreshRate;
}

QWaylandOutputMode::QWaylandOutputMode(const QWaylandOutputMode &other)
    : d(new QWaylandOutputModePrivate)
{
    d->size = other.size();
    d->refreshRate = other.refreshRate();
}

QWaylandOutputMode::~QWaylandOutputMode()
{
    delete d;
}

QWaylandOutputMode &QWaylandOutputMode::operator=(const QWaylandOutputMode &other)
{
    d->size = other.size();
    d->refreshRate = other.refreshRate();
    return *this;
}

/*!
    Returns \c true if this mode is equal to \a other,
    otherwise returns \c false.
*/
bool QWaylandOutputMode::operator==(const QWaylandOutputMode &other) const
{
    return size() == other.size() && refreshRate() == other.refreshRate();
}

/*!
    Returns \c true if this mode is not equal to \a other,
    otherwise returns \c false.
*/
bool QWaylandOutputMode::operator!=(const QWaylandOutputMode &other) const
{
    return size() != other.size() || refreshRate() != other.refreshRate();
}

/*!
    Returns whether this mode contains a valid resolution and refresh rate.
*/
bool QWaylandOutputMode::isValid() const
{
    return !d->size.isEmpty() && d->refreshRate > 0;
}

/*!
    Returns the resolution in pixels.
*/
QSize QWaylandOutputMode::size() const
{
    return d->size;
}

/*!
    Returns the refresh rate in mHz.
*/
int QWaylandOutputMode::refreshRate() const
{
    return d->refreshRate;
}

/*!
 * \internal
 */
void QWaylandOutputMode::setSize(const QSize &size)
{
    d->size = size;
}

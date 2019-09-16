/****************************************************************************
**
** Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MOCKXDGOUTPUTV1_H
#define MOCKXDGOUTPUTV1_H

#include <QPoint>
#include <QSize>
#include <QString>

#include "qwayland-xdg-output-unstable-v1.h"

class MockXdgOutputV1 : public QtWayland::zxdg_output_v1
{
public:
    explicit MockXdgOutputV1(struct ::zxdg_output_v1 *object);
    ~MockXdgOutputV1();

    QString name;
    QString description;
    QPoint logicalPosition;
    QSize logicalSize;

    struct {
        QString name;
        QString description;
        QPoint logicalPosition;
        QSize logicalSize;
    } pending;

protected:
    void zxdg_output_v1_logical_position(int32_t x, int32_t y) override;
    void zxdg_output_v1_logical_size(int32_t width, int32_t height) override;
    void zxdg_output_v1_done() override;
    void zxdg_output_v1_name(const QString &name) override;
    void zxdg_output_v1_description(const QString &description) override;
};

#endif // MOCKXDGOUTPUTV1_H

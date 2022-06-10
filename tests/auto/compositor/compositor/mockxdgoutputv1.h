// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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

// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef VULKANWRAPPER_H
#define VULKANWRAPPER_H

#include <QOpenGLContext>

QT_BEGIN_NAMESPACE

class VulkanWrapper;
struct VulkanImageWrapper;
class VulkanWrapperPrivate;

class QOpenGLContext;
class QImage;

class VulkanWrapper
{
public:
    VulkanWrapper(QOpenGLContext *glContext);

    VulkanImageWrapper *createTextureImage(const QImage &img);
    VulkanImageWrapper *createTextureImageFromData(const uchar *pixels, uint bufferSize, const QSize &size, uint glInternalFormat);
    int getImageInfo(const VulkanImageWrapper *imgWrapper, int *memSize, int *w = nullptr, int *h = nullptr);
    void freeTextureImage(VulkanImageWrapper *imageWrapper);

private:
    VulkanWrapperPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // VULKANWRAPPER_H

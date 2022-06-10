// Copyright (C) 2021 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.VirtualKeyboard

InputPanel {
    visible: active
    y: active ? parent.height - height : parent.height
    anchors.left: parent.left
    anchors.right: parent.right
}

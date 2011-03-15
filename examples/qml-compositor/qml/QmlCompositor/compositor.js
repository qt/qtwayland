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

var windowList = null;

function relayout() {
    if (windowList.length == 0)
        return;

    var dim = Math.ceil(Math.sqrt(windowList.length));
    var w = root.width / dim;
    var h = root.height / dim;

    var cols = dim;
    var rows = Math.floor(windowList.length / cols);
    var i;
    var ix = 0;
    var iy = 0;
    var lastDim = 1;

    for (i = 0; i < windowList.length; ++i) {
        if (i > 0) {
            var currentDim = Math.ceil(Math.sqrt(i + 1));
            if (currentDim == lastDim) {
                if (iy < currentDim - 1) {
                    ++iy;
                    if (iy == currentDim - 1)
                        ix = 0;
                } else {
                    ++ix;
                }
            } else {
                iy = 0;
                ix = currentDim - 1;
            }
            lastDim = currentDim;
        }

        var cx = (ix + 0.5) * w;
        var cy = (iy + 0.5) * h;

        windowList[i].scale = 0.98 * Math.min(w / windowList[i].width, h / windowList[i].height);

        windowList[i].x = (cx - windowList[i].width / 2);
        windowList[i].y = (cy - windowList[i].height / 2);
    }
}


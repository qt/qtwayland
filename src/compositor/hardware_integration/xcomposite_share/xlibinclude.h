#ifndef XLIBINCLUDE_H
#define XLIBINCLUDE_H

#include <QtCore/QEvent>
#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>
#include <QtGui/QCursor>
#include <QtGui/private/qguiapplication_p.h>

#include <X11/Xlib.h>
#include "X11/extensions/Xcomposite.h"

enum {
    XFocusOut = FocusOut,
    XFocusIn = FocusIn,
    XKeyPress = KeyPress,
    XKeyRelease = KeyRelease,
    XNone = None,
    XRevertToParent = RevertToParent,
    XGrayScale = GrayScale,
    XCursorShape = CursorShape
};
#undef FocusOut
#undef FocusIn
#undef KeyPress
#undef KeyRelease
#undef None
#undef RevertToParent
#undef GrayScale
#undef CursorShape

#ifdef FontChange
#undef FontChange
#endif

#endif //XLIBINCLUDE_H

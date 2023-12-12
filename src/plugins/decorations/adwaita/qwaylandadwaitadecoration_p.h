// Copyright (C) 2023 Jan Grulich <jgrulich@redhat.com>
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDADWAITADECORATION_P_H
#define QWAYLANDADWAITADECORATION_P_H

#include <QtWaylandClient/private/qwaylandabstractdecoration_p.h>

#include <QtCore/QDateTime>

QT_BEGIN_NAMESPACE

class QDBusVariant;
class QPainter;

namespace QtWaylandClient {

//
//      INFO
//  -------------
//
// This is a Qt decoration plugin implementing Adwaita-like (GNOME) client-side
// window decorations. It uses xdg-desktop-portal to get the user configuration.
// This plugin was originally part of QGnomePlatform and later made a separate
// project named QAdwaitaDecorations.
//
// INFO: How SVG icons are used here?
// We try to find an SVG icon for a particular button from the current icon theme.
// This icon is then opened as a file, it's content saved and later loaded to be
// painted with QSvgRenderer, but before it's painted, we try to find following
// patterns:
//  1) fill=[\"']#[0-9A-F]{6}[\"']
//  2) fill:#[0-9A-F]{6}
//  3) fill=[\"']currentColor[\"']
// The color in this case doesn't match the theme and is replaced by Foreground color.
//
// FIXME/TODO:
// This plugin currently have all the colors for the decorations hardcoded.
// There might be a way to get these from GTK/libadwaita (not sure), but problem is
// we want Gtk4 version and using Gtk4 together with QGtk3Theme from QtBase that links
// to Gtk3 will not work out. Possibly in future we can make a QGtk4Theme providing us
// what we need to paint the decorations without having to deal with the colors ourself.
//
// TODO: Implement shadows


class QWaylandAdwaitaDecoration : public QWaylandAbstractDecoration
{
    Q_OBJECT
public:
    enum ColorType {
        Background,
        BackgroundInactive,
        Foreground,
        ForegroundInactive,
        Border,
        BorderInactive,
        ButtonBackground,
        ButtonBackgroundInactive,
        HoveredButtonBackground,
        PressedButtonBackground
    };

    enum Placement {
        Left = 0,
        Right = 1
    };

    enum Button {
        None = 0x0,
        Close = 0x1,
        Minimize = 0x02,
        Maximize = 0x04
    };
    Q_DECLARE_FLAGS(Buttons, Button);

    enum ButtonIcon {
        CloseIcon,
        MinimizeIcon,
        MaximizeIcon,
        RestoreIcon
    };

    QWaylandAdwaitaDecoration();
    virtual ~QWaylandAdwaitaDecoration() = default;

protected:
    QMargins margins(MarginsType marginsType = Full) const override;
    void paint(QPaintDevice *device) override;
    bool handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
                     Qt::MouseButtons b, Qt::KeyboardModifiers mods) override;
    bool handleTouch(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
                     QEventPoint::State state, Qt::KeyboardModifiers mods) override;

private Q_SLOTS:
    void settingChanged(const QString &group, const QString &key, const QDBusVariant &value);

private:
    // Makes a call to xdg-desktop-portal (Settings) to load initial configuration
    void loadConfiguration();
    // Updates color scheme from light to dark and vice-versa
    void updateColors(bool isDark);
    // Updates titlebar layout with position and button order
    void updateTitlebarLayout(const QString &layout);

    // Returns a bounding rect for a given button type
    QRectF buttonRect(Button button) const;
    // Draw given button type using SVG icon (when found) or fallback to QPixmap icon
    void drawButton(Button button, QPainter *painter);

    // Returns color for given type and button
    QColor color(ColorType type, Button button = None);

    // Returns whether the left button was clicked i.e. pressed and released
    bool clickButton(Qt::MouseButtons b, Button btn);
    // Returns whether the left button was double-clicked
    bool doubleClickButton(Qt::MouseButtons b, const QPointF &local, const QDateTime &currentTime);
    // Updates button hover state
    void updateButtonHoverState(Button hoveredButton);

    void processMouseTop(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,
                         Qt::KeyboardModifiers mods);
    void processMouseBottom(QWaylandInputDevice *inputDevice, const QPointF &local,
                            Qt::MouseButtons b, Qt::KeyboardModifiers mods);
    void processMouseLeft(QWaylandInputDevice *inputDevice, const QPointF &local,
                          Qt::MouseButtons b, Qt::KeyboardModifiers mods);
    void processMouseRight(QWaylandInputDevice *inputDevice, const QPointF &local,
                           Qt::MouseButtons b, Qt::KeyboardModifiers mods);
    // Request to repaint the decorations. This will be invoked when button hover changes or
    // when there is a setting change (e.g. layout change).
    void requestRepaint() const;

    // Button states
    Button m_clicking = None;
    Buttons m_hoveredButtons = None;
    QDateTime m_lastButtonClick;
    QPointF m_lastButtonClickPosition;

    // Configuration
    QMap<Button, uint> m_buttons;
    QMap<ColorType, QColor> m_colors;
    QMap<ButtonIcon, QString> m_icons;
    std::unique_ptr<QFont> m_font;
    Placement m_placement = Right;

    QStaticText m_windowTitle;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QWaylandAdwaitaDecoration::Buttons)

} // namespace QtWaylandClient

QT_END_NAMESPACE

#endif // QWAYLANDADWAITADECORATION_P_H

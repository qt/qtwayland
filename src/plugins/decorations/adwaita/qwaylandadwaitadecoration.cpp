// Copyright (C) 2023 Jan Grulich <jgrulich@redhat.com>
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwaylandadwaitadecoration_p.h"

// QtCore
#include <QtCore/QLoggingCategory>
#include <QScopeGuard>

// QtDBus
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingCall>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusVariant>
#include <QtDBus/QtDBus>

// QtGui
#include <QtGui/QColor>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>

#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformtheme.h>

// QtSvg
#include <QtSvg/QSvgRenderer>

// QtWayland
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#include <QtWaylandClient/private/qwaylandshmbackingstore_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace QtWaylandClient {

static constexpr int ceButtonSpacing = 12;
static constexpr int ceButtonWidth = 24;
static constexpr int ceCornerRadius = 12;
static constexpr int ceShadowsWidth = 10;
static constexpr int ceTitlebarHeight = 38;
static constexpr int ceWindowBorderWidth = 1;

static QMap<QWaylandAdwaitaDecoration::ButtonIcon, QString> buttonMap = {
    { QWaylandAdwaitaDecoration::CloseIcon, "window-close-symbolic"_L1 },
    { QWaylandAdwaitaDecoration::MinimizeIcon, "window-minimize-symbolic"_L1 },
    { QWaylandAdwaitaDecoration::MaximizeIcon, "window-maximize-symbolic"_L1 },
    { QWaylandAdwaitaDecoration::RestoreIcon, "window-restore-symbolic"_L1 }
};

const QDBusArgument &operator>>(const QDBusArgument &argument, QMap<QString, QVariantMap> &map)
{
    argument.beginMap();
    map.clear();

    while (!argument.atEnd()) {
        QString key;
        QVariantMap value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        map.insert(key, value);
    }

    argument.endMap();
    return argument;
}

Q_LOGGING_CATEGORY(lcQWaylandAdwaitaDecorationLog, "qt.qpa.qwaylandadwaitadecoration", QtWarningMsg)

QWaylandAdwaitaDecoration::QWaylandAdwaitaDecoration()
    : QWaylandAbstractDecoration()
{
    m_lastButtonClick = QDateTime::currentDateTime();

    QTextOption option(Qt::AlignHCenter | Qt::AlignVCenter);
    option.setWrapMode(QTextOption::NoWrap);
    m_windowTitle.setTextOption(option);
    m_windowTitle.setTextFormat(Qt::PlainText);

    const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme();
    if (const QFont *font = theme->font(QPlatformTheme::TitleBarFont))
        m_font = std::make_unique<QFont>(*font);
    if (!m_font) // Fallback to GNOME's default font
        m_font = std::make_unique<QFont>("Cantarell"_L1, 10);

    QTimer::singleShot(0, this, &QWaylandAdwaitaDecoration::loadConfiguration);
}

QMargins QWaylandAdwaitaDecoration::margins(QWaylandAbstractDecoration::MarginsType marginsType) const
{
    const bool onlyShadows = marginsType == QWaylandAbstractDecoration::ShadowsOnly;
    const bool shadowsExcluded = marginsType == ShadowsExcluded;

    if (waylandWindow()->windowStates() & Qt::WindowMaximized) {
        // Maximized windows don't have anything around, no shadows, border,
        // etc. Only report titlebar height in case we are not asking for shadow
        // margins.
        return QMargins(0, onlyShadows ? 0 : ceTitlebarHeight, 0, 0);
    }

    const QWaylandWindow::ToplevelWindowTilingStates tilingStates = waylandWindow()->toplevelWindowTilingStates();

    // Since all sides (left, right, bottom) are going to be same
    const int marginsBase = shadowsExcluded ? ceWindowBorderWidth : ceShadowsWidth + ceWindowBorderWidth;
    const int sideMargins = onlyShadows ? ceShadowsWidth : marginsBase;
    const int topMargins = onlyShadows ? ceShadowsWidth : ceTitlebarHeight + marginsBase;

    return QMargins(tilingStates & QWaylandWindow::WindowTiledLeft ? 0 : sideMargins,
                    tilingStates & QWaylandWindow::WindowTiledTop ? onlyShadows ? 0 : ceTitlebarHeight : topMargins,
                    tilingStates & QWaylandWindow::WindowTiledRight ? 0 : sideMargins,
                    tilingStates & QWaylandWindow::WindowTiledBottom ? 0 : sideMargins);
}

QRect QWaylandAdwaitaDecoration::surfaceRect() const
{
    return QRect(QPoint(), waylandWindow()->geometry().marginsAdded(margins()).size());
}

void QWaylandAdwaitaDecoration::paintFrame(QPainter *p, const QRect &clip)
{
    const QRect surfaceRect = this->surfaceRect();

    p->setRenderHint(QPainter::Antialiasing);

    /*
     * Titlebar and window border
     */
    QPainterPath path;
    const QPointF topLeft = { margins(ShadowsOnly).left() + qreal(0.5),
                              margins(ShadowsOnly).top() - qreal(0.5) };
    const int frameWidth = surfaceRect.width() - margins(ShadowsOnly).left()
            - margins(ShadowsOnly).right() - qreal(0.5);
    const int frameHeight = surfaceRect.height() - margins(ShadowsOnly).top()
            - margins(ShadowsOnly).bottom() + qreal(0.5);
    const QRectF fullFrameRect = QRectF(topLeft, QSizeF(frameWidth, frameHeight));

    // Maximized or tiled won't have rounded corners
    if (waylandWindow()->windowStates() & Qt::WindowMaximized
        || waylandWindow()->toplevelWindowTilingStates() != QWaylandWindow::WindowNoState) {
        path.addRect(fullFrameRect);
    } else {
        const QSizeF radiusRectSize = QSizeF(ceCornerRadius * 2, ceCornerRadius * 2);
        path.moveTo(fullFrameRect.bottomLeft());
        path.lineTo(fullFrameRect.topLeft() + QPointF(0, ceCornerRadius));
        path.arcTo(QRectF(fullFrameRect.topLeft(), radiusRectSize), 180, -90);
        path.lineTo(fullFrameRect.topRight() - QPointF(ceCornerRadius, 0));
        path.arcTo(QRectF(fullFrameRect.topRight() - QPointF(ceCornerRadius * 2, 0),
                          radiusRectSize),
                    90, -90);
        path.lineTo(fullFrameRect.bottomRight());
        path.closeSubpath();
    }

    p->save();
    p->setClipRect(clip);
    p->setPen(color(Border));
    p->fillPath(path.simplified(), color(Background));
    p->drawPath(path);
    p->restore();
}

void QWaylandAdwaitaDecoration::paintTop(QPainter *p)
{
    const QRect surfaceRect = this->surfaceRect();

    paintFrame(p, QRect(0, 0, surfaceRect.width(), margins().top()));

    /*
     * Window title
     */
    const QRect top = QRect(margins().left(), margins().bottom(), surfaceRect.width(),
                            margins().top() - margins().bottom());
    const QString windowTitleText = waylandWindow()->windowTitle();
    if (!windowTitleText.isEmpty()) {
        if (m_windowTitle.text() != windowTitleText) {
            m_windowTitle.setText(windowTitleText);
            m_windowTitle.prepare();
        }

        QRect titleBar = top;
        if (m_placement == Right) {
            titleBar.setLeft(margins().left());
            titleBar.setRight(static_cast<int>(buttonRect(Minimize).left()) - 8);
        } else {
            titleBar.setLeft(static_cast<int>(buttonRect(Minimize).right()) + 8);
            titleBar.setRight(surfaceRect.width() - margins().right());
        }

        p->save();
        p->setClipRect(titleBar);
        p->setPen(color(Foreground));
        QSize size = m_windowTitle.size().toSize();
        int dx = (top.width() - size.width()) / 2;
        int dy = (top.height() - size.height()) / 2;
        p->setFont(*m_font);
        QPoint windowTitlePoint(top.topLeft().x() + dx, top.topLeft().y() + dy);
        p->drawStaticText(windowTitlePoint, m_windowTitle);
        p->restore();
    }


    /*
     * Buttons
     */
    if (m_buttons.contains(Close))
        drawButton(Close, p);

    if (m_buttons.contains(Maximize))
        drawButton(Maximize, p);

    if (m_buttons.contains(Minimize))
        drawButton(Minimize, p);
}

void QWaylandAdwaitaDecoration::paintLeft(QPainter *p)
{
    const QRect surfaceRect = this->surfaceRect();
    const QMargins m = margins();
    paintFrame(p, QRect(0, m.top(), m.left(), surfaceRect.height() - m.top() - m.bottom()));
}

void QWaylandAdwaitaDecoration::paintRight(QPainter *p)
{
    const QRect surfaceRect = this->surfaceRect();
    const QMargins m = margins();
    paintFrame(p, QRect(surfaceRect.width() - m.right(), m.top(), m.right(),
                        surfaceRect.height() - m.top() - m.bottom()));
}

void QWaylandAdwaitaDecoration::paintBottom(QPainter *p)
{
    const QRect surfaceRect = this->surfaceRect();
    const QMargins m = margins();
    paintFrame(p, QRect(0, surfaceRect.height() - m.bottom(), surfaceRect.width(), m.bottom()));
}

#if QT_CONFIG(cursor)
// The decoration is painted on subsurfaces, so QWaylandWindow::applyCursor() would
// reject these since the pointer focus is not on the main surface.
static void applyCursor(QWaylandInputDevice *inputDevice, Qt::CursorShape shape)
{
    QCursor cursor(shape);
    inputDevice->setCursor(&cursor, {}, 1);
}
#endif

bool QWaylandAdwaitaDecoration::handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local,
                                            const QPointF &global, Qt::MouseButtons b,
                                            Qt::KeyboardModifiers mods)
{
    Q_UNUSED(global)

    if (local.y() > margins().top())
        updateButtonHoverState(Button::None);

    // Figure out what area mouse is in
    QRect surfaceRect = this->surfaceRect();
    if (local.y() <= surfaceRect.top() + margins().top())
        processMouseTop(inputDevice, local, b, mods);
    else if (local.y() > surfaceRect.bottom() - margins().bottom())
        processMouseBottom(inputDevice, local, b, mods);
    else if (local.x() <= surfaceRect.left() + margins().left())
        processMouseLeft(inputDevice, local, b, mods);
    else if (local.x() > surfaceRect.right() - margins().right())
        processMouseRight(inputDevice, local, b, mods);

    // Reset clicking state in case a button press is released outside
    // the button area
    if (isLeftReleased(b)) {
        m_clicking = None;
        requestRepaint();
    }

    setMouseButtons(b);
    return false;
}

bool QWaylandAdwaitaDecoration::handleTouch(QWaylandInputDevice *inputDevice, const QPointF &local,
                                            const QPointF &global, QEventPoint::State state,
                                            Qt::KeyboardModifiers mods)
{
    Q_UNUSED(inputDevice)
    Q_UNUSED(global)
    Q_UNUSED(mods)

    bool handled = state == QEventPoint::Pressed;

    if (handled) {
        if (buttonRect(Close).contains(local))
            QWindowSystemInterface::handleCloseEvent(window());
        else if (m_buttons.contains(Maximize) && buttonRect(Maximize).contains(local))
            window()->setWindowStates(window()->windowStates() ^ Qt::WindowMaximized);
        else if (m_buttons.contains(Minimize) && buttonRect(Minimize).contains(local))
            window()->setWindowState(Qt::WindowMinimized);
        else if (local.y() <= margins().top())
            waylandWindow()->shellSurface()->move(inputDevice);
        else
            handled = false;
    }

    return handled;
}

QString getIconSvg(const QString &iconName)
{
    const QStringList themeNames = { QIcon::themeName(), QIcon::fallbackThemeName(), "Adwaita"_L1 };

    qCDebug(lcQWaylandAdwaitaDecorationLog) << "Searched icon themes: " << themeNames;

    for (const QString &themeName : themeNames) {
        if (themeName.isEmpty())
            continue;

        for (const QString &path : QIcon::themeSearchPaths()) {
            if (path.startsWith(QLatin1Char(':')))
                continue;

            const QString fullPath = QString("%1/%2").arg(path).arg(themeName);
            QDirIterator dirIt(fullPath, {"*.svg"}, QDir::Files, QDirIterator::Subdirectories);
            while (dirIt.hasNext()) {
                const QString fileName = dirIt.next();
                const QFileInfo fileInfo(fileName);

                if (fileInfo.fileName() == iconName) {
                    qCDebug(lcQWaylandAdwaitaDecorationLog) << "Using " << iconName << " from " << themeName << " theme";
                    QFile readFile(fileInfo.filePath());
                    if (!readFile.open(QFile::ReadOnly)) {
                        qCWarning(lcQWaylandAdwaitaDecorationLog, "Failed to open %ls: %ls",
                                  qUtf16Printable(readFile.fileName()),
                                  qUtf16Printable(readFile.errorString()));
                    }
                    return readFile.readAll();
                }
            }
        }
    }

    qCWarning(lcQWaylandAdwaitaDecorationLog) << "Failed to find an svg icon for " << iconName;

    return QString();
}

void QWaylandAdwaitaDecoration::loadConfiguration()
{
    qRegisterMetaType<QDBusVariant>();
    qDBusRegisterMetaType<QMap<QString, QVariantMap>>();

    QDBusConnection connection = QDBusConnection::sessionBus();

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.portal.Desktop"_L1,
                                                          "/org/freedesktop/portal/desktop"_L1,
                                                          "org.freedesktop.portal.Settings"_L1,
                                                          "ReadAll"_L1);
    message << QStringList{ { "org.gnome.desktop.wm.preferences"_L1 },
                            { "org.freedesktop.appearance"_L1 } };

    QDBusPendingCall pendingCall = connection.asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QMap<QString, QVariantMap>> reply = *watcher;
        if (reply.isValid()) {
            QMap<QString, QVariantMap> settings = reply.value();
            if (!settings.isEmpty()) {
                const uint colorScheme = settings.value("org.freedesktop.appearance"_L1).value("color-scheme"_L1).toUInt();
                updateColors(colorScheme == 1); // 1 == Prefer Dark
                const QString buttonLayout = settings.value("org.gnome.desktop.wm.preferences"_L1).value("button-layout"_L1).toString();
                if (!buttonLayout.isEmpty())
                    updateTitlebarLayout(buttonLayout);
                // Do not rely on titlebar-font in case titlebar-uses-desktop-font is set to true
                // https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas/-/blob/main/schemas/org.gnome.desktop.wm.preferences.gschema.xml.in
                const bool titlebarUseDesktopFont = settings.value("org.gnome.desktop.wm.preferences"_L1).value("titlebar-uses-desktop-font"_L1).toBool();
                if (!titlebarUseDesktopFont) {
                    // Workaround for QGtkStyle not having correct titlebar font
                    const QString titlebarFont =
                        settings.value("org.gnome.desktop.wm.preferences"_L1).value("titlebar-font"_L1).toString();
                    if (titlebarFont.contains("bold"_L1, Qt::CaseInsensitive)) {
                        m_font->setBold(true);
                    }
                }
            }
        }
        watcher->deleteLater();
    });

    QDBusConnection::sessionBus().connect(QString(), "/org/freedesktop/portal/desktop"_L1,
                                          "org.freedesktop.portal.Settings"_L1, "SettingChanged"_L1, this,
                                          SLOT(settingChanged(QString,QString,QDBusVariant)));

    // Load SVG icons
    for (auto mapIt = buttonMap.constBegin(); mapIt != buttonMap.constEnd(); mapIt++) {
        const QString fullName = mapIt.value() + QStringLiteral(".svg");
        m_icons[mapIt.key()] = getIconSvg(fullName);
    }

    updateColors(false);
}

void QWaylandAdwaitaDecoration::updateColors(bool isDark)
{
    qCDebug(lcQWaylandAdwaitaDecorationLog) << "Color scheme changed to: " << (isDark ? "dark" : "light");

    m_colors = { { Background, isDark ? QColor(0x2e2e32) : QColor(0xffffff) },
                 { BackgroundInactive, isDark ? QColor(0x222226) : QColor(0xfafafb) },
                 { Foreground, isDark ? QColor(0xffffff) : QColor(0x333338) },
                 { ForegroundInactive, isDark ? QColor(0x919193) : QColor(0x969699) },
                 { Border, isDark ? QColor(0x3d3d40) : QColor(0xc8c8c8) },
                 { BorderInactive, isDark ? QColor(0x313135) : QColor(0xe2e2e2) },
                 { Shadow, isDark ? QColor(0x2e2e32) : QColor(0x2e2e32) },
                 { ButtonBackground, isDark ? QColor(0x434347) : QColor(0xebebeb) },
                 { ButtonBackgroundInactive, isDark ? QColor(0x2d2d31) : QColor(0xf0f0f1) },
                 { HoveredButtonBackground, isDark ? QColor(0x4d4d51) : QColor(0xe0e0e1) },
                 { PressedButtonBackground, isDark ? QColor(0x6c6c6f) : QColor(0xc2c2c3) } };
    requestRepaint();
}

void QWaylandAdwaitaDecoration::updateTitlebarLayout(const QString &layout)
{
    const QStringList layouts = layout.split(QLatin1Char(':'));
    if (layouts.count() != 2)
        return;

    // Remove previous configuration
    m_buttons.clear();

    const QString &leftLayout = layouts.at(0);
    const QString &rightLayout = layouts.at(1);
    m_placement = leftLayout.contains("close"_L1) ? Left : Right;

    int pos = 1;
    const QString &buttonLayout = m_placement == Right ? rightLayout : leftLayout;

    QStringList buttonList = buttonLayout.split(QLatin1Char(','));
    if (m_placement == Right)
        std::reverse(buttonList.begin(), buttonList.end());

    for (const QString &button : buttonList) {
        if (button == "close"_L1)
            m_buttons.insert(Close, pos);
        else if (button == "maximize"_L1)
            m_buttons.insert(Maximize, pos);
        else if (button == "minimize"_L1)
            m_buttons.insert(Minimize, pos);

        pos++;
    }

    qCDebug(lcQWaylandAdwaitaDecorationLog) << "Button layout changed to: " << layout;

    requestRepaint();
}

void QWaylandAdwaitaDecoration::settingChanged(const QString &group, const QString &key,
                                               const QDBusVariant &value)
{
    if (group == "org.gnome.desktop.wm.preferences"_L1 && key == "button-layout"_L1) {
        const QString layout = value.variant().toString();
        updateTitlebarLayout(layout);
    } else if (group == "org.freedesktop.appearance"_L1 && key == "color-scheme"_L1) {
        const uint colorScheme = value.variant().toUInt();
        updateColors(colorScheme == 1); // 1 == Prefer Dark
    }
}

QRectF QWaylandAdwaitaDecoration::buttonRect(Button button) const
{
    int xPos;
    int yPos;
    const int btnPos = m_buttons.value(button);

    const QRect surfaceRect = this->surfaceRect();
    if (m_placement == Right) {
        xPos = surfaceRect.width();
        xPos -= ceButtonWidth * btnPos;
        xPos -= ceButtonSpacing * btnPos;
        xPos -= margins(ShadowsOnly).right();
    } else {
        xPos = 0;
        xPos += ceButtonWidth * btnPos;
        xPos += ceButtonSpacing * btnPos;
        xPos += margins(ShadowsOnly).left();
        // We are painting from the left to the right so the real
        // position doesn't need to by moved by the size of the button.
        xPos -= ceButtonWidth;
    }

    yPos = margins().top();
    yPos += margins().bottom();
    yPos -= ceButtonWidth;
    yPos /= 2;

    return QRectF(xPos, yPos, ceButtonWidth, ceButtonWidth);
}

static void renderFlatRoundedButtonFrame(QPainter *painter, const QRect &rect, const QColor &color)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawEllipse(rect);
    painter->restore();
}

static void renderButtonIcon(const QString &svgIcon, QPainter *painter, const QRect &rect, const QColor &color)
{
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing, true);

    QString icon = svgIcon;
    QRegularExpression regexp("fill=[\"']#[0-9A-F]{6}[\"']", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression regexpAlt("fill:#[0-9A-F]{6}", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression regexpCurrentColor("fill=[\"']currentColor[\"']");
    QRegularExpression regexpCurrentColorCss("fill:currentColor",
                                             QRegularExpression::CaseInsensitiveOption);
    icon.replace(regexp, QString("fill=\"%1\"").arg(color.name()));
    icon.replace(regexpAlt, QString("fill:%1").arg(color.name()));
    icon.replace(regexpCurrentColor, QString("fill=\"%1\"").arg(color.name()));
    icon.replace(regexpCurrentColorCss, QString("fill:%1").arg(color.name()));
    QSvgRenderer svgRenderer(icon.toLocal8Bit());
    svgRenderer.render(painter, rect);

    painter->restore();
}

static void renderButtonIcon(QWaylandAdwaitaDecoration::ButtonIcon buttonIcon, QPainter *painter, const QRect &rect)
{
    QString iconName = buttonMap[buttonIcon];

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing, true);
    painter->drawPixmap(rect, QIcon::fromTheme(iconName).pixmap(ceButtonWidth, ceButtonWidth));
    painter->restore();
}

static QWaylandAdwaitaDecoration::ButtonIcon iconFromButtonAndState(QWaylandAdwaitaDecoration::Button button, bool maximized)
{
    if (button == QWaylandAdwaitaDecoration::Close)
        return QWaylandAdwaitaDecoration::CloseIcon;
    else if (button == QWaylandAdwaitaDecoration::Minimize)
        return QWaylandAdwaitaDecoration::MinimizeIcon;
    else if (button == QWaylandAdwaitaDecoration::Maximize && maximized)
        return QWaylandAdwaitaDecoration::RestoreIcon;
    else
        return QWaylandAdwaitaDecoration::MaximizeIcon;
}

void QWaylandAdwaitaDecoration::drawButton(Button button, QPainter *painter)
{
    const Qt::WindowStates windowStates = waylandWindow()->windowStates();
    const bool maximized = windowStates & Qt::WindowMaximized;

    const QRect btnRect = buttonRect(button).toRect();
    renderFlatRoundedButtonFrame(painter, btnRect, color(ButtonBackground, button));

    QRect adjustedBtnRect = btnRect;
    adjustedBtnRect.setSize(QSize(16, 16));
    adjustedBtnRect.translate(4, 4);
    const QString svgIcon = m_icons[iconFromButtonAndState(button, maximized)];
    if (!svgIcon.isEmpty())
        renderButtonIcon(svgIcon, painter, adjustedBtnRect, color(Foreground));
    else // Fallback to use QIcon
        renderButtonIcon(iconFromButtonAndState(button, maximized), painter, adjustedBtnRect);
}

QColor QWaylandAdwaitaDecoration::color(ColorType type, Button button)
{
    const bool active = waylandWindow()->windowStates() & Qt::WindowActive;

    switch (type) {
    case Background:
    case BackgroundInactive:
        return active ? m_colors[Background] : m_colors[BackgroundInactive];
    case Foreground:
    case ForegroundInactive:
        return active ? m_colors[Foreground] : m_colors[ForegroundInactive];
    case Border:
    case BorderInactive:
        return active ? m_colors[Border] : m_colors[BorderInactive];
    case ButtonBackground:
    case ButtonBackgroundInactive:
    case HoveredButtonBackground: {
        if (m_clicking == button) {
            return m_colors[PressedButtonBackground];
        } else if (m_hoveredButtons.testFlag(button)) {
            return m_colors[HoveredButtonBackground];
        }
        return active ? m_colors[ButtonBackground] : m_colors[ButtonBackgroundInactive];
    }
    default:
        return m_colors[Background];
    }
}

bool QWaylandAdwaitaDecoration::clickButton(Qt::MouseButtons b, Button btn)
{
    auto repaint = qScopeGuard([this] { requestRepaint(); });

    if (isLeftClicked(b)) {
        m_clicking = btn;
        return false;
    } else if (isLeftReleased(b)) {
        if (m_clicking == btn) {
            m_clicking = None;
            return true;
        } else {
            m_clicking = None;
        }
    }
    return false;
}

bool QWaylandAdwaitaDecoration::doubleClickButton(Qt::MouseButtons b, const QPointF &local,
                                                  const QDateTime &currentTime)
{
    if (isLeftClicked(b)) {
        const qint64 clickInterval = m_lastButtonClick.msecsTo(currentTime);
        m_lastButtonClick = currentTime;
        const int doubleClickDistance = 5;
        const QPointF posDiff = m_lastButtonClickPosition - local;
        if ((clickInterval <= 500)
            && ((posDiff.x() <= doubleClickDistance && posDiff.x() >= -doubleClickDistance)
                && ((posDiff.y() <= doubleClickDistance && posDiff.y() >= -doubleClickDistance)))) {
            return true;
        }

        m_lastButtonClickPosition = local;
    }

    return false;
}

void QWaylandAdwaitaDecoration::updateButtonHoverState(Button hoveredButton)
{
    bool currentCloseButtonState = m_hoveredButtons.testFlag(Close);
    bool currentMaximizeButtonState = m_hoveredButtons.testFlag(Maximize);
    bool currentMinimizeButtonState = m_hoveredButtons.testFlag(Minimize);

    m_hoveredButtons.setFlag(Close, hoveredButton == Button::Close);
    m_hoveredButtons.setFlag(Maximize, hoveredButton == Button::Maximize);
    m_hoveredButtons.setFlag(Minimize, hoveredButton == Button::Minimize);

    if (m_hoveredButtons.testFlag(Close) != currentCloseButtonState
        || m_hoveredButtons.testFlag(Maximize) != currentMaximizeButtonState
        || m_hoveredButtons.testFlag(Minimize) != currentMinimizeButtonState) {
        requestRepaint();
    }
}

void QWaylandAdwaitaDecoration::processMouseTop(QWaylandInputDevice *inputDevice, const QPointF &local,
                                                Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(mods)

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QRect surfaceRect = this->surfaceRect();

    if (!buttonRect(Close).contains(local) && !buttonRect(Maximize).contains(local)
        && !buttonRect(Minimize).contains(local))
        updateButtonHoverState(Button::None);

    if (local.y() <= surfaceRect.top() + margins().bottom()) {
        if (local.x() <= margins().left()) {
            // top left bit
#if QT_CONFIG(cursor)
            applyCursor(inputDevice, Qt::SizeFDiagCursor);
#endif
            startResize(inputDevice, Qt::TopEdge | Qt::LeftEdge, b);
        } else if (local.x() > surfaceRect.right() - margins().left()) {
            // top right bit
#if QT_CONFIG(cursor)
            applyCursor(inputDevice, Qt::SizeBDiagCursor);
#endif
            startResize(inputDevice, Qt::TopEdge | Qt::RightEdge, b);
        } else {
            // top resize bit
#if QT_CONFIG(cursor)
            applyCursor(inputDevice, Qt::SizeVerCursor);
#endif
            startResize(inputDevice, Qt::TopEdge, b);
        }
    } else if (local.x() <= surfaceRect.left() + margins().left()) {
        processMouseLeft(inputDevice, local, b, mods);
    } else if (local.x() > surfaceRect.right() - margins().right()) {
        processMouseRight(inputDevice, local, b, mods);
    } else if (buttonRect(Close).contains(local)) {
        if (clickButton(b, Close)) {
            QWindowSystemInterface::handleCloseEvent(window());
            m_hoveredButtons.setFlag(Close, false);
        }
        updateButtonHoverState(Close);
    } else if (m_buttons.contains(Maximize) && buttonRect(Maximize).contains(local)) {
        updateButtonHoverState(Maximize);
        if (clickButton(b, Maximize)) {
            window()->setWindowStates(window()->windowStates() ^ Qt::WindowMaximized);
            m_hoveredButtons.setFlag(Maximize, false);
        }
    } else if (m_buttons.contains(Minimize) && buttonRect(Minimize).contains(local)) {
        updateButtonHoverState(Minimize);
        if (clickButton(b, Minimize)) {
            window()->setWindowState(Qt::WindowMinimized);
            m_hoveredButtons.setFlag(Minimize, false);
        }
    } else if (doubleClickButton(b, local, currentDateTime)) {
        window()->setWindowStates(window()->windowStates() ^ Qt::WindowMaximized);
    } else {
        // Show window menu
        if (b == Qt::MouseButton::RightButton)
            waylandWindow()->shellSurface()->showWindowMenu(inputDevice);
#if QT_CONFIG(cursor)
        applyCursor(inputDevice, Qt::ArrowCursor);
#endif
        startMove(inputDevice, b);
    }
}

void QWaylandAdwaitaDecoration::processMouseBottom(QWaylandInputDevice *inputDevice, const QPointF &local,
                                                   Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(mods)
    if (local.x() <= margins().left()) {
        // bottom left bit
#if QT_CONFIG(cursor)
        applyCursor(inputDevice, Qt::SizeBDiagCursor);
#endif
        startResize(inputDevice, Qt::BottomEdge | Qt::LeftEdge, b);
    } else if (local.x() > window()->width() + margins().right()) {
        // bottom right bit
#if QT_CONFIG(cursor)
        applyCursor(inputDevice, Qt::SizeFDiagCursor);
#endif
        startResize(inputDevice, Qt::BottomEdge | Qt::RightEdge, b);
    } else {
        // bottom bit
#if QT_CONFIG(cursor)
        applyCursor(inputDevice, Qt::SizeVerCursor);
#endif
        startResize(inputDevice, Qt::BottomEdge, b);
    }
}

void QWaylandAdwaitaDecoration::processMouseLeft(QWaylandInputDevice *inputDevice, const QPointF &local,
                                                 Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(local)
    Q_UNUSED(mods)
#if QT_CONFIG(cursor)
    applyCursor(inputDevice, Qt::SizeHorCursor);
#endif
    startResize(inputDevice, Qt::LeftEdge, b);
}

void QWaylandAdwaitaDecoration::processMouseRight(QWaylandInputDevice *inputDevice, const QPointF &local,
                                                  Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(local)
    Q_UNUSED(mods)
#if QT_CONFIG(cursor)
    applyCursor(inputDevice, Qt::SizeHorCursor);
#endif
    startResize(inputDevice, Qt::RightEdge, b);
}

void QWaylandAdwaitaDecoration::requestRepaint() const
{
    // Set dirty flag
    if (waylandWindow()->decoration())
        waylandWindow()->decoration()->update();

    // Request re-paint
    waylandWindow()->window()->requestUpdate();
}

} // namespace QtWaylandClient

QT_END_NAMESPACE

#include "moc_qwaylandadwaitadecoration_p.cpp"

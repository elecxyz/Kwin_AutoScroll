// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "autoscrollinputdevice.h"

namespace AutoScroll {

AutoScrollInputDevice::AutoScrollInputDevice(QObject *parent)
    : KWin::InputDevice(parent) {}

QString AutoScrollInputDevice::name() const {
  return QStringLiteral("KWin AutoScroll virtual pointer");
}

bool AutoScrollInputDevice::isEnabled() const { return m_enabled; }

void AutoScrollInputDevice::setEnabled(bool enabled) { m_enabled = enabled; }

bool AutoScrollInputDevice::isKeyboard() const { return false; }

bool AutoScrollInputDevice::isPointer() const { return true; }

bool AutoScrollInputDevice::isTouchpad() const { return false; }

bool AutoScrollInputDevice::isTouch() const { return false; }

bool AutoScrollInputDevice::isTabletTool() const { return false; }

bool AutoScrollInputDevice::isTabletPad() const { return false; }

bool AutoScrollInputDevice::isTabletModeSwitch() const { return false; }

bool AutoScrollInputDevice::isLidSwitch() const { return false; }

} // namespace AutoScroll

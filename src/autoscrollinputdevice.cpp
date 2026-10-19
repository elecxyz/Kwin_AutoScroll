// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "autoscrollinputdevice.h"

namespace AutoScroll {
namespace {

template <typename Device>
void emitScroll(Device *device, KWin::PointerAxis axis, qreal delta,
                std::chrono::microseconds timestamp) {
  // KWin 6.8 adds an autoscroll flag. Detect the signature so older releases
  // and distribution backports use the API provided by their own headers.
  if constexpr (requires {
                  device->pointerAxisChanged(
                      axis, delta, 0, KWin::PointerAxisSource::Continuous,
                      false, true, timestamp, device);
                }) {
    Q_EMIT device->pointerAxisChanged(
        axis, delta, 0, KWin::PointerAxisSource::Continuous, false, true,
        timestamp, device);
  } else {
    Q_EMIT device->pointerAxisChanged(
        axis, delta, 0, KWin::PointerAxisSource::Continuous, false, timestamp,
        device);
  }
}

} // namespace

AutoScrollInputDevice::AutoScrollInputDevice(QObject *parent)
    : KWin::InputDevice(parent) {}

void AutoScrollInputDevice::scroll(KWin::PointerAxis axis, qreal delta,
                                  std::chrono::microseconds timestamp) {
  emitScroll(this, axis, delta, timestamp);
}

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

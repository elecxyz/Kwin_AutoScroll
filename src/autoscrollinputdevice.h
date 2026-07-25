// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <core/inputdevice.h>

namespace AutoScroll {

class AutoScrollInputDevice : public KWin::InputDevice {
  Q_OBJECT

public:
  explicit AutoScrollInputDevice(QObject *parent = nullptr);

  QString name() const override;
  bool isEnabled() const override;
  void setEnabled(bool enabled) override;

  bool isKeyboard() const override;
  bool isPointer() const override;
  bool isTouchpad() const override;
  bool isTouch() const override;
  bool isTabletTool() const override;
  bool isTabletPad() const override;
  bool isTabletModeSwitch() const override;
  bool isLidSwitch() const override;

private:
  bool m_enabled = true;
};

} // namespace AutoScroll

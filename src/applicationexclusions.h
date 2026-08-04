// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QString>
#include <QStringList>
#include <QStringView>

namespace AutoScroll {

struct ApplicationIdentity {
  QString desktopFileId;
  QString resourceClass;
};

QString normalizedDesktopFileId(QStringView desktopFileId);
QString normalizedResourceClass(QStringView resourceClass);
QString desktopApplicationExclusion(QStringView desktopFileId);
QString classApplicationExclusion(QStringView resourceClass);
QString normalizedApplicationExclusion(QStringView exclusion);
QStringList normalizedApplicationExclusions(const QStringList &exclusions);
QString applicationExclusionForIdentity(const ApplicationIdentity &identity);
bool isApplicationExcluded(const ApplicationIdentity &identity,
                           const QStringList &exclusions);

} // namespace AutoScroll

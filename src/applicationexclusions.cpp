// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "applicationexclusions.h"

#include <QRegularExpression>
#include <QSet>

namespace AutoScroll {
namespace {
const QRegularExpression DesktopFileIdPattern(
    QStringLiteral("^[A-Za-z0-9][A-Za-z0-9._-]*$"));

bool containsControlCharacter(QStringView value) {
  for (const QChar character : value) {
    if (character.isNull() || character.category() == QChar::Other_Control) {
      return true;
    }
  }
  return false;
}
} // namespace

QString normalizedDesktopFileId(QStringView desktopFileId) {
  QString normalized = desktopFileId.trimmed().toString();
  if (normalized.endsWith(QLatin1StringView(".desktop"),
                          Qt::CaseInsensitive)) {
    normalized.chop(8);
  }
  if (normalized.size() > 255 ||
      !DesktopFileIdPattern.match(normalized).hasMatch()) {
    return {};
  }
  return normalized;
}

QString normalizedResourceClass(QStringView resourceClass) {
  const QString normalized = resourceClass.trimmed().toString().toCaseFolded();
  if (normalized.isEmpty() || normalized.size() > 255 ||
      containsControlCharacter(normalized) || normalized.contains(u':')) {
    return {};
  }
  return normalized;
}

QString desktopApplicationExclusion(QStringView desktopFileId) {
  const QString normalized = normalizedDesktopFileId(desktopFileId);
  return normalized.isEmpty() ? QString()
                              : QStringLiteral("desktop:") + normalized;
}

QString classApplicationExclusion(QStringView resourceClass) {
  const QString normalized = normalizedResourceClass(resourceClass);
  return normalized.isEmpty() ? QString()
                              : QStringLiteral("class:") + normalized;
}

QString normalizedApplicationExclusion(QStringView exclusion) {
  const QString normalized = exclusion.trimmed().toString();
  const qsizetype separator = normalized.indexOf(u':');
  if (separator <= 0) {
    return {};
  }

  const QString prefix = normalized.first(separator).toLower();
  const QStringView value(normalized.constData() + separator + 1,
                          normalized.size() - separator - 1);
  if (prefix == QLatin1StringView("desktop")) {
    return desktopApplicationExclusion(value);
  }
  if (prefix == QLatin1StringView("class")) {
    return classApplicationExclusion(value);
  }
  return {};
}

QStringList normalizedApplicationExclusions(const QStringList &exclusions) {
  QStringList result;
  QSet<QString> seen;
  for (const QString &exclusion : exclusions) {
    const QString normalized = normalizedApplicationExclusion(exclusion);
    if (!normalized.isEmpty() && !seen.contains(normalized)) {
      seen.insert(normalized);
      result.append(normalized);
    }
  }
  return result;
}

QString applicationExclusionForIdentity(const ApplicationIdentity &identity) {
  const QString desktop = desktopApplicationExclusion(identity.desktopFileId);
  return desktop.isEmpty() ? classApplicationExclusion(identity.resourceClass)
                           : desktop;
}

bool isApplicationExcluded(const ApplicationIdentity &identity,
                           const QStringList &exclusions) {
  const QString candidate = applicationExclusionForIdentity(identity);
  return !candidate.isEmpty() &&
         normalizedApplicationExclusions(exclusions).contains(candidate);
}

} // namespace AutoScroll

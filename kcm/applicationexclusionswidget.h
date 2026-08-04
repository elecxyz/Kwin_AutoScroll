// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QIcon>
#include <QStringList>
#include <QVariantMap>
#include <QWidget>

class QListWidget;
class QPushButton;

namespace AutoScroll {

struct InstalledApplication {
  QString id;
  QString name;
  QString iconName;
};

QList<InstalledApplication>
discoverInstalledApplications(const QStringList &applicationLocations);
QString exclusionFromWindowInfo(const QVariantMap &windowInfo);

class ApplicationExclusionsWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QStringList excludedApplications READ excludedApplications WRITE
                 setExcludedApplications NOTIFY excludedApplicationsChanged USER
                     true)

public:
  explicit ApplicationExclusionsWidget(QWidget *parent = nullptr);

  QStringList excludedApplications() const;
  void setExcludedApplications(const QStringList &exclusions);
  bool addExclusion(const QString &exclusion);

Q_SIGNALS:
  void excludedApplicationsChanged(const QStringList &exclusions);

private:
  void selectRunningWindow();
  void chooseInstalledApplications();
  void removeSelectedApplications();
  void setPickerEnabled(bool enabled);
  void rebuildList();
  InstalledApplication presentationFor(const QString &exclusion) const;

  QStringList m_exclusions;
  QList<InstalledApplication> m_installedApplications;
  QListWidget *m_list = nullptr;
  QPushButton *m_selectWindowButton = nullptr;
  QPushButton *m_chooseApplicationButton = nullptr;
  QPushButton *m_removeButton = nullptr;
};

} // namespace AutoScroll

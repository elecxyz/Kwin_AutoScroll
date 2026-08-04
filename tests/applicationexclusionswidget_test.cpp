// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "applicationexclusionswidget.h"

#include <QDir>
#include <QFile>
#include <QListWidget>
#include <QMetaProperty>
#include <QPushButton>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include <algorithm>

using namespace AutoScroll;

namespace {
void writeDesktopFile(const QString &path, const QString &name,
                      bool noDisplay = false) {
  QFile file(path);
  QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
  const QByteArray contents =
      QByteArrayLiteral("[Desktop Entry]\nType=Application\nExec=true\nName=") +
      name.toUtf8() + QByteArrayLiteral("\nIcon=application-x-executable\n") +
      (noDisplay ? QByteArrayLiteral("NoDisplay=true\n") : QByteArray());
  QCOMPARE(file.write(contents), contents.size());
}
} // namespace

class ApplicationExclusionsWidgetTest : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void exposesKConfigUserProperty();
  void normalizesAddsAndRemovesEntries();
  void parsesWindowPickerResults();
  void discoversVisibleApplicationsWithXdgPrecedence();
};

void ApplicationExclusionsWidgetTest::exposesKConfigUserProperty() {
  ApplicationExclusionsWidget widget;
  QCOMPARE(QString::fromLatin1(widget.metaObject()->userProperty().name()),
           QStringLiteral("excludedApplications"));
  QVERIFY(widget.excludedApplications().isEmpty());
}

void ApplicationExclusionsWidgetTest::normalizesAddsAndRemovesEntries() {
  ApplicationExclusionsWidget widget;
  QSignalSpy changed(&widget,
                     &ApplicationExclusionsWidget::excludedApplicationsChanged);
  widget.setExcludedApplications({
      QStringLiteral("desktop:org.kde.kate.desktop"),
      QStringLiteral("class: Steam "),
      QStringLiteral("not-valid"),
  });
  QCOMPARE(widget.excludedApplications(),
           QStringList({QStringLiteral("desktop:org.kde.kate"),
                        QStringLiteral("class:steam")}));
  QVERIFY(!widget.addExclusion(QStringLiteral("class:STEAM")));
  QVERIFY(widget.addExclusion(QStringLiteral("desktop:org.kde.okular")));

  auto *list =
      widget.findChild<QListWidget *>(QStringLiteral("excludedApplicationsList"));
  auto *removeButton = widget.findChild<QPushButton *>(
      QStringLiteral("removeApplicationButton"));
  QVERIFY(list);
  QVERIFY(removeButton);
  QCOMPARE(list->count(), 3);
  list->item(1)->setSelected(true);
  QVERIFY(removeButton->isEnabled());
  removeButton->click();
  QCOMPARE(widget.excludedApplications(),
           QStringList({QStringLiteral("desktop:org.kde.kate"),
                        QStringLiteral("desktop:org.kde.okular")}));
  QVERIFY(changed.count() >= 3);
}

void ApplicationExclusionsWidgetTest::parsesWindowPickerResults() {
  QCOMPARE(exclusionFromWindowInfo(
               {{QStringLiteral("desktopFile"),
                 QStringLiteral("org.mozilla.firefox.desktop")},
                {QStringLiteral("resourceClass"), QStringLiteral("Firefox")}}),
           QStringLiteral("desktop:org.mozilla.firefox"));
  QCOMPARE(exclusionFromWindowInfo(
               {{QStringLiteral("resourceClass"), QStringLiteral("GameScope")}}),
           QStringLiteral("class:gamescope"));
  QVERIFY(exclusionFromWindowInfo({}).isEmpty());
}

void ApplicationExclusionsWidgetTest::
    discoversVisibleApplicationsWithXdgPrecedence() {
  QTemporaryDir userDirectory;
  QTemporaryDir systemDirectory;
  QVERIFY(userDirectory.isValid());
  QVERIFY(systemDirectory.isValid());
  QVERIFY(QDir(userDirectory.path()).mkpath(QStringLiteral("vendor")));

  writeDesktopFile(userDirectory.filePath(QStringLiteral("shared.desktop")),
                   QStringLiteral("User Shared"));
  writeDesktopFile(
      systemDirectory.filePath(QStringLiteral("shared.desktop")),
      QStringLiteral("System Shared"));
  writeDesktopFile(
      userDirectory.filePath(QStringLiteral("vendor/tool.desktop")),
      QStringLiteral("Vendor Tool"));
  writeDesktopFile(userDirectory.filePath(QStringLiteral("hidden.desktop")),
                   QStringLiteral("Hidden"), true);
  writeDesktopFile(userDirectory.filePath(QStringLiteral("masked.desktop")),
                   QStringLiteral("Masked User Entry"), true);
  writeDesktopFile(systemDirectory.filePath(QStringLiteral("masked.desktop")),
                   QStringLiteral("Masked System Entry"));

  const QList<InstalledApplication> applications =
      discoverInstalledApplications(
          {userDirectory.path(), systemDirectory.path()});
  QCOMPARE(applications.size(), 2);

  const auto shared = std::find_if(
      applications.cbegin(), applications.cend(),
      [](const InstalledApplication &application) {
        return application.id == QLatin1StringView("shared");
      });
  QVERIFY(shared != applications.cend());
  QCOMPARE(shared->name, QStringLiteral("User Shared"));

  const auto nested = std::find_if(
      applications.cbegin(), applications.cend(),
      [](const InstalledApplication &application) {
        return application.id == QLatin1StringView("vendor-tool");
      });
  QVERIFY(nested != applications.cend());
}

QTEST_MAIN(ApplicationExclusionsWidgetTest)

#include "applicationexclusionswidget_test.moc"

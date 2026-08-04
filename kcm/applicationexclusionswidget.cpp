// SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "applicationexclusionswidget.h"

#include "applicationexclusions.h"

#include <KConfigGroup>
#include <KDesktopFile>
#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>
#include <QStandardPaths>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace AutoScroll {
namespace {
constexpr int ApplicationIdRole = Qt::UserRole;

QString desktopFileId(const QDir &root, const QString &filePath) {
  QString relativePath = root.relativeFilePath(filePath);
  if (!relativePath.endsWith(QLatin1StringView(".desktop"),
                             Qt::CaseInsensitive)) {
    return {};
  }
  relativePath.chop(8);
  relativePath.replace(u'/', u'-');
  relativePath.replace(u'\\', u'-');
  return normalizedDesktopFileId(relativePath);
}

QList<InstalledApplication> systemInstalledApplications() {
  return discoverInstalledApplications(
      QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation));
}
} // namespace

QList<InstalledApplication>
discoverInstalledApplications(const QStringList &applicationLocations) {
  QList<InstalledApplication> applications;
  QSet<QString> seenIds;

  for (const QString &location : applicationLocations) {
    const QDir root(location);
    if (!root.exists()) {
      continue;
    }

    QStringList paths;
    QDirIterator iterator(location, {QStringLiteral("*.desktop")}, QDir::Files,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
      paths.append(iterator.next());
    }
    std::sort(paths.begin(), paths.end());

    for (const QString &path : std::as_const(paths)) {
      const QString id = desktopFileId(root, path);
      if (id.isEmpty() || seenIds.contains(id)) {
        continue;
      }
      seenIds.insert(id);

      KDesktopFile desktopFile(path);
      const KConfigGroup group = desktopFile.desktopGroup();
      if (!desktopFile.hasApplicationType() || desktopFile.noDisplay() ||
          group.readEntry("Hidden", false) || !desktopFile.tryExec()) {
        continue;
      }

      const QString name = desktopFile.readName().trimmed();
      if (name.isEmpty()) {
        continue;
      }

      applications.append({
          .id = id,
          .name = name,
          .iconName = desktopFile.readIcon(),
      });
    }
  }

  std::sort(applications.begin(), applications.end(),
            [](const InstalledApplication &left,
               const InstalledApplication &right) {
              const int nameComparison =
                  QString::localeAwareCompare(left.name, right.name);
              return nameComparison == 0 ? left.id < right.id
                                         : nameComparison < 0;
            });
  return applications;
}

QString exclusionFromWindowInfo(const QVariantMap &windowInfo) {
  const QString desktop =
      desktopApplicationExclusion(windowInfo.value(QStringLiteral("desktopFile"))
                                      .toString());
  if (!desktop.isEmpty()) {
    return desktop;
  }
  return classApplicationExclusion(
      windowInfo.value(QStringLiteral("resourceClass")).toString());
}

ApplicationExclusionsWidget::ApplicationExclusionsWidget(QWidget *parent)
    : QWidget(parent), m_installedApplications(systemInstalledApplications()) {
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  m_list = new QListWidget(this);
  m_list->setObjectName(QStringLiteral("excludedApplicationsList"));
  m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_list->setMinimumHeight(104);
  m_list->setAccessibleName(i18nc("@label", "Excluded applications"));
  layout->addWidget(m_list);

  auto *buttonLayout = new QHBoxLayout;
  m_selectWindowButton =
      new QPushButton(i18nc("@action:button", "Select Running Window…"), this);
  m_selectWindowButton->setObjectName(QStringLiteral("selectWindowButton"));
  m_chooseApplicationButton = new QPushButton(
      i18nc("@action:button", "Choose Installed Application…"), this);
  m_chooseApplicationButton->setObjectName(
      QStringLiteral("chooseApplicationButton"));
  m_removeButton =
      new QPushButton(i18nc("@action:button", "Remove"), this);
  m_removeButton->setObjectName(QStringLiteral("removeApplicationButton"));
  m_removeButton->setEnabled(false);

  buttonLayout->addWidget(m_selectWindowButton);
  buttonLayout->addWidget(m_chooseApplicationButton);
  buttonLayout->addStretch();
  buttonLayout->addWidget(m_removeButton);
  layout->addLayout(buttonLayout);

  connect(m_selectWindowButton, &QPushButton::clicked, this,
          &ApplicationExclusionsWidget::selectRunningWindow);
  connect(m_chooseApplicationButton, &QPushButton::clicked, this,
          &ApplicationExclusionsWidget::chooseInstalledApplications);
  connect(m_removeButton, &QPushButton::clicked, this,
          &ApplicationExclusionsWidget::removeSelectedApplications);
  connect(m_list, &QListWidget::itemSelectionChanged, this, [this]() {
    m_removeButton->setEnabled(!m_list->selectedItems().isEmpty());
  });
}

QStringList ApplicationExclusionsWidget::excludedApplications() const {
  return m_exclusions;
}

void ApplicationExclusionsWidget::setExcludedApplications(
    const QStringList &exclusions) {
  const QStringList normalized = normalizedApplicationExclusions(exclusions);
  if (m_exclusions == normalized) {
    return;
  }
  m_exclusions = normalized;
  rebuildList();
  Q_EMIT excludedApplicationsChanged(m_exclusions);
}

bool ApplicationExclusionsWidget::addExclusion(const QString &exclusion) {
  const QString normalized = normalizedApplicationExclusion(exclusion);
  if (normalized.isEmpty() || m_exclusions.contains(normalized)) {
    return false;
  }
  m_exclusions.append(normalized);
  rebuildList();
  Q_EMIT excludedApplicationsChanged(m_exclusions);
  return true;
}

void ApplicationExclusionsWidget::selectRunningWindow() {
  QDBusMessage message = QDBusMessage::createMethodCall(
      QStringLiteral("org.kde.KWin"), QStringLiteral("/KWin"),
      QStringLiteral("org.kde.KWin"), QStringLiteral("queryWindowInfo"));
  auto *watcher = new QDBusPendingCallWatcher(
      QDBusConnection::sessionBus().asyncCall(message, 120000), this);
  setPickerEnabled(false);

  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          [this, watcher]() {
            const QDBusPendingReply<QVariantMap> reply = *watcher;
            watcher->deleteLater();
            setPickerEnabled(true);

            if (reply.isError()) {
              if (reply.error().name() !=
                  QLatin1StringView("org.kde.KWin.Error.UserCancel")) {
                QMessageBox::warning(
                    this, i18nc("@title:window", "Unable to Select Window"),
                    i18nc("@info", "KWin could not provide information for "
                                     "the selected window: %1",
                          reply.error().message()));
              }
              return;
            }

            const QString exclusion = exclusionFromWindowInfo(reply.value());
            if (exclusion.isEmpty()) {
              QMessageBox::warning(
                  this, i18nc("@title:window", "Unable to Identify Application"),
                  i18nc("@info", "The selected window does not expose a stable "
                                   "application identifier."));
              return;
            }
            addExclusion(exclusion);
          });
}

void ApplicationExclusionsWidget::chooseInstalledApplications() {
  m_installedApplications = systemInstalledApplications();
  QDialog dialog(this);
  dialog.setWindowTitle(i18nc("@title:window", "Choose Applications"));
  dialog.resize(520, 440);

  auto *layout = new QVBoxLayout(&dialog);
  auto *search = new QLineEdit(&dialog);
  search->setPlaceholderText(i18nc("@info:placeholder", "Search applications…"));
  search->setClearButtonEnabled(true);
  layout->addWidget(search);

  auto *applications = new QListWidget(&dialog);
  applications->setSelectionMode(QAbstractItemView::ExtendedSelection);
  for (const InstalledApplication &application :
       std::as_const(m_installedApplications)) {
    auto *item = new QListWidgetItem(
        QIcon::fromTheme(application.iconName),
        i18nc("application name and identifier", "%1 — %2", application.name,
              application.id),
        applications);
    item->setData(ApplicationIdRole, application.id);
  }
  layout->addWidget(applications);

  auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, &dialog);
  QPushButton *addButton =
      buttons->addButton(i18nc("@action:button", "Add"),
                         QDialogButtonBox::AcceptRole);
  addButton->setEnabled(false);
  layout->addWidget(buttons);

  connect(search, &QLineEdit::textChanged, applications,
          [applications](const QString &text) {
            for (int index = 0; index < applications->count(); ++index) {
              QListWidgetItem *item = applications->item(index);
              const bool matches =
                  text.isEmpty() || item->text().contains(text, Qt::CaseInsensitive);
              item->setHidden(!matches);
              if (!matches) {
                item->setSelected(false);
              }
            }
          });
  connect(applications, &QListWidget::itemSelectionChanged, addButton,
          [applications, addButton]() {
            addButton->setEnabled(!applications->selectedItems().isEmpty());
          });
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  connect(applications, &QListWidget::itemDoubleClicked, &dialog,
          [&dialog](QListWidgetItem *) { dialog.accept(); });

  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  for (QListWidgetItem *item : applications->selectedItems()) {
    addExclusion(desktopApplicationExclusion(
        item->data(ApplicationIdRole).toString()));
  }
}

void ApplicationExclusionsWidget::removeSelectedApplications() {
  QStringList removed;
  for (QListWidgetItem *item : m_list->selectedItems()) {
    removed.append(item->data(ApplicationIdRole).toString());
  }
  if (removed.isEmpty()) {
    return;
  }
  for (const QString &exclusion : std::as_const(removed)) {
    m_exclusions.removeAll(exclusion);
  }
  rebuildList();
  Q_EMIT excludedApplicationsChanged(m_exclusions);
}

void ApplicationExclusionsWidget::setPickerEnabled(bool enabled) {
  m_selectWindowButton->setEnabled(enabled);
  m_chooseApplicationButton->setEnabled(enabled);
}

void ApplicationExclusionsWidget::rebuildList() {
  m_list->clear();
  for (const QString &exclusion : std::as_const(m_exclusions)) {
    const InstalledApplication presentation = presentationFor(exclusion);
    auto *item = new QListWidgetItem(
        QIcon::fromTheme(presentation.iconName),
        i18nc("excluded application name and identifier", "%1 — %2",
              presentation.name, exclusion),
        m_list);
    item->setData(ApplicationIdRole, exclusion);
  }
  m_removeButton->setEnabled(false);
}

InstalledApplication
ApplicationExclusionsWidget::presentationFor(const QString &exclusion) const {
  if (exclusion.startsWith(QLatin1StringView("desktop:"))) {
    const QString id = exclusion.sliced(8);
    const auto found = std::find_if(
        m_installedApplications.cbegin(), m_installedApplications.cend(),
        [&id](const InstalledApplication &application) {
          return application.id == id;
        });
    if (found != m_installedApplications.cend()) {
      return *found;
    }
    return {.id = id,
            .name = id,
            .iconName = QStringLiteral("application-x-executable")};
  }

  const QString resourceClass = exclusion.sliced(6);
  return {.id = resourceClass,
          .name = resourceClass,
          .iconName = QStringLiteral("application-x-executable")};
}

} // namespace AutoScroll

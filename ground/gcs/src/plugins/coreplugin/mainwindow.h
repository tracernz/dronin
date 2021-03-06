/**
 ******************************************************************************
 *
 * @file       mainwindow.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2013
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief Provides the GCS Main Window
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core_global.h"

#include "eventfilteringmainwindow.h"

#include <QtCore/QMap>
#include <QSettings>
#include <QFile>

QT_BEGIN_NAMESPACE
class QSettings;
class QShortcut;
class QToolButton;
class MyTabWidget;
class QFrame;
QT_END_NAMESPACE

namespace Core {

class ActionManager;
class BaseMode;
class BaseView;
class IConfigurablePlugin;
class IContext;
class IMode;
class IWizard;
class ConnectionManager;
class BoardManager;
class MessageManager;
class ModeManager;
class UniqueIDManager;
class ThreadManager;
class ViewManagerInterface;
class UAVGadgetManager;
class UAVGadgetInstanceManager;
class GlobalMessaging;

namespace Internal {

    class ActionManagerPrivate;
    class CoreImpl;
    class GeneralSettings;
    class ShortcutSettings;
    class WorkspaceSettings;
    class VersionDialog;
    class AuthorsDialog;

    class CORE_EXPORT MainWindow : public EventFilteringMainWindow
    {
        Q_OBJECT

    public:
        MainWindow();
        ~MainWindow();

        bool init(QString *errorMessage);
        void extensionsInitialized();
        void shutdown();

        IContext *contextObject(QWidget *widget);
        void addContextObject(IContext *contex);
        void removeContextObject(IContext *contex);
        void resetContext();
        void readSettings(QSettings *qs = nullptr, bool workspaceDiffOnly = false);
        void saveSettings(QSettings *qs = nullptr);
        void readSettings(IConfigurablePlugin *plugin, QSettings *qs = nullptr);
        void saveSettings(IConfigurablePlugin *plugin, QSettings *qs = nullptr);
        void deleteSettings();
        void openFiles(const QStringList &fileNames);

        Core::ActionManager *actionManager() const;
        Core::UniqueIDManager *uniqueIDManager() const;
        Core::GlobalMessaging *globalMessaging() const;
        QList<UAVGadgetManager *> uavGadgetManagers() const;
        UAVGadgetInstanceManager *uavGadgetInstanceManager() const;
        Core::ConnectionManager *connectionManager() const;
        Core::BoardManager *boardManager() const;
        Core::ThreadManager *threadManager() const;
        Core::ModeManager *modeManager() const;
        Internal::GeneralSettings *generalSettings() const;
        QSettings *settings(QSettings::Scope scope) const;
        IContext *currentContextObject() const;
        QStatusBar *statusBar() const;
        void addAdditionalContext(int context);
        void removeAdditionalContext(int context);
        bool hasContext(int context) const;

        void updateContext();

        void setSuppressNavigationWidget(bool suppress);

    signals:
        void windowActivated();
        void splashMessages(QString);
        void hideSplash();
        void showSplash();
    public slots:
        void newFile();
        void openFileWith();
        void exit();
        void setFullScreen(bool on);

        bool showOptionsDialog(const QString &category = QString(), const QString &page = QString(),
                               QWidget *parent = nullptr);

        bool showWarningWithOptions(const QString &title, const QString &text,
                                    const QString &details = QString(),
                                    const QString &settingsCategory = QString(),
                                    const QString &settingsId = QString(), QWidget *parent = nullptr);

    protected:
        virtual void changeEvent(QEvent *e);
        virtual void closeEvent(QCloseEvent *event);
        virtual void dragEnterEvent(QDragEnterEvent *event);
        virtual void dropEvent(QDropEvent *event);

    private slots:
        void openFile();
        void aboutToShowRecentFiles();
        void openRecentFile();
        void setFocusToEditor();
        void saveAll();
        void aboutGCS();
        void aboutPlugins();
        void aboutAuthors();
        void updateFocusWidget(QWidget *old, QWidget *now);
        void destroyVersionDialog();
        void destroyAuthorsDialog();
        void modeChanged(Core::IMode *mode);
        void showUavGadgetMenus(bool show, bool hasSplitter);
        void applyTabBarSettings(QTabWidget::TabPosition pos, bool movable);
        void showHelp();

    private:
        void updateContextObject(IContext *context);
        void registerDefaultContainers();
        void registerDefaultActions();
        void createWorkspaces(QSettings *qs, bool diffOnly = false);
        void readStyleSheet(QFile *file, QString name, QString *style);
        void loadStyleSheet();

        CoreImpl *m_coreImpl;
        UniqueIDManager *m_uniqueIDManager;
        QList<int> m_globalContext;
        QList<int> m_additionalContexts;
        QSettings *m_settings;
        QSettings *m_globalSettings;
        QFrame *m_contentFrame;
        bool
            m_dontSaveSettings; // In case of an Error or if we reset the settings, never save them.
        ActionManagerPrivate *m_actionManager;
        MessageManager *m_messageManager;
        GlobalMessaging *m_globalMessaging;
        ThreadManager *m_threadManager;
        ModeManager *m_modeManager;
        QList<UAVGadgetManager *> m_uavGadgetManagers;
        UAVGadgetInstanceManager *m_uavGadgetInstanceManager;
        ConnectionManager *m_connectionManager;
        BoardManager *m_boardManager;
        MyTabWidget *m_modeStack;
        Core::BaseView *m_outputView;
        VersionDialog *m_versionDialog;
        AuthorsDialog *m_authorsDialog;

        IContext *m_activeContext;

        QMap<QWidget *, IContext *> m_contextWidgets;

        GeneralSettings *m_generalSettings;
        ShortcutSettings *m_shortcutSettings;
        WorkspaceSettings *m_workspaceSettings;

        // actions
        QShortcut *m_focusToEditor;
        QAction *m_newAction;
        QAction *m_openAction;
        QAction *m_openWithAction;
        QAction *m_saveAllAction;
        QAction *m_exitAction;
        QAction *m_optionsAction;
        // UavGadgetManager actions
        QAction *m_showToolbarsAction;
        QAction *m_splitAction;
        QAction *m_splitSideBySideAction;
        QAction *m_removeCurrentSplitAction;
        QAction *m_removeAllSplitsAction;
        QAction *m_gotoOtherSplitAction;

        QString m_config_description;
        QString m_config_details;
#ifdef Q_OS_MAC
        QAction *m_minimizeAction;
        QAction *m_zoomAction;
#endif
        QAction *m_toggleFullScreenAction;
    };

} // namespace Internal
} // namespace Core

#endif // MAINWINDOW_H

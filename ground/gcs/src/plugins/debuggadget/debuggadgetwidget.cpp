/**
 ******************************************************************************
 *
 * @file       debuggadgetwidget.cpp
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup DebugGadgetPlugin Debug Gadget Plugin
 * @{
 * @brief A place holder gadget plugin
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
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "debuggadgetwidget.h"

#include <QDebug>
#include <QStringList>
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include "debugengine.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QTime>

DebugGadgetWidget::DebugGadgetWidget(QWidget *parent) : QLabel(parent)
{
    m_config = new Ui_Form();
    m_config->setupUi(this);
    DebugEngine *de = DebugEngine::getInstance();
    connect(de, SIGNAL(message(DebugEngine::Level, const QString &, const QString &, const int, const QString &)),
                this, SLOT(message(DebugEngine::Level, const QString &, const QString &, const int, const QString &)),
                Qt::QueuedConnection);
    connect(m_config->saveToFile, SIGNAL(clicked()), this, SLOT(saveLog()));
    connect(m_config->clearLog, SIGNAL(clicked()), this, SLOT(clearLog()));

    m_config->tbDebugLog->document()->setDefaultStyleSheet(m_config->tbDebugLog->styleSheet());
}

DebugGadgetWidget::~DebugGadgetWidget()
{
    // Do nothing
}

void DebugGadgetWidget::saveLog()
{
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save log File As"),
                                                    QString("gcs-debug-log-%0.html").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")),
                                                    tr("HTML (*.html)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) &&
        (file.write(m_config->tbDebugLog->toHtml().toLatin1()) != -1)) {
        file.close();
    } else {
        QMessageBox::critical(0,
                              tr("Log Save"),
                              tr("Unable to save log: ") + fileName,
                              QMessageBox::Ok);
        return;
    }
}

void DebugGadgetWidget::clearLog()
{
    m_config->tbDebugLog->clear();
}

void DebugGadgetWidget::message(DebugEngine::Level level, const QString &msg, const QString &file, const int line, const QString &function)
{
    Q_UNUSED(file); Q_UNUSED(line);

    QString type;
    switch (level) {
    case DebugEngine::DEBUG:
        type = "debug";
        break;
    case DebugEngine::INFO:
        type = "info";
        break;
    case DebugEngine::WARNING:
        type = "warning";
        break;
    case DebugEngine::CRITICAL:
        type = "critical";
        break;
    case DebugEngine::FATAL:
        type = "fatal";
        break;
    }

    QString out = QString("<span class='timestamp'>%0</span>").arg(QTime::currentTime().toString());
    out += QString("<span class='%0'>[%1]</span>").arg(type).arg(level >= DebugEngine::WARNING ? type.toUpper() : type);
    if (function.length())
        out += QString("<span class='function'>&lt;%0&gt;</span>").arg(function);
    out += QString("<span class='message'>%0</span>").arg(msg);

    m_config->tbDebugLog->append(out);

    QScrollBar *sb = m_config->tbDebugLog->verticalScrollBar();
    sb->setValue(sb->maximum());
}

/**
 * @}
 * @}
 */

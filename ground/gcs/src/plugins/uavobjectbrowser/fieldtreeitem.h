/**
 ******************************************************************************
 *
 * @file       fieldtreeitem.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectBrowserPlugin UAVObject Browser Plugin
 * @{
 * @brief The UAVObject Browser gadget plugin
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

#ifndef FIELDTREEITEM_H
#define FIELDTREEITEM_H

#include "treeitem.h"
#include <QtCore/QStringList>
#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <qscispinbox/QScienceSpinBox.h>
#include <QComboBox>
#include <limits>

#define QINT8MIN std::numeric_limits<qint8>::min()
#define QINT8MAX std::numeric_limits<qint8>::max()
#define QUINTMIN std::numeric_limits<quint8>::min()
#define QUINT8MAX std::numeric_limits<quint8>::max()
#define QINT16MIN std::numeric_limits<qint16>::min()
#define QINT16MAX std::numeric_limits<qint16>::max()
#define QUINT16MAX std::numeric_limits<quint16>::max()
#define QINT32MIN std::numeric_limits<qint32>::min()
#define QINT32MAX std::numeric_limits<qint32>::max()
#define QUINT32MAX std::numeric_limits<qint32>::max()

class FieldTreeItem : public TreeItem
{
Q_OBJECT
public:
    typedef struct { int base; int v; } FieldInt;
    typedef struct { int base; unsigned v; } FieldUInt;

    FieldTreeItem(int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            TreeItem(data, parent), m_index(index) { }
    FieldTreeItem(int index, const QVariant &data, TreeItem *parent = 0) :
            TreeItem(data, parent), m_index(index) { }
    bool isEditable() { return true; }
    virtual QWidget *createEditor(QWidget *parent) = 0;
    virtual QVariant getEditorValue(QWidget *editor) = 0;
    virtual void setEditorValue(QWidget *editor, QVariant value) = 0;
    virtual void apply() { }
protected:
    int m_index;
};

class EnumFieldTreeItem : public FieldTreeItem
{
Q_OBJECT
public:
    EnumFieldTreeItem(QSharedPointer<UAVObjectField> field, int index, const QList<QVariant> &data,
                      TreeItem *parent = 0) :
    FieldTreeItem(index, data, parent), m_enumOptions(field->getOptions()), m_field(field) { }
    EnumFieldTreeItem(QSharedPointer<UAVObjectField> field, int index, const QVariant &data,
                      TreeItem *parent = 0) :
    FieldTreeItem(index, data, parent), m_enumOptions(field->getOptions()), m_field(field) { }
    void setData(QVariant value, int column) {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return;
        }
        QStringList options = m_field->getOptions();
        QVariant tmpValue = m_field->getValue(m_index);
        int tmpValIndex = options.indexOf(tmpValue.toString());
        setChanged(tmpValIndex != value);
        TreeItem::setData(value, column);
    }
    QString enumOptions(int index) {
        if((index < 0) || (index >= m_enumOptions.length())) {
            return QString("Invalid Value (") + QString().setNum(index) + QString(")");
        }
        return m_enumOptions.at(index);
    }
    void apply() {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return;
        }
        int value = data(dataColumn).toInt();
        if (value == -1) {
            qDebug() << "Warning, UAVO browser field is outside range. This should never happen!";
            Q_ASSERT(0);
            return;
        }
        QStringList options = m_field->getOptions();
        m_field->setValue(options[value], m_index);
        setChanged(false);
    }
    void update() {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return;
        }
        QStringList options = m_field->getOptions();
        QVariant value = m_field->getValue(m_index);
        int valIndex = options.indexOf(value.toString());
        if (data() != valIndex || changed()) {
            TreeItem::setData(valIndex);
            setHighlight(true);
        }
        auto obj = m_field->getObject().dynamicCast<UAVDataObject>();
        if (obj && obj->isSettings())
            setIsDefaultValue(m_field->isDefaultValue(m_index));
    }
    QWidget *createEditor(QWidget *parent) {
        QComboBox *editor = new QComboBox(parent);
        editor->setFocusPolicy(Qt::ClickFocus);
        foreach (QString option, m_enumOptions)
            editor->addItem(option);
        return editor;
    }

    QVariant getEditorValue(QWidget *editor) {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        return comboBox->currentIndex();
    }

    void setEditorValue(QWidget *editor, QVariant value) {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex(value.toInt());
    }
private:
    QStringList m_enumOptions;
    QSharedPointer<UAVObjectField> m_field;
};

class IntFieldTreeItem : public FieldTreeItem
{
Q_OBJECT
public:
    IntFieldTreeItem(QSharedPointer<UAVObjectField> field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), m_field(field) {
        setMinMaxValues();
    }
    IntFieldTreeItem(QSharedPointer<UAVObjectField> field, int index, const QVariant &data, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), m_field(field) {
        setMinMaxValues();
    }

    void setMinMaxValues() {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return;
        }
        switch (m_field->getType()) {
        case UAVObjectField::INT8:
            m_minValue = QINT8MIN;
            m_maxValue = QINT8MAX;
            break;
        case UAVObjectField::INT16:
            m_minValue = QINT16MIN;
            m_maxValue = QINT16MAX;
            break;
        case UAVObjectField::INT32:
            m_minValue = QINT32MIN;
            m_maxValue = QINT32MAX;
            break;
        case UAVObjectField::UINT8:
            m_minValue = QUINTMIN;
            m_maxValue = QUINT8MAX;
            break;
        case UAVObjectField::UINT16:
            m_minValue = QUINTMIN;
            m_maxValue = QUINT16MAX;
            break;
        case UAVObjectField::UINT32:
            m_minValue = QUINTMIN;
            m_maxValue = QUINT32MAX;
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }

    QWidget *createEditor(QWidget *parent) {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return Q_NULLPTR;
        }
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(m_minValue);
        editor->setMaximum(m_maxValue);
        editor->setDisplayIntegerBase(m_field->getRadix());
        switch (m_field->getRadix()) {
        case UAVObjectField::BIN:
            editor->setPrefix("0b");
            break;
        case UAVObjectField::OCT:
            editor->setPrefix("0o");
            break;
        case UAVObjectField::HEX:
            editor->setPrefix("0x");
            break;
        default:
            break;
        }
        return editor;
    }

    QVariant getEditorValue(QWidget *editor) {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return QVariant();
        }
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        switch (m_field->getType()) {
        case UAVObjectField::INT8:
        case UAVObjectField::INT16:
        case UAVObjectField::INT32:
        {
            FieldInt v;
            v.v = spinBox->value();
            v.base = m_field->getRadix();
            return QVariant::fromValue(v);
        }
        case UAVObjectField::UINT8:
        case UAVObjectField::UINT16:
        case UAVObjectField::UINT32:
        {
            FieldUInt v;
            v.v = spinBox->value();
            v.base = m_field->getRadix();
            return QVariant::fromValue(v);
        }
        default:
            Q_ASSERT(false);
            return 0;
        }
    }

    void setEditorValue(QWidget *editor, QVariant value) {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return;
        }
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        switch (m_field->getType()) {
        case UAVObjectField::INT8:
        case UAVObjectField::INT16:
        case UAVObjectField::INT32:
            if (value.userType() == QMetaType::type("FieldTreeItem::FieldInt")) {
                FieldInt v = value.value<FieldInt>();
                spinBox->setValue(v.v);
            } else {
                spinBox->setValue(value.toInt());
            }
            break;
        case UAVObjectField::UINT8:
        case UAVObjectField::UINT16:
        case UAVObjectField::UINT32:
            if (value.userType() == QMetaType::type("FieldTreeItem::FieldUInt")) {
                FieldUInt v = value.value<FieldUInt>();
                spinBox->setValue(v.v);
            } else {
                spinBox->setValue(value.toUInt());
            }
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }
    void setData(QVariant value, int column) {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return;
        }
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply() {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return;
        }
        QVariant d = data(dataColumn);
        switch (m_field->getType()) {
        case UAVObjectField::INT8:
        case UAVObjectField::INT16:
        case UAVObjectField::INT32:
            if (d.userType() == QMetaType::type("FieldTreeItem::FieldInt")) {
                FieldInt v = d.value<FieldInt>();
                m_field->setValue(v.v, m_index);
            } else {
                m_field->setValue(d.toInt(), m_index);
            }
            break;
        case UAVObjectField::UINT8:
        case UAVObjectField::UINT16:
        case UAVObjectField::UINT32:
            if (d.userType() == QMetaType::type("FieldTreeItem::FieldUInt")) {
                FieldUInt v = d.value<FieldUInt>();
                m_field->setValue(v.v, m_index);
            } else {
                m_field->setValue(d.toUInt(), m_index);
            }
            break;
        default:
            Q_ASSERT(false);
            break;
        }
        setChanged(false);
    }
    void update() {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return;
        }
        QVariant value = m_field->getValue(m_index);
        if (data() != value || changed()) {
            QVariant newVal;
            switch (m_field->getType()) {
            case UAVObjectField::INT8:
            case UAVObjectField::INT16:
            case UAVObjectField::INT32:
            {
                FieldInt v;
                v.v = value.toInt();
                v.base = m_field->getRadix();
                newVal.setValue(v);
            }
                break;
            case UAVObjectField::UINT8:
            case UAVObjectField::UINT16:
            case UAVObjectField::UINT32:
            {
                FieldUInt v;
                v.v = value.toUInt();
                v.base = m_field->getRadix();
                newVal.setValue(v);
            }
                break;
            default:
                Q_ASSERT(false);
                break;
            }
            TreeItem::setData(newVal);
            setHighlight(true);
        }

        auto obj = m_field->getObject().dynamicCast<UAVDataObject>();
        if (obj && obj->isSettings())
            setIsDefaultValue(m_field->isDefaultValue(m_index));
    }

private:
    QSharedPointer<UAVObjectField> m_field;
    int m_minValue;
    int m_maxValue;
};

class FloatFieldTreeItem : public FieldTreeItem
{
Q_OBJECT
public:
    FloatFieldTreeItem(QSharedPointer<UAVObjectField> field, int index, const QList<QVariant> &data, bool scientific = false, TreeItem *parent = 0) :
        FieldTreeItem(index, data, parent), m_field(field), m_useScientificNotation(scientific){}
    FloatFieldTreeItem(QSharedPointer<UAVObjectField> field, int index, const QVariant &data, bool scientific = false, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), m_field(field), m_useScientificNotation(scientific) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply() {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return;
        }
        m_field->setValue(data(dataColumn).toDouble(), m_index);
        setChanged(false);
        auto obj = m_field->getObject().dynamicCast<UAVDataObject>();
        if (obj && obj->isSettings())
            setIsDefaultValue(m_field->isDefaultValue(m_index));
    }
    void update() {
        if (!m_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid UAVObjectField!";
            return;
        }
        double value = m_field->getValue(m_index).toDouble();
        if (data() != value || changed()) {
            TreeItem::setData(value);
            setHighlight(true);
        }
    }

    QWidget *createEditor(QWidget *parent) {
        if(m_useScientificNotation) {
            QScienceSpinBox *editor = new QScienceSpinBox(parent);
            editor->setDecimals(6);
            editor->setMinimum(-std::numeric_limits<float>::max());
            editor->setMaximum(std::numeric_limits<float>::max());
            return editor;
        } else {
			QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
			editor->setDecimals(8);
            editor->setMinimum(-std::numeric_limits<float>::max());
            editor->setMaximum(std::numeric_limits<float>::max());
            return editor;
        }
    }

    QVariant getEditorValue(QWidget *editor) {
        if(m_useScientificNotation) {
            QScienceSpinBox *spinBox = static_cast<QScienceSpinBox*>(editor);
            spinBox->interpretText();
            return spinBox->value();
        } else {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->interpretText();
            return spinBox->value();
        }
    }

    void setEditorValue(QWidget *editor, QVariant value) {
        if(m_useScientificNotation) {
            QScienceSpinBox *spinBox = static_cast<QScienceSpinBox*>(editor);
            spinBox->setValue(value.toDouble());
        } else {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->setValue(value.toDouble());
        }
    }
private:
    QSharedPointer<UAVObjectField> m_field;
    bool m_useScientificNotation;

};

Q_DECLARE_METATYPE(FieldTreeItem::FieldInt)
Q_DECLARE_METATYPE(FieldTreeItem::FieldUInt)

#endif // FIELDTREEITEM_H

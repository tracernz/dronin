/**
 ******************************************************************************
 * @file       bluetoothplugin.h
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2017
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup BluetoothPlugin Bluetooth (LE) Plugin
 * @{
 * @brief Support for Bluetooth Low Energy devices
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
 *
 * Additional note on redistribution: The copyright and license notices above
 * must be maintained in each individual source file that is a derivative work
 * of this source file; otherwise redistribution is prohibited.
 */

#ifndef BLUETOOTHPLUGIN_H
#define BLUETOOTHPLUGIN_H

#include "coreplugin/iconnection.h"
#include "coreplugin/idevice.h"
#include <extensionsystem/iplugin.h>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QIODevice>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QMutex>

class BluetoothDeviceInfo : public Core::IDevice
{
    Q_OBJECT
public:
    BluetoothDeviceInfo(const QBluetoothDeviceInfo &devInfo) : m_devInfo(devInfo) {
        setName(m_devInfo.name());
        setDisplayName(QString("%0 (%1 dB)").arg(m_devInfo.name()).arg(m_devInfo.rssi()));
    }
    QBluetoothDeviceInfo &deviceInfo() { return m_devInfo; }

private:
    QBluetoothDeviceInfo m_devInfo;
};

class BluetoothIODevice : public QIODevice
{
    Q_OBJECT
public:
    BluetoothIODevice(const QBluetoothDeviceInfo &devInfo);
    ~BluetoothIODevice();

    bool open(QIODevice::OpenMode mode);
    void close();
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;
    bool isSequential() const { return true; }

signals:
    void serviceReady();

private slots:
    void serviceDiscovered(QBluetoothUuid uuid);
    void controllerError(QLowEnergyController::Error error);
    void deviceConnected();
    void deviceDisconnected();
    void characteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray data);
    void serviceStateChanged(QLowEnergyService::ServiceState state);
    void serviceError(QLowEnergyService::ServiceError error);

private:
    QBluetoothDeviceInfo m_devInfo;
    QLowEnergyController *m_control;
    QLowEnergyService *m_service;
    QLowEnergyDescriptor m_notificationDesc;
    bool m_serviceFound;
    bool m_readyWrite;
    QByteArray m_readBuffer;
    QMutex m_readMutex;

    const quint16 hm10ServiceId = 0xffe0;
    const quint16 hm10CharacteristicId = 0xffe1;
};


class BluetoothConnection : public Core::IConnection
{
    Q_OBJECT
public:
    BluetoothConnection();
    ~BluetoothConnection();

    /**
    *   Return the list of devices found on the system
    */
    virtual QList<Core::IDevice *> availableDevices();

    /**
    *   Open a device, and return a QIODevice interface from it
    *   It should be a dynamically created object as it will be
    *   deleted by the connection manager.
    */
    virtual QIODevice *openDevice(Core::IDevice *device);

    virtual void closeDevice(const QString &deviceName);

    /**
    *   Connection type name
    */
    virtual QString connectionName() { return "BLE"; }

    /**
    *   Short name to display in a combo box
    */
    virtual QString shortName() { return connectionName(); }

    /**
     * Manage whether the plugin is allowed to poll for devices
     * or not
     */
    virtual void suspendPolling() {}
    virtual void resumePolling() {}

    /**
     * @brief Used to flag that the device wants that we try to reconnect if it gets disconnected
     * Currently this should be used only with bluetooth devices
     * @return true if the device want's us to try to reconnect it
     */
    virtual bool reconnect() { return false; }

signals:
    /**
    *   Available devices list has changed, signal it to connection manager (and whoever wants to know)
    */
    void availableDevChanged(IConnection *);

private:
    QList<QBluetoothDeviceInfo>devices;
    BluetoothIODevice *m_device;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &devInfo);
};

class BluetoothPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.dronin.plugins.Bluetooth")
public:
    BluetoothPlugin() {}
    ~BluetoothPlugin() {}

    virtual bool initialize(const QStringList &arguments, QString *errorMessage)
    {
        Q_UNUSED(arguments) Q_UNUSED(errorMessage)
        m_connection = new BluetoothConnection;
        return true;
    }

    virtual void extensionsInitialized() { addAutoReleasedObject(m_connection); }
private:
    BluetoothConnection *m_connection;
};

#endif // BLUETOOTHPLUGIN_H

/**
 * @}
 * @}
 */

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

#include "bluetoothplugin.h"

#include <QDebug>
#include <QBluetoothUuid>
#include <QEventLoop>
#include <QMutexLocker>
#include <QTimer>

BluetoothConnection::BluetoothConnection() : m_device(Q_NULLPTR)
{
    // Create a discovery agent and connect to its signals
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    if (m_discoveryAgent) {
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                this, &BluetoothConnection::deviceDiscovered);

        // Start a discovery
        m_discoveryAgent->start();
    }
}

BluetoothConnection::~BluetoothConnection()
{
    if (m_discoveryAgent)
        m_discoveryAgent->stop();
}

QList<Core::IDevice *> BluetoothConnection::availableDevices()
{
    QList<Core::IDevice *> devList;

    for (const auto &dev : devices)
        devList.append(new BluetoothDeviceInfo(dev));

    return devList;
}

void BluetoothConnection::deviceDiscovered(const QBluetoothDeviceInfo &devInfo)
{
    if (!(devInfo.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) || devInfo.name() != "dRonin")
        return;
    for (const auto &d : devices) {
#ifdef Q_OS_MAC
        if (d.deviceUuid() == devInfo.deviceUuid())
#else
        if (d.address() == devInfo.address())
#endif
            return;
    }

    qWarning() << "Found new device:" << devInfo.name() << '(' << devInfo.address().toString() << '|' << devInfo.deviceUuid().toString() << ')';

    devices.append(devInfo);
    emit availableDevChanged(this);
}

QIODevice *BluetoothConnection::openDevice(Core::IDevice *_device)
{
    auto *device = qobject_cast<BluetoothDeviceInfo *>(_device);
    if (!device)
        return Q_NULLPTR;

    m_device = new BluetoothIODevice(device->deviceInfo());
    if (!m_device->open(QIODevice::ReadWrite)) {
        m_device->deleteLater();
        m_device = Q_NULLPTR;
    }

    return m_device;
}

void BluetoothConnection::closeDevice(const QString &deviceName)
{
    Q_UNUSED(deviceName)
    if (!m_device || !m_device->isOpen())
        return;
    m_device->close();
}

BluetoothIODevice::BluetoothIODevice(const QBluetoothDeviceInfo &devInfo) : m_devInfo(devInfo), m_serviceFound(false), m_readyWrite(false)
{
}

BluetoothIODevice::~BluetoothIODevice()
{
    if (isOpen())
        close();
}

bool BluetoothIODevice::open(QIODevice::OpenMode mode)
{
    m_control = new QLowEnergyController(m_devInfo, this);
    connect(m_control, SIGNAL(serviceDiscovered(QBluetoothUuid)), this, SLOT(serviceDiscovered(QBluetoothUuid)));
    connect(m_control, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(controllerError(QLowEnergyController::Error)));
    connect(m_control, SIGNAL(connected()), this, SLOT(deviceConnected()));
    connect(m_control, SIGNAL(disconnected()), this, SLOT(deviceDisconnected()));

    qWarning() << "connecting to device...";
    QEventLoop loop;
    QTimer timeout;
    connect(m_control, SIGNAL(discoveryFinished()), &loop, SLOT(quit()));
    connect(&timeout, SIGNAL(timeout()), &loop, SLOT(quit()));

    timeout.setSingleShot(true);
    timeout.start(10000);
    m_control->connectToDevice();
    loop.exec();
    disconnect(m_control, SIGNAL(discoveryFinished()), &loop, SLOT(quit()));

    if (!timeout.remainingTime() || !m_serviceFound) {
        m_control->disconnectFromDevice();
        qWarning() << "Timeout while connecting BLE device!";
        return false;
    }

    m_service = m_control->createServiceObject(QBluetoothUuid(hm10ServiceId), this);
    if (!m_service) {
        qWarning() << "Error connecting to BLE device!";
        return false;
    }

    connect(m_service, SIGNAL(stateChanged(QLowEnergyService::ServiceState)),
            this, SLOT(serviceStateChanged(QLowEnergyService::ServiceState)));
    connect(m_service, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
            this, SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));
    connect(m_service, SIGNAL(error(QLowEnergyService::ServiceError)),
            this, SLOT(serviceError(QLowEnergyService::ServiceError)));
    connect(this, SIGNAL(serviceReady()), &loop, SLOT(quit()));

    timeout.start(10000);
    m_service->discoverDetails();
    loop.exec();
    if (!timeout.remainingTime() || !m_serviceFound) {
        qWarning() << "Timeout while connecting BLE device!";
        return false;
    }

    qWarning() << "connected!";
    return QIODevice::open(mode);
}

void BluetoothIODevice::close()
{
    if (!isOpen())
        return;
    if (m_service) {
        QIODevice::close();
        if (m_notificationDesc.isValid()) {
            // unsubscribe from notifications, must wait for an ack before disconnecting the dev
            QTimer timeout;
            timeout.setSingleShot(true);
            QEventLoop loop;
            connect(m_service, SIGNAL(descriptorWritten(QLowEnergyDescriptor,QByteArray)), &loop, SLOT(quit()));

            timeout.start(2000);
            m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0000"));
            loop.exec();
            if (!timeout.remainingTime())
                qWarning() << "Failed to unsubscribe from BLE notifications";
        }
        m_service->deleteLater();
    }
    if (m_control) {
        m_control->disconnectFromDevice();
        m_control->deleteLater();
    }
    m_serviceFound = false;
    m_readyWrite = false;
    qWarning() << "disconnected";
}

void BluetoothIODevice::serviceStateChanged(QLowEnergyService::ServiceState state)
{
    qWarning() << "Service state changed: " << state;
    switch (state) {
    case QLowEnergyService::ServiceDiscovered:
    {
        const QLowEnergyCharacteristic hrChar = m_service->characteristic(
                    QBluetoothUuid(hm10CharacteristicId));
        if (!hrChar.isValid()) {
            qWarning() << "Invalid characteristic";
            break;
        }

        m_notificationDesc = hrChar.descriptor(
                    QBluetoothUuid::ClientCharacteristicConfiguration);
        if (!m_notificationDesc.isValid()) {
            qWarning() << "Could not set notifications";
            break;
        }

        m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
        m_readyWrite = true;
        emit serviceReady();

        break;
    }
    default:
        //nothing for now
        break;
    }
}

void BluetoothIODevice::serviceError(QLowEnergyService::ServiceError error)
{
    qWarning() << "Service error: " << error;
}

void BluetoothIODevice::characteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray data)
{
    if (characteristic.uuid() != QBluetoothUuid(hm10CharacteristicId))
        return;
    QMutexLocker lock(&m_readMutex);
    m_readBuffer.append(data);
    lock.unlock();
    emit readyRead();
}

qint64 BluetoothIODevice::readData(char *data, qint64 maxlen)
{
    QMutexLocker lock(&m_readMutex);
    qint64 len = qMin(maxlen, static_cast<qint64>(m_readBuffer.length()));
    memcpy(data, m_readBuffer.constData(), len);
    m_readBuffer.remove(0, len);
    return len;
}

qint64 BluetoothIODevice::writeData(const char *data, qint64 len)
{
    if (!m_readyWrite)
        return 0;

    // the characteristic can handle 20 bytes of data, QIODevice docs say we should always send all requested bytes
    qint64 pos = 0;
    while (pos < len) {
        QByteArray buf(&data[pos], qMin(20LL, len - pos));
        pos += buf.length();
        m_service->writeCharacteristic(m_service->characteristic(QBluetoothUuid(hm10CharacteristicId)), buf, QLowEnergyService::WriteWithoutResponse);
    }
    return len;
}

qint64 BluetoothIODevice::bytesAvailable() const
{
    return m_readBuffer.length();
}

void BluetoothIODevice::serviceDiscovered(QBluetoothUuid uuid)
{
    qWarning() << "discovered" << uuid.toString() << QBluetoothUuid(hm10ServiceId).toString();
    m_serviceFound |= uuid == QBluetoothUuid(hm10ServiceId);
}

void BluetoothIODevice::controllerError(QLowEnergyController::Error error)
{
    qWarning() << "bt error" << error;
}

void BluetoothIODevice::deviceConnected()
{
    qWarning() << "ble dev connected";
    m_control->discoverServices();
}

void BluetoothIODevice::deviceDisconnected()
{
    qWarning() << "device disconnected";
    if (isOpen())
        close();
}

/**
  * @}
  * @}
  */

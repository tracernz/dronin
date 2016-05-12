/**
 ******************************************************************************
 * @file       abstract_dfu.h
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2014
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Uploader Uploader Plugin
 * @{
 * @brief Abstract DFU interface
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
 *
 * Additional note on redistribution: The copyright and license notices above
 * must be maintained in each individual source file that is a derivative work
 * of this source file; otherwise redistribution is prohibited.
 */

#ifndef ABSTRACT_DFU_H_
#define ABSTRACT_DFU_H_

namespace dfu {

enum Status
{
    DFUidle,//0
    uploading,//1
    wrong_packet_received,//2
    too_many_packets,//3
    too_few_packets,//4
    Last_operation_Success,//5
    downloading,//6
    idle,//7
    Last_operation_failed,//8
    uploadingStarting,//9
    outsideDevCapabilities,//10
    CRC_Fail,//11
    failed_jump,//12
    abort,//13
    not_in_dfu
};

struct Device
{
    quint16 ID;
    quint32 FW_CRC;
    quint8 BL_Version;
    int SizeOfDesc;
    quint32 SizeOfCode;
    bool Readable;
    bool Writable;
    QVector<quint32> PartitionSizes;
    int HW_Rev;
    bool CapExt;
};

class DfuObject : public QThread
{
public:
	typedef struct statusReport {
        quint32 additional;
        tl_dfu::Status status;
    } statusReport;

    DfuObject();
    ~DfuObject();

    // Service commands:
    virtual int JumpToApp(bool);
    virtual int ResetDevice(void);
    virtual bool OpenBootloaderComs();
    virtual void CloseBootloaderComs();

    // Partition operations:
    virtual bool UploadPartitionThreaded(QByteArray &sourceArray, dfu_partition_label partition, int size);
    virtual bool DownloadPartitionThreaded(QByteArray *firmwareArray, dfu_partition_label partition, int size);
    virtual bool WipePartition(dfu_partition_label partition);
    virtual QByteArray DownloadDescriptionAsByteArray(int const &numberOfChars);

public slots:
    virtual Device findCapabilities();
    virtual QString partitionStringFromLabel(dfu_partition_label label);

signals:
    virtual void downloadFinished(bool);
    virtual void uploadFinished(dfu::Status);
    virtual void operationProgress(QString status, int progress);

protected:
    virtual void run();// Executes the upload or download operations

    typedef struct ThreadJobStruc {
        enum Actions
        {
            Download,
            Upload
        };
        qint32 requestSize;
        dfu_partition_label requestTransferType;
        QByteArray *requestStorage;
        quint32 partition_size;
        Actions requestedOperation;
    } ThreadJobStruc;

    ThreadJobStruc threadJob;
};

Q_DECLARE_METATYPE(dfu::Status)

} // namespace dfu

#endif // ABSTTACT_DFU_H_

 /**
  * @}
  * @}
  */
#include "FrameProcessor.h"
#include "Frame.h"
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QElapsedTimer>
#include <QtCharts/QXYSeries>
#include <QThread>

FrameProcessor::FrameProcessor(QQueue<Frame*> *frame_queue, QObject *parent) : QObject(parent)
{
    qDebug()<<"Processor thread,"<< QThread::currentThreadId();

    m_frame_queue = frame_queue;

    for(int i = 0; i < 32; i++)
    {
        m_counts_x.append(0);
        m_counts_y.append(0);
    }

}

// Start to listening the port.
bool FrameProcessor::openServer(QString addr)
{
    emit openLocalServer(addr);

    // delay a while.
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<10);

    return true;
}

// Stop to listening the port.
bool FrameProcessor::closeServer()
{
    emit closeLocalServer();

    // delay a while.
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<10);


    return true;
}

// Discover interested device
void FrameProcessor::scanDevice(void)
{
    quint16 cmd = CMD_DISCOVER;
    quint16 data_length = 0;
    Frame *frameToSend = new Frame(cmd, data_length);
    QHostAddress broadcast_address = QHostAddress::Broadcast;
   
    emit sendFrame(broadcast_address.toString(), frameToSend);
}

// Response to device server
void FrameProcessor::responseToDevice(QString addr)
{
    quint16 cmd = CMD_DISCOVER_RESPONSE;
    quint16 data_length = 0;
    Frame *frameToSend = new Frame(cmd, data_length);

    emit sendFrame(addr, frameToSend);
}

// Get the local ip list
QStringList FrameProcessor::localIPList()
{
    return m_local_ip_list;
}

void FrameProcessor::refreshLocalIP(QStringList local_ip_list)
{
    m_local_ip_list = local_ip_list;

    emit localIPListChanged();
}

// Get the device list
QStringList FrameProcessor::deviceList()
{
    return m_device_list;
}
// Get the device ip list
QStringList FrameProcessor::deviceIPList()
{
    return m_device_ip_list;
}

// Set device.
bool FrameProcessor::setDevice(unsigned char mode, int lthd_a, int lthd_b,
                            unsigned char coin_time_a, unsigned char coin_time_b,
                            QString addr)
{
    // refresh settings.
    m_settings.mode = mode;
    // m_settings.lthd_a = ThresholdToCode(lthd_a, VCC, VEE, RESOLUTION_THD, INTERCEPT);
    // m_settings.lthd_b = ThresholdToCode(lthd_b, VCC, VEE, RESOLUTION_THD, INTERCEPT);
    m_settings.lthd_a = lthd_a;
    m_settings.lthd_b = lthd_b;
    m_settings.coin_time_a = coin_time_a;
    m_settings.coin_time_b = coin_time_b;

    qDebug() << "lthd_a, " << dec << int(lthd_a) << "lthd_b" << lthd_b;

    QByteArray data;
    data.resize(16);
        data[0] = m_settings.name[0];
        data[1] = m_settings.name[1];
        data[2] = m_settings.ip >> 24;
        data[3] = m_settings.ip >> 16;
        data[4] = m_settings.ip >> 8;
        data[5] = m_settings.ip;
        data[6] = m_settings.mode;
        data[7] = m_settings.channel;
        data[8] = m_settings.hv >> 8;
        data[9] = m_settings.hv;
        data[10] = m_settings.lthd_a >> 8;
        data[11] = m_settings.lthd_a;
        data[12] = m_settings.lthd_b >> 8;
        data[13] = m_settings.lthd_b;
        data[14] = m_settings.coin_time_a;
        data[15] = m_settings.coin_time_b;

    quint16 cmd = CMD_SET_SETTINGS;
    quint16 data_length = 16;
    Frame *frameToSend = new Frame(cmd, data_length, data);
   
    emit sendFrame(addr, frameToSend);

    // delay a while.
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<10);

    return true;
}

// read the device settings.
QVariantList FrameProcessor::readDeviceSettings(void)
{
    QVariantList device_settings;

    device_settings.append(int(m_settings.mode));
    // device_settings.append(CodeToThreshold(m_settings.lthd_a, VCC, VEE, RESOLUTION_THD, INTERCEPT));
    // device_settings.append(CodeToThreshold(m_settings.lthd_b, VCC, VEE, RESOLUTION_THD, INTERCEPT));
    device_settings.append(int(m_settings.lthd_a));
    device_settings.append(int(m_settings.lthd_b));
    device_settings.append(int(m_settings.coin_time_a));
    device_settings.append(int(m_settings.coin_time_b));

    return device_settings;
}

// Send start command to the device.
bool FrameProcessor::startDevice(QString addr)
{
    quint16 cmd = CMD_START;
    quint16 data_length = 0;
    Frame *frameToSend = new Frame(cmd, data_length);

    emit sendFrame(addr, frameToSend);
    emit autoSaveData();

    return true;
}

// Send start command to the device.
bool FrameProcessor::stopDevice(QString addr)
{
    quint16 cmd = CMD_STOP;
    quint16 data_length = 0;
    Frame *frameToSend = new Frame(cmd, data_length);

    emit sendFrame(addr, frameToSend);
    emit stopSaveData();

    return true;
}

// Proccess the coming data.
void FrameProcessor::FrameIncoming(QString device_ip, Frame *frame)
{
//    qDebug() << "queue size," << m_frame_queue->size();
//    m_frame_queue->clear();

//    Frame *current_frame;
//    int frame_num = m_frame_queue->size();
//    for(int i = 0; i < frame_num; i++)
//    {
//    mutex.lock();
//    current_frame = m_frame_queue->dequeue();
//    mutex.unlock();
    if (frame != nullptr)
    {

        quint16 cmd = frame->GetCmd();
        quint16 frame_length = frame->GetDataLength();
        QByteArray frame_buffer = frame->GetBuffer();
        // qDebug() << "current queue," << i << "," << frame_length << "," << frame_buffer.count();

        switch (cmd)
        {
            case CMD_DISCOVER_RESPONSE:    // Response of client
            {
                m_settings.name[0] = char(frame_buffer[4]);
                m_settings.name[1] = char(frame_buffer[5]);
                m_settings.mode = frame_buffer[10];
                m_settings.channel = frame_buffer[11];
                m_settings.hv = (qint16(frame_buffer[12]) << 8) + frame_buffer[13];
                m_settings.lthd_a = (qint16(frame_buffer[14]) << 8) + frame_buffer[15];
                m_settings.lthd_b = (qint16(frame_buffer[16]) << 8) + frame_buffer[17];
                m_settings.coin_time_a = frame_buffer[18];
                m_settings.coin_time_b = frame_buffer[19];

                qDebug() << "lthd_a, " << dec << m_settings.lthd_a << "lthd_b" << m_settings.lthd_b;

                QString name =  QString::fromLocal8Bit(m_settings.name, 2);
                m_device_list.append(name);
                m_device_ip_list.append(device_ip);

                qDebug() << "AccessPoint:" << (name + ',' + device_ip);

                emit deviceListChanged();
                emit deviceIPListChanged();

                break;
            }
            case CMD_DISCOVER:    // Discover command from server
            {
                if(m_debug_enable)
                {
                    responseToDevice(device_ip);
                }
                break;
            }
            case CMD_DATA_LIST:     // Data in event mode of the hole detector.
            {
                break;
            }
            default:
                break;
        }

        // delete frame;
    }
}

int FrameProcessor::refreshChannelCountsX(QAbstractSeries *series)
{
    int max = 0;

    emit updateChannelCountsX(&m_counts_x);
    // delay a while.
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<10);

    if (series) {
        QXYSeries *xySeries = static_cast<QXYSeries *>(series);

        QVector<QPointF> points;
        // Append the new data depending on the type
        for (int i = 0; i < 32; i++) {
            points.append(QPoint(i, m_counts_x[i]));
            if(m_counts_x[i] > max)
            {
                max = m_counts_x[i];
            }
        }

        // Use replace instead of clear + append, it's optimized for performance
        xySeries->replace(points);
    }

    return max;
}

int FrameProcessor::refreshChannelCountsY(QAbstractSeries *series)
{
    int max = 0;

    emit updateChannelCountsY(&m_counts_y);
    // delay a while.
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<10);

    if (series) {
        QXYSeries *xySeries = static_cast<QXYSeries *>(series);

        QVector<QPointF> points;
        // Append the new data depending on the type
        for (int i = 0; i < 32; i++) {
            points.append(QPoint(i, m_counts_y[i]));
            if(m_counts_y[i] > max)
            {
                max = m_counts_y[i];
            }
        }

        // Use replace instead of clear + append, it's optimized for performance
        xySeries->replace(points);
    }

    return max;
}

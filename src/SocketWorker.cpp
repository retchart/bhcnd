#include "SocketWorker.h"
#include <QEventLoop>
#include <QThread>
#include <QNetworkInterface>
#include <QDebug>
#include <QDateTime>
#include <QIODevice>


/* Command List
        The BHCND command Packets format is "$ Address CMD_MSB CMD_LSB LEN_MSB
    LEN_LSB (*Data*) CHKSUM \n"
*/


SocketWorker::SocketWorker(QQueue<Frame*> *frame_queue, QObject *parent) :
    QObject(parent)
{
    m_frame_queue = frame_queue;

    for(int i = 0; i < 32; i++)
    {
        m_counts_x.append(0);
        m_counts_y.append(0);
    }

    _working =false;
    _abort = false;

    // Search all avilable host address
    QList<QHostAddress> host_address_list = QNetworkInterface::allAddresses();

    foreach (QHostAddress address, host_address_list) {
        if (QAbstractSocket::IPv4Protocol == address.protocol()) {
            m_local_ip_list.append(address.toString());
        }
    }
}

SocketWorker::~SocketWorker()
{
    if(m_udp_socket != nullptr)
    {
        delete m_udp_socket;
    }

}

// Initialse host ip etc.
void SocketWorker::updateLocalIP()
{
    emit checkLocalIP(m_local_ip_list);
}

// Start to lisen port
bool SocketWorker::openLocalServer(QString addr)
{
    // Set host address
    QHostAddress hostAddress(addr);
    quint16 host_port = m_port;

    qDebug() << addr << ":" << m_port;

    // construct QUdpSocket.
    m_udp_socket = new QUdpSocket(this);

    // connect signals and slots
    if (m_udp_socket->bind(hostAddress, host_port, QUdpSocket::ShareAddress))
    {
        connect(m_udp_socket, SIGNAL(readyRead()), this, SLOT(readPendingDiagrams()));

        qDebug()<<"Start success!";
        qDebug() << "This thread," << QThread::currentThreadId();

        return true;
    } else // if error ocurrs
    {
        qDebug() << QString("%1, %2").arg(m_udp_socket->error()).arg(m_udp_socket->errorString());
        
        return false;
    }
}

// Close server
bool SocketWorker::closeLocalServer(void)
{
    m_udp_socket->close();
    m_udp_socket->disconnect(this);

    if(m_udp_socket != nullptr)
    {
        delete m_udp_socket;
    }

    qDebug() << "Connection closed.";

    return true;
}

// data process
void SocketWorker::readPendingDiagrams() 
{
    quint8 in_byte;

    QUdpSocket *socket = qobject_cast<QUdpSocket *>(sender());
    if (!socket) return;

    while(socket->hasPendingDatagrams())
    {
        qint64 buf_length = socket->pendingDatagramSize();
        if (buf_length < 0) {
            qDebug() << "there is no available data";
        }

        // create buffers
        char buf[MAX_FRAME_LENGTH];
        QHostAddress device_address;
        quint16 device_port(0);

        qint64 io_length = socket->readDatagram(buf, buf_length, &device_address, &device_port);

        // memcpy(read_data_buf, buf, io_length);

        // qDebug() << QDateTime::currentDateTime() << device_address.toString()
        //          << ":" << device_port << " " << read_data_buf;
        // qDebug() << "io_length, " <<  io_length;
        quint16 dst_address = 0;
        for(int i = 0; i < io_length; i++)
        {
            // in_byte = quint8(buf[i]);

            switch(m_receiver_status)
            {
                // Receive header (address).
                case RCV_ST_IDLE:
                {
                    if(m_state_counts == 0)
                    {
                        if(m_in_frame == nullptr)
                            m_in_frame = new Frame();
                        else
                            m_in_frame->Clear();
                        // m_in_frame->AddByte(buf[i]);
                        dst_address = quint16(buf[i]) << 8;
                        m_state_counts += 1;
                    }
                    else
                    {
                        // m_in_frame->AddByte(buf[i]);
                        dst_address += quint16(buf[i]);
                        if(dst_address == ADDRESS)
                            m_receiver_status = RCV_ST_CMD;
                        else
                            m_receiver_status = RCV_ST_IDLE;
                        m_state_counts = 0;
                    }
                } break;
                // Receive command.
                case RCV_ST_CMD:
                {
                    if(m_state_counts == 0)
                    {
                        m_in_frame->AddByte(buf[i]);
                        m_state_counts += 1;
                    }
                    else
                    {
                        m_in_frame->AddByte(buf[i]);
                        m_state_counts = 0;
                        m_receiver_status = RCV_ST_DATA_LENGTH;
                    }
                } break;
                // Receive data length.
                case RCV_ST_DATA_LENGTH:
                {
                    if(m_state_counts == 0)
                    {
                        m_in_frame->AddByte(buf[i]);
                        m_data_length = quint16(buf[i]) << 8;
                        m_state_counts += 1;
                    }
                    else
                    {
                        m_in_frame->AddByte(buf[i]);
                        m_data_length += quint16(buf[i]);
                        m_state_counts = 0;
                        // If the received command has no data, it is just acknkowledgement.
                        if(m_data_length == 0)
                            m_receiver_status = RCV_ST_IDLE;
                        else
                        {
                            // qDebug() << "data length," << m_data_length;
                            m_receiver_status = RCV_ST_DATA;
                        }
                    }
                } break;
                // Receive data.
                case RCV_ST_DATA:
                {
                    if(m_state_counts < m_data_length - 1)
                    {
                        m_data_buf[m_state_counts++] = buf[i];
                    }
                    else
                    {
                        m_data_buf[m_state_counts++] = buf[i];
                        // Only emit the command frame, and the data is processed here.
                        if(m_state_counts < 32)
                            m_in_frame->AddByteArray(QByteArray(m_data_buf, m_state_counts));
                        m_state_counts = 0;
                        m_receiver_status = RCV_ST_CHECKSUM;
                    }
                } break;
                // Confirm the checksum. Here, the checksum is omite.
                case RCV_ST_CHECKSUM:
                {
                    if(m_state_counts == 0)
                    {
                        m_checksum = quint16(buf[i]) << 8;
                        m_state_counts += 1;
                    }
                    else
                    {
                        m_checksum += quint16(buf[i]);
                        // Emit the frame received signal.
                        if(m_in_frame->GetCmd() == CMD_DATA_LIST)
                        {
                            parseFrameData(m_data_buf, m_data_length);
                            m_frame_counts += 1;
                        }
                        else
                        {
                            emit this->frameReceived(device_address.toString(), m_in_frame);
                        }
                        m_state_counts = 0;
                        m_receiver_status = RCV_ST_IDLE;
                    }
                } break;
            }
        }

    }
}

quint8 SocketWorker::calculateChecksum(QByteArray buffer)
{
    quint8 rv = 0;
    for (int i = 0; i < buffer.count(); i++)
        rv += quint8(buffer[i]);
    return rv;
}

void SocketWorker::sendFrame(QString addr, Frame *frame)
{
    QByteArray frame_buffer = frame->GetBuffer();
    quint16 frame_length = frame_buffer.count();
    // Add 2 bytes address and 2 bytes checkbit to the frame.
    QByteArray out_buffer;
    out_buffer.resize(4 + frame_length);
    quint16 checksum = 0x0A0A;

    out_buffer[0] = quint8(ADDRESS >> 8);
    out_buffer[1] = quint8(ADDRESS);
    for(int i = 0; i < frame_length; i++)
    {
        out_buffer[i + 2] = frame_buffer[i];
    }

    out_buffer[frame_length + 2] = quint8(checksum >> 8);
    out_buffer[frame_length + 3] = quint8(checksum);
    // qDebug() << out_buffer;

    sendData(addr, out_buffer);
}

void SocketWorker::sendData(QString &dst_ip, const QByteArray &data_array)
{
    // create char buffer for send.
    char buf[MAX_FRAME_LENGTH];
    qint64 send_length = data_array.length();
    memcpy(buf, data_array.data(), send_length);

    qint16 write_length = 0;
    qint16 io_length = m_udp_socket->writeDatagram(buf, send_length, QHostAddress(dst_ip),
        m_port);

    // If send not complete.
    while (io_length > 0) {
        write_length += io_length;

        io_length = (write_length >= send_length) ? 0 :
            m_udp_socket->writeDatagram(buf + write_length, send_length - write_length,
            QHostAddress(dst_ip), m_port);
    }

    // If error comes.
    if (write_length != send_length) {
        qDebug() << (QString("failed to send data to %1:%2 [%3]")
                .arg(dst_ip).arg(m_port).arg(write_length));

        return;
    }
}

void SocketWorker::autoSaveData()
{
    // creat file according to current time.
    QString file_nanme = QDateTime::currentDateTime().toString("yyyyMMdd_hhmm") + ".csv";
    qDebug() << file_nanme;
    m_data_file.setFileName(file_nanme);
    m_data_file.open(QIODevice::WriteOnly);
    m_data_file_stream.setDevice(&m_data_file);
    m_data_file_buffer.append("Time,channel,width\n");
    m_data_file_stream << m_data_file_buffer[0];
    m_data_file_buffer.clear();

    // clear old data.
    for(int i = 0; i < 32; i++)
    {
        m_counts_x[i] = 0;
        m_counts_y[i] = 0;
    }
    m_frame_counts = 0;
}

void SocketWorker::stopSaveData()
{

    if(m_data_file_buffer.size() > 0)
    {
        for(int m = 0; m < m_data_file_buffer.size(); m++)
        {
            m_data_file_stream << m_data_file_buffer[m];
        }
        m_data_file_buffer.clear();
    }

    m_data_file.close();

    qDebug() << "frames," << m_frame_counts;
    m_frame_counts = 0;
}

// void SocketWorker::parseFrameData(Frame *frame)
void SocketWorker::parseFrameData(char *frame_buffer, int frame_length)
{
    quint64 trig_time = 0;
    quint8 trig_channel;
    quint16 trig_height;
    QString trig_event;
//    quint16 frame_length = frame->GetDataLength();
//    char* frame_buffer = frame->GetBuffer().data();
    // QByteArray frame_buffer = frame->GetBuffer();
    // construct event string
    char trig_event_buf[16+3+5+2];
    // command and length has 4 bytes total.
    // make sure an event (8bytes) left.
    // for(int i = 4; i < frame_length - 8; i+= 8)
    for(int i = 0; i < frame_length - 8; i+= 8)
    {
        trig_time = 0xFF00000000 & (quint64(frame_buffer[i]) << 32);
        trig_time |= 0x00FF000000 & (quint64(frame_buffer[i + 1]) << 24);
        trig_time |= 0x0000FF0000 & (quint64(frame_buffer[i + 2]) << 16);
        trig_time |= 0x000000FF00 & (quint64(frame_buffer[i + 3]) << 8);
        trig_time |= 0x00000000FF & (quint64(frame_buffer[i + 4]));
        trig_channel = quint8(frame_buffer[i + 5]);
        trig_height = 0xFF00 & (quint16(frame_buffer[i + 6]) << 8);
        trig_height |= 0x00FF & (quint16(frame_buffer[i + 7]));

        // refresh counts and phs.
        if(trig_channel < 32)
        {
            m_counts_x[trig_channel] += 1;
        }
        else if(trig_channel < 64)
        {
            m_counts_y[trig_channel - 32] += 1;
        }

        // save valid data to file buffer.
        // sprintf(trig_event_buf, "%llu,%u,%u\n",trig_time, trig_channel, trig_height);
        // save data.
        // m_data_file_stream << trig_event_buf;
    }
}

void SocketWorker::updateChannelCountsX(QVector<int>* counts_x)
{
    counts_x->resize(32);
    for(int i = 0; i < 32; i++)
    {
        (*counts_x)[i] = m_counts_x[i];
    }
}


void SocketWorker::updateChannelCountsY(QVector<int>* counts_y)
{
    counts_y->resize(32);
    for(int i = 0; i < 32; i++)
    {
        (*counts_y)[i] = m_counts_y[i];
    }
}

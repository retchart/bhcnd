#include "UdpServer.h"
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QNetworkInterface>
// #include <QHostAddress>
#include <QDateTime>

// class define
UdpServer:: UdpServer(QObject *parent):QObject(parent)
{
    m_device_list.append(m_device);
    m_device_ip_list.append(m_device_ip.toString());

    initialize();
}

UdpServer *UdpServer::instance()
{
    static UdpServer server;
    return &server;
}

UdpServer::~UdpServer()
{

}

// Initialse host ip etc.
void UdpServer::initialize()
{
    // Search all avilable host address
    QList<QHostAddress> host_address_list = QNetworkInterface::allAddresses();

    foreach (QHostAddress address, host_address_list) {
        if (QAbstractSocket::IPv4Protocol == address.protocol()) {
            m_local_ip_list.append(address.toString());
        }
    }
    emit localIPListChanged();
}

// Get the local ip list
QStringList UdpServer::localIPList()
{
    return m_local_ip_list;
}

// Discover interested device
void UdpServer::discover()
{
    m_device_list.clear();
    m_device_ip_list.clear();
    m_udp_server.writeDatagram(cmd_discover, sizeof(cmd_discover), QHostAddress::Broadcast, m_port);
}

// Enable debug
void UdpServer::debugEnable(bool debug_enable)
{
    m_debug_enable = debug_enable;
}

// Get the device list
QStringList UdpServer::deviceList()
{
    return m_device_list;
}

// Get the device list
QStringList UdpServer::deviceIPList()
{
    return m_device_ip_list;
}

// Response to device server
void UdpServer::response(const QHostAddress &address)
{
    m_udp_server.writeDatagram(cmd_response, sizeof(cmd_response), address, m_port);
}

// Start to lisen port
bool UdpServer::openLocalServer(QString addr, QString port)
{
    openServer(addr, port);

    return true;
}

// Open a udp server
bool UdpServer::openServer(QString &addr, QString &port)
{
    // Set host address
    QHostAddress hostAddress(addr);
    quint16 host_port = port.toInt();

    qDebug() << addr << ", " << port;

    // connect signals and slots
    if (m_udp_server.bind(hostAddress, host_port, QUdpSocket::ShareAddress))
    {
        connect(&m_udp_server, SIGNAL(readyRead()), this, SLOT(newData()));
        connect(&m_udp_server, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error()));
        connect(&m_timer, SIGNAL(timeout()), this, SLOT(onwaitTimeout()));

        qDebug()<<"Start success!";
        emit sendErrorMsg(QString("Start success!"));

        return true;
    } else // if error ocurrs
    {
        qDebug() << QString("%1, %2").arg(m_udp_server.error()).arg(m_udp_server.errorString());
        emit sendErrorMsg(QString("%1").arg(m_udp_server.errorString()));
        
        return false;
    }
}

// Close server
bool UdpServer::closeServer(void) {
    m_udp_server.close();
    m_udp_server.disconnect(this);
    m_timer.stop();
    m_timer.disconnect(this);

    qDebug() << "Connection closed.";

    return false;
}

// Clear data buffer
void UdpServer::clearBuffer() {
    array_read_data.clear();
    read_data_length = 0;
}

// data process
void UdpServer::newData() 
{
    QUdpSocket *socket = qobject_cast<QUdpSocket *>(sender());
    if (!socket) return;

    qint64 buf_length = socket->pendingDatagramSize();
    if (buf_length < 0) {
        qDebug() << "there is no available data";
    }

    // create buffers
    char buf[MAX_FRAME_LENGTH];
    QByteArray array_buf(MAX_FRAME_LENGTH, 'Q');
    QHostAddress device_address;
    quint16 device_port(0);

    qint64 io_length = socket->readDatagram(buf, buf_length, &device_address, &device_port);

    array_buf.resize(sizeof(io_length));
    memcpy(array_buf.data(), buf, io_length);
    array_read_data += array_buf;
    read_data_length = array_read_data.length(); 

    qDebug() << QDateTime::currentDateTime() << device_address.toString()
             << ":" << device_port << " " << array_buf;

    unsigned short head, command, length;
    unsigned char checkbit;

    head = (quint16(array_read_data.data()[0]) << 8) + array_read_data.data()[1];
    command = (quint16(array_read_data.data()[2]) << 8) + array_read_data.data()[3];
    length = (quint16(array_read_data.data()[4]) << 8) + array_read_data.data()[5];
    checkbit = array_read_data.data()[array_read_data.length() - 2];

    // If head is command
    if(head == 0x2400)
    {
        switch(command)
        {
            case 0x0100:    // Response of client
            {
                char name_char[3];
                name_char[0] = array_read_data.data()[7];
                name_char[1] = array_read_data.data()[6];
                name_char[2] = '\0';
                QString name =  QString(name_char);
                m_device_list.append(name);
                m_device_ip_list.append(device_address.toString());

                qDebug() << "AccessPoint: [" << name << "]===[" << device_address
                    << ":" << device_port << "]";
                qDebug() << m_device_list;
                emit deviceListChanged();
                break;
            }
            case 0x0101:    // Discover command from server
                if(m_debug_enable)
                {
                    response(device_address);
                }
            default:
                break;
        }
    } else if(device_address == m_device_ip && device_port == m_port)
    {
        // Wait for 50 ms
        m_timer.start(50);

        return;
    }

    // clear buffer
    clearBuffer();
}

// Parse the command or data received.
void UdpServer::parseData()
{
    unsigned short head, command, length;
    unsigned char checkbit;

    head = (quint16(array_read_data.data()[0]) << 8) + array_read_data.data()[1];
    command = (quint16(array_read_data.data()[2]) << 8) + array_read_data.data()[3];
    length = (quint16(array_read_data.data()[4]) << 8) + array_read_data.data()[5];
    checkbit = array_read_data.data()[array_read_data.length() - 2];

    // If head is data
    if(head == 0x2300)
    {
        switch(command)
        {
            case 0xA0A0:    // pixel data
                
                break;
            case 0xA0A1:    // List data

                break;
            default:
                break;
        }
    }

    // clear data buffer.
    clearBuffer();
}

// Check wether there is data when timeout.
void UdpServer::onwaitTimeout(void) 
{
    // Stop timer
    m_timer.stop();

    // emit signal
    emit dataCome();
    clearBuffer();
}

// Send data to a destination.
void UdpServer::sendToDst(QString &dst_ip, const QByteArray &bin)
{
    // create char buffer for send.
    char buf[MAX_FRAME_LENGTH];
    qint64 send_length = bin.length();
    memcpy(buf, bin.data(), send_length);

    qint64 write_length = 0;
    qint64 io_length = m_udp_server.writeDatagram(buf, send_length, QHostAddress(dst_ip),
        m_port);

    // If send not complete.
    while (io_length > 0) {
        write_length += io_length;

        io_length = (write_length >= send_length) ? 0 :
            m_udp_server.writeDatagram(buf + write_length, send_length - write_length, 
            QHostAddress(dst_ip), m_port);
    }

    // If error comes.
    if (write_length != send_length) {
        qDebug() << (QString("failed to send data to %1:%2 [%3]")
                .arg(dst_ip).arg(m_port).arg(write_length));
        emit sendErrorMsg(QString("failed to send data to %1:%2 [%3]")
                .arg(dst_ip).arg(m_port).arg(write_length));
        return;
    }
}

// Error process
void UdpServer::error()
{

}
/**
 * Title:   UdpServer.h
 * Author:  liulixing, PhD, 
            Tsinghua University
            liulx18@mails.tsinghua.edu.cn
 * Abstract:Udp server backend.
 */

#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QUdpSocket>
#include <QTimer>

#define MAX_FRAME_LENGTH    1024

/* Command List
        The BHCND command Packets format is "$ Address CMD_MSB CMD_LSB LEN_MSB
    LEN_LSB (*Data*) CHKSUM \n"
        The parameter is:
            ** 0x00 - 0x10
            2 bytes -- name
            4 bytes -- ip
            2 bytes -- port
            1 bytes -- start or stop
            7 bytes -- unused
            ** 0x10 -- 0x20
            2 bytes -- hv (not used)
            2 bytes -- corse gain (not used)
            2 bytes -- fine gain (not used)
            2 bytes -- shapping time
            2 bytes -- lower level discriminator
            2 bytes -- upper level discriminator (not used)
            2 bytes -- gate enable (not used)
            2 bytes -- unused

    Discover    		2400 0101 0000 24 0A
                            2400 0100 0002 'A''A' 27 0A
    Set parameter       2400 0103 0020 (parameter) ** 0A
                            ACK
    Get pixel data		2400 0201 0000 0A 0A
                            data
    Get phs data		2400 0203 0000 08 0A
                            data
    Get list data  		2400 0205 0000 0E 0A
                            data

    ACK List
        The BHCND acknowledge packets format is "$ Address CMD_MSB CMD_LSB
    LEN_MSB LEN_LSB (*Data*) CHKBIT 0A"
    Ok                  2400 FF00 0000 DB 0A
    Command Error       2400 FF01 0000 DA 0A
    Length Error        2400 FF02 0000 D9 0A
    Checksum Error      2400 FF03 0000 D8 0A

    Data List
        The MWPC data packets format is "# Address TYPE_MSB TYPE_LSB
    LEN_MSB LEN_LSB (*data*) CHKBIT 0A"
    pixel data  		2300 0200 **** (data) ** 0A	(pixel)
    phs data            2300 0202 **** (data) ** 0A	(phs)
    list data           2300 0204 **** (data) ** 0A	(list)
*/
class Device
{
public:
    QString name;
    QHostAddress address;
    quint16 port;
};

class UdpServer : public QObject
{
    Q_OBJECT

public:   
    UdpServer(QObject *parent = nullptr);
    static UdpServer* instance();
    ~UdpServer();

    void setBounded(bool bound);
    bool checkBounded(void);

    // variables and functions exposure to qml
    Q_PROPERTY(QStringList localIPList READ localIPList NOTIFY localIPListChanged)
    Q_PROPERTY(QStringList deviceList READ deviceList NOTIFY deviceListChanged)
    Q_PROPERTY(QStringList deviceIPList READ deviceIPList NOTIFY deviceIPListChanged)

    QStringList localIPList();
    QStringList deviceList();
    QStringList deviceIPList();
    void setDevice(const QString &device);

    Q_INVOKABLE bool openLocalServer(QString addr, QString port);
    Q_INVOKABLE bool openServer(QString &addr, QString &port);
    Q_INVOKABLE void initialize();
    Q_INVOKABLE void discover();
    Q_INVOKABLE void debugEnable(bool debug_enable);
    Q_INVOKABLE void response(const QHostAddress &address);
    Q_INVOKABLE void clearBuffer();
    Q_INVOKABLE bool closeServer();

signals:
    void localIPListChanged();
    void deviceListChanged();
    void deviceIPListChanged();
    void dataCome();
    void sendErrorMsg(const QString & msg);

private slots:
    void newData();
    void onwaitTimeout(void);
    void error();

private:
    QUdpSocket m_udp_server;
    QTimer m_timer;
    bool m_debug_enable = false;

    // Device* device[64];

    QStringList m_local_ip_list;
    QStringList m_device_list;
    QStringList m_device_ip_list;

    QString m_device = "Default";
    QHostAddress m_device_ip = QHostAddress("192.168.0.253");
    quint16 m_port = 2001;

    QByteArray array_read_data;
    int read_data_length;
    
    char cmd_discover[9] = {static_cast<char>(0x24),static_cast<char>(0x00),
			static_cast<char>(0x01),static_cast<char>(0x01),static_cast<char>(0x00),
			static_cast<char>(0x00),static_cast<char>(0x24),static_cast<char>(0x0A),'\0'};
    char cmd_response[11] = {static_cast<char>(0x24),static_cast<char>(0x00),
			static_cast<char>(0x01),static_cast<char>(0x00),static_cast<char>(0x00),
            static_cast<char>(0x02),static_cast<char>('A'),static_cast<char>('A'),
            static_cast<char>(0x27),static_cast<char>(0x0A),'\0'};
    char cmd_get_piexl[9] = {static_cast<char>(0x24),static_cast<char>(0x00),
			static_cast<char>(0x02),static_cast<char>(0x01),static_cast<char>(0x00),
			static_cast<char>(0x00),static_cast<char>(0x27),static_cast<char>(0x0A),'\0'};
    char cmd_get_phs[9] = {static_cast<char>(0x24),static_cast<char>(0x00),
			static_cast<char>(0x02),static_cast<char>(0x03),static_cast<char>(0x00),
			static_cast<char>(0x00),static_cast<char>(0x25),static_cast<char>(0x0A),'\0'};
    char cmd_get_list[9] = {static_cast<char>(0x24),static_cast<char>(0x00),
            static_cast<char>(0x02),static_cast<char>(0x03),static_cast<char>(0x00),
            static_cast<char>(0x00),static_cast<char>(0x25),static_cast<char>(0x0A),'\0'};

    void sendToDst(QString &name, const QByteArray &bin);
    void parseData();
};

#endif // UdpServer_H

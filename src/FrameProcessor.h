#ifndef FRAMEPROCESSOR_H
#define FRAMEPROCESSOR_H

#include "Frame.h"
#include "SocketWorker.h"
#include <QObject>
#include <QQueue>
#include <QFile>
#include <QtCharts/QAbstractSeries>
#include <QMutex>

QT_CHARTS_USE_NAMESPACE

/* Command List
        The BHCND command Packets format is "$ Address CMD_MSB CMD_LSB LEN_MSB
    LEN_LSB (*Data*) CHKSUM \n"
        The parameter is:
            ** 0x00 - 0x10
            2 bytes -- name
            4 bytes -- ip
            1 bytes -- mode(4 MSB: 0 - gate off; 1 - gate on;
                4 LSB: 0-pixel data; 1-list data)
            1 bytes -- channel
            2 bytes -- anode high voltage
            2 bytes -- lthd of anode
            2 bytes -- lthd of cathode
            1 bytes -- jitter_time (unit is 0.1us. The range is 0.1 - 1.6us.)
            1 byte  -- coin_time (unit is 0.1us. The range is 0.1 - 3.2us.)
            ** 0x10 -- 0x20
            2 bytes -- corse gain (not used)
            2 bytes -- fine gain (not used)
            2 bytes -- shapping time
            2 bytes -- unused
            2 bytes -- unused
            2 bytes -- unused
            2 bytes -- unused
            2 bytes -- unused

    Discover    		2400 0101 0000 24 0A
                            2400 0100 0010 (Settings) ** 0A
    Start       		2400 0103 0000 26 0A
                            ACK
    Stop        		2400 0102 0000 27 0A
                            ACK
    Set settings        2400 0105 0010 (settings) ** 0A
                            ACK
    Get pos data        2400 0201 0000 22 0A
                            pos data
    Get list data       2400 0203 0000 2C 0A

    ACK List
        The BHCND acknowledge packets format is "$ Address CMD_MSB CMD_LSB
    LEN_MSB LEN_LSB (*Data*) CHKBIT 0A"
    Ok                  2400 FF00 0000 DB 0A
    Command Error       2400 FF01 0000 DA 0A
    Length Error        2400 FF02 0000 D9 0A
    Checksum Error      2400 FF03 0000 D8 0A

    Data List
        The MWPC data packets format is "$ Address TYPE_MSB TYPE_LSB
    LEN_MSB LEN_LSB (*data*) CHKBIT 0A"
    pixel data  		2400 0200 **** (data) ** 0A	(pixel)
    list data           2400 0202 **** (data) ** 0A	(list)

    Data structure:
        pixel data: time(16bit) posA(8bit) posB(8bit)
        list data:  time(16bit) trigger_width(16bit) channel(8bit)
            time_beat_counts(24bits)
*/

// Setting struct.
struct Settings{
    char name[2];                   // 2 bytes
    unsigned int ip;                // 4 bytes
    unsigned char mode;             // 1 bytes
    unsigned char channel;          // 1 bytes
    unsigned short hv;              // 2 bytes
    unsigned short lthd_a;          // 2 bytes
    unsigned short lthd_b;          // 2 bytes
    unsigned char coin_time_a;      // 1 byte
    unsigned char coin_time_b;      // 1 byte
};

class Device
{
public:
    QString name;
    QHostAddress address;
    quint16 port;
};

class FrameProcessor : public QObject
{
    Q_OBJECT
public:
    explicit FrameProcessor(QQueue<Frame*> *frame_queue, QObject *parent = nullptr);

    static const quint16 CMD_DISCOVER           = 0x0101;
    static const quint16 CMD_DISCOVER_RESPONSE  = 0x0100;
    static const quint16 CMD_START              = 0x0103;
    static const quint16 CMD_STOP               = 0x0102;
    static const quint16 CMD_SET_SETTINGS       = 0x0105;
    static const quint16 CMD_DATA_POS           = 0x0200;
    static const quint16 CMD_DATA_LIST          = 0x0201;

    static constexpr float slop_factor = 0.88;
    static constexpr int MAX_FRAME_LENGTH = 8192;
    static constexpr int RESOLUTION_PWS = 4096 * 16;
    static constexpr int VCC = (int)(2500 * slop_factor);
    static constexpr int VEE = (int)(-2500 * slop_factor);
    static constexpr int RESOLUTION_THD = 1024;
    static constexpr int INTERCEPT = -30;

    // variables and functions exposure to qml
    Q_PROPERTY(QStringList localIPList READ localIPList NOTIFY localIPListChanged)
    Q_PROPERTY(QStringList deviceList READ deviceList NOTIFY deviceListChanged)
    Q_PROPERTY(QStringList deviceIPList READ deviceIPList NOTIFY deviceIPListChanged)

    QStringList localIPList();
    QStringList deviceList();
    QStringList deviceIPList();

    Q_INVOKABLE bool openServer(QString addr);
    Q_INVOKABLE bool closeServer(void);

    Q_INVOKABLE void scanDevice(void);
    Q_INVOKABLE void responseToDevice(QString addr);
    Q_INVOKABLE bool setDevice(unsigned char mode, int lthd_a, int lthd_b,
                               unsigned char coin_time_a, unsigned char coin_time_b,
                               QString addr);
    Q_INVOKABLE QVariantList readDeviceSettings(void);
    Q_INVOKABLE bool startDevice(QString addr);
    Q_INVOKABLE bool stopDevice(QString addr);

    Q_INVOKABLE int refreshChannelCountsX(QAbstractSeries *series);
    Q_INVOKABLE int refreshChannelCountsY(QAbstractSeries *series);

private:    
    QStringList m_local_ip_list;
    QStringList m_device_list;
    QStringList m_device_ip_list;

    QString m_device = "Default";
    QHostAddress m_device_ip = QHostAddress("192.168.0.3");
    quint16 m_port = 8080;

    Settings m_settings {
        {'A','A'},0xC0A80002,1,0,1000,320,320,100,100
    };

    QQueue<Frame*> *m_frame_queue;

    QVector<int> m_counts_x;
    QVector<int> m_counts_y;

    int m_debug_enable = 0;
    QMutex mutex;

signals:
    void localIPListChanged();
    void deviceListChanged();
    void deviceIPListChanged();

    bool openLocalServer(QString addr);
    bool closeLocalServer(void);
    void sendFrame(QString addr, Frame* frame);
    void autoSaveData();
    void stopSaveData();
    void updateChannelCountsX(QVector<int>* counts_x);
    void updateChannelCountsY(QVector<int>* counts_y);
    
public slots:
    void refreshLocalIP(QStringList local_ip_list);
    void FrameIncoming(QString device_ip, Frame *frame);

};
#endif // FRAMEPROCESSOR_H

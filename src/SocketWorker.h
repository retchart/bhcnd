#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include "Frame.h"
#include <QObject>
#include <QMutex>
#include <QUdpSocket>
#include <QQueue>
#include <QString>
#include <QVector>
#include <QFile>


class SocketWorker : public QObject
{
    Q_OBJECT
public:
    explicit SocketWorker(QQueue<Frame*> *frame_queue, QObject *parent = nullptr);
    ~SocketWorker();
    void requestWork();
    void abort();
    void updateLocalIP();

private:
    static const int RCV_ST_IDLE        = 0;
    static const int RCV_ST_CMD         = 1;
    static const int RCV_ST_DATA_LENGTH = 2;
    static const int RCV_ST_DATA        = 3;
    static const int RCV_ST_CHECKSUM    = 4;

    static const quint16 ADDRESS        = 0x2400;

    static const int MAX_FRAME_LENGTH   = 8192;

    static const quint16 CMD_DISCOVER           = 0x0101;
    static const quint16 CMD_DISCOVER_RESPONSE  = 0x0100;
    static const quint16 CMD_START              = 0x0103;
    static const quint16 CMD_STOP               = 0x0102;
    static const quint16 CMD_SET_SETTINGS       = 0x0105;
    static const quint16 CMD_DATA_POS           = 0x0200;
    static const quint16 CMD_DATA_LIST          = 0x0201;

    bool _abort;
    bool _working;

    QMutex mutex;
    QUdpSocket *m_udp_socket;
    QQueue<Frame*> *m_frame_queue;
    int m_receiver_status = RCV_ST_IDLE;
    Frame *m_in_frame = nullptr;
    quint16 m_data_length = 0, m_checksum = 0;
    int m_state_counts = 0;
    char m_data_buf[MAX_FRAME_LENGTH];
    
    // Device* device[64];
    QStringList m_local_ip_list;
    quint16 m_port = 8080;

    int m_debug_enable = 0;

    QFile m_data_file;
    QVector <QString> m_data_file_buffer;
    QTextStream m_data_file_stream;
    int m_frame_counts = 0;

    QVector<int> m_counts_x;
    QVector<int> m_counts_y;

    quint8 calculateChecksum(QByteArray buffer);
    void sendData(QString &dst_ip, const QByteArray &data_array);
    void parseFrameData(Frame *frame);
    void parseFrameData(char *frame_buffer, int frame_length);

signals:
    void checkLocalIP(QStringList local_ip_list);
    void frameReceived(QString addr, Frame *frame);

public slots:
    bool openLocalServer(QString addr);
    bool closeLocalServer(void);
    void sendFrame(QString addr, Frame* frame);
    void readPendingDiagrams();
    void autoSaveData();
    void stopSaveData();
    void updateChannelCountsX(QVector<int>* counts_x);
    void updateChannelCountsY(QVector<int>* counts_y);

};

#endif // SERIALWORKER_H

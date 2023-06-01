#ifndef FRAME_H
#define FRAME_H

#include <QObject>

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

class Frame : public QObject
{
    Q_OBJECT

public:

private:
    QByteArray m_buffer;


public:
    explicit Frame(QObject *parent = nullptr);
    explicit Frame(quint16 cmd, quint16 data_length, QObject *parent = nullptr);
    explicit Frame(quint16 cmd, quint16 data_length, QByteArray data, QObject *parent = nullptr);
     ~Frame();

    quint16  GetCmd();
    quint16  GetDataLength();
    quint8  CalculateChecksum();
    void    Clear();
    void    AddByte(quint8 data);
    void    AddByteArray(QByteArray data);
    QByteArray GetBuffer();

signals:

public slots:


};

#endif // FRAME_H

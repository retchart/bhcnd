#include "Frame.h"
#include <QDebug>


Frame::Frame(QObject *parent) : QObject(parent)
{

}

Frame::Frame(quint16 cmd, quint16 data_length, QObject *parent) : QObject(parent)
{
    m_buffer.resize(4);
   m_buffer[0] = quint8(cmd >> 8);
   m_buffer[1] = quint8(cmd);
   m_buffer[2] = quint8(data_length >> 8);
   m_buffer[3] = quint8(data_length);
}

Frame::Frame(quint16 cmd, quint16 data_length, QByteArray data, QObject *parent) : QObject(parent)
{
    m_buffer.resize(4 + data.count());
   m_buffer[0] = quint8(cmd >> 8);
   m_buffer[1] = quint8(cmd);
   m_buffer[2] = quint8(data_length >> 8);
   m_buffer[3] = quint8(data_length);

    if(data.count() > 0)
    {
        for(int i = 0; i < data.count(); i++)
        {
            m_buffer[i + 4] = quint8(data[i]);
        }
    }
}


Frame::~Frame()
{
    m_buffer.clear();
}

quint16 Frame::GetCmd()
{
    quint16 cmd = 0;
    if(m_buffer.count() >= 2)
        cmd = (quint16(m_buffer[0]) << 8)
                + quint16(m_buffer[1]);
    return cmd;
}

quint16  Frame::GetDataLength()
{
    quint16 data_length = 0;
    if(m_buffer.count() >= 4)
        data_length = (quint16(m_buffer[2]) << 8)
                + quint16(m_buffer[3]);
    return data_length;
}

quint8  Frame::CalculateChecksum()
{
    quint8 rv = 0;
    for (int i = 0; i < m_buffer.count(); i++)
    {
        rv += quint8(m_buffer[i]);
    }
    return rv;
}

void Frame::Clear()
{
    m_buffer.clear();
}

void Frame::AddByte(quint8 data)
{
    m_buffer.append(data);
}

void Frame::AddByteArray(QByteArray data)
{
    m_buffer.append(data);
}

QByteArray Frame::GetBuffer()
{
    return m_buffer;
}

#ifndef COMUTILS_H
#define	COMUTILS_H

#include <array>
#include <functional>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

#include <QObject>
#include <QString>
QT_USE_NAMESPACE

class SerialPort;

//------------------------------------------------------------------------------
class DeviceLink : public QObject
{
    Q_OBJECT

    DeviceLink()                              = delete;
    DeviceLink            (const DeviceLink&) = delete;
    DeviceLink& operator =(const DeviceLink&) = delete;
public:
    enum class DeviceType {
        UNKNOWN = 0,
        LDU, RDU, MDU,
        NUM_TYPES // the last
    };
    typedef std::array<uint8_t, 26>  packet_t; // 16 = PACKET_SIZE
    typedef std::function<void(const QString&)>  MsgToDisplayCbk;
    typedef std::function<void(const packet_t&)> PacketRecvdCbk;
    typedef std::function<void()>                SeriesEndCbk;
    typedef std::function<void(bool)>            FinishedCbk;

    DeviceLink(MsgToDisplayCbk msgCbk, PacketRecvdCbk packetCbk, 
               SeriesEndCbk seriesEndCbk, FinishedCbk finishCbk);

    void DownloadRecorded(); // returns packets via PacketRecvdCbk
    void ClearRecordings();
    void UploadToDevice(const std::vector<uint8_t>& data);

private:
    void DownloadThreadFn();
    void ClearThreadFn();
    void UploadThreadFn();

    std::unique_ptr<SerialPort> EstablishConnection(size_t& port_idx, uint8_t& handshakeReply);

    void SendMessage(const QString& msg) const { if (m_messageCbk)     m_messageCbk(msg); }
    void ReceivePacket (const packet_t& p)const{ if (m_packetRecvdCbk) m_packetRecvdCbk(p); }
    void SendFinished(bool ok)           const { if (m_finishCbk)      m_finishCbk(ok); }

    typedef std::vector<uint8_t> dataVec_t;

    template<size_t N> static bool ChecksumOk(uint8_t* data) {
        size_t sum = 0;
        for (size_t i = 0;  i + 1 < N;  ++i) {
            sum += data[i];
        }
        const bool ok = ((sum & 0xFF) == data[N - 1]);
        return ok;
    }

//data:
    MsgToDisplayCbk m_messageCbk;
    PacketRecvdCbk  m_packetRecvdCbk; 
    SeriesEndCbk    m_seriesEndCbk;
    FinishedCbk     m_finishCbk;

    std::vector<QString> m_portNames;

    DeviceType m_deviceType = DeviceType::UNKNOWN;
    int m_firmwareVersionX100 = 0; // unknown

//statics const:
    static const std::array<uint8_t,2> HANDSHAKE;            // "DZ"
    static const std::array<uint8_t,2> START_DOWNLOAD;       // "DK"
    static const std::array<uint8_t,1> CONTINUE_DOWNLOAD;    // 0x01
    static const std::array<uint8_t,2> CLEAR_VALUES;         // "DE" - not sure what it does...
    static const std::array<uint8_t,2> CLEAR_MEMORY;         // "DB" 
    static const std::array<uint8_t,2> GET_FIRMWARE_VERSION; // "DV" 

    static const std::array<uint8_t,5> HANDSHAKE_REPLY;

    static const std::array<QString, (size_t)DeviceType::NUM_TYPES> DEVICE_NAME;
};
//------------------------------------------------------------------------------

#endif // COMUTILS_H

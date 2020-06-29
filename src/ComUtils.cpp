// On Mac, install FTDI driver from here:
// http://www.ftdichip.com/Drivers/VCP/MacOSX/FTDIUSBSerialDriver_v2_2_18.dmg

#include "ComUtils.h"

#include <assert.h>
#include <iostream>
#include <iomanip>

#include <QDebug>
#include <QThread>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
QT_USE_NAMESPACE

//==============================================================================
//static 
const std::array<uint8_t, 2> DeviceLink::HANDSHAKE            = { 'D','Z' };
const std::array<uint8_t, 2> DeviceLink::START_DOWNLOAD       = { 'D','K' };
const std::array<uint8_t, 1> DeviceLink::CONTINUE_DOWNLOAD    = { 0x01 };
const std::array<uint8_t, 2> DeviceLink::CLEAR_VALUES         = { 'D','E' };
const std::array<uint8_t, 2> DeviceLink::CLEAR_MEMORY         = { 'D','B' };
const std::array<uint8_t, 2> DeviceLink::GET_FIRMWARE_VERSION = { 'D','V' };

const std::array<uint8_t, 5> DeviceLink::HANDSHAKE_REPLY = {
    0xFA, // MDU: No Recording sessions available
    0xFB, // LDU (and do no data, as it has no on-board memory)
    0xFD, // RDU : No Recording sessions available

    0xFC, // RDU : Recording session(s) present, ready for download
    0xFE  // MDU : Recording session(s) present, ready for download
};

//==============================================================================
class SerialPort
{
    SerialPort(const SerialPort&)             = delete;
    SerialPort& operator =(const SerialPort&) = delete;
    SerialPort()                              = delete;
public:
    SerialPort(const QString& port_name, int baud_rate, int timeout, int buffer_size = 0)
        : m_timeout(timeout)
        , m_port(new (std::nothrow) QSerialPort(port_name), 
                 [](QSerialPort* p){ if(p) p->close(); delete p; })
    {
        m_port->setBaudRate(baud_rate);

        // Hyperion port settings
        m_port->setDataBits   (QSerialPort::Data8);
        m_port->setParity     (QSerialPort::NoParity);
        m_port->setStopBits   (QSerialPort::OneStop);
        m_port->setFlowControl(QSerialPort::NoFlowControl);
        
        if (buffer_size > 0) {
            m_port->setReadBufferSize(buffer_size);
        }

        m_port->clear();
    }

    bool open() {
        return m_port->open(QIODevice::ReadWrite);
    }

    template<size_t N> bool write(const std::array<uint8_t, N>& data) {
        bool ok = false;
        auto bytes_written = m_port->write(reinterpret_cast<const char*>(&data[0]), N);
        if ((ok = (bytes_written == N)) == true) {
            ok = m_port->waitForBytesWritten(m_timeout);
        }
        return ok;
    }

    bool read(uint8_t* data, size_t size, bool* timedOut = nullptr) {
        bool ok = false;
        if ((ok = m_port->waitForReadyRead(m_timeout)) == true)
        {
            QByteArray receivedData = m_port->readAll();
            while (m_port->waitForReadyRead(20)) {
                receivedData += m_port->readAll();
            }
            if ((ok = (static_cast<int>(size) >= receivedData.size())) == true) {
                memcpy(data, receivedData.constData(), receivedData.size());
                if (timedOut != nullptr) {
                    *timedOut = false;
                }
           }
        } else { // waitForReadyRead timed out
            if (timedOut != nullptr) {
                *timedOut = true;
            }
        }
        return ok;
    }
private:
    const int m_timeout;
    std::unique_ptr<QSerialPort, void(*)(QSerialPort*)> m_port;
};
//==============================================================================
class WorkerThread : public QThread
{
public:
    static void Start(QObject* parent, std::function<void()> tf) {
        if (WorkerThread* thr = new (std::nothrow) WorkerThread(parent, tf)) {
            connect(thr, &WorkerThread::finished, thr, &QObject::deleteLater);
            thr->start();
        }
    }
private:
    WorkerThread(QObject* parent, std::function<void()> tf) : QThread(parent), m_threadFn(tf) {}
    virtual void run() Q_DECL_OVERRIDE {
        if (m_threadFn)
            m_threadFn();
    }
//data:
    std::function<void()> m_threadFn;
};
//==============================================================================
DeviceLink::DeviceLink(DeviceLink::MsgToDisplayCbk msgCbk, 
                       DeviceLink::PacketRecvdCbk  packetCbk,
                       DeviceLink::SeriesEndCbk    seriesEndCbk,
                       DeviceLink::FinishedCbk     finishCbk)
    : m_messageCbk    (msgCbk)
    , m_packetRecvdCbk(packetCbk)
    , m_seriesEndCbk  (seriesEndCbk)
    , m_finishCbk     (finishCbk)
{
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    m_portNames.reserve(ports.size());
    for (const QSerialPortInfo& info : ports) {
        m_portNames.emplace_back(info.portName());
    }
}
//------------------------------------------------------------------------------
void DeviceLink::DownloadRecorded()
{
    WorkerThread::Start(this, [this]{ DownloadThreadFn(); });
}
//------------------------------------------------------------------------------
void DeviceLink::ClearRecordings()
{
    WorkerThread::Start(this, [this]{ ClearThreadFn(); });
}
//------------------------------------------------------------------------------
void DeviceLink::UploadToDevice(const std::vector<uint8_t>&)// data)
{
    // TBD
}
//------------------------------------------------------------------------------
void DeviceLink::DownloadThreadFn()
{
    bool ok = false;
    auto return_ok_on_exit = gsl::finally([this, &ok](){ if (m_finishCbk) m_finishCbk(ok); });

    // Find a compatible device
    SendMessage(tr("Looking for a device..."));
    size_t  port_idx = 0;
    uint8_t handshakeReply = 0; // invalid
    std::unique_ptr<SerialPort> com = EstablishConnection(port_idx, handshakeReply);

    if (com.get() == nullptr || port_idx > m_portNames.size()) {
        SendMessage(tr("Failed to connect"));
        return; // connection timed out
    }
    
    const QString portName = m_portNames[port_idx];

    bool hasData = false;
    if (handshakeReply == 0xFA) {
        m_deviceType = Hyperion::MDU;
    } else if (handshakeReply == 0xFB) {
        m_deviceType = Hyperion::LDU;
    } else if (handshakeReply == 0xFD) {
        m_deviceType = Hyperion::RDU;
    } else if ((hasData = (handshakeReply == 0xFC)) == true) {
        m_deviceType = Hyperion::RDU;
    } else if ((hasData = (handshakeReply == 0xFE)) == true) {
        m_deviceType = Hyperion::MDU;
    } else {
        m_deviceType = Hyperion::UNKNOWN_DEVICE;
    }

    // Get firmware version
    if (m_deviceType != Hyperion::UNKNOWN_DEVICE) {
        if (com->write(GET_FIRMWARE_VERSION)) {
            dataVec_t reply(1, 0);
            if (com->read(&reply[0], 1)) {
                m_firmwareVersionX100 = reply[0];
                SendMessage(tr("Detected %1 v%2.%3%4 on %5\n")
                            .arg(Hyperion::DEVICE_NAME[m_deviceType])
                            .arg(m_firmwareVersionX100 / 100)
                            .arg(m_firmwareVersionX100 % 100, 2, 10, QChar('0'))
                            .arg((hasData) ? " with data" : "")
                            .arg(portName));
            }
        }
    }
 
    if (!hasData) {
        SendMessage(tr("%1 has no data").arg(Hyperion::DEVICE_NAME[m_deviceType]));
        return; // noting to do
    }

    try
    {
        // Connected => download
        for (size_t sessionIdx = 0; ; ++sessionIdx)
        {
            try
            {
                if ((ok = com->write(HANDSHAKE)) == true)
                {
                    dataVec_t reply(1, 0);
                    if ((ok = com->read(&reply[0], 1)) == true)
                    {
                        const uint8_t& r = reply[0];
                        handshakeReply = ((ok = std::any_of(std::begin(HANDSHAKE_REPLY), std::end(HANDSHAKE_REPLY), [&](uint8_t i) { return i == r; })) == true)
                            ? r
                            : 0; // invalid
                    }
                }
            } catch (...) { // something is wrong
                ok = false;
                SendMessage(tr("Error reading session %1 from %2").arg(sessionIdx).arg(portName));
                break;
            }

            if (!ok || (handshakeReply != 0xFC && handshakeReply != 0xFE)) {
                SendMessage(tr("No more data on the device"));
                break;
            }

            ok = true;
            for (bool firstPass=true, timedOut=false;  ok && !timedOut;  firstPass = false)
            {
                try {
                    ok = (firstPass)? com->write(START_DOWNLOAD) : com->write(CONTINUE_DOWNLOAD);
                    if (ok) {
                        std::array<uint8_t, Hyperion::RECORD_LENGTH + 1> reply;
                        reply.fill( 0 );
                        if ((ok = com->read(&reply[0], Hyperion::RECORD_LENGTH + 1, &timedOut)) == true)
                        {
                            // Looks like QtSerialPort bug: the last two bytes are swapped...
                            std::swap(reply[Hyperion::RECORD_LENGTH-1], reply[Hyperion::RECORD_LENGTH]);

                            if ((ok = ChecksumOk(reply)) == true) {
                                ReceivePacket({ &reply[0], static_cast<int64_t>(Hyperion::RECORD_LENGTH) });
                            } else {
                                SendMessage(tr("Corrupt data (CRC failure)"));
                            }
                        } else { // ok == false, i.e. read failed
                            if (timedOut) {
                                ok = true;  // timeout is normal at the end of a recording
                                if (m_seriesEndCbk) { m_seriesEndCbk(); }
                                SendMessage(tr("Finished downloading recording #%1\n").arg(sessionIdx + 1));
                            }  // it'll break out of the loop
                        }
                    }
                } catch (...) {
                    ok = false;
                }
            }
            //break; // <<< DEBUG only download the 1st recording
        }
    } catch(...) {
        // "Error: " << e.what()
    }
    /*ok =*/ //com->write(CLEAR_MEMORY);
    SendMessage(tr("Finished downloading the data\n"));
}
//------------------------------------------------------------------------------
void DeviceLink::ClearThreadFn()
{
    bool ok = false;
    auto return_ok_on_exit = gsl::finally([this, &ok](){ if (m_finishCbk) m_finishCbk(ok); });

    // Find a compatible device
    SendMessage(tr("Looking for a device..."));
    size_t  port_idx = 0;
    uint8_t handshakeReply = 0; // invalid
    std::unique_ptr<SerialPort> com = EstablishConnection(port_idx, handshakeReply);

    if (com.get() == nullptr || port_idx > m_portNames.size()) {
        SendMessage(tr("Failed to connect"));
        return; // connection timed out
    }
    
    const QString portName = m_portNames[port_idx];

    bool hasData = false;
    if (handshakeReply == 0xFA) {
        m_deviceType = Hyperion::MDU;
    } else if (handshakeReply == 0xFB) {
        m_deviceType = Hyperion::LDU;
    } else if (handshakeReply == 0xFD) {
        m_deviceType = Hyperion::RDU;
    } else if ((hasData = (handshakeReply == 0xFC)) == true) {
        m_deviceType = Hyperion::RDU;
    } else if ((hasData = (handshakeReply == 0xFE)) == true) {
        m_deviceType = Hyperion::MDU;
    } else {
        m_deviceType = Hyperion::UNKNOWN_DEVICE;
    }

    // Get firmware version
    if (m_deviceType != Hyperion::UNKNOWN_DEVICE) {
        if (com->write(GET_FIRMWARE_VERSION)) {
            dataVec_t reply(1, 0);
            if (com->read(&reply[0], 1)) {
                m_firmwareVersionX100 = reply[0];
                SendMessage(tr("Detected %1 v%2.%3%4 on %5\n")
                            .arg(Hyperion::DEVICE_NAME[m_deviceType])
                            .arg(m_firmwareVersionX100 / 100)
                            .arg(m_firmwareVersionX100 % 100, 2, 10, QChar('0'))
                            .arg((hasData) ? " with data" : "")
                            .arg(portName));
            }
        }
    }
 
    if (!hasData) {
        SendMessage(tr("%1 has no data").arg(Hyperion::DEVICE_NAME[m_deviceType]));
        return; // noting to do
    }

    try
    {
        // Connected => erase
        do
        {
            try
            {
                if ((ok = com->write(HANDSHAKE)) == true)
                {
                    dataVec_t reply(1, 0);
                    if ((ok = com->read(&reply[0], 1)) == true)
                    {
                        const uint8_t& r = reply[0];
                        handshakeReply = ((ok = std::any_of(std::begin(HANDSHAKE_REPLY), std::end(HANDSHAKE_REPLY), [&](uint8_t i) { return i == r; })) == true)
                            ? r
                            : 0; // invalid
                    }
                }
            } catch (...) { // something is wrong
                ok = false;
                SendMessage(tr("Error erasing the data"));
                break;
            }

            if (!ok || (handshakeReply != 0xFC && handshakeReply != 0xFE)) {
                SendMessage(tr("No more data on the device"));
                break;
            }

            ok = true;
            try {
                ok = com->write(CLEAR_MEMORY);
            } catch (...) {
                ok = false;
            }
            if (ok) {
                if (com->write(GET_FIRMWARE_VERSION)) {
                    dataVec_t reply(1, 0);
                    if (com->read(&reply[0], 1)) {} // do nothing 
                }
            }
        } while (false);
    } catch(...) {
        // "Error: " << e.what()
    }
    ok = false; // so that the data header doesn't get shown
    SendMessage(tr("Finished erasing the data\n"));
}
//------------------------------------------------------------------------------
void DeviceLink::UploadThreadFn()
{
    // TBD
}
//------------------------------------------------------------------------------
std::unique_ptr<SerialPort> DeviceLink::EstablishConnection(size_t& port_idx, uint8_t& handshakeReply)
{
    const size_t N_PORTS = m_portNames.size();

    typedef std::pair<size_t, SerialPort*> idx_ptr_pair;
    std::vector<idx_ptr_pair> ports;  
    ports.reserve(N_PORTS);

    // First, gather all the available ports
    for (size_t prt_idx = 0;  prt_idx < N_PORTS;  ++prt_idx)
    {
        SerialPort* port_ptr = nullptr;
        try  {
            if ((port_ptr = new (std::nothrow) SerialPort(m_portNames[prt_idx], 115200, 500)) != nullptr) { // "500" = timeout 0.5 sec
                if (port_ptr->open()) {
                    ports.emplace_back(prt_idx, port_ptr);
                } else {
                    delete port_ptr; 
                }
            }
        } catch (...) {
            delete port_ptr;
        }
    }

    if (ports.empty()) {
        if (m_messageCbk) { 
            m_messageCbk(tr("Error: USB adapter not connected..."));
        }
        return nullptr;
    }
    
    // Next, try all these port (make several attempts, if found several ports)
    const size_t N_ports = ports.size();
    const size_t N_attempts        = (N_ports == 1)? 1 : 20; // each timeout is 0.5 sec
    const size_t N_timout_attempts = (N_ports == 1)? 20 : 1;
    SerialPort* serial_port = nullptr; // to be returned 
    for (size_t attempt=0;  serial_port==nullptr && attempt < N_attempts;  ++attempt)
    {
        for (const auto& idx_port : ports) 
        {
            port_idx    = idx_port.first;
            serial_port = idx_port.second;

            if (serial_port != nullptr)
            {
                SendMessage(tr("Waiting on %1...").arg(m_portNames[port_idx]));

                // Since the port may not replay right away, try sending handshake after each timeout
                bool reply_good = false;
                for (size_t to_attempt=0;  to_attempt < N_timout_attempts && !reply_good;  ++to_attempt)
                {
                    try 
                    {
                        const bool sentOk = serial_port->write(HANDSHAKE);
                        if (sentOk)
                        {
                            dataVec_t reply(1, 0);
                            const bool recvdOk = serial_port->read(&reply[0], 1);
                            if (recvdOk)
                            {
                                const uint8_t& r = reply[0];
                                reply_good = std::any_of(std::begin(HANDSHAKE_REPLY), std::end(HANDSHAKE_REPLY), 
                                                         [&](uint8_t i) { return i == r; });
                                if (reply_good) {
                                    handshakeReply = r;
                                    break;
                                }
                                // if reply_good, will break out of loop
                            }
                        }
                    } catch (...) { // something is wrong
                        serial_port = nullptr;
                        SendMessage(m_portNames[port_idx] + tr(": failed"));
                        break;
                    } 
                } // end of timout_attempt loop
                if (!reply_good) {
                    serial_port = nullptr;
                }
            }

            if (serial_port != nullptr) {
                break;
            }
        } // end of ports loop
    } // end of attempt loop

    // Delete the not needed ports 
    for (const auto& idx_port : ports) {
        if (serial_port != idx_port.second) {
            delete idx_port.second;
        }
    }

    return std::unique_ptr<SerialPort>(serial_port);
}
//------------------------------------------------------------------------------

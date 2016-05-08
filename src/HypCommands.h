#pragma once
#include <array>
#include <stdint.h>

namespace Hyperion 
{

const size_t RECORD_LENGTH = 26;

enum DeviceType {
    UNKNOWN_DEVICE = 0,
    LDU, RDU, MDU,
    NUM_DEVICE_TYPES // the last
};

const std::array<const char*, NUM_DEVICE_TYPES> DEVICE_NAME = {
    "UNKNOWN", "LDU", "RDU", "MDU"
};

const std::array<uint8_t, 2> HANDSHAKE            = { 'D','Z' };
const std::array<uint8_t, 2> START_DOWNLOAD       = { 'D','K' };
const std::array<uint8_t, 1> CONTINUE_DOWNLOAD    = { 0x01 };
const std::array<uint8_t, 2> CLEAR_VALUES         = { 'D','E' };
const std::array<uint8_t, 2> CLEAR_MEMORY         = { 'D','B' };
const std::array<uint8_t, 2> GET_FIRMWARE_VERSION = { 'D','V' };

const std::array<uint8_t, 5> HANDSHAKE_REPLY = {
    0xFA, // MDU: No Recording sessions available
    0xFB, // LDU (and do no data, as it has no on-board memory)
    0xFD, // RDU : No Recording sessions available

    0xFC, // RDU : Recording session(s) present, ready for download
    0xFE  // MDU : Recording session(s) present, ready for download
};

}
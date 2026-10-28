// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    
//
// Includes
//

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



//
// Defines
//

/** Callsign characters in one address field (space padded on the air). */
#define UNI_AX25_CALLSIGN_SIZE      (6U)

/** Size of one encoded address field: callsign and SSID byte. */
#define UNI_AX25_ADDRESS_SIZE       (UNI_AX25_CALLSIGN_SIZE + 1U)

/** Destination, source and up to 8 repeaters. */
#define UNI_AX25_PEERS_MAXCOUNT     (10U)



//
// Typedefs
//

typedef enum {
    UNI_AX25_PID_ISO8208_PLP            = 0x01,
    UNI_AX25_PID_RFC1144_COMPRESSED     = 0x06,
    UNI_AX25_PID_RFC1144_UNCOMPRESSED   = 0x07,
    UNI_AX25_PID_SEGMENTATIION_FRAGMENT = 0x08,
    UNI_AX25_PID_TEXNET                 = 0xC3,
    UNI_AX25_PID_LINK_QUALITY           = 0xC4,
    UNI_AX25_PID_APPLETALK              = 0xCA,
    UNI_AX25_PID_APPLETALK_ARP          = 0xCB,
    UNI_AX25_PID_ARPA_IP                = 0xCC,
    UNI_AX25_PID_ARPA_ARP               = 0xCD,
    UNI_AX25_PID_FLEXNET                = 0xCE,
    UNI_AX25_PID_NETROM                 = 0xCF,
    UNI_AX25_PID_NO_L3                  = 0xF0,
    UNI_AX25_PID_ESCAPE                 = 0xFF,
} uni_ax25_pid_e;


typedef enum {
    UNI_AX25_CONTROL_UI = 0x03
} uni_ax25_control_e;


typedef enum {
    APP_AX25_PEER_DST        = 0,
    APP_AX25_PEER_SRC        = 1,
    APP_AX25_PEER_REPEATER_1 = 2,
    APP_AX25_PEER_REPEATER_2 = 3,
    APP_AX25_PEER_REPEATER_3 = 4,
    APP_AX25_PEER_REPEATER_4 = 5,
    APP_AX25_PEER_REPEATER_5 = 6,
    APP_AX25_PEER_REPEATER_6 = 7,
    APP_AX25_PEER_REPEATER_7 = 8,
    APP_AX25_PEER_REPEATER_8 = 9,
} uni_ax25_peer_e;


typedef struct {
    /**
     * Peer callsign without the space padding, NUL-terminated (the last byte is always NUL after
     * uni_ax25_decode(); encoding stops at the first NUL)
     */
    char callsign[UNI_AX25_CALLSIGN_SIZE + 1U];

    /**
     * Peer SSID
     */
    uint8_t ssid;

    /**
      * C/H bit
     */
    bool ch_bit;
} uni_ax25_peer_t;


/**
 * AX.25 message header
 */
typedef struct {
    /**
     * Peers
     */
    uni_ax25_peer_t peers[UNI_AX25_PEERS_MAXCOUNT];

    /**
     * Control field
     */
    uint8_t control;

    /**
     * Protocol ID
     */
    uint8_t pid;
} uni_ax25_data_t;


//
// Functions
//

/**
 * Decode the AX.25 address field followed by one control byte and one PID byte (UI / I frame
 * header with modulo-8 control field).
 *
 * The address field must hold 2 to UNI_AX25_PEERS_MAXCOUNT addresses, and only the last one may
 * have the extension bit set. @p ax25_header is cleared first; unused peers stay empty. Its
 * contents are unspecified on failure.
 *
 * @return header size in bytes (offset of the information field), or SIZE_MAX when the address
 *         field is malformed or @p data is too short
 */
size_t uni_ax25_decode(const uint8_t *data, size_t data_len, uni_ax25_data_t *ax25_header);

/**
 * Encode the address field, control byte and PID byte.
 *
 * @return header size in bytes, or SIZE_MAX when an argument is invalid or the header does not fit
 */
size_t uni_ax25_encode(const uni_ax25_data_t *ax25_header, uint8_t *data, size_t data_len);

/**
 * Encode the address field only: destination, source and the repeaters up to the first one with
 * an empty callsign. The extension bit is set on the last encoded address only.
 *
 * @return address field size in bytes, or SIZE_MAX when an argument is invalid or the whole address
 *         field does not fit (a truncated address field is never produced)
 */
size_t uni_ax25_encode_address(const uni_ax25_data_t *ax25_header, uint8_t *data, size_t data_len);

/**
 * Find @p callsign (NUL-terminated, at most UNI_AX25_CALLSIGN_SIZE characters, SSID ignored).
 *
 * @return peer index, or SIZE_MAX when not found or @p callsign is empty or too long
 */
size_t uni_ax25_contains_callsign(const uni_ax25_data_t *ax25_header, const char* callsign);

#ifdef __cplusplus
}
#endif

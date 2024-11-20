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

#define UNI_AX25_CALLSIGN_SIZE      (6U)
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
     * Peer callsign
     */
    char callsign[UNI_AX25_CALLSIGN_SIZE];

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

size_t uni_ax25_decode(const uint8_t *data, size_t data_len, uni_ax25_data_t *ax25_header);

size_t uni_ax25_encode(const uni_ax25_data_t *ax25_header, uint8_t *data, size_t data_len);

size_t uni_ax25_encode_address(const uni_ax25_data_t *ax25_header, uint8_t *data, size_t data_len);

size_t uni_ax25_contains_callsign(const uni_ax25_data_t *ax25_header, const char* callsign);

#ifdef __cplusplus
}
#endif

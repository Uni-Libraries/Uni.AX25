// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

//
// Includes
//

// stdlib
#include <string.h>

// Uni.AX25
#include "uni_ax25.h"



//
// Functions
//

size_t uni_ax25_decode(const uint8_t *data, size_t data_len, uni_ax25_data_t *ax25_header) {
    size_t result = SIZE_MAX;
    size_t off = 0U;

    if (data != NULL && ax25_header != NULL) {
        for (size_t peer_idx = 0; peer_idx < UNI_AX25_PEERS_MAXCOUNT; peer_idx++) {
            // check for size
            if (off + UNI_AX25_CALLSIGN_SIZE + 1 > data_len) {
                break;
            }

            // check for repeaters
            if (peer_idx > 1 && (data[off - 1] & 0x01)) {
                break;
            }

            // parse callsign
            for (size_t idx = 0; idx < UNI_AX25_CALLSIGN_SIZE; idx++) {
                uint8_t byte = data[off++] >> 1;
                if (byte == ' ') {
                    continue;
                }
                ax25_header->peers[peer_idx].callsign[idx] = byte;
            }

            // parse ssid
            ax25_header->peers[peer_idx].ssid = (data[off] >> 1) & 0x0FU;
            ax25_header->peers[peer_idx].ch_bit = (data[off] >> 7);
            off++;
        }

        // parse control and pid
        if (off + 2 <= data_len) {
            ax25_header->control = data[off++];
            ax25_header->pid = data[off++];
            result = off;
        }
    }

    return result;
}


size_t uni_ax25_encode(const uni_ax25_data_t *ax25_header, uint8_t *data, size_t data_len) {
    size_t result = SIZE_MAX;

    size_t off = uni_ax25_encode_address(ax25_header, data, data_len);
    if (off != SIZE_MAX) { // parse control and pid
        if (off + 2 <= data_len) {
            data[off++] = ax25_header->control;
            data[off++] = ax25_header->pid;
            result = off;
        }
    }

    return result;
}


size_t uni_ax25_encode_address(const uni_ax25_data_t *ax25_header, uint8_t *data, size_t data_len) {
    size_t result = SIZE_MAX;
    size_t off = 0U;

    if (ax25_header != NULL && data != NULL) {
        for (size_t peer_idx = 0; peer_idx < UNI_AX25_PEERS_MAXCOUNT; peer_idx++) {
            // check for size
            if (off + UNI_AX25_CALLSIGN_SIZE + 1 > data_len) {
                break;
            }

            // check for repeaters
            if (peer_idx > 1 && ax25_header->peers[peer_idx].callsign[0] == '\0') {
                break;
            }

            // write callsign
            for (size_t idx = 0; idx < UNI_AX25_CALLSIGN_SIZE; idx++) {
                uint8_t byte = ax25_header->peers[peer_idx].callsign[idx];
                if (!byte) {
                    byte = ' ';
                }
                data[off++] = byte << 1;
            }

            // write ssid
            uint8_t ssid = (1 << 6) | (1 << 5);                      // reserved bits
            ssid |= (ax25_header->peers[peer_idx].ssid & 0x0F) << 1; // SSID
            if (ax25_header->peers[peer_idx].ch_bit) {
                ssid |= (1 << 7); // C/H bit
            }
            if (peer_idx == UNI_AX25_PEERS_MAXCOUNT - 1 || ax25_header->peers[peer_idx + 1].callsign[0] == '\0') {
                ssid |= (1 << 0); // last callsign bit
            }

            data[off++] = ssid;
            result = off;
        }
    }

    return result;
}


size_t uni_ax25_contains_callsign(const uni_ax25_data_t *ax25_header, const char *callsign) {
    size_t result = SIZE_MAX;

    if (ax25_header != NULL && callsign != NULL) {
        for (size_t peer_idx = 0; peer_idx < UNI_AX25_PEERS_MAXCOUNT; peer_idx++) {
            if (memcmp(ax25_header->peers[peer_idx].callsign, callsign, UNI_AX25_CALLSIGN_SIZE) == 0) {
                result = peer_idx;
                break;
            }
        }
    }

    return result;
}

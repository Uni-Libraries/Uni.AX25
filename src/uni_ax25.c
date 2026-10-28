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
// Defines
//

#define UNI_AX25_SSID_EXTENSION_BIT (0x01U) // last address of the address field
#define UNI_AX25_SSID_RESERVED_BITS (0x60U)
#define UNI_AX25_SSID_CH_BIT        (0x80U)



//
// Private
//

static size_t uni_ax25_callsign_length(const char *callsign, size_t max_len) {
    size_t len = 0U;
    while (len < max_len && callsign[len] != '\0') {
        len++;
    }
    return len;
}


static size_t uni_ax25_peer_count(const uni_ax25_data_t *ax25_header) {
    // destination and source are always present; repeaters end at the first empty callsign
    size_t count = 2U;
    while (count < UNI_AX25_PEERS_MAXCOUNT && ax25_header->peers[count].callsign[0] != '\0') {
        count++;
    }
    return count;
}



//
// Functions
//

size_t uni_ax25_decode(const uint8_t *data, size_t data_len, uni_ax25_data_t *ax25_header) {
    size_t off = 0U;
    size_t peer_count = 0U;
    bool last = false;

    if (data == NULL || ax25_header == NULL) {
        return SIZE_MAX;
    }
    memset(ax25_header, 0, sizeof(*ax25_header));

    while (!last && peer_count < UNI_AX25_PEERS_MAXCOUNT) {
        uni_ax25_peer_t *peer = &ax25_header->peers[peer_count];

        if (data_len - off < UNI_AX25_ADDRESS_SIZE) {
            return SIZE_MAX;
        }

        // callsign characters are shifted left by one bit and padded with spaces
        for (size_t idx = 0U; idx < UNI_AX25_CALLSIGN_SIZE; idx++) {
            peer->callsign[idx] = (char)(data[off + idx] >> 1U);
        }
        // strip the padding: the callsign ends at the first NUL, without trailing spaces
        size_t callsign_len = uni_ax25_callsign_length(peer->callsign, UNI_AX25_CALLSIGN_SIZE);
        while (callsign_len > 0U && peer->callsign[callsign_len - 1U] == ' ') {
            callsign_len--;
        }
        memset(&peer->callsign[callsign_len], 0, sizeof(peer->callsign) - callsign_len);

        const uint8_t ssid = data[off + UNI_AX25_CALLSIGN_SIZE];
        peer->ssid = (uint8_t)((ssid >> 1U) & 0x0FU);
        peer->ch_bit = (ssid & UNI_AX25_SSID_CH_BIT) != 0U;
        last = (ssid & UNI_AX25_SSID_EXTENSION_BIT) != 0U;

        off += UNI_AX25_ADDRESS_SIZE;
        peer_count++;
    }

    // the address field must be terminated and hold at least destination and source
    if (!last || peer_count < 2U) {
        return SIZE_MAX;
    }

    // control and pid
    if (data_len - off < 2U) {
        return SIZE_MAX;
    }
    ax25_header->control = data[off++];
    ax25_header->pid = data[off++];

    return off;
}


size_t uni_ax25_encode(const uni_ax25_data_t *ax25_header, uint8_t *data, size_t data_len) {
    size_t off = uni_ax25_encode_address(ax25_header, data, data_len);
    if (off == SIZE_MAX || data_len - off < 2U) {
        return SIZE_MAX;
    }

    data[off++] = ax25_header->control;
    data[off++] = ax25_header->pid;
    return off;
}


size_t uni_ax25_encode_address(const uni_ax25_data_t *ax25_header, uint8_t *data, size_t data_len) {
    size_t off = 0U;

    if (ax25_header == NULL || data == NULL) {
        return SIZE_MAX;
    }

    const size_t peer_count = uni_ax25_peer_count(ax25_header);
    if (data_len < peer_count * UNI_AX25_ADDRESS_SIZE) {
        return SIZE_MAX;
    }

    for (size_t peer_idx = 0U; peer_idx < peer_count; peer_idx++) {
        const uni_ax25_peer_t *peer = &ax25_header->peers[peer_idx];
        const size_t callsign_len = uni_ax25_callsign_length(peer->callsign, UNI_AX25_CALLSIGN_SIZE);

        // write callsign, space padded
        for (size_t idx = 0U; idx < UNI_AX25_CALLSIGN_SIZE; idx++) {
            const uint8_t byte = idx < callsign_len ? (uint8_t)peer->callsign[idx] : (uint8_t)' ';
            data[off++] = (uint8_t)(byte << 1U);
        }

        // write ssid
        uint8_t ssid = UNI_AX25_SSID_RESERVED_BITS;
        ssid |= (uint8_t)((peer->ssid & 0x0FU) << 1U);
        if (peer->ch_bit) {
            ssid |= UNI_AX25_SSID_CH_BIT;
        }
        if (peer_idx == peer_count - 1U) {
            ssid |= UNI_AX25_SSID_EXTENSION_BIT;
        }
        data[off++] = ssid;
    }

    return off;
}


size_t uni_ax25_contains_callsign(const uni_ax25_data_t *ax25_header, const char *callsign) {
    if (ax25_header == NULL || callsign == NULL) {
        return SIZE_MAX;
    }

    // never read more of the caller's string than its terminator or one byte past the limit
    const size_t len = uni_ax25_callsign_length(callsign, UNI_AX25_CALLSIGN_SIZE + 1U);
    if (len == 0U || len > UNI_AX25_CALLSIGN_SIZE) {
        return SIZE_MAX;
    }

    for (size_t peer_idx = 0U; peer_idx < UNI_AX25_PEERS_MAXCOUNT; peer_idx++) {
        const char *peer_callsign = ax25_header->peers[peer_idx].callsign;
        if (memcmp(peer_callsign, callsign, len) == 0 && (len == UNI_AX25_CALLSIGN_SIZE || peer_callsign[len] == '\0')) {
            return peer_idx;
        }
    }

    return SIZE_MAX;
}

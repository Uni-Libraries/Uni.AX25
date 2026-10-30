// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText : 2026 Uni-Libraries contributors

//
// Includes
//

#include "uni_ax25_hdlc.h"



//
// Defines
//

/** Reflected form of the CRC-16/CCITT polynomial x^16 + x^12 + x^5 + 1. */
#define UNI_AX25_FCS_POLYNOMIAL     (0x8408U)

/** Run of ones after which the transmitter stuffs a zero. */
#define UNI_AX25_HDLC_MAX_ONES      (5U)



//
// Functions
//

uint16_t uni_ax25_fcs_update(uint16_t fcs, uint8_t byte) {
    fcs = (uint16_t)(fcs ^ byte);
    for (unsigned int bit = 0U; bit < 8U; ++bit) {
        fcs = (uint16_t)((fcs & 1U) != 0U ? (fcs >> 1U) ^ UNI_AX25_FCS_POLYNOMIAL : fcs >> 1U);
    }
    return fcs;
}


static uint16_t uni_ax25_fcs_register(const uint8_t *data, size_t data_len) {
    uint16_t fcs = UNI_AX25_FCS_INITIAL;
    for (size_t index = 0U; index < data_len; ++index) {
        fcs = uni_ax25_fcs_update(fcs, data[index]);
    }
    return fcs;
}


uint16_t uni_ax25_fcs(const uint8_t *data, size_t data_len) {
    return (uint16_t)(uni_ax25_fcs_register(data, data_len) ^ UNI_AX25_FCS_XOR_OUT);
}


bool uni_ax25_fcs_valid(const uint8_t *data_with_fcs, size_t data_len) {
    return uni_ax25_fcs_register(data_with_fcs, data_len) == UNI_AX25_FCS_GOOD_RESIDUE;
}


bool uni_ax25_hdlc_stuff_bit(uni_ax25_hdlc_stuffing_t *state, uint8_t input_bit, uint8_t *output_bit, bool *input_consumed) {
    if (input_bit > 1U) {
        return false;
    }
    if (state->zero_pending) {
        *output_bit = 0U;
        *input_consumed = false;
        state->zero_pending = false;
        state->consecutive_ones = 0U;
        return true;
    }

    *output_bit = input_bit;
    *input_consumed = true;
    if (input_bit == 0U) {
        state->consecutive_ones = 0U;
    } else if (++state->consecutive_ones == UNI_AX25_HDLC_MAX_ONES) {
        state->zero_pending = true;
    }
    return true;
}


uni_ax25_hdlc_destuff_result_e uni_ax25_hdlc_destuff_bit(uni_ax25_hdlc_destuffing_t *state, uint8_t wire_bit, uint8_t *output_bit) {
    if (wire_bit > 1U) {
        return UNI_AX25_HDLC_DESTUFF_INVALID;
    }
    if (wire_bit == 0U) {
        const bool stuffed = state->consecutive_ones == UNI_AX25_HDLC_MAX_ONES;
        state->consecutive_ones = 0U;
        if (stuffed) {
            return UNI_AX25_HDLC_DESTUFF_SUPPRESS;
        }
        *output_bit = 0U;
        return UNI_AX25_HDLC_DESTUFF_EMIT;
    }
    if (++state->consecutive_ones > UNI_AX25_HDLC_MAX_ONES) {
        return UNI_AX25_HDLC_DESTUFF_INVALID;
    }
    *output_bit = 1U;
    return UNI_AX25_HDLC_DESTUFF_EMIT;
}

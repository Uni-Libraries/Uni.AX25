// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText : 2026 Uni-Libraries contributors

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

/** HDLC frame delimiter. */
#define UNI_AX25_HDLC_FLAG          (0x7EU)

/** FCS register value before the first byte. */
#define UNI_AX25_FCS_INITIAL        (0xFFFFU)

/** Value XORed onto the register to get the transmitted FCS. */
#define UNI_AX25_FCS_XOR_OUT        (0xFFFFU)

/** Register value after a frame followed by its own FCS. */
#define UNI_AX25_FCS_GOOD_RESIDUE   (0xF0B8U)



//
// Typedefs
//

/** Bit stuffing state; zero-initialize before the first bit of a frame. */
typedef struct {
    uint8_t consecutive_ones;
    bool zero_pending;
} uni_ax25_hdlc_stuffing_t;


/** Bit destuffing state; zero-initialize before the first bit after a flag. */
typedef struct {
    uint8_t consecutive_ones;
} uni_ax25_hdlc_destuffing_t;


typedef enum {
    /** The output bit is a frame bit. */
    UNI_AX25_HDLC_DESTUFF_EMIT = 0,
    /** The wire bit was a stuffed zero; there is no output bit. */
    UNI_AX25_HDLC_DESTUFF_SUPPRESS,
    /** Six ones in a row or a wire bit other than 0 or 1: not part of a valid frame. */
    UNI_AX25_HDLC_DESTUFF_INVALID,
} uni_ax25_hdlc_destuff_result_e;



//
// Functions
//

/**
 * Bit @p bit_index of @p byte. AX.25 transmits octets least significant bit first, so index 0 is
 * the first bit on the wire.
 */
static inline uint8_t uni_ax25_hdlc_lsb_bit(uint8_t byte, size_t bit_index) {
    return (uint8_t)((byte >> bit_index) & 1U);
}

/**
 * Feed one byte into the frame check sequence register (CRC-16/X-25: reflected polynomial 0x8408).
 * Start with UNI_AX25_FCS_INITIAL.
 *
 * @return the updated register
 */
uint16_t uni_ax25_fcs_update(uint16_t fcs, uint8_t byte);

/**
 * Frame check sequence of @p data_len bytes, as transmitted (the check value of "123456789" is
 * 0x906E). It goes on the wire low byte first.
 */
uint16_t uni_ax25_fcs(const uint8_t *data, size_t data_len);

/**
 * Check a frame followed by its two FCS bytes.
 *
 * @return true when the residue of the whole buffer matches UNI_AX25_FCS_GOOD_RESIDUE
 */
bool uni_ax25_fcs_valid(const uint8_t *data_with_fcs, size_t data_len);

/**
 * Produce the next wire bit of a frame. Flags are not stuffed: send them around the frame directly.
 *
 * @param input_bit next frame bit (0 or 1); it is not used while a stuffed zero is pending
 * @param output_bit receives the wire bit
 * @param input_consumed receives false when the wire bit is a stuffed zero, so the caller offers
 *        the same input bit again
 * @return false when @p input_bit is neither 0 nor 1 (nothing is produced)
 */
bool uni_ax25_hdlc_stuff_bit(uni_ax25_hdlc_stuffing_t *state, uint8_t input_bit, uint8_t *output_bit, bool *input_consumed);

/**
 * Remove stuffing from one wire bit. Flags must be recognized and removed before this call.
 *
 * @param output_bit receives the frame bit when the result is UNI_AX25_HDLC_DESTUFF_EMIT
 */
uni_ax25_hdlc_destuff_result_e uni_ax25_hdlc_destuff_bit(uni_ax25_hdlc_destuffing_t *state, uint8_t wire_bit, uint8_t *output_bit);

#ifdef __cplusplus
}
#endif

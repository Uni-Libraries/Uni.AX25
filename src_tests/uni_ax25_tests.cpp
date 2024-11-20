// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

//
// Includes
//

// stdlib
#include <cstring>

// catch
#include <catch2/catch_test_macros.hpp>

// Uni.AX25
#include "uni_ax25.h"



//
// tests
//

TEST_CASE("ax25_encode", "[ax25]") {
    SECTION("encode_1") {
        std::vector<uint8_t> bytes1{0x8E, 0x82, 0xAE, 0x40, 0x40, 0x40, 0xE4, 0x9A, 0x8A, 0x9E, 0xAE, 0x40, 0x40, 0x63, 0x04, 0x03};
        std::vector<uint8_t> bytes2(bytes1.size());

        uni_ax25_data_t ax25{};
        REQUIRE(uni_ax25_decode(bytes1.data(), bytes1.size(), &ax25) == bytes1.size());
        REQUIRE(uni_ax25_encode(&ax25, bytes2.data(), bytes2.size()) == bytes2.size());
        REQUIRE(memcmp(bytes1.data(), bytes2.data(), bytes1.size() * sizeof(uint8_t)) == 0);
    }
}


TEST_CASE("ax25_parse", "[ax25]") {
    SECTION("p2p_1") {
        std::vector<uint8_t> bytes{0x8E, 0x82, 0xAE, 0x40, 0x40, 0x40, 0xE4, 0x9A, 0x8A, 0x9E, 0xAE, 0x40, 0x40, 0x63, 0x04, 0x03};

        uni_ax25_data_t ax25{};
        auto result = uni_ax25_decode(bytes.data(), bytes.size(), &ax25);
        REQUIRE(result == bytes.size());
        REQUIRE(strcmp(ax25.peers[APP_AX25_PEER_DST].callsign, "GAW") == 0);
        REQUIRE(strcmp(ax25.peers[APP_AX25_PEER_SRC].callsign, "MEOW") == 0);
        REQUIRE(ax25.peers[APP_AX25_PEER_DST].ssid == 0x02);
        REQUIRE(ax25.peers[APP_AX25_PEER_SRC].ssid == 0x01);
        REQUIRE(ax25.control == 0x04);
        REQUIRE(ax25.pid == 0x03);
    }

    SECTION("p2p_2") {
        std::vector<uint8_t> bytes{0x9C, 0x94, 0x6E, 0xA0, 0x40, 0x40, 0xE0, 0x9C, 0x6E, 0x98, 0x8A, 0x9A, 0x40, 0x61, 0x3E, 0xF0};

        uni_ax25_data_t ax25{};
        auto result = uni_ax25_decode(bytes.data(), bytes.size(), &ax25);
        REQUIRE(result == bytes.size());
        REQUIRE(strcmp(ax25.peers[APP_AX25_PEER_DST].callsign, "NJ7P") == 0);
        REQUIRE(strcmp(ax25.peers[APP_AX25_PEER_SRC].callsign, "N7LEM") == 0);
    }

    SECTION("p2p_3") {
        std::vector<uint8_t> bytes{0x8E, 0x82, 0xAE, 0x40, 0x40, 0x40, 0xE4, 0x9A, 0x8A, 0x9E, 0xAE, 0x40,
                                   0x40, 0x63, 0x04, 0x03, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
                                   0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF};

        uni_ax25_data_t ax25{};
        auto result = uni_ax25_decode(bytes.data(), bytes.size(), &ax25);
        REQUIRE(result == 16);
        REQUIRE(strcmp(ax25.peers[APP_AX25_PEER_DST].callsign, "GAW") == 0);
        REQUIRE(strcmp(ax25.peers[APP_AX25_PEER_SRC].callsign, "MEOW") == 0);
        REQUIRE(ax25.peers[APP_AX25_PEER_DST].ssid == 0x2);
        REQUIRE(ax25.peers[APP_AX25_PEER_SRC].ssid == 0x1);
        REQUIRE(ax25.control == 0x04);
        REQUIRE(ax25.pid == 0x03);
    }

    SECTION("repeater_1") {
        std::vector<uint8_t> bytes{0x9C, 0x94, 0x6E, 0xA0, 0x40, 0x40, 0xE0, 0x9C, 0x6E, 0x98, 0x8A, 0x9A,
                                   0x40, 0x60, 0x9C, 0x6E, 0x9E, 0x9E, 0x40, 0x40, 0xE3, 0x3E, 0xF0};
        uni_ax25_data_t ax25{};
        auto result = uni_ax25_decode(bytes.data(), bytes.size(), &ax25);
        REQUIRE(result == bytes.size());
        REQUIRE(strcmp(ax25.peers[APP_AX25_PEER_DST].callsign, "NJ7P") == 0);
        REQUIRE(strcmp(ax25.peers[APP_AX25_PEER_SRC].callsign, "N7LEM") == 0);
        REQUIRE(strcmp(ax25.peers[APP_AX25_PEER_REPEATER_1].callsign, "N7OO") == 0);
        REQUIRE(ax25.peers[APP_AX25_PEER_DST].ssid == 0x00);
        REQUIRE(ax25.peers[APP_AX25_PEER_SRC].ssid == 0x00);
        REQUIRE(ax25.peers[APP_AX25_PEER_REPEATER_1].ssid == 0x01);
        REQUIRE(ax25.peers[APP_AX25_PEER_REPEATER_1].ch_bit == 1);
    }
}

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

//
// Includes
//

// stdlib
#include <cstdint>
#include <cstring>
#include <memory>
#include <random>
#include <vector>

// catch
#include <catch2/catch_test_macros.hpp>

// Uni.AX25
#include "uni_ax25.h"



//
// helpers
//

namespace {

using Bytes = std::vector<uint8_t>;

Bytes Address(const char *callsign, uint8_t ssid, bool last) {
    Bytes result(UNI_AX25_ADDRESS_SIZE, static_cast<uint8_t>(' ' << 1));
    for (size_t idx = 0; idx < UNI_AX25_CALLSIGN_SIZE && callsign[idx] != '\0'; idx++) {
        result[idx] = static_cast<uint8_t>(callsign[idx] << 1);
    }
    result[UNI_AX25_CALLSIGN_SIZE] = static_cast<uint8_t>(0x60U | ((ssid & 0x0FU) << 1) | (last ? 1U : 0U));
    return result;
}

Bytes Header(size_t peer_count, size_t terminated_at) {
    Bytes result;
    for (size_t idx = 0; idx < peer_count; idx++) {
        const Bytes address = Address(idx == 0 ? "DST" : "PEER", static_cast<uint8_t>(idx), idx == terminated_at);
        result.insert(result.end(), address.begin(), address.end());
    }
    result.push_back(0x03);
    result.push_back(0xF0);
    return result;
}

// Independent reference: size of a valid header or SIZE_MAX.
size_t ReferenceHeaderSize(const Bytes &data) {
    for (size_t peer = 0; peer < UNI_AX25_PEERS_MAXCOUNT; peer++) {
        const size_t end = (peer + 1U) * UNI_AX25_ADDRESS_SIZE;
        if (end > data.size()) {
            return SIZE_MAX;
        }
        if (data[end - 1U] & 0x01U) {
            if (peer == 0U || end + 2U > data.size()) {
                return SIZE_MAX;
            }
            return end + 2U;
        }
    }
    return SIZE_MAX;
}

} // namespace



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

    SECTION("nullptr") {
        uni_ax25_data_t ax25{};
        uint8_t data[32]{};
        REQUIRE(uni_ax25_encode(nullptr, data, sizeof(data)) == SIZE_MAX);
        REQUIRE(uni_ax25_encode(&ax25, nullptr, sizeof(data)) == SIZE_MAX);
        REQUIRE(uni_ax25_encode_address(nullptr, data, sizeof(data)) == SIZE_MAX);
    }

    SECTION("a buffer too small for the whole header fails instead of truncating the address field") {
        // regression: encode_address returned the addresses that fitted, without an extension bit,
        // and encode then appended control and pid to the truncated address field
        uni_ax25_data_t ax25{};
        strcpy(ax25.peers[APP_AX25_PEER_DST].callsign, "DST");
        strcpy(ax25.peers[APP_AX25_PEER_SRC].callsign, "SRC");
        strcpy(ax25.peers[APP_AX25_PEER_REPEATER_1].callsign, "RPT");
        ax25.control = UNI_AX25_CONTROL_UI;
        ax25.pid = UNI_AX25_PID_NO_L3;

        const size_t full_size = 3U * UNI_AX25_ADDRESS_SIZE + 2U;
        for (size_t size = 0; size < full_size; size++) {
            auto data = std::make_unique<uint8_t[]>(size == 0 ? 1 : size);
            REQUIRE(uni_ax25_encode(&ax25, data.get(), size) == SIZE_MAX);
        }
        Bytes data(full_size);
        REQUIRE(uni_ax25_encode(&ax25, data.data(), data.size()) == full_size);
        REQUIRE((data[UNI_AX25_ADDRESS_SIZE - 1U] & 0x01U) == 0U);
        REQUIRE((data[2U * UNI_AX25_ADDRESS_SIZE - 1U] & 0x01U) == 0U);
        REQUIRE((data[3U * UNI_AX25_ADDRESS_SIZE - 1U] & 0x01U) == 1U);
    }

    SECTION("an empty source still ends the address field once, on the source") {
        uni_ax25_data_t ax25{};
        strcpy(ax25.peers[APP_AX25_PEER_DST].callsign, "DST");
        Bytes data(2U * UNI_AX25_ADDRESS_SIZE);
        REQUIRE(uni_ax25_encode_address(&ax25, data.data(), data.size()) == data.size());
        REQUIRE((data[UNI_AX25_ADDRESS_SIZE - 1U] & 0x01U) == 0U);
        REQUIRE((data[2U * UNI_AX25_ADDRESS_SIZE - 1U] & 0x01U) == 1U);
    }

    SECTION("callsign characters after the terminator are padding") {
        uni_ax25_data_t ax25{};
        memcpy(ax25.peers[APP_AX25_PEER_DST].callsign, "AB\0XYZ", 7);
        strcpy(ax25.peers[APP_AX25_PEER_SRC].callsign, "SRC");
        Bytes data(2U * UNI_AX25_ADDRESS_SIZE);
        REQUIRE(uni_ax25_encode_address(&ax25, data.data(), data.size()) == data.size());
        const Bytes expected = Address("AB", 0, false);
        REQUIRE(Bytes(data.begin(), data.begin() + UNI_AX25_ADDRESS_SIZE) == expected);
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

    SECTION("nullptr") {
        uni_ax25_data_t ax25{};
        const Bytes bytes = Header(2, 1);
        REQUIRE(uni_ax25_decode(nullptr, bytes.size(), &ax25) == SIZE_MAX);
        REQUIRE(uni_ax25_decode(bytes.data(), bytes.size(), nullptr) == SIZE_MAX);
    }

    SECTION("unterminated address field is rejected") {
        // regression: running out of data or peers before the extension bit was accepted, and
        // control/pid were then read from address bytes
        uni_ax25_data_t ax25{};
        const Bytes unterminated = Header(2, SIZE_MAX);
        REQUIRE(uni_ax25_decode(unterminated.data(), unterminated.size(), &ax25) == SIZE_MAX);
        const Bytes ten_unterminated = Header(UNI_AX25_PEERS_MAXCOUNT, SIZE_MAX);
        REQUIRE(uni_ax25_decode(ten_unterminated.data(), ten_unterminated.size(), &ax25) == SIZE_MAX);
        const Bytes ten_terminated = Header(UNI_AX25_PEERS_MAXCOUNT, UNI_AX25_PEERS_MAXCOUNT - 1U);
        REQUIRE(uni_ax25_decode(ten_terminated.data(), ten_terminated.size(), &ax25) == ten_terminated.size());
    }

    SECTION("a single address is rejected") {
        uni_ax25_data_t ax25{};
        const Bytes single = Header(2, 0);
        REQUIRE(uni_ax25_decode(single.data(), single.size(), &ax25) == SIZE_MAX);
        // 9 bytes: one address, control and pid
        const Bytes short_single(single.begin(), single.begin() + UNI_AX25_ADDRESS_SIZE);
        Bytes nine = short_single;
        nine.push_back(0x03);
        nine.push_back(0xF0);
        REQUIRE(uni_ax25_decode(nine.data(), nine.size(), &ax25) == SIZE_MAX);
    }

    SECTION("missing control or pid is rejected") {
        uni_ax25_data_t ax25{};
        const Bytes header = Header(2, 1);
        for (size_t size = 0; size < header.size(); size++) {
            REQUIRE(uni_ax25_decode(header.data(), size, &ax25) == SIZE_MAX);
        }
    }

    SECTION("previous contents are cleared and callsigns are always terminated") {
        uni_ax25_data_t ax25;
        memset(&ax25, 'X', sizeof(ax25));
        Bytes bytes = Address("ABCDEF", 1, false);
        const Bytes src = Address("Q", 2, true);
        bytes.insert(bytes.end(), src.begin(), src.end());
        bytes.push_back(0x03);
        bytes.push_back(0xF0);
        REQUIRE(uni_ax25_decode(bytes.data(), bytes.size(), &ax25) == bytes.size());
        REQUIRE(strcmp(ax25.peers[APP_AX25_PEER_DST].callsign, "ABCDEF") == 0);
        REQUIRE(strcmp(ax25.peers[APP_AX25_PEER_SRC].callsign, "Q") == 0);
        for (size_t idx = APP_AX25_PEER_REPEATER_1; idx < UNI_AX25_PEERS_MAXCOUNT; idx++) {
            REQUIRE(ax25.peers[idx].callsign[0] == '\0');
        }
    }
}


TEST_CASE("ax25_contains_callsign", "[ax25]") {
    const Bytes bytes{0x9C, 0x94, 0x6E, 0xA0, 0x40, 0x40, 0xE0, 0x9C, 0x6E, 0x98, 0x8A, 0x9A,
                      0x40, 0x60, 0x9C, 0x6E, 0x9E, 0x9E, 0x40, 0x40, 0xE3, 0x3E, 0xF0};
    uni_ax25_data_t ax25{};
    REQUIRE(uni_ax25_decode(bytes.data(), bytes.size(), &ax25) == bytes.size());

    // exactly-sized heap strings: the old implementation read 6 bytes of the argument
    auto heap_string = [](const char *text) {
        const size_t size = strlen(text) + 1U;
        auto copy = std::make_unique<char[]>(size);
        memcpy(copy.get(), text, size);
        return copy;
    };

    REQUIRE(uni_ax25_contains_callsign(&ax25, heap_string("NJ7P").get()) == APP_AX25_PEER_DST);
    REQUIRE(uni_ax25_contains_callsign(&ax25, heap_string("N7LEM").get()) == APP_AX25_PEER_SRC);
    REQUIRE(uni_ax25_contains_callsign(&ax25, heap_string("N7OO").get()) == APP_AX25_PEER_REPEATER_1);
    REQUIRE(uni_ax25_contains_callsign(&ax25, heap_string("N7").get()) == SIZE_MAX);
    REQUIRE(uni_ax25_contains_callsign(&ax25, heap_string("N7LEMX").get()) == SIZE_MAX);
    REQUIRE(uni_ax25_contains_callsign(&ax25, heap_string("N7LEMXY").get()) == SIZE_MAX);
    REQUIRE(uni_ax25_contains_callsign(&ax25, heap_string("").get()) == SIZE_MAX);
    REQUIRE(uni_ax25_contains_callsign(&ax25, nullptr) == SIZE_MAX);
    REQUIRE(uni_ax25_contains_callsign(nullptr, "N7OO") == SIZE_MAX);
}


TEST_CASE("ax25_fuzz", "[ax25][fuzz]") {
    std::mt19937 rng(0x41583235U);

    for (int iteration = 0; iteration < 20000; iteration++) {
        Bytes data;
        if (iteration % 2 == 0) {
            data.resize(rng() % 80U);
            for (auto &byte : data) {
                // mostly clear extension bits so that long address fields are generated
                byte = static_cast<uint8_t>(rng() & (rng() % 4U == 0U ? 0xFFU : 0xFEU));
            }
        } else {
            const size_t peers = 1U + rng() % UNI_AX25_PEERS_MAXCOUNT;
            data = Header(peers, rng() % 4U == 0U ? rng() % (peers + 1U) : peers - 1U);
            const uint32_t mutations = rng() % 3U;
            for (uint32_t m = 0; m < mutations && !data.empty(); m++) {
                const size_t pos = rng() % data.size();
                if (rng() % 2U == 0U) {
                    data[pos] = static_cast<uint8_t>(data[pos] ^ (1U << (rng() % 8U)));
                } else {
                    data.resize(pos);
                }
            }
        }

        // exactly-sized copy so that AddressSanitizer catches over-reads
        auto exact = std::make_unique<uint8_t[]>(data.empty() ? 1U : data.size());
        if (!data.empty()) {
            memcpy(exact.get(), data.data(), data.size());
        }

        uni_ax25_data_t ax25{};
        const size_t size = uni_ax25_decode(exact.get(), data.size(), &ax25);
        REQUIRE(size == ReferenceHeaderSize(data));
        if (size == SIZE_MAX) {
            continue;
        }

        size_t peers = 0U;
        while (peers < UNI_AX25_PEERS_MAXCOUNT && (peers < 2U || ax25.peers[peers].callsign[0] != '\0')) {
            REQUIRE(ax25.peers[peers].callsign[UNI_AX25_CALLSIGN_SIZE] == '\0');
            peers++;
        }
        if (peers * UNI_AX25_ADDRESS_SIZE + 2U != size) {
            continue; // a repeater with an empty callsign ends the repeater list when re-encoding
        }

        Bytes encoded(size);
        REQUIRE(uni_ax25_encode(&ax25, encoded.data(), encoded.size()) == size);
        uni_ax25_data_t again{};
        REQUIRE(uni_ax25_decode(encoded.data(), encoded.size(), &again) == size);
        for (size_t idx = 0; idx < peers; idx++) {
            REQUIRE(strcmp(again.peers[idx].callsign, ax25.peers[idx].callsign) == 0);
            REQUIRE(again.peers[idx].ssid == ax25.peers[idx].ssid);
            REQUIRE(again.peers[idx].ch_bit == ax25.peers[idx].ch_bit);
        }
        REQUIRE(again.control == ax25.control);
        REQUIRE(again.pid == ax25.pid);
    }
}

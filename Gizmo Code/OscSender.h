#pragma once
// Minimal, dependency-free OSC-over-UDP sender (Windows / Winsock).
// Only what Gizmo needs: /cv/person <int present> <float x> <float y>.
// No external library -- drop this header into the Visual Studio project.

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <cstring>
#pragma comment(lib, "Ws2_32.lib")

class OscSender {
public:
    bool open(const char* host, uint16_t port) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
        sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_ == INVALID_SOCKET) return false;
        memset(&dst_, 0, sizeof(dst_));
        dst_.sin_family = AF_INET;
        dst_.sin_port = htons(port);
        return inet_pton(AF_INET, host, &dst_.sin_addr) == 1;
    }

    void close() {
        if (sock_ != INVALID_SOCKET) { closesocket(sock_); sock_ = INVALID_SOCKET; }
        WSACleanup();
    }

    // OSC message: /cv/person ,iff <present> <x> <y>
    void sendPerson(int present, float x, float y) {
        uint8_t buf[64];
        size_t n = 0;
        n += putString(buf + n, "/cv/person");
        n += putString(buf + n, ",iff");
        n += putInt32(buf + n, present);
        n += putFloat(buf + n, x);
        n += putFloat(buf + n, y);
        sendto(sock_, reinterpret_cast<const char*>(buf), static_cast<int>(n), 0,
               reinterpret_cast<sockaddr*>(&dst_), sizeof(dst_));
    }

private:
    // OSC string: content + null terminator, zero-padded to a multiple of 4 bytes.
    static size_t putString(uint8_t* p, const char* s) {
        size_t len = strlen(s);
        size_t padded = ((len + 1) + 3) & ~size_t(3);
        memcpy(p, s, len);
        for (size_t i = len; i < padded; ++i) p[i] = 0;
        return padded;
    }
    static size_t putInt32(uint8_t* p, int32_t v) {
        uint32_t be = htonl(static_cast<uint32_t>(v));   // OSC is big-endian
        memcpy(p, &be, 4);
        return 4;
    }
    static size_t putFloat(uint8_t* p, float f) {
        uint32_t bits;
        memcpy(&bits, &f, 4);
        bits = htonl(bits);
        memcpy(p, &bits, 4);
        return 4;
    }

    SOCKET sock_ = INVALID_SOCKET;
    sockaddr_in dst_{};
};

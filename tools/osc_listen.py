#!/usr/bin/env python3
"""Gizmo OSC listener: shows the /cv/person messages that FaceTrack.exe sends.

Usage:   python osc_listen.py [port]        (default port 7562)
Stop:    Ctrl+C

Standard library only -- nothing to install. Prints a one-line summary every second
(message count, rate, latest present/x/y) and an immediate line whenever "present" flips.
"""
import socket
import struct
import sys
import time

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 7562
sys.stdout.reconfigure(line_buffering=True)   # show each line immediately, even when piped


def read_string(buf, i):
    """OSC string: text + null, padded with zeros to a multiple of 4 bytes."""
    end = buf.index(b"\0", i)
    text = buf[i:end].decode("ascii")
    nxt = i + ((end - i + 1) + 3) // 4 * 4
    return text, nxt


def parse_message(buf):
    """Return (address, [args]) for one OSC message (i / f / s argument types)."""
    address, i = read_string(buf, 0)
    typetag, i = read_string(buf, i)
    if not typetag.startswith(","):
        raise ValueError("missing type tag")
    args = []
    for t in typetag[1:]:
        if t == "i":
            args.append(struct.unpack(">i", buf[i:i + 4])[0])
            i += 4
        elif t == "f":
            args.append(struct.unpack(">f", buf[i:i + 4])[0])
            i += 4
        elif t == "s":
            s, i = read_string(buf, i)
            args.append(s)
        else:
            raise ValueError("unsupported type tag '%s'" % t)
    return address, args


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", PORT))
    sock.settimeout(1.0)
    print("Listening for OSC on UDP port %d  (Ctrl+C to stop)" % PORT)

    count = 0
    last = None            # latest (present, x, y)
    last_present = None
    window_start = time.time()
    got_any = False

    try:
        while True:
            try:
                data, addr = sock.recvfrom(1024)
            except socket.timeout:
                data = None

            if data is not None:
                try:
                    address, args = parse_message(data)
                except Exception as e:
                    print("  bad packet from %s: %s" % (addr[0], e))
                    continue
                if not got_any:
                    print("First packet from %s" % addr[0])
                    got_any = True
                if address == "/cv/person" and len(args) == 3:
                    count += 1
                    last = tuple(args)
                    if last_present is not None and args[0] != last_present:
                        print("  >> person %s" % ("APPEARED" if args[0] else "LEFT"))
                    last_present = args[0]
                else:
                    print("  other message: %s %s" % (address, args))

            now = time.time()
            if now - window_start >= 1.0:
                rate = count / (now - window_start)
                if last is None:
                    print("no packets yet -- check the IP, port and that FaceTrack is running")
                else:
                    print("%5.1f msg/s   present=%d   x=%+.3f   y=%+.3f" % (rate, last[0], last[1], last[2]))
                count = 0
                window_start = now
    except KeyboardInterrupt:
        print("\nStopped.")


if __name__ == "__main__":
    main()

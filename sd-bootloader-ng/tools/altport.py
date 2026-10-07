#!/usr/bin/env python3
"""Writes the altPort patch, which makes the original firmware (3.1.0 BF2, 3.3.0,
3.4.0) connect to another port than 443.

    altport.py 10443 [--check mcuimg.bin ...]
"""
import argparse
import json
import sys


def thumb_movw(rd, value):
    imm4, i, imm3, imm8 = value >> 12, (value >> 11) & 1, (value >> 8) & 7, value & 0xFF
    hw1 = 0xF240 | (i << 10) | imm4
    hw2 = (imm3 << 12) | (rd << 8) | imm8
    return [hw1 & 0xFF, hw1 >> 8, hw2 & 0xFF, hw2 >> 8]


def htons(port):
    return ((port & 0xFF) << 8) | (port >> 8)


def h(s):
    return [None if b == "??" else int(b, 16) for b in s.split()]


def entries(port):
    # (description, search, offset of the replacement, replacement)
    return [
        ("connect 3.1.0 BF2: movw r3, #htons(port)",
         h("02 23 ad f8 10 30 ?? ?? 4b f6 01 33 ad f8 0c 30 ad f8 12 30"),
         8, thumb_movw(3, htons(port))),
        ("connect 3.3.0/3.4.0: sockaddr literal AF_INET:port",
         h("02 00 01 bb"),
         2, [port >> 8, port & 0xFF]),
        ("log BF2/3.3.0/3.4.0: connect failed, movw r3, #port",
         h("8b 48 87 4a 40 f2 bb 13 ?? ?? ?? ?? 89 49 b4 f9 00 00"),
         4, thumb_movw(3, port)),
        ("log BF2/3.3.0/3.4.0: cloud request, movw sl, #port",
         h("40 f2 bb 1a 02 46 cd f8 04 a0 40 f2 11 30 cd f8 00 80 ?? ?? ?? ?? "
           "1c 22 00 21 11 a8 ?? ?? ?? ?? ?? 4a 4e f6 60 23"),
         0, thumb_movw(10, port)),
    ]


def hexlist(bs):
    return ", ".join('"??"' if b is None else '"%02x"' % b for b in bs)


def patch_json(port):
    items = []
    for desc, search, off, repl in entries(port):
        replace = [None] * len(search)
        replace[off:off + len(repl)] = repl
        items.append('    {\n        "_desc": "%s",\n        "search":  [%s],\n        "replace": [%s]\n    }'
                     % (desc, hexlist(search), hexlist(replace)))
    return ('{\n    "general": {\n'
            '        "_desc": "Changes the port the box connects to (API and RTNL) from 443 to %d.",\n'
            '        "_memPos": "",\n'
            '        "_fwVer": "3.1.0+"\n'
            '    },\n    "searchAndReplace": [\n%s\n    ]\n}\n') % (port, ",\n".join(items))


def find(image, search):
    hits, n = [], len(search)
    for off in range(len(image) - n + 1):
        if all(s is None or image[off + k] == s for k, s in enumerate(search)):
            hits.append(off)
    return hits


def apply(image, port):
    """Like the bootloader: each entry replaces its first match."""
    image = bytearray(image)
    report = []
    for desc, search, off, repl in entries(port):
        hits = find(image, search)
        if hits:
            image[hits[0] + off:hits[0] + off + len(repl)] = bytes(repl)
        report.append((desc, hits))
    return bytes(image), report


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("port", type=int)
    ap.add_argument("-o", "--output", help="patch file (default altPort.<port>.json)")
    ap.add_argument("--check", nargs="+", metavar="IMAGE", help="firmware images to test the patch on")
    args = ap.parse_args()
    if not 1 <= args.port <= 65535:
        sys.exit("port must be 1..65535")

    text = patch_json(args.port)
    json.loads(text)
    out = args.output or "altPort.%d.json" % args.port
    with open(out, "w") as f:
        f.write(text)
    print("wrote", out)

    for path in args.check or []:
        with open(path, "rb") as f:
            image = f.read()
        _, report = apply(image, args.port)
        print(path)
        for desc, hits in report:
            print("  %-60s %s" % (desc, ", ".join("0x%x" % o for o in hits) or "-"))


if __name__ == "__main__":
    main()

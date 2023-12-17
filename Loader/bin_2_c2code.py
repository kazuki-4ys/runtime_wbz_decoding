#!/usr/bin/env python3
import sys

def stringToNum(str):
    if str[:2].upper() == "0X":
        return int(str[2:], 16)
    return int(str)

def bytesToU32(buf, offset, isLE):
    num = 0
    for i in range(4):
        num <<= 8
        if isLE:
            num += buf[offset + 3 - i]
        else:
            num += buf[offset + i]
    return num

def fileToBytes(fileName):
    f = open(fileName, 'rb')
    dest = f.read()
    f.close()
    return dest

def u32ToText(u32val):
    dest = hex(u32val)[2:]
    while len(dest) < 8:
        dest = "0" + dest
    return dest

if len(sys.argv) < 4:
    print("usage: /path/to/compiled_code.bin [injection target address] /path/to/output.txt")
    exit()
code = fileToBytes(sys.argv[1])
while len(code) % 4 != 0:
    code += bytearray(1)
if len(code) % 8 == 0:
    code += bytearray([0x60, 0, 0, 0, 0, 0, 0, 0])
else:
    code += bytearray(4)
lineCount = int(len(code) / 8)
destText = ""
destText += u32ToText(0xC2000000 + (stringToNum(sys.argv[2]) & 0xFFFFFF))
destText += " "
destText += u32ToText(lineCount)
for i in range(lineCount):
    destText += "\n"
    destText += u32ToText(bytesToU32(code, i * 8, False))
    destText += " "
    destText += u32ToText(bytesToU32(code, i * 8 + 4, False))

f = open(sys.argv[3], "w")
f.write(destText)
f.close()
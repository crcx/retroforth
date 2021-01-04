#!/usr/bin/env python3

# retro-extend is a tool to load additional code into an image file. It
# takes the name of an image file and one or more source files to load
# into the image. After completion the image file will be updated with
# the changes.
#
# Copyright (c) 2010 - 2021, Charles Childers
# Optimizations and process() rewrite by Greg Copeland
#
# Usage:
#
#     retro-extend.py image filename [filenames]


import os, sys, math, time, struct
from struct import pack, unpack

ip = 0
stack = [] * 128
address = []
memory = []
Interpreter = 0

notfound = 0
s_eq = 0
d_addheader = 0
d_lookup = 0
Dictionary = dict()

def rxDivMod(a, b):
    x = abs(a)
    y = abs(b)
    q, r = divmod(x, y)
    if a < 0 and b < 0:
        r *= -1
    elif a > 0 and b < 0:
        q *= -1
    elif a < 0 and b > 0:
        r *= -1
        q *= -1
    return q, r


def findEntry(named):
    if named in Dictionary:
      return Dictionary[named]

    header = memory[2]
    Done = False
    while header != 0 and not Done:
        if named == extractString(header + 3):
            Dictionary[named] = header
            Done = True
        else:
            header = memory[header]
    return header


def rxGetInput():
    return ord(sys.stdin.read(1))


def rxDisplayCharacter():
    global stack
    if stack[-1] > 0 and stack[-1] < 128:
        if stack[-1] == 8:
            sys.stdout.write(chr(stack.pop()))
            sys.stdout.write(chr(32))
            sys.stdout.write(chr(8))
        else:
            sys.stdout.write(chr(stack.pop()))
    else:
        sys.stdout.write("\033[2J\033[1;1H")
        stack.pop()
    sys.stdout.flush()


def i_no():
    pass


def i_li():
    global ip, memory, stack, address
    ip += 1
    stack.append(memory[ip])


def i_du():
    global ip, memory, stack, address
    stack.append(stack[-1])


def i_dr():
    global ip, memory, stack, address
    stack.pop()


def i_sw():
    global ip, memory, stack, address
    a = stack[-2]
    stack[-2] = stack[-1]
    stack[-1] = a


def i_pu():
    global ip, memory, stack, address
    address.append(stack.pop())


def i_po():
    global ip, memory, stack, address
    stack.append(address.pop())


def i_ju():
    global ip, memory, stack, address
    ip = stack.pop() - 1


def i_ca():
    global ip, memory, stack, address
    address.append(ip)
    ip = stack.pop() - 1


def i_cc():
    global ip, memory, stack, address
    target = stack.pop()
    if stack.pop() != 0:
        address.append(ip)
        ip = target - 1


def i_re():
    global ip, memory, stack, address
    ip = address.pop()


def i_eq():
    global ip, memory, stack, address
    a = stack.pop()
    b = stack.pop()
    if b == a:
        stack.append(-1)
    else:
        stack.append(0)


def i_ne():
    global ip, memory, stack, address
    a = stack.pop()
    b = stack.pop()
    if b != a:
        stack.append(-1)
    else:
        stack.append(0)


def i_lt():
    global ip, memory, stack, address
    a = stack.pop()
    b = stack.pop()
    if b < a:
        stack.append(-1)
    else:
        stack.append(0)


def i_gt():
    global ip, memory, stack, address
    a = stack.pop()
    b = stack.pop()
    if b > a:
        stack.append(-1)
    else:
        stack.append(0)


def i_fe():
    global ip, memory, stack, address
    if stack[-1] == -1:
        stack[-1] = len(stack) - 1
    elif stack[-1] == -2:
        stack[-1] = len(address)
    elif stack[-1] == -3:
        stack[-1] = len(memory)
    elif stack[-1] == -4:
        stack[-1] = -2147483648
    elif stack[-1] == -5:
        stack[-1] = 2147483647
    else:
        stack[-1] = memory[stack[-1]]


def i_st():
    global ip, memory, stack, address
    mi = stack.pop()
    memory[mi] = stack.pop()


def i_ad():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] += t
    stack[-1] = unpack("=l", pack("=L", stack[-1] & 0xFFFFFFFF))[0]


def i_su():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] -= t
    stack[-1] = unpack("=l", pack("=L", stack[-1] & 0xFFFFFFFF))[0]


def i_mu():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] *= t
    stack[-1] = unpack("=l", pack("=L", stack[-1] & 0xFFFFFFFF))[0]


def i_di():
    global ip, memory, stack, address
    a = stack[-1]
    b = stack[-2]
    stack[-1], stack[-2] = rxDivMod(b, a)
    stack[-1] = unpack("=l", pack("=L", stack[-1] & 0xFFFFFFFF))[0]
    stack[-2] = unpack("=l", pack("=L", stack[-2] & 0xFFFFFFFF))[0]


def i_an():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] &= t


def i_or():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] |= t


def i_xo():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] ^= t


def i_sh():
    global ip, memory, stack, address
    t = stack.pop()
    if t < 0:
        stack[-1] <<= t * -1
    else:
        stack[-1] >>= t


def i_zr():
    global ip, memory, stack, address
    if stack[-1] == 0:
        stack.pop()
        ip = address.pop()


def i_ha():
    global ip, memory, stack, address
    ip = 9000000


def i_ie():
    stack.push(1)


def i_iq():
    stack.pop()
    stack.push(0)
    stack.push(0)


def i_ii():
    stack.pop()
    rxDisplayCharacter()


instructions = [
    i_no,
    i_li,
    i_du,
    i_dr,
    i_sw,
    i_pu,
    i_po,
    i_ju,
    i_ca,
    i_cc,
    i_re,
    i_eq,
    i_ne,
    i_lt,
    i_gt,
    i_fe,
    i_st,
    i_ad,
    i_su,
    i_mu,
    i_di,
    i_an,
    i_or,
    i_xo,
    i_sh,
    i_zr,
    i_ha,
    i_ie,
    i_iq,
    i_ii,
]


def validateOpcode(opcode):
    I0 = opcode & 0xFF
    I1 = (opcode >> 8) & 0xFF
    I2 = (opcode >> 16) & 0xFF
    I3 = (opcode >> 24) & 0xFF
    if (
        (I0 >= 0 and I0 <= 29)
        and (I1 >= 0 and I1 <= 29)
        and (I2 >= 0 and I2 <= 29)
        and (I3 >= 0 and I3 <= 29)
    ):
        return True
    else:
        return False


def extractString(at):
    i = at
    s = ""
    while memory[i] != 0:
        s = s + chr(memory[i])
        i = i + 1
    return s


def injectString(s, to):
    global memory
    i = to
    for c in s:
        memory[i] = ord(c)
        i = i + 1
    memory[i] = 0


def execute(word, output="console"):
    global ip, memory, stack, address
    ip = word
    address.append(0)
    while ip < 1000000 and len(address) > 0:
        opcode = memory[ip]
        if ip == s_eq:
            a = extractString(stack.pop())
            b = extractString(stack.pop())
            if a == b:
                stack.append(-1)
            else:
                stack.append(0)
            ip = address.pop()
        elif ip == d_lookup:
            a = extractString(stack.pop())
            header = findEntry(a)
            stack.append(header)
            ip = address.pop()
        else:
          if validateOpcode(opcode):
              I0 = opcode & 0xFF
              I1 = (opcode >> 8) & 0xFF
              I2 = (opcode >> 16) & 0xFF
              I3 = (opcode >> 24) & 0xFF
              if I0 != 0:
                  instructions[I0]()
              if I1 != 0:
                  instructions[I1]()
              if I2 != 0:
                  instructions[I2]()
              if I3 != 0:
                  instructions[I3]()
          else:
              print("Invalid Bytecode", opcode, ip)
              ip = 2000000
        ip = ip + 1
    return


def load_image():
    global memory
    cells = int(os.path.getsize(sys.argv[1]) / 4)
    f = open("ngaImage", "rb")
    memory = list(struct.unpack(cells * "i", f.read()))
    f.close()
    remaining = 1000000 - cells
    memory.extend([0] * remaining)


def process(line):
    for Token in line.split():
      injectString(Token, 1025)
      stack.append(1025)
      execute(Interpreter)


def process_files():
    for f in sys.argv[2:]:
        print(f)
        in_block = False
        with open(f, "r") as source:
            for line in source.readlines():
                if line.rstrip() == "~~~":
                    in_block = not in_block
                elif in_block:
                    process(line.strip())


def save_image():
    print("Writing {0} cells to {1}".format(memory[3], sys.argv[1]))
    with open(sys.argv[1], "wb") as file:
        j = 0
        while j <= memory[3]:
            file.write(struct.pack("<i", memory[j]))
            j = j + 1

if __name__ == "__main__":
    load_image()

    header = memory[2]
    Done = False
    while header != 0:
        named = extractString(header + 3)
        Dictionary[named] = header
        header = memory[header]

    Interpreter = memory[findEntry("interpret") + 1]
    notfound = memory[findEntry("err:notfound") + 1]
    s_eq = memory[findEntry("s:eq?") + 1]
    d_addheader = memory[findEntry("d:add-header") + 1]
    d_lookup = memory[findEntry("d:lookup") + 1]
    process_files()
    save_image()

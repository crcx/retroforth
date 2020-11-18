#!/usr/bin/env python3

# Nga: a Virtual Machine
# Copyright (c) 2010 - 2020, Charles Childers
# Floating Point I/O by Arland Childers, (c) 2020
# Optimizations and process() rewrite by Greg Copeland
# -----------------------------------------------------

import os, sys, math, time, struct, random, datetime
from struct import pack, unpack

from ClockDevice import Clock
from RNGDevice import RNG

from FloatStack import FloatStack
from IntegerStack import IntegerStack
from Memory import Memory

ip = 0
stack = IntegerStack()
address = []
memory = Memory("ngaImage", 1000000)

clock = Clock()
rng = RNG()

floats = FloatStack()
afloats = FloatStack()

files = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]


def file_open():
    global files
    slot = 0
    i = 1
    while i < 8:
        if files[i] == 0:
            slot = i
        i += 1
    mode = stack.pop()
    name = extract_string(stack.pop())
    if slot > 0:
        if mode == 0:
            if os.path.exists(name):
                files[slot] = open(name, "r")
            else:
                slot = 0
        elif mode == 1:
            files[slot] = open(name, "w")
        elif mode == 2:
            files[slot] = open(name, "a")
        elif mode == 3:
            if os.path.exists(name):
                files[slot] = open(name, "r+")
            else:
                slot = 0
    return slot


def file_read():
    global stack
    slot = stack.pop()
    return ord(files[slot].read(1))


def file_write():
    global stack
    slot = stack.pop()
    files[slot].write(chr(stack.pop()))
    return 1


def file_close():
    global files, stack
    slot = stack.pop()
    files[slot].close()
    files[slot] = 0
    return 0


def file_pos():
    global stack
    slot = stack.pop()
    return files[slot].tell()


def file_seek():
    global stack
    slot = stack.pop()
    pos = stack.pop()
    return files[slot].seek(pos, 0)


def file_size():
    global stack
    slot = stack.pop()
    at = files[slot].tell()
    files[slot].seek(0, 2)  # SEEK_END
    end = files[slot].tell()
    files[slot].seek(at, 0)  # SEEK_SET
    return end


def file_delete():
    global stack
    name = extract_string(stack.pop())
    i = 0
    if os.path.exists(name):
        os.remove(name)
        i = 1
    return i


def div_mod(a, b):
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


def find_entry(named):
    header = memory.fetch(2)
    Done = False
    while header != 0 and not Done:
        if named == extract_string(header + 3):
            Done = True
        else:
            header = memory.fetch(header)
    return header


def get_input():
    return ord(sys.stdin.read(1))


def display_character():
    global stack
    if stack.tos() > 0 and stack.tos() < 128:
        if stack.tos() == 8:
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
    stack.push(memory.fetch(ip))


def i_du():
    stack.dup()

def i_dr():
    stack.drop()


def i_sw():
     stack.swap()


def i_pu():
    global ip, memory, stack, address
    address.append(stack.pop())


def i_po():
    global ip, memory, stack, address
    stack.push(address.pop())


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
        stack.push(-1)
    else:
        stack.push(0)


def i_ne():
    global ip, memory, stack, address
    a = stack.pop()
    b = stack.pop()
    if b != a:
        stack.push(-1)
    else:
        stack.push(0)


def i_lt():
    global ip, memory, stack, address
    a = stack.pop()
    b = stack.pop()
    if b < a:
        stack.push(-1)
    else:
        stack.push(0)


def i_gt():
    global ip, memory, stack, address
    a = stack.pop()
    b = stack.pop()
    if b > a:
        stack.push(-1)
    else:
        stack.push(0)


def i_fe():
    target = stack.pop()
    if target == -1:
        stack.push(stack.depth())
    elif target == -2:
        stack.push(len(address))
    elif target == -3:
        stack.push(len(memory))
    elif target == -4:
        stack.push(2147483648)
    elif target == -5:
        stack.push(2147483647)
    else:
        stack.push(memory.fetch(target))


def i_st():
    global ip, memory, stack, address
    mi = stack.pop()
    memory.store(stack.pop(), mi)


def i_ad():
    t = stack.pop()
    v = stack.pop()
    stack.push(unpack("=l", pack("=L", (t + v) & 0xFFFFFFFF))[0])


def i_su():
    t = stack.pop()
    v = stack.pop()
    stack.push(unpack("=l", pack("=L", (v - t) & 0xFFFFFFFF))[0])


def i_mu():
    t = stack.pop()
    v = stack.pop()
    stack.push(unpack("=l", pack("=L", (v * t) & 0xFFFFFFFF))[0])


def i_di():
    t = stack.pop()
    v = stack.pop()
    b, a = div_mod(v, t)
    stack.push(unpack("=l", pack("=L", a & 0xFFFFFFFF))[0])
    stack.push(unpack("=l", pack("=L", b & 0xFFFFFFFF))[0])


def i_an():
    global ip, memory, stack, address
    t = stack.pop()
    m = stack.pop()
    stack.push(m & t)


def i_or():
    global ip, memory, stack, address
    t = stack.pop()
    m = stack.pop()
    stack.push(m | t)


def i_xo():
    global ip, memory, stack, address
    t = stack.pop()
    m = stack.pop()
    stack.push(m ^ t)


def i_sh():
    global ip, memory, stack, address
    t = stack.pop()
    v = stack.pop()
    
    if t < 0:
        v <<= t * -1
    else:
        v >>= t
        
    stack.push(v)


def i_zr():
    global ip, memory, stack, address
    if stack.tos() == 0:
        stack.pop()
        ip = address.pop()


def i_ha():
    global ip, memory, stack, address
    ip = 9000000


def i_ie():
    stack.push(6)


def i_iq():
    device = stack.pop()
    if device == 0:  # generic output
        stack.push(0)
        stack.push(0)
    if device == 1:  # floating point
        stack.push(1)
        stack.push(2)
    if device == 2:  # files
        stack.push(0)
        stack.push(4)
    if device == 3:  # rng
        stack.push(0)
        stack.push(10)
    if device == 4:  # time
        stack.push(0)
        stack.push(5)
    if device == 5:  # scripting
        stack.push(0)
        stack.push(9)


float_instr = {
    0: lambda: floats.push(float(stack.pop())),  # number to float
    1: lambda: floats.push(float(extract_string(stack.pop()))),  # string to float
    2: lambda: stack.push(int(floats.pop())),  # float to number
    3: lambda: inject_string(str(floats.pop()), stack.pop()),  # float to string
    4: lambda: floats.add(),  # add
    5: lambda: floats.sub(),  # sub
    6: lambda: floats.mul(),  # mul
    7: lambda: floats.div(),  # div
    8: lambda: floats.floor(),  # floor
    9: lambda: floats.ceil(),  # ceil
    10: lambda: floats.sqrt(),  # sqrt
    11: lambda: stack.push(floats.eq()),  # eq
    12: lambda: stack.push(floats.neq()),  # -eq
    13: lambda: stack.push(floats.lt()),  # lt
    14: lambda: stack.push(floats.gt()),  # gt
    15: lambda: stack.push(floats.depth()),  # depth
    16: lambda: floats.dup(),  # dup
    17: lambda: floats.drop(),  # drop
    18: lambda: floats.swap(),  # swap
    19: lambda: floats.log(),  # log
    20: lambda: floats.pow(),  # pow
    21: lambda: floats.sin(),  # sin
    22: lambda: floats.cos(),  # cos
    23: lambda: floats.tan(),  # tan
    24: lambda: floats.asin(),  # asin
    25: lambda: floats.atan(),  # atan
    26: lambda: floats.acos(),  # acos
    27: lambda: afloats.push(floats.pop()),  # to alt
    28: lambda: floats.push(afloats.pop()),  # from alt
    29: lambda: stack.push(afloats.depth()),  # alt. depth
}
files_instr = {
    0: lambda: stack.push(file_open()),
    1: lambda: file_close(),
    2: lambda: stack.push(file_read()),
    3: lambda: file_write(),
    4: lambda: stack.push(file_pos()),
    5: lambda: file_seek(),
    6: lambda: stack.push(file_size()),
    7: lambda: file_delete(),
    8: lambda: 1 + 1,
}

rng_instr = {0: lambda: stack.push(rng())}

clock_instr = {
    0: lambda: stack.push(int(time.time())),
    1: lambda: stack.push(clock["day"]),
    2: lambda: stack.push(clock["month"]),
    3: lambda: stack.push(clock["year"]),
    4: lambda: stack.push(clock["hour"]),
    5: lambda: stack.push(clock["minute"]),
    6: lambda: stack.push(clock["second"]),
    7: lambda: stack.push(clock["day_utc"]),
    8: lambda: stack.push(clock["month_utc"]),
    9: lambda: stack.push(clock["year_utc"]),
    10: lambda: stack.push(clock["hour_utc"]),
    11: lambda: stack.push(clock["minute_utc"]),
    12: lambda: stack.push(clock["second_utc"]),
}


def i_ii():
    global stack, memory, floats, files
    device = stack.pop()
    if device == 0:  # generic output
        display_character()
    if device == 1:  # floating point
        action = stack.pop()
        float_instr[int(action)]()
    if device == 2:  # files
        action = stack.pop()
        files_instr[int(action)]()
    if device == 3:  # rng
        rng_instr[0]()
    if device == 4:  # clock
        action = stack.pop()
        clock_instr[int(action)]()
    if device == 5:  # scripting
        action = stack.pop()
        if action == 0:
            stack.push(len(sys.argv) - 2)
        if action == 1:
            a = stack.pop()
            b = stack.pop()
            stack.push(inject_string(sys.argv[a + 2], b))
        if action == 2:
            run_file(extract_string(stack.pop()))
        if action == 3:
            b = stack.pop()
            stack.push(inject_string(sys.argv[0], b))


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


def validate_opcode(opcode):
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


def extract_string(at):
    s = ""
    while memory.fetch(at) != 0:
        s = s + chr(memory.fetch(at))
        at = at + 1
    return s


def inject_string(s, to):
    global memory
    i = to
    for c in s:
        memory.store(ord(c), i)
        i = i + 1
    memory.store(0, i)
    return to


def execute(word, notfound, output="console"):
    global ip, memory, stack, address
    ip = word
    if len(address) == 0:
        address.append(0)
    while ip < 100000:
        if ip == notfound:
            print("ERROR: word not found!")
        opcode = memory.fetch(ip)
        if validate_opcode(opcode):
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
            print("Invalid Bytecode: ", opcode, "at", ip)
            ip = 2000000
        if len(address) == 0:
            ip = 2000000
        ip = ip + 1
    return


def run():
    done = False
    while not done:
        line = input("\nOk> ")
        if line == "bye":
            done = True
        else:
            for token in line.split(" "):
                inject_string(token, 1025)
                stack.push(1025)
                execute(interpreter, not_found)


def run_file(file):
    if not os.path.exists(file):
        print("File '{0}' not found".format(file))
        return

    in_block = False
    with open(file, "r") as source:
        for line in source.readlines():
            if line.rstrip() == "~~~":
                in_block = not in_block
            elif in_block:
                for token in line.strip().split(" "):
                    if token != "":
                        inject_string(token, 1025)
                        stack.push(1025)
                        execute(interpreter, not_found)


def update_image():
    import requests
    import shutil

    data = requests.get("http://forth.works/ngaImage", stream=True)
    with open("ngaImage", "wb") as f:
        data.raw.decode_content = True
        shutil.copyfileobj(data.raw, f)


def interactive_startup():
    cmd = input("Would you like to download the latest image?\n (y/n)\t")
    if cmd.lower().strip() == "y":
        update_image()
    cmd = input("Would you like to run a file?\n (y/n)\t")
    if cmd.lower().strip() == "n":
        run()
    else:
        cmd = input("Enter the name of the file you wish to run\t")
        if os.path.exists(cmd):
            run_file(cmd)
        else:
            print(" Error: File not found. ")


if __name__ == "__main__":
    interpreter = memory.fetch(find_entry("interpret") + 1)
    not_found = memory.fetch(find_entry("err:notfound") + 1)

    if len(sys.argv) == 1:
        run()

    if len(sys.argv) == 2:
        run_file(sys.argv[1])

    sources = []

    if len(sys.argv) > 2:
        i = 1
        e = len(sys.argv)
        while i < e:
            param = sys.argv[i]
            if param == "-f":
                i += 1
                sources.append(sys.argv[i])
            i += 1

    if len(sys.argv) > 2 and sys.argv[1][0] != "-":
        run_file(sys.argv[1])
    else:
        for source in sources:
            run_file(source)


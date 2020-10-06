#!/usr/bin/env python3

# Nga: a Virtual Machine
# Copyright (c) 2010 - 2019, Charles Childers
# Floating Point I/O by Arland Childers, (c) 2020
# Optimizations and process() rewrite by Greg Copeland
# -----------------------------------------------------

import os, sys, math, time, struct, random, datetime
from struct import pack, unpack

ip = 0
stack = [] * 128
address = []
memory = []


class clock:
    def __getitem__(self, id):
        now = datetime.datetime.now()
        ids = {
            "time": time.time,
            "year": now.year,
            "month": now.month,
            "day": now.day,
            "hour": now.hour,
            "minute": now.minute,
            "second": now.second,
            # No time_utc?w
            "year_utc": now.utcnow().year,
            "month_utc": now.utcnow().month,
            "day_utc": now.utcnow().day,
            "hour_utc": now.utcnow().hour,
            "minute_utc": now.utcnow().minute,
            "second_utc": now.utcnow().second,
        }
        return ids[id]


clock = clock()


class rng:
    def __call__(self, seed=None, new_seed=True):
        return random.randint(-2147483647, 2147483646)


rng = rng()


class float_stack(object):
    def __init__(self, *d):
        self.data = list(d)

    def __getitem__(self, id):
        return self.data[id]

    def add(self):
        self.data.append(self.data.pop() + self.data.pop())

    def sub(self):
        self.data.append(0 - (self.data.pop() - self.data.pop()))

    def mul(self):
        self.data.append(self.data.pop() * self.data.pop())

    def div(self):
        a, b = self.data.pop(), self.data.pop()
        self.data.append(b / a)

    def ceil(self):
        self.data.append(math.ceil(self.data.pop()))

    def floor(self):
        self.data.append(math.floor(self.data.pop()))

    def eq(self):
        return 0 - (self.data.pop() == self.data.pop())

    def neq(self):
        return 0 - (self.data.pop() != self.data.pop())

    def gt(self):
        a, b = self.data.pop(), self.data.pop()
        return 0 - (b > a)

    def lt(self):
        a, b = self.data.pop(), self.data.pop()
        return 0 - (b < a)

    def depth(self):
        return len(self.data)

    def drop(self):
        self.data.pop()

    def pop(self):
        return self.data.pop()

    def swap(self):
        a, b = self.data.pop(), self.data.pop()
        self.data += [a, b]

    def push(self, n):
        self.data.append(n)

    def log(self):
        a, b = self.data.pop(), self.data.pop()
        self.data.append(math.log(b, a))

    def power(self):
        a, b = self.data.pop(), self.data.pop()
        self.data.append(math.pow(a, b))

    def sin(self):
        self.data.append(math.sin(self.data.pop()))

    def cos(self):
        self.data.append(math.cos(self.data.pop()))

    def tan(self):
        self.data.append(math.tan(self.data.pop()))

    def asin(self):
        self.data.append(math.asin(self.data.pop()))

    def acos(self):
        self.data.append(math.acos(self.data.pop()))

    def atan(self):
        self.data.append(math.atan(self.data.pop()))


floats = float_stack()
afloats = float_stack()

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
    header = memory[2]
    Done = False
    while header != 0 and not Done:
        if named == extract_string(header + 3):
            Done = True
        else:
            header = memory[header]
    return header


def get_input():
    return ord(sys.stdin.read(1))


def display_character():
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
    stack[-1], stack[-2] = div_mod(b, a)
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
    stack.append(5)


def i_iq():
    device = stack.pop()
    if device == 0:  # generic output
        stack.append(0)
        stack.append(0)
    if device == 1:  # floating point
        stack.append(1)
        stack.append(2)
    if device == 2:  # files
        stack.append(0)
        stack.append(4)
    if device == 3:  # rng
        stack.append(0)
        stack.append(10)
    if device == 4:  # time
        stack.append(0)
        stack.append(5)


float_instr = {
    0: lambda: floats.push(float(stack.pop())),  # number to float
    1: lambda: floats.push(float(extract_string(stack.pop()))),  # string to float
    2: lambda: stack.append(int(floats.pop())),  # float to number
    3: lambda: inject_string(str(floats.pop()), stack.pop()),  # float to string
    4: lambda: floats.add(),  # add
    5: lambda: floats.sub(),  # sub
    6: lambda: floats.mul(),  # mul
    7: lambda: floats.div(),  # div
    8: lambda: floats.floor(),  # floor
    9: lambda: floats.ceil(),  # ceil
    10: lambda: floats.sqrt(),  # sqrt
    11: lambda: stack.append(floats.eq()),  # eq
    12: lambda: stack.append(floats.neq()),  # -eq
    13: lambda: stack.append(floats.lt()),  # lt
    14: lambda: stack.append(floats.gt()),  # gt
    15: lambda: stack.append(floats.depth()),  # depth
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
    29: lambda: stack.append(afloats.depth()),  # alt. depth
}
files_instr = {
    0: lambda: stack.append(file_open()),
    1: lambda: file_close(),
    2: lambda: stack.append(file_read()),
    3: lambda: file_write(),
    4: lambda: stack.append(file_pos()),
    5: lambda: file_seek(),
    6: lambda: stack.append(file_size()),
    7: lambda: file_delete(),
    8: lambda: 1 + 1,
}

rng_instr = {0: lambda: stack.append(rng())}

clock_instr = {
    0: lambda: stack.append(int(time.time())),
    1: lambda: stack.append(clock["day"]),
    2: lambda: stack.append(clock["month"]),
    3: lambda: stack.append(clock["year"]),
    4: lambda: stack.append(clock["hour"]),
    5: lambda: stack.append(clock["minute"]),
    6: lambda: stack.append(clock["second"]),
    7: lambda: stack.append(clock["day_utc"]),
    8: lambda: stack.append(clock["month_utc"]),
    9: lambda: stack.append(clock["year_utc"]),
    10: lambda: stack.append(clock["hour_utc"]),
    11: lambda: stack.append(clock["minute_utc"]),
    12: lambda: stack.append(clock["second_utc"]),
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
    i = at
    s = ""
    while memory[i] != 0:
        s = s + chr(memory[i])
        i = i + 1
    return s


def inject_string(s, to):
    global memory
    i = to
    for c in s:
        memory[i] = ord(c)
        i = i + 1
    memory[i] = 0


def execute(word, notfound, output="console"):
    global ip, memory, stack, address
    ip = word
    address.append(0)
    while ip < 100000 and len(address) > 0:
        if ip == notfound:
            print("ERROR: word not found!")
        opcode = memory[ip]
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
            print("Invalid Bytecode", opcode, ip)
            ip = 2000000
        ip = ip + 1
    return


def load_image():
    global memory
    cells = int(os.path.getsize("ngaImage") / 4)
    f = open("ngaImage", "rb")
    memory = list(struct.unpack(cells * "i", f.read()))
    f.close()
    remaining = 1000000 - cells
    memory.extend([0] * remaining)


def run():
    done = False
    while not done:
        line = input("\nOk> ")
        if line == "bye":
            done = True
        else:
            for token in line.split(" "):
                inject_string(token, 1025)
                stack.append(1025)
                execute(interpreter, not_found)


def run_file(file):  # untested, but should work.
    in_block = False
    with open(file, "r") as source:
        for line in source.readlines():
            if line.rstrip() == "~~~":
                in_block = not in_block
            elif in_block:
                for token in line.strip().split(" "):
                    if token != "":
                        inject_string(token, 1025)
                        stack.append(1025)
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
    load_image()
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
    load_image()
    interpreter = memory[find_entry("interpret") + 1]
    not_found = memory[find_entry("err:notfound") + 1]
    if len(sys.argv) > 1:
        for source in sys.argv[1:]:
            if os.path.exists(source):
                run_file(source)
            else:
                print("File '{0}' not found".format(source))
    else:
        run()

#!/usr/bin/env python3

# Nga: a Virtual Machine
# Copyright (c) 2010 - 2020, Charles Childers
# Floating Point I/O by Arland Childers, (c) 2020
# Optimizations and process() rewrite by Greg Copeland
# -------------------------------------------------------------
# This implementation of the VM differs from the reference
# model in a number of important ways.
#
# To aid in performance, it does the following:
# - caching the Retro dictionary in a Python dict()
# - replaces some Retro words with implementations in Python
#   - s:eq?
#   - s:length
#   - s:to-number
#   - d:lookup
#
# Each major component is managed as a separate class. We have
# a class for each I/O device, for each stack, and for the
# memory pool. The main VM is also in a separate class.
#
# It's intended that an amalgamation tool will be developed to
# combine the separate files into a single one for deployment.
# -------------------------------------------------------------

import os, sys, math, time, struct, random, datetime


class Clock:
    def __getitem__(self, id):
        import datetime
        import time

        now = datetime.datetime.now()
        ids = {
            "time": time.time,
            "year": now.year,
            "month": now.month,
            "day": now.day,
            "hour": now.hour,
            "minute": now.minute,
            "second": now.second,
            # No time_utc?
            "year_utc": now.utcnow().year,
            "month_utc": now.utcnow().month,
            "day_utc": now.utcnow().day,
            "hour_utc": now.utcnow().hour,
            "minute_utc": now.utcnow().minute,
            "second_utc": now.utcnow().second,
        }
        return ids[id]


import random


class RNG:
    def __call__(self):
        return random.randint(-2147483647, 2147483646)


import os


class FileSystem:
    def __init__(self):
        self.files = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

    def open(self, params):
        name, mode = params
        slot = 0
        i = 1
        while i < 8:
            if self.files[i] == 0:
                slot = i
            i += 1
        if slot > 0:
            if mode == 0:
                if os.path.exists(name):
                    self.files[slot] = open(name, "r")
                else:
                    slot = 0
            elif mode == 1:
                self.files[slot] = open(name, "w")
            elif mode == 2:
                self.files[slot] = open(name, "a")
            elif mode == 3:
                if os.path.exists(name):
                    self.files[slot] = open(name, "r+")
                else:
                    slot = 0
        return slot

    def read(self, slot):
        return ord(self.files[slot].read(1))

    def write(self, params):
        slot, char = params
        self.files[slot].write(chr(stack.pop()))
        return 1

    def close(self, slot):
        self.files[slot].close()
        self.files[slot] = 0
        return 0

    def pos(self, slot):
        return self.files[slot].tell()

    def seek(slot, pos):
        return self.files[slot].seek(pos, 0)

    def size(self, slot):
        at = self.files[slot].tell()
        self.files[slot].seek(0, 2)  # SEEK_END
        end = self.files[slot].tell()
        self.files[slot].seek(at, 0)  # SEEK_SET
        return end

    def delete(self, name):
        name = extract_string(stack.pop())
        i = 0
        if os.path.exists(name):
            os.remove(name)
            i = 1
        return i


class FloatStack(object):
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


class IntegerStack(list):
    def __init__(self):
        stack = [] * 128
        self.extend(stack)

    def depth(self):
        return len(self)

    def tos(self):
        return self[-1]

    def push(self, v):
        self.append(v)

    def dup(self):
        self.append(self[-1])

    def drop(self):
        self.pop()

    def swap(self):
        a = self[-2]
        self[-2] = self[-1]
        self[-1] = a


import os
import struct


class Memory(list):
    def __init__(self, source, initial, size):
        m = [0] * size
        self.extend(m)
        if len(initial) == 0:
            cells = int(os.path.getsize(source) / 4)
            f = open(source, "rb")
            i = 0
            for cell in list(struct.unpack(cells * "i", f.read())):
                self[i] = cell
                i = i + 1
            f.close()
        else:
            i = 0
            for cell in initial:
                self[i] = cell
                i = i + 1

    def load_image(self, name):
        cells = int(os.path.getsize(name) / 4)
        f = open(name, "rb")
        i = 0
        for cell in list(struct.unpack(cells * "i", f.read())):
            self[i] = cell
            i = i + 1
        f.close()

    def size(self):
        return len(self)


InitialImage = []


class Retro:
    def map_in(self, name):
        return self.memory[self.find_entry(name) + 1]

    def __init__(self):
        self.ip = 0
        self.stack = IntegerStack()
        self.address = IntegerStack()
        self.memory = Memory("ngaImage", InitialImage, 1000000)
        self.clock = Clock()
        self.rng = RNG()
        self.files = FileSystem()
        self.floats = FloatStack()
        self.afloats = FloatStack()
        self.Dictionary = self.populate_dictionary()
        self.Cached = self.cache_words()

        self.setup_devices()
        self.instructions = [
            self.i_nop,
            self.i_lit,
            self.i_dup,
            self.i_drop,
            self.i_swap,
            self.i_push,
            self.i_pop,
            self.i_jump,
            self.i_call,
            self.i_ccall,
            self.i_return,
            self.i_eq,
            self.i_neq,
            self.i_lt,
            self.i_gt,
            self.i_fetch,
            self.i_store,
            self.i_add,
            self.i_subtract,
            self.i_multiply,
            self.i_divmod,
            self.i_and,
            self.i_or,
            self.i_xor,
            self.i_shift,
            self.i_zreturn,
            self.i_halt,
            self.i_ienumerate,
            self.i_iquery,
            self.i_iinvoke,
        ]

    def div_mod(self, a, b):
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

    def cache_words(self):
        Cached = dict()
        Cached["interpreter"] = self.map_in("interpret")
        Cached["not_found"] = self.map_in("err:notfound")
        Cached["s:eq?"] = self.map_in("s:eq?")
        Cached["s:to-number"] = self.map_in("s:to-number")
        Cached["s:length"] = self.map_in("s:length")
        Cached["d:lookup"] = self.map_in("d:lookup")
        Cached["d:add-header"] = self.map_in("d:add-header")
        return Cached

    def populate_dictionary(self):
        Dictionary = dict()
        header = self.memory[2]
        while header != 0:
            named = self.extract_string(header + 4)
            if not named in Dictionary:
                Dictionary[named] = header
            header = self.memory[header]
        return Dictionary

    def find_entry(self, named):
        if named in self.Dictionary:
            return self.Dictionary[named]

        header = self.memory[2]
        Done = False
        while header != 0 and not Done:
            if named == self.extract_string(header + 4):
                self.Dictionary[named] = header
                Done = True
            else:
                header = self.memory[header]
        return header

    def get_input(self):
        return ord(sys.stdin.read(1))

    def display_character(self):
        if self.stack.tos() > 0 and self.stack.tos() < 128:
            if self.stack.tos() == 8:
                sys.stdout.write(chr(self.stack.pop()))
                sys.stdout.write(chr(32))
                sys.stdout.write(chr(8))
            else:
                sys.stdout.write(chr(self.stack.pop()))
        else:
            sys.stdout.write("\033[2J\033[1;1H")
            self.stack.pop()
        sys.stdout.flush()

    def i_nop(self):
        pass

    def i_lit(self):
        self.ip += 1
        self.stack.push(self.memory[self.ip])

    def i_dup(self):
        self.stack.dup()

    def i_drop(self):
        self.stack.drop()

    def i_swap(self):
        self.stack.swap()

    def i_push(self):
        self.address.push(self.stack.pop())

    def i_pop(self):
        self.stack.push(self.address.pop())

    def i_jump(self):
        self.ip = self.stack.pop() - 1

    def i_call(self):
        self.address.push(self.ip)
        self.ip = self.stack.pop() - 1

    def i_ccall(self):
        target = self.stack.pop()
        if self.stack.pop() != 0:
            self.address.push(self.ip)
            self.ip = target - 1

    def i_return(self):
        self.ip = self.address.pop()

    def i_eq(self):
        a = self.stack.pop()
        b = self.stack.pop()
        if b == a:
            self.stack.push(-1)
        else:
            self.stack.push(0)

    def i_neq(self):
        a = self.stack.pop()
        b = self.stack.pop()
        if b != a:
            self.stack.push(-1)
        else:
            self.stack.push(0)

    def i_lt(self):
        a = self.stack.pop()
        b = self.stack.pop()
        if b < a:
            self.stack.push(-1)
        else:
            self.stack.push(0)

    def i_gt(self):
        a = self.stack.pop()
        b = self.stack.pop()
        if b > a:
            self.stack.push(-1)
        else:
            self.stack.push(0)

    # The fetch instruction also handles certain
    # introspection queries.
    #
    # Of note is the min and max values for a cell.
    # In most VM implementations, this is limited
    # to 32 bit or 64 bit ranges, but Python allows
    # an unlimited range.
    #
    # I report as if the cells are capped at 128 bits
    # but you can safely ignore this if running on
    # the Python VM. (This does have an impact on
    # floating point values, if using the `e:` words
    # for converting them to/from an encoded format in
    # standard cells, but should not affect anything
    # else in the standard system)

    def i_fetch_query(self, target):
        if target == -1:
            self.stack.push(self.stack.depth())
        elif target == -2:
            self.stack.push(self.address.depth())
        elif target == -3:
            self.stack.push(self.memory.size())
        elif target == -4:
            self.stack.push(-2147483647)
        elif target == -5:
            self.stack.push(2147483646)
        else:
            raise IndexError

    def i_fetch(self):
        target = self.stack.pop()
        if target >= 0:
            self.stack.push(self.memory[target])
        else:
            self.i_fetch_query(target)

    def i_store(self):
        mi = self.stack.pop()
        self.memory[mi] = self.stack.pop()

    def i_add(self):
        t = self.stack.pop()
        v = self.stack.pop()
        self.stack.push(t + v)

    def i_subtract(self):
        t = self.stack.pop()
        v = self.stack.pop()
        self.stack.push(v - t)

    def i_multiply(self):
        t = self.stack.pop()
        v = self.stack.pop()
        self.stack.push(v * t)

    def i_divmod(self):
        t = self.stack.pop()
        v = self.stack.pop()
        b, a = self.div_mod(v, t)
        self.stack.push(a)
        self.stack.push(b)

    def i_and(self):
        t = self.stack.pop()
        m = self.stack.pop()
        self.stack.push(m & t)

    def i_or(self):
        t = self.stack.pop()
        m = self.stack.pop()
        self.stack.push(m | t)

    def i_xor(self):
        t = self.stack.pop()
        m = self.stack.pop()
        self.stack.push(m ^ t)

    def i_shift(self):
        t = self.stack.pop()
        v = self.stack.pop()

        if t < 0:
            v <<= t * -1
        else:
            v >>= t

        self.stack.push(v)

    def i_zreturn(self):
        if self.stack.tos() == 0:
            self.stack.pop()
            self.ip = self.address.pop()

    def i_halt(self):
        self.ip = 9000000

    def i_ienumerate(self):
        self.stack.push(6)

    def i_iquery(self):
        device = self.stack.pop()
        if device == 0:  # generic output
            self.stack.push(0)
            self.stack.push(0)
        if device == 1:  # floating point
            self.stack.push(1)
            self.stack.push(2)
        if device == 2:  # files
            self.stack.push(0)
            self.stack.push(4)
        if device == 3:  # rng
            self.stack.push(0)
            self.stack.push(10)
        if device == 4:  # time
            self.stack.push(0)
            self.stack.push(5)
        if device == 5:  # scripting
            self.stack.push(0)
            self.stack.push(9)

    def file_open_params(self):
        mode = self.stack.pop()
        name = self.extract_string(self.stack.pop())
        return name, mode

    def file_write_params(self):
        slot = self.stack.pop()
        char = self.stack.pop()
        return slot, char

    def setup_devices(self):
        self.files_instr = {
            0: lambda: self.stack.push(self.files.open(self.file_open_params())),
            1: lambda: self.files.close(self.stack.pop()),
            2: lambda: self.stack.push(self.files.read(self.stack.pop())),
            3: lambda: self.files.write(self.file_write_params()),
            4: lambda: self.stack.push(self.files.pos(self.stack.pop())),
            5: lambda: self.files.seek(),
            6: lambda: self.stack.push(self.files.size(self.stack.pop())),
            7: lambda: self.files.delete(self.extract_string(self.stack.pop())),
            8: lambda: 1 + 1,
        }

        self.rng_instr = {0: lambda: self.stack.push(self.rng())}

        self.clock_instr = {
            0: lambda: self.stack.push(int(time.time())),
            1: lambda: self.stack.push(self.clock["day"]),
            2: lambda: self.stack.push(self.clock["month"]),
            3: lambda: self.stack.push(self.clock["year"]),
            4: lambda: self.stack.push(self.clock["hour"]),
            5: lambda: self.stack.push(self.clock["minute"]),
            6: lambda: self.stack.push(self.clock["second"]),
            7: lambda: self.stack.push(self.clock["day_utc"]),
            8: lambda: self.stack.push(self.clock["month_utc"]),
            9: lambda: self.stack.push(self.clock["year_utc"]),
            10: lambda: self.stack.push(self.clock["hour_utc"]),
            11: lambda: self.stack.push(self.clock["minute_utc"]),
            12: lambda: self.stack.push(self.clock["second_utc"]),
        }

        self.float_instr = {
            0: lambda: self.floats.push(float(self.stack.pop())),
            1: lambda: self.floats.push(float(self.extract_string(self.stack.pop()))),
            2: lambda: self.stack.push(int(self.floats.pop())),
            3: lambda: self.inject_string(str(self.floats.pop()), self.stack.pop()),
            4: lambda: self.floats.add(),
            5: lambda: self.floats.sub(),
            6: lambda: self.floats.mul(),
            7: lambda: self.floats.div(),
            8: lambda: self.floats.floor(),
            9: lambda: self.floats.ceil(),
            10: lambda: self.floats.sqrt(),
            11: lambda: self.stack.push(self.floats.eq()),
            12: lambda: self.stack.push(self.floats.neq()),
            13: lambda: self.stack.push(self.floats.lt()),
            14: lambda: self.stack.push(self.floats.gt()),
            15: lambda: self.stack.push(self.floats.depth()),
            16: lambda: self.floats.dup(),
            17: lambda: self.floats.drop(),
            18: lambda: self.floats.swap(),
            19: lambda: self.floats.log(),
            20: lambda: self.floats.pow(),
            21: lambda: self.floats.sin(),
            22: lambda: self.floats.cos(),
            23: lambda: self.floats.tan(),
            24: lambda: self.floats.asin(),
            25: lambda: self.floats.atan(),
            26: lambda: self.floats.acos(),
            27: lambda: self.afloats.push(self.floats.pop()),
            28: lambda: self.floats.push(self.afloats.pop()),
            29: lambda: self.stack.push(self.afloats.depth()),
        }

    def i_iinvoke(self):
        device = self.stack.pop()
        #        print('dev:', device)
        if device == 0:
            self.display_character()
        if device == 1:
            action = self.stack.pop()
            self.float_instr[int(action)]()
        if device == 2:
            action = self.stack.pop()
            self.files_instr[int(action)]()
        if device == 3:
            self.rng_instr[0]()
        if device == 4:
            action = self.stack.pop()
            self.clock_instr[int(action)]()
        if device == 5:
            action = self.stack.pop()
            if action == 0:
                self.stack.push(len(sys.argv) - 2)
            if action == 1:
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.push(self.inject_string(sys.argv[a + 2], b))
            if action == 2:
                self.run_file(self.extract_string(self.stack.pop()))
            if action == 3:
                b = self.stack.pop()
                self.stack.push(self.inject_string(sys.argv[0], b))

    def validate_opcode(self, I0, I1, I2, I3):
        if (
            (I0 >= 0 and I0 <= 29)
            and (I1 >= 0 and I1 <= 29)
            and (I2 >= 0 and I2 <= 29)
            and (I3 >= 0 and I3 <= 29)
        ):
            return True
        else:
            return False

    def extract_string(self, at):
        s = ""
        while self.memory[at] != 0:
            s = s + chr(self.memory[at])
            at = at + 1
        return s

    def inject_string(self, s, to):
        for c in s:
            self.memory[to] = ord(c)
            to = to + 1
        self.memory[to] = 0

    def execute(self, word, notfound):
        self.ip = word
        if self.address.depth() == 0:
            self.address.push(0)
        while self.ip < 1000000:
            if self.ip == self.Cached["s:eq?"]:
                a = self.extract_string(self.stack.pop())
                b = self.extract_string(self.stack.pop())
                if a == b:
                    self.stack.push(-1)
                else:
                    self.stack.push(0)
                self.ip = self.address.pop()
            elif self.ip == self.Cached["d:lookup"]:
                name = self.extract_string(self.stack.pop())
                header = self.find_entry(name)
                self.stack.push(header)
                self.memory[self.Cached["d:lookup"] - 20] = header  # "which"
                self.ip = self.address.pop()
            elif self.ip == self.Cached["s:to-number"]:
                n = self.extract_string(self.stack.pop())
                self.stack.push(int(n))
                self.ip = self.address.pop()
            elif self.ip == self.Cached["s:length"]:
                n = len(self.extract_string(self.stack.pop()))
                self.stack.push(n)
                self.ip = self.address.pop()
            else:
                if self.ip == notfound:
                    print("ERROR: word not found!")
                if self.ip == self.Cached["d:add-header"]:
                    self.Dictionary[self.extract_string(self.stack[-3])] = self.memory[
                        3
                    ]
                opcode = self.memory[self.ip]
                I0 = opcode & 0xFF
                I1 = (opcode >> 8) & 0xFF
                I2 = (opcode >> 16) & 0xFF
                I3 = (opcode >> 24) & 0xFF
                if self.validate_opcode(I0, I1, I2, I3):
                    # print("Bytecode: ", I0, I1, I2, I3, "at", self.ip)
                    if I0 != 0:
                        self.instructions[I0]()
                    if I1 != 0:
                        self.instructions[I1]()
                    if I2 != 0:
                        self.instructions[I2]()
                    if I3 != 0:
                        self.instructions[I3]()
                else:
                    print("Invalid Bytecode: ", opcode, "at", self.ip)
                    self.ip = 2000000
            if self.address.depth() == 0:
                self.ip = 2000000
            self.ip = self.ip + 1
        return

    def run(self):
        done = False
        while not done:
            line = input("\nOk> ")
            if line == "bye":
                done = True
            else:
                for token in line.split():
                    self.inject_string(token, self.memory[7])
                    self.stack.push(self.memory[7])
                    self.execute(self.Cached["interpreter"], self.Cached["not_found"])

    def run_file(self, file):
        if not os.path.exists(file):
            print("File '{0}' not found".format(file))
            return

        in_block = False
        with open(file, "r") as source:
            for line in source.readlines():
                if line.rstrip() == "~~~":
                    in_block = not in_block
                elif in_block:
                    for token in line.strip().split():
                        self.inject_string(token, self.memory[7])
                        self.stack.push(self.memory[7])
                        self.execute(
                            self.Cached["interpreter"], self.Cached["not_found"]
                        )

    def update_image(self):
        import requests
        import shutil

        data = requests.get("https://forthworks.com/retro/ngaImage", stream=True)
        with open("ngaImage", "wb") as f:
            data.raw.decode_content = True
            shutil.copyfileobj(data.raw, f)

    def save_image(self):
        print("Writing {0} cells to {1}".format(self.memory[3], sys.argv[1]))
        with open(sys.argv[1], "wb") as file:
            j = 0
            while j <= self.memory[3]:
                cell = struct.unpack(
                    "=l", struct.pack("=L", self.memory[j] & 0xFFFFFFFF)
                )[0]
                file.write(struct.pack("i", cell))
                j = j + 1


if __name__ == "__main__":
    retro = Retro()
    for f in sys.argv[2:]:
        retro.run_file(f)
    retro.save_image()

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

import os, sys, math, time, struct, random, datetime, decimal

from ClockDevice import Clock
from RNGDevice import RNG
from FileSystemDevice import FileSystem

from FloatStack import FloatStack
from DecimalDevice import DecimalStack
from IntegerStack import IntegerStack
from Memory import Memory
from InitialImage import InitialImage


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
        self.decimals = DecimalStack()
        self.adecimals = DecimalStack()

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
            named = self.extract_string(header + 3)
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
            if named == self.extract_string(header + 3):
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
            self.stack.push(-170141183460469231731687303715884105728)
        elif target == -5:
            self.stack.push(170141183460469231731687303715884105727)
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
        self.stack.push(7)

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
        if device == 6:  # decimal
            self.stack.push(0)
            self.stack.push(20)

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

        self.decimal_instr = {
            0: lambda: self.decimals.push(decimal.Decimal(self.stack.pop())),
            1: lambda: self.decimals.push(decimal.Decimal(self.extract_string(self.stack.pop()))),
            2: lambda: self.stack.push(int(self.decimals.pop())),
            3: lambda: self.inject_string(str(self.decimals.pop()), self.stack.pop()),
            4: lambda: self.decimals.add(),
            5: lambda: self.decimals.sub(),
            6: lambda: self.decimals.mul(),
            7: lambda: self.decimals.div(),
            8: lambda: self.decimals.floor(),
            9: lambda: self.decimals.ceil(),
            10: lambda: self.decimals.sqrt(),
            11: lambda: self.stack.push(self.decimals.eq()),
            12: lambda: self.stack.push(self.decimals.neq()),
            13: lambda: self.stack.push(self.decimals.lt()),
            14: lambda: self.stack.push(self.decimals.gt()),
            15: lambda: self.stack.push(self.decimals.depth()),
            16: lambda: self.decimals.dup(),
            17: lambda: self.decimals.drop(),
            18: lambda: self.decimals.swap(),
            19: lambda: self.decimals.log(),
            20: lambda: self.decimals.pow(),
            21: lambda: self.decimals.sin(),
            22: lambda: self.decimals.cos(),
            23: lambda: self.decimals.tan(),
            24: lambda: self.decimals.asin(),
            25: lambda: self.decimals.atan(),
            26: lambda: self.decimals.acos(),
            27: lambda: self.adecimals.push(self.decimals.pop()),
            28: lambda: self.decimals.push(self.adecimals.pop()),
            29: lambda: self.stack.push(self.adecimals.depth()),
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
                prior = self.address
                prior_ip = self.ip
                self.address = IntegerStack()
                self.run_file(self.extract_string(self.stack.pop()))
                self.address = prior
                self.ip = prior_ip
            if action == 3:
                b = self.stack.pop()
                self.stack.push(self.inject_string(sys.argv[0], b))
        if device == 6:
            action = self.stack.pop()
            self.decimal_instr[int(action)]()

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
        if self.address.depth() == 0:
            self.address.push(0)
        self.ip = word
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
                    self.inject_string(token, 1024)
                    self.stack.push(1024)
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
                        self.inject_string(token, 1024)
                        self.stack.push(1024)
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


if __name__ == "__main__":
    retro = Retro()
    if len(sys.argv) == 1:
        retro.run()

    if len(sys.argv) == 2:
        retro.run_file(sys.argv[1])

    sources = []

    interactive = False

    if len(sys.argv) > 2:
        i = 1
        e = len(sys.argv)
        while i < e:
            param = sys.argv[i]
            if param == "-f":
                i += 1
                sources.append(sys.argv[i])
            elif param == "-u":
                i += 1
                retro.memory.load_image(sys.argv[i])
            elif param == "-i":
                interactive = True
            i += 1

    if len(sys.argv) > 2 and sys.argv[1][0] != "-":
        retro.run_file(sys.argv[1])
    else:
        for source in sources:
            retro.run_file(source)

    if interactive:
        retro.run()

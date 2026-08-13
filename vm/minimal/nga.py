#!/usr/bin/env python3
"""Minimal Nga virtual machine for RETRO.

Run with: python3 nga.py
"""

import struct
import sys


IMAGE_SIZE = 65536
ADDRESSES = 256
STACK_DEPTH = 256
CELL_MIN = -2147483647
CELL_MAX = 2147483646
CELL_MASK = 0xFFFFFFFF


def as_cell(value):
    """Convert value to a signed, 32-bit Nga cell."""
    value &= CELL_MASK
    return value if value < 0x80000000 else value - 0x100000000


class NgaVm:
    def __init__(self):
        self.sp = 0
        self.rp = 0
        self.ip = 0
        self.data = [0] * STACK_DEPTH
        self.address = [0] * ADDRESSES
        self.memory = [0] * (IMAGE_SIZE + 1)

    def halt(self):
        self.ip = IMAGE_SIZE

    @staticmethod
    def valid_memory_address(value):
        return 0 <= value <= IMAGE_SIZE

    def can_push_data(self):
        return self.sp < STACK_DEPTH - 1

    def can_pop_data(self):
        return self.sp > 0

    def can_push_address(self):
        return self.rp < ADDRESSES - 1

    def can_pop_address(self):
        return self.rp > 0

    def tos(self):
        return self.data[self.sp]

    def nos(self):
        return self.data[self.sp - 1]

    def tors(self):
        return self.address[self.rp]

    def stack_push(self, value):
        if not self.can_push_data():
            self.halt()
            return
        self.sp += 1
        self.data[self.sp] = as_cell(value)

    def stack_pop(self):
        if not self.can_pop_data():
            self.halt()
            return 0
        value = self.data[self.sp]
        self.data[self.sp] = 0
        self.sp -= 1
        return value

    def load_image(self, image_name):
        with open(image_name, "rb") as image:
            for index in range(IMAGE_SIZE):
                cell = image.read(4)
                if not cell:
                    return
                if len(cell) != 4:
                    raise ValueError("image is not a sequence of 32-bit cells")
                self.memory[index] = struct.unpack("<i", cell)[0]

    def execute(self, cell):
        self.rp = 1
        self.ip = cell
        while 0 <= self.ip < IMAGE_SIZE:
            self.process_opcode_bundle(self.memory[self.ip])
            self.ip += 1
            if self.rp == 0:
                self.ip = IMAGE_SIZE

    def inst_li(self):
        if self.ip >= IMAGE_SIZE - 1 or not self.can_push_data():
            self.halt()
            return
        self.ip += 1
        self.stack_push(self.memory[self.ip])

    def inst_du(self):
        if not self.can_pop_data():
            self.halt()
            return
        self.stack_push(self.tos())

    def inst_dr(self):
        self.stack_pop()

    def inst_sw(self):
        if self.sp < 2:
            self.halt()
            return
        self.data[self.sp], self.data[self.sp - 1] = self.nos(), self.tos()

    def inst_pu(self):
        if not self.can_push_address() or not self.can_pop_data():
            self.halt()
            return
        self.rp += 1
        self.address[self.rp] = self.tos()
        self.inst_dr()

    def inst_po(self):
        if not self.can_pop_address() or not self.can_push_data():
            self.halt()
            return
        self.stack_push(self.tors())
        self.rp -= 1

    def inst_ju(self):
        if not self.can_pop_data() or not 0 <= self.tos() < IMAGE_SIZE:
            self.halt()
            return
        self.ip = self.tos() - 1
        self.inst_dr()

    def inst_ca(self):
        if not self.can_push_address() or not self.can_pop_data() or not 0 <= self.tos() < IMAGE_SIZE:
            self.halt()
            return
        self.rp += 1
        self.address[self.rp] = self.ip
        self.ip = self.tos() - 1
        self.inst_dr()

    def inst_cc(self):
        if self.sp < 2:
            self.halt()
            return
        target = self.stack_pop()
        flag = self.stack_pop()
        if flag:
            if not self.can_push_address() or not 0 <= target < IMAGE_SIZE:
                self.halt()
                return
            self.rp += 1
            self.address[self.rp] = self.ip
            self.ip = target - 1

    def inst_re(self):
        if not self.can_pop_address() or not 0 <= self.tors() < IMAGE_SIZE:
            self.halt()
            return
        self.ip = self.tors()
        self.rp -= 1

    def compare(self, result):
        if self.sp < 2:
            self.halt()
            return
        self.data[self.sp - 1] = -1 if result else 0
        self.inst_dr()

    def inst_fe(self):
        if not self.can_pop_data():
            self.halt()
            return
        value = self.tos()
        if value == -1:
            self.data[self.sp] = self.sp - 1
        elif value == -2:
            self.data[self.sp] = self.rp
        elif value == -3:
            self.data[self.sp] = IMAGE_SIZE
        elif value == -4:
            self.data[self.sp] = CELL_MIN
        elif value == -5:
            self.data[self.sp] = CELL_MAX
        elif self.valid_memory_address(value):
            self.data[self.sp] = self.memory[value]
        else:
            self.halt()

    def inst_st(self):
        if self.sp < 2:
            self.halt()
            return
        address = self.tos()
        if not self.valid_memory_address(address):
            self.halt()
            return
        self.memory[address] = self.nos()
        self.inst_dr()
        self.inst_dr()

    def inst_di(self):
        if self.sp < 2:
            self.halt()
            return
        divisor = self.tos()
        dividend = self.nos()
        if divisor == 0 or (divisor == -1 and dividend == -2147483648):
            self.halt()
            return
        quotient = abs(dividend) // abs(divisor)
        if (dividend < 0) != (divisor < 0):
            quotient = -quotient
        self.data[self.sp] = quotient
        self.data[self.sp - 1] = dividend - quotient * divisor

    def inst_sh(self):
        if self.sp < 2:
            self.halt()
            return
        shift = self.tos()
        value = self.nos()
        if not -31 <= shift <= 31:
            self.halt()
            return
        if shift < 0:
            result = as_cell((value & CELL_MASK) << -shift)
        else:
            result = value >> shift
        self.data[self.sp - 1] = result
        self.inst_dr()

    def inst_zr(self):
        if not self.can_pop_data():
            self.halt()
            return
        if self.tos() != 0:
            return
        if not self.can_pop_address() or not 0 <= self.tors() < IMAGE_SIZE:
            self.halt()
            return
        self.inst_dr()
        self.ip = self.tors()
        self.rp -= 1

    def inst_iq(self):
        if not self.can_pop_data():
            self.halt()
        elif self.tos() == 0:
            self.inst_dr()
            self.stack_push(0)
            self.stack_push(0)
        elif self.tos() == 1:
            self.inst_dr()
            self.stack_push(1)
            self.stack_push(1)

    def inst_ii(self):
        if not self.can_pop_data():
            self.halt()
        elif self.tos() == 0:
            self.inst_dr()
            value = self.stack_pop()
            if self.ip != IMAGE_SIZE:
                sys.stdout.buffer.write(bytes((value & 0xFF,)))
                sys.stdout.buffer.flush()
        elif self.tos() == 1:
            value = sys.stdin.buffer.read(1)
            if not value:
                self.halt()
                return
            self.inst_dr()
            self.stack_push(value[0])
        else:
            self.inst_dr()

    def instruction(self, opcode):
        if opcode == 0:
            return
        if opcode == 1:
            self.inst_li()
        elif opcode == 2:
            self.inst_du()
        elif opcode == 3:
            self.inst_dr()
        elif opcode == 4:
            self.inst_sw()
        elif opcode == 5:
            self.inst_pu()
        elif opcode == 6:
            self.inst_po()
        elif opcode == 7:
            self.inst_ju()
        elif opcode == 8:
            self.inst_ca()
        elif opcode == 9:
            self.inst_cc()
        elif opcode == 10:
            self.inst_re()
        elif opcode == 11:
            self.compare(self.nos() == self.tos()) if self.sp >= 2 else self.halt()
        elif opcode == 12:
            self.compare(self.nos() != self.tos()) if self.sp >= 2 else self.halt()
        elif opcode == 13:
            self.compare(self.nos() < self.tos()) if self.sp >= 2 else self.halt()
        elif opcode == 14:
            self.compare(self.nos() > self.tos()) if self.sp >= 2 else self.halt()
        elif opcode == 15:
            self.inst_fe()
        elif opcode == 16:
            self.inst_st()
        elif opcode == 17:
            self.compare_binary(lambda a, b: as_cell(a + b))
        elif opcode == 18:
            self.compare_binary(lambda a, b: as_cell(a - b))
        elif opcode == 19:
            self.compare_binary(lambda a, b: as_cell(a * b))
        elif opcode == 20:
            self.inst_di()
        elif opcode == 21:
            self.compare_binary(lambda a, b: a & b)
        elif opcode == 22:
            self.compare_binary(lambda a, b: a | b)
        elif opcode == 23:
            self.compare_binary(lambda a, b: a ^ b)
        elif opcode == 24:
            self.inst_sh()
        elif opcode == 25:
            self.inst_zr()
        elif opcode == 26:
            self.halt()
        elif opcode == 27:
            self.stack_push(2)
        elif opcode == 28:
            self.inst_iq()
        elif opcode == 29:
            self.inst_ii()
        else:
            self.halt()

    def compare_binary(self, operation):
        if self.sp < 2:
            self.halt()
            return
        self.data[self.sp - 1] = operation(self.nos(), self.tos())
        self.inst_dr()

    def process_opcode_bundle(self, opcode):
        opcode &= CELL_MASK
        for shift in (0, 8, 16, 24):
            self.instruction((opcode >> shift) & 0xFF)


def main():
    vm = NgaVm()
    try:
        vm.load_image("ngaImage")
    except (OSError, ValueError):
        return
    vm.execute(0)


if __name__ == "__main__":
    main()

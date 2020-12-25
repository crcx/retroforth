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

    def size(self):
        return len(self)

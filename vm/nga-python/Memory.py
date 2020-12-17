import os
import struct


class Memory(list):
    def __init__(self, source, size):
        m = [0] * size
        self.extend(m)
        cells = int(os.path.getsize(source) / 4)
        f = open(source, "rb")
        i = 0
        for cell in list(struct.unpack(cells * "i", f.read())):
            self[i] = cell
            i = i + 1
        f.close()

    def size(self):
        return len(self)

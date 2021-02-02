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
                if type(cell) == list:
                    for v in range(0, cell[0]):
                        self[i] = cell[1]
                        i = i + 1
                else:
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

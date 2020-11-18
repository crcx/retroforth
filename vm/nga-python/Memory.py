import os
import struct

class Memory():
    def __init__(self, source, size):
        self.memory = [0] * size
        cells = int(os.path.getsize(source) / 4)
        f = open(source, "rb")
        i = 0
        for cell in list(struct.unpack(cells * "i", f.read())):
            self.memory[i] = cell
            i = i + 1
        f.close()
        self.length = size
    
    def store(self, value, address):
        self.memory[address] = value
    
    def fetch(self, address):
        return self.memory[address]
    
    def size(self):
        return self.length

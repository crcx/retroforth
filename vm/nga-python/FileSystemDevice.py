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

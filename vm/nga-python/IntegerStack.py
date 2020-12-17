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

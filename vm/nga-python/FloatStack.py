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

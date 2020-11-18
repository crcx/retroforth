class IntegerStack():
    def __init__(self):
        self.stack = [] * 128
    
    def depth(self):
        return len(self.stack)
        
    def tos(self):
        return self.stack[-1]
    
    def push(self, v):
        self.stack.append(v)

    def append(self, v):
        self.stack.append(v)
    
    def pop(self):
        return self.stack.pop()
    
    def dup(self):
        self.stack.append(self.stack[-1])
    
    def drop(self):
        self.stack.pop()
    
    def swap(self):
        a = self.stack[-2]
        self.stack[-2] = self.stack[-1]
        self.stack[-1] = a


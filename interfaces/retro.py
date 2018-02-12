#!/usr/bin/env python3

# Nga: a Virtual Machine
# Copyright (c) 2010 - 2018, Charles Childers
# Optimizations and process() rewrite by Greg Copeland
# -----------------------------------------------------

import os, sys, math, time, struct
from struct import pack, unpack

ip = 0
stack = [] * 128
address = []
memory = []

EXIT = 0x0FFFFFFF
ext = EXIT

def rxDivMod( a, b ):
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


def findEntry(named):
    header = memory[2]
    Done = False
    while (header != 0 and not Done):
        if named == extractString(header + 3):
            Done = True
        else:
            header = memory[header]
    return header

def rxGetInput():
    return ord(sys.stdin.read(1))

def rxDisplayCharacter():
    global stack
    if stack[-1] > 0 and stack[-1] < 128:
      if stack[-1] == 8:
        sys.stdout.write(chr(stack.pop()))
        sys.stdout.write(chr(32))
        sys.stdout.write(chr(8))
      else:
        sys.stdout.write(chr(stack.pop()))
    else:
      sys.stdout.write("\033[2J\033[1;1H")
      stack.pop()
    sys.stdout.flush()
    return ip

def processOpcode(opcode):
      global ip, stack, address, memory
      if   opcode ==  0:   # nop
        pass

def i_no():
    pass

def i_li():
    global ip, memory, stack, address
    ip += 1
    stack.append( memory[ip] )
        
def i_du():
    global ip, memory, stack, address
    stack.append( stack[-1] )
        
def i_dr():
    global ip, memory, stack, address
    stack.pop()

def i_sw():
    global ip, memory, stack, address
    a = stack[-2]
    stack[-2] = stack[-1]
    stack[-1] = a

def i_pu():
    global ip, memory, stack, address
    address.append(stack.pop())

def i_po():
    global ip, memory, stack, address
    stack.append(address.pop())

def i_ju():
    global ip, memory, stack, address
    ip = stack.pop() - 1
    
def i_ca():
    global ip, memory, stack, address
    address.append(ip)
    ip = stack.pop() - 1
   
def i_cc():
    global ip, memory, stack, address
    target = stack.pop()
    if stack.pop() != 0:
        address.append(ip)
        ip = target - 1

def i_re():
    global ip, memory, stack, address
    ip = address.pop()

def i_eq():
    global ip, memory, stack, address
    a = stack.pop()
    b = stack.pop()
    if b == a:
        stack.append(-1)
    else:
        stack.append(0)

def i_ne():
    global ip, memory, stack, address
    a = stack.pop()
    b = stack.pop()
    if b != a:
        stack.append(-1)
    else:
        stack.append(0)

def i_lt():
    global ip, memory, stack, address
    a = stack.pop()
    b = stack.pop()
    if b < a:
        stack.append(-1)
    else:
        stack.append(0)

def i_gt():
    global ip, memory, stack, address
    a = stack.pop()
    b = stack.pop()
    if b > a:
        stack.append(-1)
    else:
        stack.append(0)

def i_fe():
    global ip, memory, stack, address
    stack[-1] = memory[stack[-1]]

def i_st():
    global ip, memory, stack, address
    mi = stack.pop()
    memory[mi] = stack.pop()

def i_ad():
    global ip, memory, stack, address
    t = stack.pop()
    stack[ -1 ] += t
    stack[-1] = unpack('=l', pack('=L', stack[-1] & 0xffffffff))[0]

def i_su():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] -= t
    stack[-1] = unpack('=l', pack('=L', stack[-1] & 0xffffffff))[0]
            
def i_mu():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] *= t
    stack[-1] = unpack('=l', pack('=L', stack[-1] & 0xffffffff))[0]
        
def i_di():
    global ip, memory, stack, address
    a = stack[-1]
    b = stack[-2]
    stack[-1], stack[-2] = rxDivMod( b, a )
    stack[-1] = unpack('=l', pack('=L', stack[-1] & 0xffffffff))[0]
    stack[-2] = unpack('=l', pack('=L', stack[-2] & 0xffffffff))[0]
    
def i_an():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] &= t

def i_or():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] |= t
   
def i_xo():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] ^= t

def i_sh():
    global ip, memory, stack, address
    t = stack.pop()
    stack[-1] <<= t
    t = stack.pop()
    stack[-1] >>= t

def i_zr():
    global ip, memory, stack, address
    if stack[-1] == 0:
        stack.pop()
        ip = address.pop()
          
def i_en():
    global ip, memory, stack, address
    ip = 9000000

instructions = [i_no, i_li, i_du, i_dr, i_sw, i_pu, i_po, i_ju, i_ca, i_cc, i_re, i_eq, i_ne, i_lt, i_gt, i_fe, i_st, i_ad, i_su, i_mu, i_di, i_an, i_or, i_xo, i_sh, i_zr, i_en]

def validateOpcode(opcode):
    I0 = opcode & 0xFF
    I1 = (opcode >> 8) & 0xFF
    I2 = (opcode >> 16) & 0xFF
    I3 = (opcode >> 24) & 0xFF
    if (I0 >= 0 and I0 <= 26) and (I1 >= 0 and I1 <= 26) and (I2 >= 0 and I2 <= 26) and (I3 >= 0 and I3 <= 26):
       return True
    else:
       return False

def extractString( at ):
    i = at
    s = ''
    while (memory[i] != 0):
        s = s + chr(memory[i])
        i = i + 1
    return s

def injectString( s, to ):
    global memory
    i = to
    for c in s:
        memory[i] = ord(c)
        i = i + 1
    memory[i] = 0

def execute(word):
    global ip, memory, stack, address
    ip = word
    address.append(0)
    while ip < 100000 and len(address) > 0:
        if ip == memory[findEntry('err:notfound') + 1]:
            print('ERROR: word not found!')
        opcode = memory[ip]
        if validateOpcode(opcode):
            I0 = opcode & 0xFF
            I1 = (opcode >> 8) & 0xFF
            I2 = (opcode >> 16) & 0xFF
            I3 = (opcode >> 24) & 0xFF
            if I0 != 0: instructions[I0]()
            if I1 != 0: instructions[I1]()
            if I2 != 0: instructions[I2]()
            if I3 != 0: instructions[I3]()
        else:
            if opcode == 1000:
                rxDisplayCharacter()
            else:
                print('Invalid Bytecode', opcode, ip)
                ip = 2000000
        ip = ip + 1

def words():
    header = memory[2]
    while (header != 0):
        print(header, extractString(header + 3))
        header = memory[header]

def run():
  global memory
  cells = int(os.path.getsize('ngaImage') / 4)

  f = open( 'ngaImage', 'rb' )
  memory = list(struct.unpack( cells * 'i', f.read() ))
  f.close()

  remaining = 1000000 - cells
  memory.extend( [0] * remaining )

  s = 'words' #input('OK:> ')
  injectString(s, 1025)
  stack.append(1025)
  header = findEntry('interpret')
  execute(memory[header + 1])

if __name__ == "__main__":
  run()


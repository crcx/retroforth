#!/usr/bin/env python

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

      elif opcode ==  1:   # lit
        ip += 1
        stack.append( memory[ip] )

      elif opcode ==  2:   # dup
        stack.append( stack[-1] )

      elif opcode ==  3:   # drop
        stack.pop()

      elif opcode ==  4:   # swap
        a = stack[-2]
        stack[-2] = stack[-1]
        stack[-1] = a

      elif opcode ==  5:   # push
        address.append( stack.pop() )

      elif opcode ==  6:   # pop
        stack.append( address.pop() )

      elif opcode ==  7:   # jump
        ip = stack.pop() - 1

      elif opcode ==  8:   # call
        address.append(ip)
        ip = stack.pop() - 1
        
      elif opcode == 9:    # ccall
        target = stack.pop()
        flag = stack.pop()
        if (flag != 0):
          address.append(ip)
          ip = target - 1

      elif opcode ==  10:  # return
        ip = address.pop()

      elif opcode == 11:   # eq
        a = stack.pop()
        if stack[-1] == a:
          stack[-1] = -1
        else:
          stack[-1] = 0

      elif opcode == 12:   # neq
        a = stack.pop()
        if stack[-1] != a:
          stack[-1] = -1
        else:
          stack[-1] = 0

      elif opcode == 13:   # lt
        a = stack.pop()
        if stack[-1] < a:
          stack[-1] = -1
        else:
          stack[-1] = 0

      elif opcode == 14:   # gt
        a = stack.pop()
        if stack[-1] > a:
          stack[-1] = -1
        else:
          stack[-1] = 0

      elif opcode == 15:   # @
        stack[-1] = memory[stack[-1]]

      elif opcode == 16:   # !
        mi = stack.pop()
        memory[mi] = stack.pop()

      elif opcode == 17:   # +
        t = stack.pop()
        stack[ -1 ] += t
        stack[-1] = unpack('=l', pack('=L', stack[-1] & 0xffffffff))[0]

      elif opcode == 18:   # -
        t = stack.pop()
        stack[-1] -= t
        stack[-1] = unpack('=l', pack('=L', stack[-1] & 0xffffffff))[0]

      elif opcode == 19:   # *
        t = stack.pop()
        stack[-1] *= t
        stack[-1] = unpack('=l', pack('=L', stack[-1] & 0xffffffff))[0]

      elif opcode == 20:   # /mod
        a = stack[-1]
        b = stack[-2]
        stack[-1], stack[-2] = rxDivMod( b, a )
        stack[-1] = unpack('=l', pack('=L', stack[-1] & 0xffffffff))[0]
        stack[-2] = unpack('=l', pack('=L', stack[-2] & 0xffffffff))[0]

      elif opcode == 21:   # and
        t = stack.pop()
        stack[-1] &= t

      elif opcode == 22:   # or
        t = stack.pop()
        stack[-1] |= t

      elif opcode == 23:   # xor
        t = stack.pop()
        stack[-1] ^= t

      elif opcode == 24:   # <<
        t = stack.pop()
        stack[-1] <<= t
        t = stack.pop()
        stack[-1] >>= t

      elif opcode == 25:   # 0;
        if stack[-1] == 0:
          stack.pop()
          ip = address.pop()

      elif opcode == 26:   # inc
        stack[-1] += 1

      elif opcode == 26:   # dec
        ip = 1000000

def validateOpcode(opcode):
    I0 = opcode & 0xFF
    I1 = (opcode >> 8) & 0xFF
    I2 = (opcode >> 16) & 0xFF
    I3 = (opcode >> 24) & 0xFF
    if (I0 >= 0 and I0 <= 26) and (I1 >= 0 and I1 <= 26) and (I2 >= 0 and I2 <= 26) and (I3 >= 0 and I3 <= 26):
       return True
    else:
       return False

def execute(word):
    global ip, memory, stack, address
    ip = word
    address.append(0)
    while ip < 100000 and len(address) > 0:
        if ip == memory[findEntry('err:notfound') + 1]:
            print('ooo: word not found!')
        opcode = memory[ip]
        if validateOpcode(opcode):
            I0 = opcode & 0xFF
            I1 = (opcode >> 8) & 0xFF
            I2 = (opcode >> 16) & 0xFF
            I3 = (opcode >> 24) & 0xFF
            if I0 != 0: processOpcode(I0)
            if I1 != 0: processOpcode(I1)
            if I2 != 0: processOpcode(I2)
            if I3 != 0: processOpcode(I3)
        else:
            if opcode == 1000:
                rxDisplayCharacter()
            else:
                print('Invalid Bytecode', opcode, ip)
                ip = 2000000
        ip = ip + 1

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

  try:
      while True:
          s = input('OK:> ')
          injectString(s, 1025)
          stack.append(1025)
          header = findEntry('interpret')
          execute(memory[header + 1])
  except:
      print('Error!')
      pass

if __name__ == "__main__":
  run()


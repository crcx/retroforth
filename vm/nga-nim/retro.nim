#  Copyright (c) 2021 Jorge Acereda Macia <jacereda@gmail.com>
#  Copyright (c) 2021 Charles Childers <crc@forth.works>

import strutils
import logging

proc go() =
  type Op = enum
      VM_NOP
      VM_LIT
      VM_DUP
      VM_DROP
      VM_SWAP
      VM_PUSH
      VM_POP
      VM_JUMP
      VM_CALL
      VM_CCALL
      VM_RETURN
      VM_EQ
      VM_NEQ
      VM_LT
      VM_GT
      VM_FETCH
      VM_STORE
      VM_ADD
      VM_SUB
      VM_MUL
      VM_DIVMOD
      VM_AND
      VM_OR
      VM_XOR
      VM_SHIFT
      VM_ZRET
      VM_HALT
      VM_IE
      VM_IQ
      VM_II
      VM_RES0
      VM_RES1

  type CELL = int32
  type UCELL = uint32

  const IMAGE_SIZE = 524288           # number of cells in image
  const DSTACK_DEPTH = 32             # max depth for data stack
  const RSTACK_DEPTH   = 128          # max depth for address stack
  const XT = 1                        # dictionary header offset for XT

  var ds: array[DSTACK_DEPTH, CELL]   # data stack values
  var rs: array[RSTACK_DEPTH, CELL]   # address stack values
  var im: array[IMAGE_SIZE, CELL]     # image memory
  var ts: CELL = 0                    # top of stack
  var sp: UCELL = 0                   # data stack pointer
  var rp: UCELL = 0                   # return stack pointer

  var log = newConsoleLogger()
  addHandler(log)

  proc loadImage(path: string) =
    let f = path.open()
    discard f.readBuffer(addr im[0], sizeof(im))
    f.close()

  proc extract(starting: int): string =
    var done = false
    var i = starting
    while not done:
      let c = im[i]
      inc i
      if c == 0:
        done = true
      else:
        result.add char(c)

  proc inject(s: string, at: int) =
    var i = at
    for c in s:
      im[i] = CELL(c)
      inc i
    im[i] = 0

  proc lookup(name: string): CELL =
    var i = im[2]
    while im[i] != 0:
      let target = extract(i + 3)
      if name == target:
        return i
      else:
        i = im[i]
    return 0

  loadImage("ngaImage")

  proc ddump() : string =
    result = " ds:"&sp.toHex
    for i in 2..sp:
      result = result & " " & ds[i].toHex
    if sp > 0:
      result = result & " " & ts.toHex

  proc rdump() : string =
    result = " rs:"&rp.toHex
    if rp < RSTACK_DEPTH:
      for i in 0..rp:
        result = result & " " & rs[i].toHex

  while true:
    let input = stdin.readLine()
    for word in input.split(' '):
      if word == "bye":
        return
      inject(word, im[7])
      inc sp
      ds[sp] = ts
      ts = im[7]
      rp = 0
      var ip: UCELL = UCELL(im[XT+lookup("interpret")]) - 1
      var opcode : UCELL = 0
      while true:
        {.computedGoto.}
        let op = Op(opcode and 255)
        # echo $op & ddump() & rdump()
        opcode = opcode shr 8
        case op:
          of VM_NOP:
            if opcode == 0:
              if ip < IMAGE_SIZE:
                inc ip
                opcode = im[ip].UCELL
                if (opcode and 0xe0e0e0e0.UCELL) != 0:
                  break
                if rp >= RSTACK_DEPTH:
                  break
              else:
                break
          of VM_LIT:
            inc sp
            ds[sp] = ts
            inc ip
            ts = im[ip]
          of VM_DUP:
            inc sp
            ds[sp] = ts
          of VM_DROP:
            ts = ds[sp]
            dec sp
          of VM_SWAP:
            let x = ts
            ts = ds[sp]
            ds[sp] = x
          of VM_PUSH:
            inc rp
            rs[rp] = ts
            ts = ds[sp]
            dec sp
          of VM_POP:
            inc sp
            ds[sp] = ts
            ts = rs[rp]
            dec rp
          of VM_CALL:
            inc rp
            rs[rp] = ip.CELL
            ip = UCELL(ts - 1)
            ts = ds[sp]
            dec sp
          of VM_CCALL:
            if ds[sp] == -1:
              inc rp
              rs[rp] = ip.CELL
              ip = UCELL(ts - 1)
            dec sp
            ts = ds[sp]
            dec sp
          of VM_JUMP:
            ip = UCELL(ts - 1)
            ts = ds[sp]
            dec sp
          of VM_RETURN:
            ip = rs[rp].UCELL
            dec rp
          of VM_GT:
            if ds[sp] > ts:
              ts = -1
            else:
              ts = 0
            dec sp
          of VM_LT:
            if ds[sp] < ts:
              ts = -1
            else:
              ts = 0
            dec sp
          of VM_NEQ:
            if ds[sp] != ts:
              ts = -1
            else:
              ts = 0
            dec sp
          of VM_EQ:
            if ds[sp] == ts:
              ts = -1
            else:
              ts = 0;
            dec sp
          of VM_FETCH:
            case ts:
              of -1:
                ts = CELL(sp - 1)
              of -2:
                ts = CELL(rp - 1)
              of -3:
                ts = IMAGE_SIZE;
              of -4:
                ts = CELL.low
              of -5:
                ts = CELL.high
              else:
                ts = im[ts]
          of VM_STORE:
            im[ts] = ds[sp]
            dec sp
            ts = ds[sp]
            dec sp
          of VM_ADD:
            ts += ds[sp]
            dec sp
          of VM_SUB:
            ts = ds[sp] - ts
            dec sp
          of VM_MUL:
            ts *= ds[sp]
            dec sp
          of VM_DIVMOD:
            let x = ts
            let y = ds[sp]
            ts = y div x
            ds[sp] = y mod x
          of VM_AND:
            ts = ts and ds[sp]
            dec sp
          of VM_OR:
            ts = ts or ds[sp]
            dec sp
          of VM_XOR:
            ts = ts xor ds[sp]
            dec sp
          of VM_SHIFT:
            if ts < 0 :
              ts = ds[sp] shl (0 - ts)
            else:
              ts = ds[sp] shr ts
            dec sp
          of VM_ZRET:
            if ts == 0:
              ts = ds[sp]
              dec sp
              ip = rs[rp].UCELL
              dec rp
          of VM_HALT:
            ip = IMAGE_SIZE
          of VM_IE:
            ts = 1
          of VM_IQ:
            ts = 0
            inc sp
            ds[sp] = ts
          of VM_II:
            ts = ds[sp]
            dec sp
            discard stdout.writeChars([ts.char],0,1)
            ts = ds[sp]
            dec sp
          of VM_RES0:
            fatal "reserved"
          of VM_RES1:
            fatal "reserved"

      let notfound = im[XT+lookup("err:notfound")].UCELL
      if ip == notfound:
        echo("\n" & extract(1471) & " ?\n")

go()

# RETRO is a clean, elegant, and pragmatic dialect of Forth.
# It provides a simple alternative for those willing to make a
# break from legacy systems.
#
# The language draws influences from many sources including
# traditional Forth systems, cmForth, colorForth, Factor, and
# Parable. It was designed to be easy to grasp and adapt to
# specific uses.
#
# The basic language is very portable and runs on a tiny
# virtual machine.
#
# This file contains a minimal implementation of the virtual
# machine.
#
# Copyright (c) 2008 - 2022, Charles Childers
# Copyright (c) 2009 - 2010, Luke Parrish
# Copyright (c) 2010,        Marc Simpson
# Copyright (c) 2010,        Jay Skeer
# Copyright (c) 2011,        Kenneth Keating

import std/[streams, os]

const
  IMAGE_SIZE* = 65536
  ADDRESSES* = 256
  STACK_DEPTH* = 256

type
  Cell* = int32
  NgaVm* = object
    sp, rp: int
    ip: int
    data: array[STACK_DEPTH, Cell]
    address: array[ADDRESSES, Cell]
    memory: array[IMAGE_SIZE + 1, Cell]

const
  CELL_MIN* = Cell(int32.low + 1)
  CELL_MAX* = Cell(int32.high - 1)

template TOS*(vm: NgaVm): Cell = vm.data[vm.sp]
template NOS*(vm: NgaVm): Cell = vm.data[vm.sp - 1]
template TORS*(vm: NgaVm): Cell = vm.address[vm.rp]

proc stackPop*(vm: var NgaVm): Cell =
  vm.sp.dec
  result = vm.data[vm.sp + 1]

proc stackPush*(vm: var NgaVm, value: Cell) =
  vm.sp.inc
  vm.data[vm.sp] = value

proc loadImage*(vm: var NgaVm, imageFile: string): bool =
  try:
    let file = newFileStream(imageFile, fmRead)
    defer: file.close()
    
    var cellCount = 0
    while not file.atEnd() and cellCount < IMAGE_SIZE:
      vm.memory[cellCount] = file.readInt32()
      cellCount.inc
    
    return true
  except:
    return false

proc prepareVm*(vm: var NgaVm) =
  vm.ip = 0
  vm.sp = 0
  vm.rp = 0
  
  for i in 0..<IMAGE_SIZE:
    vm.memory[i] = 0
  for i in 0..<STACK_DEPTH:
    vm.data[i] = 0
  for i in 0..<ADDRESSES:
    vm.address[i] = 0

proc instNo*(vm: var NgaVm) = discard

proc instLi*(vm: var NgaVm) =
  vm.ip.inc
  vm.stackPush(vm.memory[vm.ip])

proc instDu*(vm: var NgaVm) =
  vm.stackPush(vm.TOS)

proc instDr*(vm: var NgaVm) =
  vm.data[vm.sp] = 0
  vm.sp.dec
  if vm.sp < 0:
    vm.ip = IMAGE_SIZE

proc instSw*(vm: var NgaVm) =
  let a = vm.TOS
  vm.data[vm.sp] = vm.NOS
  vm.data[vm.sp - 1] = a

proc instPu*(vm: var NgaVm) =
  vm.rp.inc
  vm.address[vm.rp] = vm.TOS
  vm.instDr()

proc instPo*(vm: var NgaVm) =
  vm.stackPush(vm.TORS)
  vm.rp.dec

proc instJu*(vm: var NgaVm) =
  vm.ip = int(vm.TOS - 1)
  vm.instDr()

proc instCa*(vm: var NgaVm) =
  vm.rp.inc
  vm.address[vm.rp] = Cell(vm.ip)
  vm.ip = int(vm.TOS - 1)
  vm.instDr()

proc instCc*(vm: var NgaVm) =
  let a = vm.TOS
  vm.instDr()
  let b = vm.TOS
  vm.instDr()
  if b != 0:
    vm.rp.inc
    vm.address[vm.rp] = Cell(vm.ip)
    vm.ip = int(a - 1)

proc instRe*(vm: var NgaVm) =
  vm.ip = int(vm.TORS)
  vm.rp.dec

proc instEq*(vm: var NgaVm) =
  vm.data[vm.sp - 1] = if vm.NOS == vm.TOS: -1 else: 0
  vm.instDr()

proc instNe*(vm: var NgaVm) =
  vm.data[vm.sp - 1] = if vm.NOS != vm.TOS: -1 else: 0
  vm.instDr()

proc instLt*(vm: var NgaVm) =
  vm.data[vm.sp - 1] = if vm.NOS < vm.TOS: -1 else: 0
  vm.instDr()

proc instGt*(vm: var NgaVm) =
  vm.data[vm.sp - 1] = if vm.NOS > vm.TOS: -1 else: 0
  vm.instDr()

proc instFe*(vm: var NgaVm) =
  vm.data[vm.sp] = case vm.TOS
    of -1: Cell(vm.sp - 1)
    of -2: Cell(vm.rp)
    of -3: Cell(IMAGE_SIZE)
    of -4: CELL_MIN
    of -5: CELL_MAX
    else: vm.memory[vm.TOS]

proc instSt*(vm: var NgaVm) =
  if vm.TOS >= 0 and vm.TOS <= IMAGE_SIZE:
    vm.memory[vm.TOS] = vm.NOS
    vm.instDr()
    vm.instDr()
  else:
    vm.ip = IMAGE_SIZE

proc instAd*(vm: var NgaVm) =
  vm.data[vm.sp - 1] = vm.NOS + vm.TOS
  vm.instDr()

proc instSu*(vm: var NgaVm) =
  vm.data[vm.sp - 1] = vm.NOS - vm.TOS
  vm.instDr()

proc instMu*(vm: var NgaVm) =
  vm.data[vm.sp - 1] = vm.NOS * vm.TOS
  vm.instDr()

proc instDi*(vm: var NgaVm) =
  let a = vm.TOS
  let b = vm.NOS
  vm.data[vm.sp] = b div a
  vm.data[vm.sp - 1] = b mod a

proc instAn*(vm: var NgaVm) =
  vm.data[vm.sp - 1] = vm.TOS and vm.NOS
  vm.instDr()

proc instOr*(vm: var NgaVm) =
  vm.data[vm.sp - 1] = vm.TOS or vm.NOS
  vm.instDr()

proc instXo*(vm: var NgaVm) =
  vm.data[vm.sp - 1] = vm.TOS xor vm.NOS
  vm.instDr()

proc instSh*(vm: var NgaVm) =
  let y = vm.TOS
  let x = vm.NOS
  if y < 0:
    vm.data[vm.sp - 1] = x shl (-y)
  else:
    if x < 0 and y > 0:
      vm.data[vm.sp - 1] = (x shr y) or (not(0'i32) shr y)
    else:
      vm.data[vm.sp - 1] = x shr y
  vm.instDr()

proc instZr*(vm: var NgaVm) =
  if vm.TOS == 0:
    vm.instDr()
    vm.ip = int(vm.TORS)
    vm.rp.dec

proc instHa*(vm: var NgaVm) =
  vm.ip = IMAGE_SIZE

proc instIe*(vm: var NgaVm) =
  vm.stackPush(2)

proc instIq*(vm: var NgaVm) =
  if vm.TOS == 0:
    vm.instDr()
    vm.stackPush(0)
    vm.stackPush(0)
  elif vm.TOS == 1:
    vm.instDr()
    vm.stackPush(1)
    vm.stackPush(1)

proc instIi*(vm: var NgaVm) =
  if vm.TOS == 0:
    vm.instDr()
    let c = vm.stackPop()
    stdout.write(char(c))
    stdout.flushFile()
  elif vm.TOS == 1:
    let c = stdin.readChar()
    vm.instDr()
    vm.stackPush(Cell(ord(c)))
  else:
    vm.instDr()

const instructions = [
  instNo, instLi, instDu, instDr, instSw, instPu, instPo,
  instJu, instCa, instCc, instRe, instEq, instNe, instLt,
  instGt, instFe, instSt, instAd, instSu, instMu, instDi,
  instAn, instOr, instXo, instSh, instZr, instHa, instIe,
  instIq, instIi
]

proc processOpcodeBundle*(vm: var NgaVm, opcode: Cell) =
  instructions[opcode and 0xFF](vm)
  instructions[(opcode shr 8) and 0xFF](vm)
  instructions[(opcode shr 16) and 0xFF](vm)
  instructions[(opcode shr 24) and 0xFF](vm)

proc execute*(vm: var NgaVm, cell: Cell) =
  vm.rp = 1
  vm.ip = int(cell)
  while vm.ip < IMAGE_SIZE:
    let opcode = vm.memory[vm.ip]
    vm.processOpcodeBundle(opcode)
    vm.ip.inc
    if vm.rp == 0:
      vm.ip = IMAGE_SIZE

when isMainModule:
  var vm: NgaVm
  vm.prepareVm()
  if vm.loadImage("ngaImage"):
    vm.execute(0)
  else:
    echo "Error loading ngaImage"
    quit(1)

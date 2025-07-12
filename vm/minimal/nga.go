/* RETRO is a clean, elegant, and pragmatic dialect of Forth.
   It provides a simple alternative for those willing to make a
   break from legacy systems.

   The language draws influences from many sources including
   traditional Forth systems, cmForth, colorForth, Factor, and
   Parable. It was designed to be easy to grasp and adapt to
   specific uses.

   The basic language is very portable and runs on a tiny
   virtual machine.

   This file contains a minimal implementation of the virtual
   machine.

   Copyright (c) 2008 - 2022, Charles Childers
   Copyright (c) 2009 - 2010, Luke Parrish
   Copyright (c) 2010,        Marc Simpson
   Copyright (c) 2010,        Jay Skeer
   Copyright (c) 2011,        Kenneth Keating
*/

package main

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"os"
)

var stdinReader = bufio.NewReader(os.Stdin)

const (
	ImageSize   = 65536
	Addresses   = 256
	StackDepth  = 256
	CellMin     = int32(-2147483647)
	CellMax     = int32(2147483646)
)

type Cell = int32

type NgaVM struct {
	sp      int
	rp      int
	ip      int
	data    [StackDepth]Cell
	address [Addresses]Cell
	memory  [ImageSize + 1]Cell
}

func NewNgaVM() *NgaVM {
	return &NgaVM{}
}

func (vm *NgaVM) TOS() Cell {
	return vm.data[vm.sp]
}

func (vm *NgaVM) NOS() Cell {
	return vm.data[vm.sp-1]
}

func (vm *NgaVM) TORS() Cell {
	return vm.address[vm.rp]
}

func (vm *NgaVM) setTOS(value Cell) {
	vm.data[vm.sp] = value
}

func (vm *NgaVM) setNOS(value Cell) {
	vm.data[vm.sp-1] = value
}

func (vm *NgaVM) setTORS(value Cell) {
	vm.address[vm.rp] = value
}

func (vm *NgaVM) stackPop() Cell {
	vm.sp--
	return vm.data[vm.sp+1]
}

func (vm *NgaVM) stackPush(value Cell) {
	vm.sp++
	vm.data[vm.sp] = value
}

func (vm *NgaVM) LoadImage(imageFile string) error {
	file, err := os.Open(imageFile)
	if err != nil {
		return err
	}
	defer file.Close()

	i := 0
	for i < ImageSize {
		var cell Cell
		err := binary.Read(file, binary.LittleEndian, &cell)
		if err != nil {
			break
		}
		vm.memory[i] = cell
		i++
	}
	return nil
}

func (vm *NgaVM) PrepareVM() {
	vm.ip = 0
	vm.sp = 0
	vm.rp = 0

	for i := 0; i < ImageSize; i++ {
		vm.memory[i] = 0
	}
	for i := 0; i < StackDepth; i++ {
		vm.data[i] = 0
	}
	for i := 0; i < Addresses; i++ {
		vm.address[i] = 0
	}
}

func (vm *NgaVM) instNo() {
}

func (vm *NgaVM) instLi() {
	vm.ip++
	vm.stackPush(vm.memory[vm.ip])
}

func (vm *NgaVM) instDu() {
	vm.stackPush(vm.TOS())
}

func (vm *NgaVM) instDr() {
	vm.data[vm.sp] = 0
	vm.sp--
	if vm.sp < 0 {
		vm.ip = ImageSize
	}
}

func (vm *NgaVM) instSw() {
	a := vm.TOS()
	vm.setTOS(vm.NOS())
	vm.setNOS(a)
}

func (vm *NgaVM) instPu() {
	vm.rp++
	vm.setTORS(vm.TOS())
	vm.instDr()
}

func (vm *NgaVM) instPo() {
	vm.stackPush(vm.TORS())
	vm.rp--
}

func (vm *NgaVM) instJu() {
	vm.ip = int(vm.TOS() - 1)
	vm.instDr()
}

func (vm *NgaVM) instCa() {
	vm.rp++
	vm.setTORS(Cell(vm.ip))
	vm.ip = int(vm.TOS() - 1)
	vm.instDr()
}

func (vm *NgaVM) instCc() {
	a := vm.TOS()
	vm.instDr()
	b := vm.TOS()
	vm.instDr()
	if b != 0 {
		vm.rp++
		vm.setTORS(Cell(vm.ip))
		vm.ip = int(a - 1)
	}
}

func (vm *NgaVM) instRe() {
	vm.ip = int(vm.TORS())
	vm.rp--
}

func (vm *NgaVM) instEq() {
	if vm.NOS() == vm.TOS() {
		vm.setNOS(-1)
	} else {
		vm.setNOS(0)
	}
	vm.instDr()
}

func (vm *NgaVM) instNe() {
	if vm.NOS() != vm.TOS() {
		vm.setNOS(-1)
	} else {
		vm.setNOS(0)
	}
	vm.instDr()
}

func (vm *NgaVM) instLt() {
	if vm.NOS() < vm.TOS() {
		vm.setNOS(-1)
	} else {
		vm.setNOS(0)
	}
	vm.instDr()
}

func (vm *NgaVM) instGt() {
	if vm.NOS() > vm.TOS() {
		vm.setNOS(-1)
	} else {
		vm.setNOS(0)
	}
	vm.instDr()
}

func (vm *NgaVM) instFe() {
	switch vm.TOS() {
	case -1:
		vm.setTOS(Cell(vm.sp - 1))
	case -2:
		vm.setTOS(Cell(vm.rp))
	case -3:
		vm.setTOS(Cell(ImageSize))
	case -4:
		vm.setTOS(CellMin)
	case -5:
		vm.setTOS(CellMax)
	default:
		addr := vm.TOS()
		vm.setTOS(vm.memory[addr])
	}
}

func (vm *NgaVM) instSt() {
	addr := vm.TOS()
	if addr <= ImageSize && addr >= 0 {
		vm.memory[addr] = vm.NOS()
		vm.instDr()
		vm.instDr()
	} else {
		vm.ip = ImageSize
	}
}

func (vm *NgaVM) instAd() {
	vm.setNOS(vm.NOS() + vm.TOS())
	vm.instDr()
}

func (vm *NgaVM) instSu() {
	vm.setNOS(vm.NOS() - vm.TOS())
	vm.instDr()
}

func (vm *NgaVM) instMu() {
	vm.setNOS(vm.NOS() * vm.TOS())
	vm.instDr()
}

func (vm *NgaVM) instDi() {
	a := vm.TOS()
	b := vm.NOS()
	vm.setTOS(b / a)
	vm.setNOS(b % a)
}

func (vm *NgaVM) instAn() {
	vm.setNOS(vm.TOS() & vm.NOS())
	vm.instDr()
}

func (vm *NgaVM) instOr() {
	vm.setNOS(vm.TOS() | vm.NOS())
	vm.instDr()
}

func (vm *NgaVM) instXo() {
	vm.setNOS(vm.TOS() ^ vm.NOS())
	vm.instDr()
}

func (vm *NgaVM) instSh() {
	y := vm.TOS()
	x := vm.NOS()
	if y < 0 {
		vm.setNOS(x << uint(-y))
	} else {
		if x < 0 && y > 0 {
			vm.setNOS((x >> uint(y)) | (^Cell(0) >> uint(y)))
		} else {
			vm.setNOS(x >> uint(y))
		}
	}
	vm.instDr()
}

func (vm *NgaVM) instZr() {
	if vm.TOS() == 0 {
		vm.instDr()
		vm.ip = int(vm.TORS())
		vm.rp--
	}
}

func (vm *NgaVM) instHa() {
	vm.ip = ImageSize
}

func (vm *NgaVM) instIe() {
	vm.stackPush(2)
}

func (vm *NgaVM) instIq() {
	if vm.TOS() == 0 {
		vm.instDr()
		vm.stackPush(0)
		vm.stackPush(0)
	} else if vm.TOS() == 1 {
		vm.instDr()
		vm.stackPush(1)
		vm.stackPush(1)
	}
}

func (vm *NgaVM) instIi() {
	if vm.TOS() == 0 {
		vm.instDr()
		c := vm.stackPop()
		fmt.Printf("%c", c)
	} else if vm.TOS() == 1 {
		c, err := stdinReader.ReadByte()
		if err != nil {
			os.Exit(0)
		}
		vm.instDr()
		vm.stackPush(Cell(c))
	} else {
		vm.instDr()
	}
}

var instructions = []func(*NgaVM){
	(*NgaVM).instNo, (*NgaVM).instLi, (*NgaVM).instDu, (*NgaVM).instDr, (*NgaVM).instSw, (*NgaVM).instPu, (*NgaVM).instPo,
	(*NgaVM).instJu, (*NgaVM).instCa, (*NgaVM).instCc, (*NgaVM).instRe, (*NgaVM).instEq, (*NgaVM).instNe, (*NgaVM).instLt,
	(*NgaVM).instGt, (*NgaVM).instFe, (*NgaVM).instSt, (*NgaVM).instAd, (*NgaVM).instSu, (*NgaVM).instMu, (*NgaVM).instDi,
	(*NgaVM).instAn, (*NgaVM).instOr, (*NgaVM).instXo, (*NgaVM).instSh, (*NgaVM).instZr, (*NgaVM).instHa, (*NgaVM).instIe,
	(*NgaVM).instIq, (*NgaVM).instIi,
}

func (vm *NgaVM) processOpcodeBundle(opcode Cell) {
	instructions[opcode&0xFF](vm)
	instructions[(opcode>>8)&0xFF](vm)
	instructions[(opcode>>16)&0xFF](vm)
	instructions[(opcode>>24)&0xFF](vm)
}

func (vm *NgaVM) Execute(cell Cell) {
	vm.rp = 1
	vm.ip = int(cell)
	for vm.ip < ImageSize {
		opcode := vm.memory[vm.ip]
		vm.processOpcodeBundle(opcode)
		vm.ip++
		if vm.rp == 0 {
			vm.ip = ImageSize
		}
	}
}

func main() {
	vm := NewNgaVM()
	vm.PrepareVM()
	err := vm.LoadImage("ngaImage")
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error loading image: %v\n", err)
		os.Exit(1)
	}
	vm.Execute(0)
}

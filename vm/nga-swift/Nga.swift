//
//  Nga.swift
//
//  Nga is the virtual machine at the heart
//  of RETRO. It provides a MISC based dual
//  stack processor that's capable of hosting
//  RETRO or other environments.
//
//  This is a reimplementation in Swift. I've
//  held off doing this until now, but as I
//  plan to continue using Apple computers and
//  iPads, it's becoming clear that they see
//  this as the future, and it'll become more
//  difficult to justify holding out on
//  Objective-C going forward.
//
//  So here we go...
//
//  Copyright 2020, Charles Childers
//

import Foundation

// Limits
let imageSize: Int = 512000
let stackSize: Int = 2048
let CELLMAX = Int64.max - 1
let CELLMIN = Int64.min

// Notes:
//   While Nga is designed as a 32-bit system,
//   this implementation is 64-bit internally
//   to allow greater numeric range.

// I'm adding some extensions to existing types
// for easier access to individual characters
// in strings and bytes in integers.

extension StringProtocol {
    var asciiValues: [UInt8] { compactMap(\.asciiValue) }
}

extension FixedWidthInteger where Self: SignedInteger {
    var bytes: [Int8] {
        var _endian = littleEndian
        let bytePtr = withUnsafePointer(to: &_endian) {
            $0.withMemoryRebound(to: Int8.self, capacity: MemoryLayout<Self>.size) {
                UnsafeBufferPointer(start: $0, count: MemoryLayout<Self>.size)
            }
        }
        return [Int8](bytePtr)
    }
}

// I implement a stack class here for handling
// the details of the stacks. Stacks in Nga are
// LIFO, holding only signed integer values.

class Stack {
    private var data = [Int64](repeating: 0, count: stackSize)
    private var sp: Int = 0
    
    public func tos() -> Int64 {
        return data[sp]
    }
    
    public func nos() -> Int64 {
        return data[sp - 1]
    }
    
    public func push(_ n: Int64) {
        sp += 1
        data[sp] = n
    }
    
    public func pop() -> Int64 {
        let v = data[sp]
        data[sp] = 0
        sp -= 1
        return v
    }
    
    public func drop() {
        data[sp] = 0
        sp -= 1
    }
    
    public func swap() {
        let a = tos()
        let b = nos()
        data[sp] = b
        data[sp - 1] = a
    }
    
    public func depth() -> Int {
        return sp
    }
    
    public func item(_ n: Int) -> Int64 {
        return data[n]
    }
}

// Instruction Pointer and Memory
// I'm adding some extra padding at EOM for now.
// This may be changed in the future.

var ip: Int = 0
var memory = [Int64](repeating: 0, count: imageSize + 1024)

// Create the stacks

var data = Stack()
var address = Stack()

// Now, I implement the instructions. Each gets
// a dedicated function.

func inst_no() {
}

func inst_li() {
    ip += 1
    data.push(memory[ip])
}

func inst_du() {
    data.push(data.tos())
}

func inst_dr() {
    data.drop()
}

func inst_sw() {
    data.swap()
}

func inst_pu() {
    address.push(data.pop())
}

func inst_po() {
    data.push(address.pop())
}

func inst_ju() {
    ip = Int(data.pop() - 1)
}

func inst_ca() {
    address.push(Int64(ip))
    ip = Int(data.pop() - 1)
}

func inst_cc() {
    let dest = data.pop()
    let flag = data.pop()
    if (flag != 0) {
        address.push(Int64(ip))
        ip = Int(dest - 1)
    }
}

func inst_re() {
    ip = Int(address.pop())
}

func inst_eq() {
    let tos = data.pop()
    let nos = data.pop()
    data.push((nos == tos) ? -1 : 0)
}

func inst_ne() {
    let tos = data.pop()
    let nos = data.pop()
    data.push((nos != tos) ? -1 : 0)
}

func inst_lt() {
    let tos = data.pop()
    let nos = data.pop()
    data.push((nos < tos) ? -1 : 0)
}

func inst_gt() {
    let tos = data.pop()
    let nos = data.pop()
    data.push((nos > tos) ? -1 : 0)
}

func inst_fe() {
    let target = data.pop()
    switch (target) {
    case -1:
        data.push(Int64(data.depth()))
    case -2:
        data.push(Int64(address.depth()))
    case -3:
        data.push(Int64(imageSize))
    case -4:
        data.push(CELLMIN)
    case -5:
        data.push(CELLMAX)
    default:
        data.push(memory[Int(target)])
    }
}

func inst_st() {
    let addr = data.pop()
    let value = data.pop()
    memory[Int(addr)] = value;
}

func inst_ad() {
    let tos = data.pop()
    let nos = data.pop()
    data.push(nos + tos)
}

func inst_su() {
    let tos = data.pop()
    let nos = data.pop()
    data.push(nos - tos)
}

func inst_mu() {
    let tos = data.pop()
    let nos = data.pop()
    data.push(nos * tos)
}

func inst_di() {
    let a = data.pop()
    let b = data.pop()
    data.push(b % a)
    data.push(b / a)
}

func inst_an() {
    let tos = data.pop()
    let nos = data.pop()
    data.push(tos & nos)
}

func inst_or() {
    let tos = data.pop()
    let nos = data.pop()
    data.push(nos | tos)
}

func inst_xo() {
    let tos = data.pop()
    let nos = data.pop()
    data.push(nos ^ tos)
}

func inst_sh() {
    let tos = data.pop()
    let nos = data.pop()
    if (tos < 0) {
        data.push(nos << (tos * -1))
    }
    else {
        if (nos < 0 && tos > 0) {
            data.push(nos >> tos | ~(~0 >> tos))
        }
        else {
            data.push(nos >> tos)
        }
    }
}

func inst_zr() {
    if (data.tos() == 0) {
        data.drop()
        ip = Int(address.pop())
    }
}

func inst_ha() {
    ip = imageSize;
}

func inst_ie() {
    data.push(1)
}

func inst_iq() {
    data.drop()
    data.push(0)
    data.push(0)
}

func inst_ii() {
    data.drop()
    let v = UnicodeScalar(Int(data.pop())) ?? UnicodeScalar(32)
    print(Character(v!), terminator: "")
}

// With those done, I turn to handling the
// instructions.

// In C, I use a jump table for this. I don't
// know how to do this in Swift.

func ngaProcessOpcode(opcode: Int64) {
    switch opcode {
    case  0:
        ()
    case  1:
        inst_li()
    case  2:
        inst_du()
    case  3:
        inst_dr()
    case  4:
        inst_sw()
    case  5:
        inst_pu()
    case  6:
        inst_po()
    case  7:
        inst_ju()
    case  8:
        inst_ca()
    case  9:
        inst_cc()
    case  10:
        inst_re()
    case  11:
        inst_eq()
    case  12:
        inst_ne()
    case  13:
        inst_lt()
    case  14:
        inst_gt()
    case  15:
        inst_fe()
    case  16:
        inst_st()
    case  17:
        inst_ad()
    case  18:
        inst_su()
    case  19:
        inst_mu()
    case  20:
        inst_di()
    case  21:
        inst_an()
    case  22:
        inst_or()
    case  23:
        inst_xo()
    case  24:
        inst_sh()
    case  25:
        inst_zr()
    case  26:
        inst_ha()
    case  27:
        inst_ie()
    case  28:
        inst_iq()
    case  29:
        inst_ii()
    default:
        inst_ha()
    }
}

// Each opcode can have up to four instructions
// This validates them, letting us know if it's
// safe to run them.

func ngaValidatePackedOpcodes(_ opcode: Int64) -> Bool {
    var valid = true
    for inst in Int32(opcode).bytes {
        if !(inst >= 0 && inst <= 29) {
            valid = false
        }
    }
    return valid
}

// This will process an opcode bundle

func ngaProcessPackedOpcodes(_ opcode: Int64) {
    for inst in Int32(opcode).bytes {
        ngaProcessOpcode(opcode: Int64(inst))
    }
}

// For debugging purposes

func dump() {
    for i in 0 ... data.depth() {
        if i == data.depth() {
            print("TOS")
        }
        print(Int(data.item(i)))
    }
}

// Interfacing

func extractString(at: Int) -> String {
    var s: String = ""
    var next = at
    while memory[next] != 0 {
        let value = Int(memory[next])
        let u = UnicodeScalar(value)
        let char = Character(u!)
        s.append(char)
        next += 1
    }
    return s
}

func injectString(_ s: String, to: Int) {
    var i: Int = 0
    for ch in s.asciiValues {
        let n = Int64(ch)
        memory[to + i] = n
        memory[to + i + 1] = 0
        i += 1
    }
}

// Run through an image, quiting when EOM
// is reached

func execute(_ word: Int) {
    address.push(0)
    ip = word
    while ip < imageSize {
//        print("ip:", ip, "bundle", memory[ip], "insts", Int32(memory[ip]).bytes)
        let opcode = memory[ip]
        if ngaValidatePackedOpcodes(opcode) {
            ngaProcessPackedOpcodes(opcode)
        } else {
            print("Error: invalid opcode")
            ip = imageSize
        }
        ip += 1
        if address.depth() == 0 {
            ip = imageSize
        }
    }
}

func process() {
    ip = 0
    while ip < imageSize - 1 {
        ngaProcessPackedOpcodes(memory[ip])
        ip += 1
    }
}

// Tests

loadImage()

// Display the dictionary
var i = memory[2]
var interpret: Int = 0
while i != 0 {
    let name = extractString(at: Int(i + 3))
//    print(name)
    if name == "interpret" {
        interpret = Int(memory[Int(i) + 1])
    }
    i = memory[Int(i)]
}


var done = false
print("RETRO (using nga.swift)")
while !done {
    let code = readLine()
    if code == "bye" {
        done = true
    } else {
        injectString(code ?? "()", to: 1025)
        data.push(1025)
        execute(interpret)
    }
}
dump()


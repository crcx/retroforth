// Minimal Nga virtual machine for RETRO, implemented for Node.js.
// Mirrors the reference VM in nga.c/nga.rs.
const fs = require('fs');

const IMAGE_SIZE = 65536;
const ADDRESSES = 256;
const STACK_DEPTH = 256;

const CELL_MIN = -0x7fffffff - 1 + 1; // INT_MIN + 1
const CELL_MAX = 0x7fffffff - 1;      // INT_MAX - 1

const toCell = (n) => (n | 0);

class NgaVm {
  constructor() {
    this.sp = 0;
    this.rp = 0;
    this.ip = 0;
    this.data = new Int32Array(STACK_DEPTH);
    this.address = new Int32Array(ADDRESSES);
    this.memory = new Int32Array(IMAGE_SIZE + 1);
  }

  halt() { this.ip = IMAGE_SIZE; }
  validAddress(addr) { return Number.isInteger(addr) && addr >= 0 && addr <= IMAGE_SIZE; }

  tos() {
    return this.data[this.sp];
  }

  nos() {
    return this.data[this.sp - 1];
  }

  tors() {
    return this.address[this.rp];
  }

  setTos(v) {
    this.data[this.sp] = toCell(v);
  }

  setNos(v) {
    this.data[this.sp - 1] = toCell(v);
  }

  setTors(v) {
    this.address[this.rp] = toCell(v);
  }

  stackPop() {
    if (this.sp <= 0) { this.halt(); return 0; }
    this.sp -= 1;
    return this.data[this.sp + 1];
  }

  stackPush(v) {
    if (this.sp >= STACK_DEPTH - 1) { this.halt(); return; }
    this.sp += 1;
    this.data[this.sp] = toCell(v);
  }

  execute(cell) {
    this.rp = 1;
    this.ip = cell | 0;
    while (this.ip >= 0 && this.ip < IMAGE_SIZE) {
      const opcode = this.memory[this.ip];
      this.processOpcodeBundle(opcode);
      this.ip += 1;
      if (this.rp === 0) {
        this.ip = IMAGE_SIZE;
      }
    }
  }

  loadImage(imageFile) {
    if (fs.statSync(imageFile).size > IMAGE_SIZE * 4) {
      throw new Error('image exceeds VM memory capacity');
    }
    const buf = fs.readFileSync(imageFile);
    const cells = Math.floor(buf.length / 4);
    for (let i = 0; i < cells && i < IMAGE_SIZE; i += 1) {
      this.memory[i] = buf.readInt32LE(i * 4);
    }
  }

  prepareVm() {
    this.ip = 0;
    this.sp = 0;
    this.rp = 0;
    this.memory.fill(0);
    this.data.fill(0);
    this.address.fill(0);
  }

  inst_no() {}

  inst_li() {
    if (this.ip >= IMAGE_SIZE - 1) { this.halt(); return; }
    this.ip += 1;
    this.stackPush(this.memory[this.ip]);
  }

  inst_du() {
    if (this.sp <= 0) { this.halt(); return; }
    this.stackPush(this.tos());
  }

  inst_dr() {
    this.data[this.sp] = 0;
    this.sp -= 1;
    if (this.sp < 0) {
      this.ip = IMAGE_SIZE;
    }
  }

  inst_sw() {
    if (this.sp < 2) { this.halt(); return; }
    const a = this.tos();
    this.setTos(this.nos());
    this.setNos(a);
  }

  inst_pu() {
    if (this.sp <= 0 || this.rp >= ADDRESSES - 1) { this.halt(); return; }
    this.rp += 1;
    this.setTors(this.tos());
    this.inst_dr();
  }

  inst_po() {
    if (this.rp <= 0) { this.halt(); return; }
    this.stackPush(this.tors());
    this.rp -= 1;
  }

  inst_ju() {
    if (this.sp <= 0 || !this.validAddress(this.tos()) || this.tos() === IMAGE_SIZE) { this.halt(); return; }
    this.ip = this.tos() - 1;
    this.inst_dr();
  }

  inst_ca() {
    if (this.sp <= 0 || this.rp >= ADDRESSES - 1 || !this.validAddress(this.tos()) || this.tos() === IMAGE_SIZE) { this.halt(); return; }
    this.rp += 1;
    this.setTors(this.ip);
    this.ip = this.tos() - 1;
    this.inst_dr();
  }

  inst_cc() {
    if (this.sp < 2) { this.halt(); return; }
    const a = this.tos();
    this.inst_dr();
    const b = this.tos();
    this.inst_dr();
    if (b !== 0) {
      if (this.rp >= ADDRESSES - 1 || !this.validAddress(a) || a === IMAGE_SIZE) { this.halt(); return; }
      this.rp += 1;
      this.setTors(this.ip);
      this.ip = a - 1;
    }
  }

  inst_re() {
    if (this.rp <= 0 || !this.validAddress(this.tors()) || this.tors() === IMAGE_SIZE) { this.halt(); return; }
    this.ip = this.tors();
    this.rp -= 1;
  }

  inst_eq() {
    this.setNos(this.nos() === this.tos() ? -1 : 0);
    this.inst_dr();
  }

  inst_ne() {
    this.setNos(this.nos() !== this.tos() ? -1 : 0);
    this.inst_dr();
  }

  inst_lt() {
    this.setNos(this.nos() < this.tos() ? -1 : 0);
    this.inst_dr();
  }

  inst_gt() {
    this.setNos(this.nos() > this.tos() ? -1 : 0);
    this.inst_dr();
  }

  inst_fe() {
    if (this.sp <= 0) { this.halt(); return; }
    switch (this.tos()) {
      case -1:
        this.setTos(this.sp - 1);
        break;
      case -2:
        this.setTos(this.rp);
        break;
      case -3:
        this.setTos(IMAGE_SIZE);
        break;
      case -4:
        this.setTos(CELL_MIN);
        break;
      case -5:
        this.setTos(CELL_MAX);
        break;
      default:
        if (!this.validAddress(this.tos())) { this.halt(); return; }
        this.setTos(this.memory[this.tos()]);
        break;
    }
  }

  inst_st() {
    const addr = this.tos();
    if (this.sp < 2) {
      this.halt();
    } else if (this.validAddress(addr)) {
      this.memory[addr] = this.nos();
      this.inst_dr();
      this.inst_dr();
    } else {
      this.ip = IMAGE_SIZE;
    }
  }

  inst_ad() {
    this.setNos(toCell(this.nos() + this.tos()));
    this.inst_dr();
  }

  inst_su() {
    this.setNos(toCell(this.nos() - this.tos()));
    this.inst_dr();
  }

  inst_mu() {
    this.setNos(toCell(this.nos() * this.tos()));
    this.inst_dr();
  }

  inst_di() {
    if (this.sp < 2 || this.tos() === 0 || (this.nos() === CELL_MIN && this.tos() === -1)) { this.halt(); return; }
    const a = this.tos();
    const b = this.nos();
    this.setTos(toCell(b / a));
    this.setNos(toCell(b % a));
  }

  inst_an() {
    this.setNos(this.tos() & this.nos());
    this.inst_dr();
  }

  inst_or() {
    this.setNos(this.tos() | this.nos());
    this.inst_dr();
  }

  inst_xo() {
    this.setNos(this.tos() ^ this.nos());
    this.inst_dr();
  }

  inst_sh() {
    const y = this.tos();
    const x = this.nos();
    let result;
    if (this.sp < 2 || y < -31 || y > 31) { this.halt(); return; }
    if (y < 0) {
      result = x << (0 - y);
    } else if (x < 0 && y > 0) {
      result = (x >> y) | (~((~0) >>> y));
    } else {
      result = x >> y;
    }
    this.setNos(result);
    this.inst_dr();
  }

  inst_zr() {
    if (this.sp <= 0) { this.halt(); return; }
    if (this.tos() === 0) {
      if (this.rp <= 0 || !this.validAddress(this.tors()) || this.tors() === IMAGE_SIZE) { this.halt(); return; }
      this.inst_dr();
      this.ip = this.tors();
      this.rp -= 1;
    }
  }

  inst_ha() {
    this.ip = IMAGE_SIZE;
  }

  inst_ie() {
    this.stackPush(2);
  }

  inst_iq() {
    if (this.sp <= 0) { this.halt(); return; }
    if (this.tos() === 0) {
      this.inst_dr();
      this.stackPush(0);
      this.stackPush(0);
    } else if (this.tos() === 1) {
      this.inst_dr();
      this.stackPush(1);
      this.stackPush(1);
    }
  }

  inst_ii() {
    if (this.sp <= 0) { this.halt(); return; }
    if (this.tos() === 0) {
      this.inst_dr();
      const c = this.stackPop();
      process.stdout.write(String.fromCharCode(c & 0xff));
    } else if (this.tos() === 1) {
      const buf = Buffer.alloc(1);
      const read = fs.readSync(0, buf, 0, 1, null);
      if (read <= 0) {
        process.exit(0);
      }
      this.inst_dr();
      this.stackPush(buf[0]);
    } else {
      this.inst_dr();
    }
  }

  processOpcodeBundle(opcode) {
    if ((opcode & 0xff) > 29 || ((opcode >> 8) & 0xff) > 29 ||
        ((opcode >> 16) & 0xff) > 29 || ((opcode >> 24) & 0xff) > 29) {
      this.halt();
      return;
    }
    this.executeInstruction(opcode & 0xff);
    this.executeInstruction((opcode >> 8) & 0xff);
    this.executeInstruction((opcode >> 16) & 0xff);
    this.executeInstruction((opcode >> 24) & 0xff);
  }

  executeInstruction(inst) {
    switch (inst) {
      case 0: this.inst_no(); break;
      case 1: this.inst_li(); break;
      case 2: this.inst_du(); break;
      case 3: this.inst_dr(); break;
      case 4: this.inst_sw(); break;
      case 5: this.inst_pu(); break;
      case 6: this.inst_po(); break;
      case 7: this.inst_ju(); break;
      case 8: this.inst_ca(); break;
      case 9: this.inst_cc(); break;
      case 10: this.inst_re(); break;
      case 11: this.inst_eq(); break;
      case 12: this.inst_ne(); break;
      case 13: this.inst_lt(); break;
      case 14: this.inst_gt(); break;
      case 15: this.inst_fe(); break;
      case 16: this.inst_st(); break;
      case 17: this.inst_ad(); break;
      case 18: this.inst_su(); break;
      case 19: this.inst_mu(); break;
      case 20: this.inst_di(); break;
      case 21: this.inst_an(); break;
      case 22: this.inst_or(); break;
      case 23: this.inst_xo(); break;
      case 24: this.inst_sh(); break;
      case 25: this.inst_zr(); break;
      case 26: this.inst_ha(); break;
      case 27: this.inst_ie(); break;
      case 28: this.inst_iq(); break;
      case 29: this.inst_ii(); break;
      default: this.inst_no(); break;
    }
  }
}

function main() {
  const vm = new NgaVm();
  vm.prepareVm();
  vm.loadImage('ngaImage');
  vm.execute(0);
}

if (require.main === module) {
  main();
}

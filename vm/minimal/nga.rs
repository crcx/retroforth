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

use std::io::{self, Read, Write};
use std::fs::File;

const IMAGE_SIZE: usize = 65536;
const ADDRESSES: usize = 256;
const STACK_DEPTH: usize = 256;

type Cell = i32;
const CELL_MIN: Cell = i32::MIN + 1;
const CELL_MAX: Cell = i32::MAX - 1;

#[derive(Debug)]
pub struct NgaVm {
    sp: isize,
    rp: isize,
    ip: usize,
    data: [Cell; STACK_DEPTH],
    address: [Cell; ADDRESSES],
    memory: [Cell; IMAGE_SIZE + 1],
}

impl NgaVm {
    pub fn new() -> Self {
        NgaVm {
            sp: 0,
            rp: 0,
            ip: 0,
            data: [0; STACK_DEPTH],
            address: [0; ADDRESSES],
            memory: [0; IMAGE_SIZE + 1],
        }
    }

    fn tos(&self) -> Cell {
        self.data[self.sp as usize]
    }

    fn nos(&self) -> Cell {
        self.data[(self.sp - 1) as usize]
    }

    fn tors(&self) -> Cell {
        self.address[self.rp as usize]
    }

    fn set_tos(&mut self, value: Cell) {
        self.data[self.sp as usize] = value;
    }

    fn set_nos(&mut self, value: Cell) {
        self.data[(self.sp - 1) as usize] = value;
    }

    fn set_tors(&mut self, value: Cell) {
        self.address[self.rp as usize] = value;
    }

    fn stack_pop(&mut self) -> Cell {
        self.sp -= 1;
        self.data[(self.sp + 1) as usize]
    }

    fn stack_push(&mut self, value: Cell) {
        self.sp += 1;
        self.data[self.sp as usize] = value;
    }

    pub fn execute(&mut self, cell: Cell) {
        self.rp = 1;
        self.ip = cell as usize;
        while self.ip < IMAGE_SIZE {
            let opcode = self.memory[self.ip];
            self.process_opcode_bundle(opcode);
            self.ip += 1;
            if self.rp == 0 {
                self.ip = IMAGE_SIZE;
            }
        }
    }

    pub fn load_image(&mut self, image_file: &str) -> Result<(), Box<dyn std::error::Error>> {
        let mut file = File::open(image_file)?;
        let mut buffer = Vec::new();
        file.read_to_end(&mut buffer)?;
        
        let _file_len = buffer.len() / std::mem::size_of::<Cell>();
        for (i, chunk) in buffer.chunks_exact(std::mem::size_of::<Cell>()).enumerate() {
            if i >= IMAGE_SIZE { break; }
            self.memory[i] = Cell::from_le_bytes([chunk[0], chunk[1], chunk[2], chunk[3]]);
        }
        
        Ok(())
    }

    pub fn prepare_vm(&mut self) {
        self.ip = 0;
        self.sp = 0;
        self.rp = 0;
        
        for i in 0..IMAGE_SIZE {
            self.memory[i] = 0;
        }
        for i in 0..STACK_DEPTH {
            self.data[i] = 0;
        }
        for i in 0..ADDRESSES {
            self.address[i] = 0;
        }
    }

    fn inst_no(&mut self) {
    }

    fn inst_li(&mut self) {
        self.ip += 1;
        self.stack_push(self.memory[self.ip]);
    }

    fn inst_du(&mut self) {
        self.stack_push(self.tos());
    }

    fn inst_dr(&mut self) {
        self.data[self.sp as usize] = 0;
        self.sp -= 1;
        if self.sp < 0 {
            self.ip = IMAGE_SIZE;
        }
    }

    fn inst_sw(&mut self) {
        let a = self.tos();
        self.set_tos(self.nos());
        self.set_nos(a);
    }

    fn inst_pu(&mut self) {
        self.rp += 1;
        self.set_tors(self.tos());
        self.inst_dr();
    }

    fn inst_po(&mut self) {
        self.stack_push(self.tors());
        self.rp -= 1;
    }

    fn inst_ju(&mut self) {
        self.ip = (self.tos() - 1) as usize;
        self.inst_dr();
    }

    fn inst_ca(&mut self) {
        self.rp += 1;
        self.set_tors(self.ip as Cell);
        self.ip = (self.tos() - 1) as usize;
        self.inst_dr();
    }

    fn inst_cc(&mut self) {
        let a = self.tos();
        self.inst_dr();
        let b = self.tos();
        self.inst_dr();
        if b != 0 {
            self.rp += 1;
            self.set_tors(self.ip as Cell);
            self.ip = (a - 1) as usize;
        }
    }

    fn inst_re(&mut self) {
        self.ip = self.tors() as usize;
        self.rp -= 1;
    }

    fn inst_eq(&mut self) {
        let result = if self.nos() == self.tos() { -1 } else { 0 };
        self.set_nos(result);
        self.inst_dr();
    }

    fn inst_ne(&mut self) {
        let result = if self.nos() != self.tos() { -1 } else { 0 };
        self.set_nos(result);
        self.inst_dr();
    }

    fn inst_lt(&mut self) {
        let result = if self.nos() < self.tos() { -1 } else { 0 };
        self.set_nos(result);
        self.inst_dr();
    }

    fn inst_gt(&mut self) {
        let result = if self.nos() > self.tos() { -1 } else { 0 };
        self.set_nos(result);
        self.inst_dr();
    }

    fn inst_fe(&mut self) {
        let value = match self.tos() {
            -1 => (self.sp - 1) as Cell,
            -2 => self.rp as Cell,
            -3 => IMAGE_SIZE as Cell,
            -4 => CELL_MIN,
            -5 => CELL_MAX,
            addr => self.memory[addr as usize],
        };
        self.set_tos(value);
    }

    fn inst_st(&mut self) {
        let addr = self.tos() as usize;
        if addr <= IMAGE_SIZE {
            self.memory[addr] = self.nos();
            self.inst_dr();
            self.inst_dr();
        } else {
            self.ip = IMAGE_SIZE;
        }
    }

    fn inst_ad(&mut self) {
        let result = self.nos().wrapping_add(self.tos());
        self.set_nos(result);
        self.inst_dr();
    }

    fn inst_su(&mut self) {
        let result = self.nos().wrapping_sub(self.tos());
        self.set_nos(result);
        self.inst_dr();
    }

    fn inst_mu(&mut self) {
        let result = self.nos().wrapping_mul(self.tos());
        self.set_nos(result);
        self.inst_dr();
    }

    fn inst_di(&mut self) {
        let a = self.tos();
        let b = self.nos();
        self.set_tos(b / a);
        self.set_nos(b % a);
    }

    fn inst_an(&mut self) {
        let result = self.tos() & self.nos();
        self.set_nos(result);
        self.inst_dr();
    }

    fn inst_or(&mut self) {
        let result = self.tos() | self.nos();
        self.set_nos(result);
        self.inst_dr();
    }

    fn inst_xo(&mut self) {
        let result = self.tos() ^ self.nos();
        self.set_nos(result);
        self.inst_dr();
    }

    fn inst_sh(&mut self) {
        let y = self.tos();
        let x = self.nos();
        let result = if y < 0 {
            x << (0 - y)
        } else {
            if x < 0 && y > 0 {
                (x >> y) | (!0i32 >> y)
            } else {
                x >> y
            }
        };
        self.set_nos(result);
        self.inst_dr();
    }

    fn inst_zr(&mut self) {
        if self.tos() == 0 {
            self.inst_dr();
            self.ip = self.tors() as usize;
            self.rp -= 1;
        }
    }

    fn inst_ha(&mut self) {
        self.ip = IMAGE_SIZE;
    }

    fn inst_ie(&mut self) {
        self.stack_push(2);
    }

    fn inst_iq(&mut self) {
        if self.tos() == 0 {
            self.inst_dr();
            self.stack_push(0);
            self.stack_push(0);
        } else if self.tos() == 1 {
            self.inst_dr();
            self.stack_push(1);
            self.stack_push(1);
        }
    }

    fn inst_ii(&mut self) {
        if self.tos() == 0 {
            self.inst_dr();
            let c = self.stack_pop();
            print!("{}", c as u8 as char);
            io::stdout().flush().unwrap();
        } else if self.tos() == 1 {
            let mut buffer = [0; 1];
            match io::stdin().read_exact(&mut buffer) {
                Ok(_) => {
                    self.inst_dr();
                    self.stack_push(buffer[0] as Cell);
                }
                Err(_) => std::process::exit(0),
            }
        } else {
            self.inst_dr();
        }
    }

    fn process_opcode_bundle(&mut self, opcode: Cell) {
        self.execute_instruction((opcode & 0xFF) as u8);
        self.execute_instruction(((opcode >> 8) & 0xFF) as u8);
        self.execute_instruction(((opcode >> 16) & 0xFF) as u8);
        self.execute_instruction(((opcode >> 24) & 0xFF) as u8);
    }

    fn execute_instruction(&mut self, instruction: u8) {
        match instruction {
            0 => self.inst_no(),
            1 => self.inst_li(),
            2 => self.inst_du(),
            3 => self.inst_dr(),
            4 => self.inst_sw(),
            5 => self.inst_pu(),
            6 => self.inst_po(),
            7 => self.inst_ju(),
            8 => self.inst_ca(),
            9 => self.inst_cc(),
            10 => self.inst_re(),
            11 => self.inst_eq(),
            12 => self.inst_ne(),
            13 => self.inst_lt(),
            14 => self.inst_gt(),
            15 => self.inst_fe(),
            16 => self.inst_st(),
            17 => self.inst_ad(),
            18 => self.inst_su(),
            19 => self.inst_mu(),
            20 => self.inst_di(),
            21 => self.inst_an(),
            22 => self.inst_or(),
            23 => self.inst_xo(),
            24 => self.inst_sh(),
            25 => self.inst_zr(),
            26 => self.inst_ha(),
            27 => self.inst_ie(),
            28 => self.inst_iq(),
            29 => self.inst_ii(),
            _ => self.inst_no(),
        }
    }
}

fn main() {
    let mut vm = NgaVm::new();
    vm.prepare_vm();
    
    match vm.load_image("ngaImage") {
        Ok(_) => vm.execute(0),
        Err(e) => eprintln!("Error loading image: {}", e),
    }
}
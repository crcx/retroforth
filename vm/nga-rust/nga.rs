use std::convert::TryInto;
use std::io::{BufRead, Read, Write};
use std::process::exit;

const IMAGE_SIZE: usize = 242000;
const NUM_DEVICES: usize = 2;

type Cell = i32;

struct VM {
    ip: Cell,
    data: Vec<Cell>,
    address: Vec<Cell>,
    memory: Vec<Cell>,
    notfound: Cell,
    tib: Cell,
    instructions: [fn(&mut VM) -> (); 30],
    io_device_handlers: [fn(&mut VM) -> (); NUM_DEVICES],
    io_query_handlers: [fn(&mut VM) -> (); NUM_DEVICES],
}

impl VM {
    fn pop_data(&mut self) -> Cell {
        if let Some(value) = self.data.pop() {
            return value;
        } else {
            panic!("stack underflow!");
        }
    }

    fn push_data(&mut self, value: Cell) {
        self.data.push(value);
    }

    fn pop_address(&mut self) -> Cell {
        if let Some(value) = self.address.pop() {
            return value;
        } else {
            panic!("stack underflow!");
        }
    }

    fn push_address(&mut self, value: Cell) {
        self.address.push(value);
    }

    fn nga_load_image(&mut self, image_file: &str) {
        if let Ok(image) = std::fs::read(image_file) {
            for (i, c) in image.chunks(4).enumerate() {
                self.memory[i] = i32::from_le_bytes(c.try_into().unwrap());
            }
        } else {
            println!("Unable to find the ngaImage!");
            exit(1);
        }
    }

    fn nga_string_extract(&self, at: usize) -> String {
        self.memory[at..]
            .into_iter()
            .take_while(|&x| *x != 0)
            .flat_map(|&x| std::char::from_u32(x as u32))
            .collect()
    }

    fn nga_string_inject(&mut self, string: String, at: usize) {
        for (i, c) in string.bytes().enumerate() {
            self.memory[at + i] = c.into();
            self.memory[at + i + 1] = 0;
        }
    }

    fn nga_lookup(&self, name: String) -> usize {
        let mut i: usize = self.memory[2] as usize;

        while self.memory[i] != 0 && i != 0 {
            let target = self.nga_string_extract(i + 4);
            if name == target {
                return i;
            } else {
                i = self.memory[i] as usize;
            }
        }

        return 0;
    }

    fn inst_nop(&mut self) {
        return ();
    }

    fn inst_lit(&mut self) {
        self.ip += 1;
        self.push_data(self.memory[self.ip as usize]);
    }

    fn inst_dup(&mut self) {
        let temp: Cell = self.pop_data();
        self.push_data(temp);
        self.push_data(temp);
    }

    fn inst_drop(&mut self) {
        if self.data.len() == 0 {
            self.ip = IMAGE_SIZE as i32;
        } else {
            self.pop_data();
        }
    }

    fn inst_swap(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();
        self.push_data(temp_a);
        self.push_data(temp_b);
    }

    fn inst_push(&mut self) {
        let temp: Cell = self.pop_data();
        self.push_address(temp);
    }

    fn inst_pop(&mut self) {
        let temp: Cell = self.pop_address();
        self.push_data(temp);
    }

    fn inst_jump(&mut self) {
        self.ip = self.pop_data() - 1;
    }

    fn inst_call(&mut self) {
        self.push_address(self.ip);
        self.ip = self.pop_data() - 1;
    }

    fn inst_ccall(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();

        if temp_b != 0 {
            self.push_address(self.ip);
            self.ip = temp_a - 1;
        }
    }

    fn inst_return(&mut self) {
        self.ip = self.pop_address();
    }

    fn inst_eq(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();

        if temp_b == temp_a {
            self.push_data(-1);
        } else {
            self.push_data(0);
        }
    }

    fn inst_neq(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();

        if temp_b != temp_a {
            self.push_data(-1);
        } else {
            self.push_data(0);
        }
    }

    fn inst_lt(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();

        if temp_b < temp_a {
            self.push_data(-1);
        } else {
            self.push_data(0);
        }
    }

    fn inst_gt(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();

        if temp_b > temp_a {
            self.push_data(-1);
        } else {
            self.push_data(0);
        }
    }

    fn inst_fetch(&mut self) {
        match self.pop_data() {
            -1 => self.push_data(self.data.len() as i32),
            -2 => self.push_data(self.address.len() as i32),
            -3 => self.push_data(IMAGE_SIZE as i32),
            -4 => self.push_data(-2147483648),
            -5 => self.push_data(2147483647),
            pt => self.push_data(self.memory[pt as usize]),
        }
    }

    fn inst_store(&mut self) {
        let addr: Cell = self.pop_data();
        let value: Cell = self.pop_data();

        self.memory[addr as usize] = value;
    }

    fn inst_add(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();
        
        self.push_data(temp_b + temp_a);
    }

    fn inst_sub(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();
        
        self.push_data(temp_b - temp_a);
    }

    fn inst_mul(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();
        
        self.push_data(temp_b * temp_a);
    }

    fn inst_divmod(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();

        self.push_data(temp_b % temp_a);
        self.push_data(temp_b / temp_a);
    }

    fn inst_and(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();

        self.push_data(temp_a & temp_b);
    }

    fn inst_or(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();

        self.push_data(temp_a | temp_b);
    }

    fn inst_xor(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();

        self.push_data(temp_a ^ temp_b);
    }

    fn inst_shift(&mut self) {
        let temp_a: Cell = self.pop_data();
        let temp_b: Cell = self.pop_data();

        if temp_b < 0 {
            self.push_data(temp_b << temp_a);
        } else {
            self.push_data(temp_b >> temp_a);
        }
    }

    fn inst_zret(&mut self) {
        match self.pop_data() {
            0 => self.ip = self.pop_address(),
            n => self.push_data(n),
        }
    }

    fn inst_halt(&mut self) {
        self.ip = IMAGE_SIZE as i32;
    }

    fn inst_ie(&mut self) {
        self.push_data(NUM_DEVICES as i32);
    }

    fn inst_iq(&mut self) {
        self.io_query_handlers[self.pop_data() as usize](self);
    }

    fn inst_ii(&mut self) {
        self.io_device_handlers[self.pop_data() as usize](self);
    }

    fn nga_process_opcode(&mut self, opcode: Cell) {
        if opcode != 0 {
            self.instructions[opcode as usize](self);
        }
    }

    fn nga_validate_packed_opcodes(&mut self, opcode: Cell) -> bool {
        opcode.to_le_bytes().iter().all(|&x| x & 0xFF <= 29)
    }

    fn nga_process_packed_opcodes(&mut self, opcode: Cell) {
        for &x in opcode.to_le_bytes().iter() {
            self.nga_process_opcode(x as Cell & 0xFF)
        }
    }

    fn generic_output(&mut self) {
        std::io::stdout()
            .lock()
            .write(&[self.pop_data() as u8])
            .unwrap();
        std::io::stdout()
            .lock()
            .flush()
            .unwrap();
    }

    fn generic_output_query(&mut self) {
        self.push_data(0);
        self.push_data(0);
    }

    fn generic_input(&mut self) {
        match std::io::stdin().lock().bytes().next().unwrap() {
            Ok(127) => self.push_data(8),
            Ok(val) => self.push_data(val as i32),
            Err(_) => exit(1),
        }
    }

    fn generic_input_query(&mut self) {
        self.push_data(0);
        self.push_data(1);
    }

    fn nga_lookup_xt(&mut self, name: String) -> Cell {
        self.memory[1 + self.nga_lookup(name)]
    }

    fn new() -> VM {
        VM {
            ip: 0,
            data: Vec::new(),
            address: Vec::new(),
            notfound: 0,
            tib: 0,
            memory: vec![0; IMAGE_SIZE],
            instructions: [
                VM::inst_nop,
                VM::inst_lit,
                VM::inst_dup,
                VM::inst_drop,
                VM::inst_swap,
                VM::inst_push,
                VM::inst_pop,
                VM::inst_jump,
                VM::inst_call,
                VM::inst_ccall,
                VM::inst_return,
                VM::inst_eq,
                VM::inst_neq,
                VM::inst_lt,
                VM::inst_gt,
                VM::inst_fetch,
                VM::inst_store,
                VM::inst_add,
                VM::inst_sub,
                VM::inst_mul,
                VM::inst_divmod,
                VM::inst_and,
                VM::inst_or,
                VM::inst_xor,
                VM::inst_shift,
                VM::inst_zret,
                VM::inst_halt,
                VM::inst_ie,
                VM::inst_iq,
                VM::inst_ii,
            ],
            io_query_handlers: [
                VM::generic_output_query,
                VM::generic_input_query,
            ],
            io_device_handlers: [
                VM::generic_output,
                VM::generic_input,
            ],
        }
    }

    fn nga_execute(&mut self, at: Cell) {
        self.ip = at;
        self.push_address(0);

        while self.ip < IMAGE_SIZE as Cell {
            if self.ip == self.notfound {
                println!("{} ?", self.nga_string_extract(self.tib as usize));
            }

            let opcode: Cell = self.memory[self.ip as usize];

            if self.nga_validate_packed_opcodes(opcode) {
                self.nga_process_packed_opcodes(opcode);
            } else {
                println!("Invalid instruction!");
                println!("ip: {}, opcode: {}", self.ip, opcode);
                exit(1);
            }

            self.ip += 1;

            if self.address.len() == 0 {
                self.ip = IMAGE_SIZE as Cell;
            }
        }
    }

    fn nga_evaluate(&mut self, string: String) {
        if &string == "bye" {
            exit(0);
        }

        self.notfound = self.nga_lookup_xt(String::from("err:notfound"));

        let interpret: Cell = self.nga_lookup_xt(String::from("interpret"));

        self.nga_string_inject(string, self.tib as usize);

        self.push_data(self.tib as i32);

        self.nga_execute(interpret);
    }
}

fn main() {
    let mut vm: VM = VM::new();

    vm.nga_load_image("ngaImage");

    vm.tib = vm.memory[7];

    println!("RETRO 12 (rx-{}.{})", vm.memory[4] / 100, vm.memory[4] % 100);

    println!("{} MAX, TIB @ {}, HEAP @ {}", IMAGE_SIZE, vm.tib, vm.memory[3]);

    for input in std::io::stdin().lock().lines() {
        for word in input.unwrap().split_whitespace() {
            vm.nga_evaluate(String::from(word));
        }
    }
}

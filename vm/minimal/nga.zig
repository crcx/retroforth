// Copyright (c) awawawawawa
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the copyright
// notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Minimal Nga virtual machine for RETRO, implemented in Zig.
// Build with: zig build-exe nga.zig
const std = @import("std");

const Cell = i32;
const IMAGE_SIZE: usize = 65536;
const ADDRESSES: usize = 256;
const STACK_DEPTH: usize = 256;
const CELL_MIN: Cell = std.math.minInt(Cell) + 1;
const CELL_MAX: Cell = std.math.maxInt(Cell) - 1;

const NgaVm = struct {
    sp: usize = 0,
    rp: usize = 0,
    ip: Cell = 0,
    data: [STACK_DEPTH]Cell = [_]Cell{0} ** STACK_DEPTH,
    address: [ADDRESSES]Cell = [_]Cell{0} ** ADDRESSES,
    memory: [IMAGE_SIZE + 1]Cell = [_]Cell{0} ** (IMAGE_SIZE + 1),

    fn halt(self: *NgaVm) void {
        self.ip = IMAGE_SIZE;
    }

    fn validMemoryAddress(value: Cell) bool {
        return value >= 0 and value <= IMAGE_SIZE;
    }

    fn canPushData(self: *const NgaVm) bool {
        return self.sp < STACK_DEPTH - 1;
    }

    fn canPopData(self: *const NgaVm) bool {
        return self.sp > 0;
    }

    fn canPushAddress(self: *const NgaVm) bool {
        return self.rp < ADDRESSES - 1;
    }

    fn canPopAddress(self: *const NgaVm) bool {
        return self.rp > 0;
    }

    fn tos(self: *const NgaVm) Cell {
        return self.data[self.sp];
    }

    fn nos(self: *const NgaVm) Cell {
        return self.data[self.sp - 1];
    }

    fn tors(self: *const NgaVm) Cell {
        return self.address[self.rp];
    }

    fn setTos(self: *NgaVm, value: Cell) void {
        self.data[self.sp] = value;
    }

    fn setNos(self: *NgaVm, value: Cell) void {
        self.data[self.sp - 1] = value;
    }

    fn setTors(self: *NgaVm, value: Cell) void {
        self.address[self.rp] = value;
    }

    fn stackPush(self: *NgaVm, value: Cell) void {
        if (!self.canPushData()) {
            self.halt();
            return;
        }
        self.sp += 1;
        self.setTos(value);
    }

    fn stackPop(self: *NgaVm) Cell {
        if (!self.canPopData()) {
            self.halt();
            return 0;
        }
        const value = self.tos();
        self.data[self.sp] = 0;
        self.sp -= 1;
        return value;
    }

    fn loadImage(self: *NgaVm, io: std.Io, image_name: []const u8) !void {
        var file = try std.Io.Dir.cwd().openFile(io, image_name, .{});
        defer file.close(io);
        var reader = file.reader(io, &.{});

        var bytes: [4]u8 = undefined;
        var index: usize = 0;
        while (index < IMAGE_SIZE) : (index += 1) {
            reader.interface.readSliceAll(&bytes) catch |err| switch (err) {
                error.EndOfStream => break,
                else => return err,
            };
            self.memory[index] = std.mem.readInt(Cell, &bytes, .little);
        }
    }

    fn execute(self: *NgaVm, io: std.Io, cell: Cell) void {
        self.rp = 1;
        self.ip = cell;
        while (self.ip >= 0 and self.ip < IMAGE_SIZE) {
            const opcode = self.memory[@intCast(self.ip)];
            self.processOpcodeBundle(io, opcode);
            self.ip += 1;
            if (self.rp == 0) self.ip = IMAGE_SIZE;
        }
    }

    fn instLi(self: *NgaVm) void {
        if (self.ip >= IMAGE_SIZE - 1 or !self.canPushData()) return self.halt();
        self.ip += 1;
        self.stackPush(self.memory[@intCast(self.ip)]);
    }

    fn instDu(self: *NgaVm) void {
        if (!self.canPopData()) return self.halt();
        self.stackPush(self.tos());
    }

    fn instDr(self: *NgaVm) void {
        _ = self.stackPop();
    }

    fn instSw(self: *NgaVm) void {
        if (self.sp < 2) return self.halt();
        const value = self.tos();
        self.setTos(self.nos());
        self.setNos(value);
    }

    fn instPu(self: *NgaVm) void {
        if (!self.canPushAddress() or !self.canPopData()) return self.halt();
        self.rp += 1;
        self.setTors(self.tos());
        self.instDr();
    }

    fn instPo(self: *NgaVm) void {
        if (!self.canPopAddress() or !self.canPushData()) return self.halt();
        self.stackPush(self.tors());
        self.rp -= 1;
    }

    fn instJu(self: *NgaVm) void {
        if (!self.canPopData() or self.tos() < 0 or self.tos() >= IMAGE_SIZE) return self.halt();
        self.ip = self.tos() - 1;
        self.instDr();
    }

    fn instCa(self: *NgaVm) void {
        if (!self.canPushAddress() or !self.canPopData() or self.tos() < 0 or self.tos() >= IMAGE_SIZE) return self.halt();
        self.rp += 1;
        self.setTors(self.ip);
        self.ip = self.tos() - 1;
        self.instDr();
    }

    fn instCc(self: *NgaVm) void {
        if (self.sp < 2) return self.halt();
        const target = self.stackPop();
        const flag = self.stackPop();
        if (flag != 0) {
            if (!self.canPushAddress() or target < 0 or target >= IMAGE_SIZE) return self.halt();
            self.rp += 1;
            self.setTors(self.ip);
            self.ip = target - 1;
        }
    }

    fn instRe(self: *NgaVm) void {
        if (!self.canPopAddress() or self.tors() < 0 or self.tors() >= IMAGE_SIZE) return self.halt();
        self.ip = self.tors();
        self.rp -= 1;
    }

    fn compare(self: *NgaVm, result: bool) void {
        if (self.sp < 2) return self.halt();
        self.setNos(if (result) -1 else 0);
        self.instDr();
    }

    fn instFe(self: *NgaVm) void {
        if (!self.canPopData()) return self.halt();
        self.setTos(switch (self.tos()) {
            -1 => @as(Cell, @intCast(self.sp - 1)),
            -2 => @as(Cell, @intCast(self.rp)),
            -3 => IMAGE_SIZE,
            -4 => CELL_MIN,
            -5 => CELL_MAX,
            else => |address| if (validMemoryAddress(address)) self.memory[@intCast(address)] else return self.halt(),
        });
    }

    fn instSt(self: *NgaVm) void {
        if (self.sp < 2) return self.halt();
        const address = self.tos();
        if (!validMemoryAddress(address)) return self.halt();
        self.memory[@intCast(address)] = self.nos();
        self.instDr();
        self.instDr();
    }

    fn instDi(self: *NgaVm) void {
        if (self.sp < 2) return self.halt();
        const divisor = self.tos();
        const dividend = self.nos();
        if (divisor == 0 or (divisor == -1 and dividend == std.math.minInt(Cell))) return self.halt();
        self.setTos(@divTrunc(dividend, divisor));
        self.setNos(@rem(dividend, divisor));
    }

    fn instSh(self: *NgaVm) void {
        if (self.sp < 2) return self.halt();
        const shift = self.tos();
        const value = self.nos();
        if (shift < -31 or shift > 31) return self.halt();
        const amount: u5 = @intCast(if (shift < 0) -shift else shift);
        const bits: u32 = @bitCast(value);
        const shifted: Cell = if (shift < 0) @bitCast(bits << amount) else value >> amount;
        self.setNos(shifted);
        self.instDr();
    }

    fn instIi(self: *NgaVm, io: std.Io) void {
        if (!self.canPopData()) return self.halt();
        switch (self.tos()) {
            0 => {
                self.instDr();
                const value = self.stackPop();
                if (self.ip == IMAGE_SIZE) return;
                var writer = std.Io.File.stdout().writer(io, &.{});
                writer.interface.writeAll(&[_]u8{@truncate(@as(u32, @bitCast(value)))}) catch self.halt();
            },
            1 => {
                var byte: [1]u8 = undefined;
                var reader = std.Io.File.stdin().reader(io, &.{});
                const count = reader.interface.readSliceShort(&byte) catch return self.halt();
                if (count == 0) return self.halt();
                self.instDr();
                self.stackPush(byte[0]);
            },
            else => self.instDr(),
        }
    }

    fn instruction(self: *NgaVm, io: std.Io, opcode: u8) void {
        switch (opcode) {
            0 => {},
            1 => self.instLi(),
            2 => self.instDu(),
            3 => self.instDr(),
            4 => self.instSw(),
            5 => self.instPu(),
            6 => self.instPo(),
            7 => self.instJu(),
            8 => self.instCa(),
            9 => self.instCc(),
            10 => self.instRe(),
            11 => if (self.sp < 2) self.halt() else self.compare(self.nos() == self.tos()),
            12 => if (self.sp < 2) self.halt() else self.compare(self.nos() != self.tos()),
            13 => if (self.sp < 2) self.halt() else self.compare(self.nos() < self.tos()),
            14 => if (self.sp < 2) self.halt() else self.compare(self.nos() > self.tos()),
            15 => self.instFe(),
            16 => self.instSt(),
            17 => {
                if (self.sp < 2) self.halt() else {
                    self.setNos(self.nos() +% self.tos());
                    self.instDr();
                }
            },
            18 => {
                if (self.sp < 2) self.halt() else {
                    self.setNos(self.nos() -% self.tos());
                    self.instDr();
                }
            },
            19 => {
                if (self.sp < 2) self.halt() else {
                    self.setNos(self.nos() *% self.tos());
                    self.instDr();
                }
            },
            20 => self.instDi(),
            21 => {
                if (self.sp < 2) self.halt() else {
                    self.setNos(self.nos() & self.tos());
                    self.instDr();
                }
            },
            22 => {
                if (self.sp < 2) self.halt() else {
                    self.setNos(self.nos() | self.tos());
                    self.instDr();
                }
            },
            23 => {
                if (self.sp < 2) self.halt() else {
                    self.setNos(self.nos() ^ self.tos());
                    self.instDr();
                }
            },
            24 => self.instSh(),
            25 => if (self.canPopData() and self.tos() == 0) {
                if (!self.canPopAddress() or self.tors() < 0 or self.tors() >= IMAGE_SIZE) self.halt() else {
                    self.instDr();
                    self.ip = self.tors();
                    self.rp -= 1;
                }
            },
            26 => self.halt(),
            27 => self.stackPush(2),
            28 => if (!self.canPopData()) self.halt() else if (self.tos() == 0) {
                self.instDr();
                self.stackPush(0);
                self.stackPush(0);
            } else if (self.tos() == 1) {
                self.instDr();
                self.stackPush(1);
                self.stackPush(1);
            },
            29 => self.instIi(io),
            else => self.halt(),
        }
    }

    fn processOpcodeBundle(self: *NgaVm, io: std.Io, opcode: Cell) void {
        const bits: u32 = @bitCast(opcode);
        inline for ([_]u5{ 0, 8, 16, 24 }) |shift| {
            self.instruction(io, @truncate(bits >> shift));
        }
    }
};

pub fn main(init: std.process.Init) void {
    var vm = NgaVm{};
    vm.loadImage(init.io, "ngaImage") catch return;
    vm.execute(init.io, 0);
}

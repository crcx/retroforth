const std = @import("std");

pub const CELL = i32;
pub const CELL_MIN = std.math.minInt(CELL) + 1;
pub const CELL_MAX = std.math.maxInt(CELL) - 1;
const CELL_1: CELL = -1;
const CELL_0: CELL = 0;

fn cell_lsb(data: CELL) u8 {
    return @truncate(@as(u32, @bitCast(data)));
}

/// Amount of RAM.
pub const IMAGE_SIZE = 524288;
/// Max address stack depth
pub const ADDRESSES = 128;
/// Max data stack depth
pub const STACK_DEPTH = 32;

// zig fmt: off
pub const Inst = enum(u8) {
    no_, li_, du_, dr_, sw_, pu_, po_, ju_,
    ca_, cc_, re_, eq_, ne_, lt_, gt_, fe_,
    st_, ad_, su_, mu_, di_, an_, or_, xo_,
    sh_, zr_, ha_, ie_, iq_, ii_,
};
// zig fmt: on

pub const VM = struct {
    /// Data Stack pointer
    sp: CELL = 0,
    /// Return Stack pointer
    rp: CELL = 0,
    /// Instruction pointer
    ip: CELL = 0,
    /// The data stack
    data: [STACK_DEPTH]CELL = .{0} ** STACK_DEPTH,
    /// The address stack
    address: [ADDRESSES]CELL = .{0} ** ADDRESSES,
    /// Image Memory
    memory: [IMAGE_SIZE]CELL = .{0} ** IMAGE_SIZE,

    fin: std.fs.File,
    fout: std.fs.File,

    pub fn init() VM {
        return .{
            .fin = std.io.getStdIn(),
            .fout = std.io.getStdOut(),
        };
    }

    // helper functions

    /// Top item on stack
    pub fn TOS(__: *VM) *CELL {
        return &__.data[@intCast(__.sp)];
    }
    /// Next Top item on stack
    pub fn NOS(__: *VM) *CELL {
        return &__.data[@intCast(__.sp - 1)];
    }
    /// Top item on address stack
    pub fn TORS(__: *VM) *CELL {
        return &__.address[@intCast(__.rp)];
    }
    /// direct memory access
    pub fn MEM(__: *VM, addr: CELL) *CELL {
        return &__.memory[@intCast(addr)];
    }
    pub fn stack_push(__: *VM, value: CELL) void {
        __.sp += 1;
        __.TOS().* = value;
    }
    pub fn stack_pop(__: *VM) CELL {
        const r = __.TOS().*;
        __.dr_();
        return r;
    }

    pub fn halt(__: *VM) void {
        @setCold(true);
        __.ip = IMAGE_SIZE;
    }
    fn not_halted(__: VM) bool {
        return __.ip < IMAGE_SIZE;
    }

    pub fn execute_from(__: *VM, initial_ip: CELL) void {
        __.rp = 1;
        __.ip = initial_ip;
        while (__.not_halted()) {
            const opcode = __.MEM(__.ip).*;
            __.process_opcode_bundle(opcode);
            if (__.rp == 0)
                // __.halt();
                break;
            __.ip += 1;
        }
    }

    pub fn process_opcode_bundle(__: *@This(), opcodes: CELL) void {
        const opcode: u32 = @bitCast(opcodes);
        __.handle_instruction(@enumFromInt(opcode & 0xFF));
        __.handle_instruction(@enumFromInt((opcode >> 8) & 0xFF));
        __.handle_instruction(@enumFromInt((opcode >> 16) & 0xFF));
        __.handle_instruction(@enumFromInt((opcode >> 24) & 0xFF));
    }

    /// instruction/io handler type
    pub fn handle_instruction(__: *VM, inst: Inst) void {
        switch (inst) {
            inline else => |tag| @field(VM, @tagName(tag))(__),
        }
    }

    pub fn no_(__: *VM) void {
        _ = __;
    }
    pub fn li_(__: *VM) void {
        __.ip += 1;
        __.stack_push(__.MEM(__.ip).*);
    }
    pub fn du_(__: *VM) void {
        __.stack_push(__.TOS().*);
    }
    pub fn dr_(__: *VM) void {
        __.TOS().* = 0; // sus: cound set to undefined
        if (__.sp == 0) { // stack exhausted
            __.halt();
        } else {
            __.sp -= 1;
        }
    }
    pub fn sw_(__: *VM) void {
        std.mem.swap(CELL, __.TOS(), __.NOS());
    }
    pub fn pu_(__: *VM) void {
        __.rp += 1;
        __.TORS().* = __.stack_pop();
    }
    pub fn po_(__: *VM) void {
        __.stack_push(__.TORS().*);
        __.rp -= 1;
    }
    pub fn ju_(__: *VM) void {
        __.ip = __.stack_pop() - 1;
    }
    pub fn ca_(__: *VM) void {
        __.rp += 1;
        __.TORS().* = __.ip;
        __.ip = __.stack_pop() - 1;
    }
    pub fn cc_(__: *VM) void {
        const a = __.stack_pop();
        const b = __.stack_pop();
        if (b != 0) {
            __.rp += 1;
            __.TORS().* = __.ip;
            __.ip = a - 1;
        }
    }
    pub fn re_(__: *VM) void {
        __.ip = __.TORS().*;
        __.rp -= 1;
    }
    pub fn eq_(__: *VM) void {
        const cond = (__.NOS().* == __.TOS().*);
        __.NOS().* = if (cond) CELL_1 else CELL_0;
        __.dr_();
    }
    pub fn ne_(__: *VM) void {
        const cond = (__.NOS().* != __.TOS().*);
        __.NOS().* = if (cond) CELL_1 else CELL_0;
        __.dr_();
    }
    pub fn lt_(__: *VM) void {
        const cond = (__.NOS().* < __.TOS().*);
        __.NOS().* = if (cond) CELL_1 else CELL_0;
        __.dr_();
    }
    pub fn gt_(__: *VM) void {
        const cond = (__.NOS().* > __.TOS().*);
        __.NOS().* = if (cond) CELL_1 else CELL_0;
        __.dr_();
    }
    pub fn fe_(__: *VM) void {
        const tos = __.TOS().*;
        __.TOS().* = switch (tos) {
            -1 => __.sp - 1,
            -2 => __.rp,
            -3 => IMAGE_SIZE,
            -4 => CELL_MIN,
            -5 => CELL_MAX,
            else => __.MEM(tos).*,
        };
    }
    pub fn st_(__: *VM) void {
        const tos = __.stack_pop();
        const nos = __.stack_pop();
        if (tos >= 0 and tos <= IMAGE_SIZE) {
            __.MEM(tos).* = nos;
        } else {
            __.halt();
        }
    }
    pub fn ad_(__: *VM) void {
        __.NOS().* +%= __.TOS().*;
        __.dr_();
    }
    pub fn su_(__: *VM) void {
        __.NOS().* -%= __.TOS().*;
        __.dr_();
    }
    pub fn mu_(__: *VM) void {
        __.NOS().* *%= __.TOS().*;
        __.dr_();
    }
    pub fn di_(__: *VM) void {
        // sus: division by zero is UB from the view of VM | handle this error
        const tos = __.TOS().*;
        const nos = __.NOS().*;
        __.TOS().* = @divTrunc(nos, tos);
        __.NOS().* = @rem(nos, tos);
    }
    pub fn an_(__: *VM) void {
        __.NOS().* &= __.TOS().*;
        __.dr_();
    }
    pub fn or_(__: *VM) void {
        __.NOS().* |= __.TOS().*;
        __.dr_();
    }
    pub fn xo_(__: *VM) void {
        __.NOS().* ^= __.TOS().*;
        __.dr_();
    }
    pub fn sh_(__: *VM) void {
        const tos = __.TOS().*;
        const nos = __.NOS().*;
        __.NOS().* = if (tos < 0)
            nos << @intCast(-tos)
        else
        // signed shift
        if (nos < 0 and tos > 0)
            nos >> @intCast(tos) | ~(~@as(CELL, 0) >> @intCast(tos))
        else
            nos >> @intCast(tos);
        __.dr_();
    }
    pub fn zr_(__: *VM) void {
        if (__.TOS().* == 0) {
            __.dr_();
            __.ip = __.TORS().*;
            __.rp -= 1;
        }
    }
    pub fn ha_(__: *VM) void {
        __.halt();
    }
    pub fn ie_(__: *VM) void {
        __.stack_push(io.devices.len);
    }
    pub fn iq_(__: *VM) void {
        const tos = __.stack_pop();
        switch (tos) {
            inline 0...io.devices.len - 1 => |i| {
                const device = io.devices[i];
                __.stack_push(device.type);
                __.stack_push(device.version);
            },
            else => @panic("iq | accessing invalid device"),
        }
    }
    pub fn ii_(__: *VM) void {
        const tos = __.stack_pop();
        switch (tos) {
            inline 0...io.devices.len - 1 => |i| {
                const device = io.devices[i];
                device.handler(__);
            },
            else => @panic("ii | accessing invalid device"),
        }
    }
};

pub const Handler = @TypeOf(VM.no_);

pub const io = struct {
    pub const DeviceDesc = struct {
        type: CELL,
        version: CELL,
        handler: Handler,
    };
    pub const devices = [_]DeviceDesc{
        .{
            .type = 0,
            .version = 0,
            .handler = putchar,
        },
        .{
            .type = 0,
            .version = 1,
            .handler = getchar,
        },
    };
    pub fn putchar(__: *VM) void {
        const char = cell_lsb(__.TOS().*);
        __.fout.writeAll(&.{char}) catch @panic("sus: handle this error");
        __.dr_();
    }
    pub fn getchar(__: *VM) void {
        const maybe_char = _: {
            var buf: [1]u8 = undefined;
            const nread = __.fin.readAll(&buf) catch @panic("sus: handle this error");
            if (nread != buf.len) // eof
                break :_ null;
            break :_ buf[0];
        };
        if (maybe_char) |char|
            __.stack_push(char)
        else // on eof
            __.halt(); // sus: could do better here
    }
};

test "VM dry test" {
    var vm = VM.init();
    const memory: [*]u8 = @ptrCast(@alignCast(&vm.memory));
    memory[0] = @intFromEnum(VM.Inst.li_);
    memory[1] = 0;
    vm.execute_from(0);
    try std.testing.expectEqual(vm.ip, 524288);
}

pub extern const ngaImageCells: c_int;
pub const ngaImage: [*c]c_int = @extern([*c]c_int, .{
    .name = "ngaImage",
});

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();

    const a = gpa.allocator();
    var vm = VM.init();
    if (std.os.argv.len > 1) {
        var p = std.ChildProcess.init(&.{ "retro-unu", std.mem.span(std.os.argv[1]) }, a);
        p.stdin_behavior = .Ignore;
        p.stdout_behavior = .Pipe;
        try p.spawn();
        // _ = try p.wait();
        vm.fin = p.stdout.?;
    }
    @memcpy(vm.memory[0..@intCast(ngaImageCells)], ngaImage[0..@intCast(ngaImageCells)]);
    vm.execute_from(0);
}

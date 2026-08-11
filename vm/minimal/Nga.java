// Minimal Nga virtual machine for RETRO, implemented in Java.
// Produces a single class file when compiled (javac Nga.java).
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;

public final class Nga {
    private static final int IMAGE_SIZE = 65536;
    private static final int ADDRESSES = 256;
    private static final int STACK_DEPTH = 256;
    private static final int CELL_MIN = Integer.MIN_VALUE + 1;
    private static final int CELL_MAX = Integer.MAX_VALUE - 1;

    private int sp;
    private int rp;
    private int ip;
    private final int[] data = new int[STACK_DEPTH];
    private final int[] address = new int[ADDRESSES];
    private final int[] memory = new int[IMAGE_SIZE + 1];

    private int tos() { return data[sp]; }
    private int nos() { return data[sp - 1]; }
    private int tors() { return address[rp]; }
    private void setTos(int v) { data[sp] = v; }
    private void setNos(int v) { data[sp - 1] = v; }
    private void setTors(int v) { address[rp] = v; }

    private int stackPop() {
        sp -= 1;
        return data[sp + 1];
    }

    private void stackPush(int v) {
        sp += 1;
        data[sp] = v;
    }

    private void execute(int cell) throws IOException {
        rp = 1;
        ip = cell;
        while (ip < IMAGE_SIZE) {
            int opcode = memory[ip];
            processOpcodeBundle(opcode);
            ip += 1;
            if (rp == 0) {
                ip = IMAGE_SIZE;
            }
        }
    }

    private void loadImage(String imageFile) throws IOException {
        try (FileInputStream in = new FileInputStream(imageFile)) {
            if (in.getChannel().size() > (long) IMAGE_SIZE * Integer.BYTES) {
                throw new IOException("image exceeds VM memory capacity");
            }
            byte[] buf = in.readAllBytes();
            ByteBuffer bb = ByteBuffer.wrap(buf).order(ByteOrder.LITTLE_ENDIAN);
            int cells = Math.min(bb.remaining() / 4, IMAGE_SIZE);
            for (int i = 0; i < cells; i++) {
                memory[i] = bb.getInt();
            }
        }
    }

    private void prepareVm() {
        ip = sp = rp = 0;
        Arrays.fill(memory, 0);
        Arrays.fill(data, 0);
        Arrays.fill(address, 0);
    }

    private void inst_no() {}

    private void inst_li() {
        ip += 1;
        stackPush(memory[ip]);
    }

    private void inst_du() {
        stackPush(tos());
    }

    private void inst_dr() {
        data[sp] = 0;
        sp -= 1;
        if (sp < 0) {
            ip = IMAGE_SIZE;
        }
    }

    private void inst_sw() {
        int a = tos();
        setTos(nos());
        setNos(a);
    }

    private void inst_pu() {
        rp += 1;
        setTors(tos());
        inst_dr();
    }

    private void inst_po() {
        stackPush(tors());
        rp -= 1;
    }

    private void inst_ju() {
        ip = tos() - 1;
        inst_dr();
    }

    private void inst_ca() {
        rp += 1;
        setTors(ip);
        ip = tos() - 1;
        inst_dr();
    }

    private void inst_cc() {
        int a = tos(); inst_dr();
        int b = tos(); inst_dr();
        if (b != 0) {
            rp += 1;
            setTors(ip);
            ip = a - 1;
        }
    }

    private void inst_re() {
        ip = tors();
        rp -= 1;
    }

    private void inst_eq() {
        setNos(nos() == tos() ? -1 : 0);
        inst_dr();
    }

    private void inst_ne() {
        setNos(nos() != tos() ? -1 : 0);
        inst_dr();
    }

    private void inst_lt() {
        setNos(nos() < tos() ? -1 : 0);
        inst_dr();
    }

    private void inst_gt() {
        setNos(nos() > tos() ? -1 : 0);
        inst_dr();
    }

    private void inst_fe() {
        switch (tos()) {
            case -1: setTos(sp - 1); break;
            case -2: setTos(rp); break;
            case -3: setTos(IMAGE_SIZE); break;
            case -4: setTos(CELL_MIN); break;
            case -5: setTos(CELL_MAX); break;
            default: setTos(memory[tos()]); break;
        }
    }

    private void inst_st() {
        int addr = tos();
        if (addr >= 0 && addr <= IMAGE_SIZE) {
            memory[addr] = nos();
            inst_dr();
            inst_dr();
        } else {
            ip = IMAGE_SIZE;
        }
    }

    private void inst_ad() {
        setNos(nos() + tos());
        inst_dr();
    }

    private void inst_su() {
        setNos(nos() - tos());
        inst_dr();
    }

    private void inst_mu() {
        setNos(nos() * tos());
        inst_dr();
    }

    private void inst_di() {
        int a = tos();
        int b = nos();
        setTos(b / a);
        setNos(b % a);
    }

    private void inst_an() {
        setNos(tos() & nos());
        inst_dr();
    }

    private void inst_or() {
        setNos(tos() | nos());
        inst_dr();
    }

    private void inst_xo() {
        setNos(tos() ^ nos());
        inst_dr();
    }

    private void inst_sh() {
        int y = tos();
        int x = nos();
        int result;
        if (y < 0) {
            result = x << (0 - y);
        } else if (x < 0 && y > 0) {
            result = (x >> y) | (~((~0) >>> y));
        } else {
            result = x >> y;
        }
        setNos(result);
        inst_dr();
    }

    private void inst_zr() {
        if (tos() == 0) {
            inst_dr();
            ip = tors();
            rp -= 1;
        }
    }

    private void inst_ha() {
        ip = IMAGE_SIZE;
    }

    private void inst_ie() {
        stackPush(2);
    }

    private void inst_iq() {
        if (tos() == 0) {
            inst_dr();
            stackPush(0);
            stackPush(0);
        } else if (tos() == 1) {
            inst_dr();
            stackPush(1);
            stackPush(1);
        }
    }

    private void inst_ii() throws IOException {
        if (tos() == 0) {
            inst_dr();
            int c = stackPop();
            System.out.write((byte)(c & 0xFF));
            System.out.flush();
        } else if (tos() == 1) {
            byte[] b = new byte[1];
            int r = System.in.read(b);
            if (r <= 0) {
                System.exit(0);
            }
            inst_dr();
            stackPush(b[0] & 0xFF);
        } else {
            inst_dr();
        }
    }

    private void processOpcodeBundle(int opcode) throws IOException {
        executeInstruction(opcode & 0xFF);
        executeInstruction((opcode >>> 8) & 0xFF);
        executeInstruction((opcode >>> 16) & 0xFF);
        executeInstruction((opcode >>> 24) & 0xFF);
    }

    private void executeInstruction(int inst) throws IOException {
        switch (inst) {
            case 0:  inst_no(); break;
            case 1:  inst_li(); break;
            case 2:  inst_du(); break;
            case 3:  inst_dr(); break;
            case 4:  inst_sw(); break;
            case 5:  inst_pu(); break;
            case 6:  inst_po(); break;
            case 7:  inst_ju(); break;
            case 8:  inst_ca(); break;
            case 9:  inst_cc(); break;
            case 10: inst_re(); break;
            case 11: inst_eq(); break;
            case 12: inst_ne(); break;
            case 13: inst_lt(); break;
            case 14: inst_gt(); break;
            case 15: inst_fe(); break;
            case 16: inst_st(); break;
            case 17: inst_ad(); break;
            case 18: inst_su(); break;
            case 19: inst_mu(); break;
            case 20: inst_di(); break;
            case 21: inst_an(); break;
            case 22: inst_or(); break;
            case 23: inst_xo(); break;
            case 24: inst_sh(); break;
            case 25: inst_zr(); break;
            case 26: inst_ha(); break;
            case 27: inst_ie(); break;
            case 28: inst_iq(); break;
            case 29: inst_ii(); break;
            default: inst_no(); break;
        }
    }

    private static void run() throws IOException {
        Nga vm = new Nga();
        vm.prepareVm();
        vm.loadImage("ngaImage");
        try {
            vm.execute(0);
        } catch (RuntimeException e) {
            /* Invalid guest state halts this VM invocation. */
        }
    }

    public static void main(String[] args) {
        try {
            run();
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
            System.exit(1);
        }
    }
}

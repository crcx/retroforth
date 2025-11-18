// Minimal Nga virtual machine for RETRO, implemented in Kotlin.
// Compiles to a single class file (NgaKt.class) with: kotlinc Nga.kt
import java.io.FileInputStream
import java.io.IOException
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.util.Arrays

private const val IMAGE_SIZE = 65536
private const val ADDRESSES = 256
private const val STACK_DEPTH = 256
private const val CELL_MIN = Int.MIN_VALUE + 1
private const val CELL_MAX = Int.MAX_VALUE - 1

private class NgaVm {
    private var sp = 0
    private var rp = 0
    private var ip = 0
    private val data = IntArray(STACK_DEPTH)
    private val address = IntArray(ADDRESSES)
    private val memory = IntArray(IMAGE_SIZE + 1)

    private val tos get() = data[sp]
    private val nos get() = data[sp - 1]
    private val tors get() = address[rp]
    private fun setTos(v: Int) { data[sp] = v }
    private fun setNos(v: Int) { data[sp - 1] = v }
    private fun setTors(v: Int) { address[rp] = v }

    private fun stackPop(): Int {
        sp -= 1
        return data[sp + 1]
    }

    private fun stackPush(v: Int) {
        sp += 1
        data[sp] = v
    }

    @Throws(IOException::class)
    fun execute(cell: Int) {
        rp = 1
        ip = cell
        while (ip < IMAGE_SIZE) {
            val opcode = memory[ip]
            processOpcodeBundle(opcode)
            ip += 1
            if (rp == 0) ip = IMAGE_SIZE
        }
    }

    @Throws(IOException::class)
    fun loadImage(imageFile: String) {
        FileInputStream(imageFile).use { fis ->
            val buf = fis.readBytes()
            val bb = ByteBuffer.wrap(buf).order(ByteOrder.LITTLE_ENDIAN)
            val cells = minOf(bb.remaining() / 4, IMAGE_SIZE)
            repeat(cells) { i -> memory[i] = bb.int }
        }
    }

    fun prepareVm() {
        ip = 0; sp = 0; rp = 0
        Arrays.fill(memory, 0)
        Arrays.fill(data, 0)
        Arrays.fill(address, 0)
    }

    private fun inst_no() {}
    private fun inst_li() { ip += 1; stackPush(memory[ip]) }
    private fun inst_du() { stackPush(tos) }
    private fun inst_dr() { data[sp] = 0; sp -= 1; if (sp < 0) ip = IMAGE_SIZE }
    private fun inst_sw() { val a = tos; setTos(nos); setNos(a) }
    private fun inst_pu() { rp += 1; setTors(tos); inst_dr() }
    private fun inst_po() { stackPush(tors); rp -= 1 }
    private fun inst_ju() { ip = tos - 1; inst_dr() }
    private fun inst_ca() { rp += 1; setTors(ip); ip = tos - 1; inst_dr() }
    private fun inst_cc() {
        val a = tos; inst_dr()
        val b = tos; inst_dr()
        if (b != 0) {
            rp += 1
            setTors(ip)
            ip = a - 1
        }
    }
    private fun inst_re() { ip = tors; rp -= 1 }
    private fun inst_eq() { setNos(if (nos == tos) -1 else 0); inst_dr() }
    private fun inst_ne() { setNos(if (nos != tos) -1 else 0); inst_dr() }
    private fun inst_lt() { setNos(if (nos < tos) -1 else 0); inst_dr() }
    private fun inst_gt() { setNos(if (nos > tos) -1 else 0); inst_dr() }
    private fun inst_fe() {
        when (tos) {
            -1 -> setTos(sp - 1)
            -2 -> setTos(rp)
            -3 -> setTos(IMAGE_SIZE)
            -4 -> setTos(CELL_MIN)
            -5 -> setTos(CELL_MAX)
            else -> setTos(memory[tos])
        }
    }
    private fun inst_st() {
        val addr = tos
        if (addr in 0..IMAGE_SIZE) {
            memory[addr] = nos
            inst_dr(); inst_dr()
        } else ip = IMAGE_SIZE
    }
    private fun inst_ad() { setNos(nos + tos); inst_dr() }
    private fun inst_su() { setNos(nos - tos); inst_dr() }
    private fun inst_mu() { setNos(nos * tos); inst_dr() }
    private fun inst_di() { val a = tos; val b = nos; setTos(b / a); setNos(b % a) }
    private fun inst_an() { setNos(tos and nos); inst_dr() }
    private fun inst_or() { setNos(tos or nos); inst_dr() }
    private fun inst_xo() { setNos(tos xor nos); inst_dr() }
    private fun inst_sh() {
        val y = tos
        val x = nos
        val result = when {
            y < 0 -> x shl (0 - y)
            x < 0 && y > 0 -> (x shr y) or (-(1 shl (32 - y)))
            else -> x shr y
        }
        setNos(result); inst_dr()
    }
    private fun inst_zr() { if (tos == 0) { inst_dr(); ip = tors; rp -= 1 } }
    private fun inst_ha() { ip = IMAGE_SIZE }
    private fun inst_ie() { stackPush(2) }
    private fun inst_iq() {
        when (tos) {
            0 -> { inst_dr(); stackPush(0); stackPush(0) }
            1 -> { inst_dr(); stackPush(1); stackPush(1) }
        }
    }
    private fun inst_ii() {
        when (tos) {
            0 -> {
                inst_dr()
                val c = stackPop()
                System.out.write(c and 0xFF)
                System.out.flush()
            }
            1 -> {
                val b = ByteArray(1)
                val r = System.`in`.read(b)
                if (r <= 0) System.exit(0)
                inst_dr()
                stackPush(b[0].toInt() and 0xFF)
            }
            else -> inst_dr()
        }
    }

    @Throws(IOException::class)
    private fun processOpcodeBundle(opcode: Int) {
        executeInstruction(opcode and 0xFF)
        executeInstruction(opcode ushr 8 and 0xFF)
        executeInstruction(opcode ushr 16 and 0xFF)
        executeInstruction(opcode ushr 24 and 0xFF)
    }

    @Throws(IOException::class)
    private fun executeInstruction(inst: Int) {
        when (inst) {
            0 -> inst_no()
            1 -> inst_li()
            2 -> inst_du()
            3 -> inst_dr()
            4 -> inst_sw()
            5 -> inst_pu()
            6 -> inst_po()
            7 -> inst_ju()
            8 -> inst_ca()
            9 -> inst_cc()
            10 -> inst_re()
            11 -> inst_eq()
            12 -> inst_ne()
            13 -> inst_lt()
            14 -> inst_gt()
            15 -> inst_fe()
            16 -> inst_st()
            17 -> inst_ad()
            18 -> inst_su()
            19 -> inst_mu()
            20 -> inst_di()
            21 -> inst_an()
            22 -> inst_or()
            23 -> inst_xo()
            24 -> inst_sh()
            25 -> inst_zr()
            26 -> inst_ha()
            27 -> inst_ie()
            28 -> inst_iq()
            29 -> inst_ii()
            else -> inst_no()
        }
    }
}

@Throws(IOException::class)
private fun runVm() {
    val vm = NgaVm()
    vm.prepareVm()
    vm.loadImage("ngaImage")
    vm.execute(0)
}

@Throws(Exception::class)
fun main() {
    try {
        runVm()
    } catch (e: IOException) {
        System.err.println("Error: ${e.message}")
        kotlin.system.exitProcess(1)
    }
}

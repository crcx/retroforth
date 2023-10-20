/* nga.d, (c) charles childers                                */

import std.stdio;
import std.stdint;
import core.stdc.stdlib;

enum int_MIN = -2147483647;
enum int_MAX = 2147483646;

enum IMAGE_SIZE = 65536; /* Amount of RAM, in ints */
enum ADDRESSES = 256;    /* Depth of address stack */
enum STACK_DEPTH = 256;  /* Depth of data stack */

int sp, rp, ip;                  /* Stack and instruction pointers */
int[STACK_DEPTH] data;           /* The data stack          */
int[ADDRESSES] address;          /* The address stack       */
int[IMAGE_SIZE] memory;          /* Image Memory            */

void stack_push(int value)
{
    sp += 1;
    data[sp] = value;
}

int stack_pop()
{
    sp -= 1;
    return data[sp + 1];
}

void execute(int c)
{
    int i;
    ip = c;
    while (ip < IMAGE_SIZE)
    {
        process_bundle(memory[ip]);
        ip += 1;
    }
}

void load_image()
{
    FILE* fp;
    long fileLen;
    if ((fp = fopen("ngaImage", "rb")) !is null)
    {
        fseek(fp, 0, SEEK_END);
        fileLen = ftell(fp);
        rewind(fp);
        fread(memory.ptr, 1, fileLen, fp);
        fclose(fp);
    }
}

void prepare_vm()
{
    ip = sp = rp = 0;
    foreach (i; 0 .. IMAGE_SIZE)
        memory[i] = 0; /* NO - nop instruction */
    foreach (i; 0 .. STACK_DEPTH)
        data[i] = 0;
    foreach (i; 0 .. ADDRESSES)
        address[i] = 0;
}

void inst_no() {}

void inst_li()
{
    ip += 1;
    stack_push(memory[ip]);
}

void inst_du()
{
    stack_push(data[sp]);
}

void inst_dr()
{
    stack_pop();
}

void inst_sw()
{
    int a = data[sp];
    data[sp] = data[sp - 1];
    data[sp - 1] = a;
}

void inst_pu()
{
    rp += 1;
    address[rp] = stack_pop();
}

void inst_po()
{
    stack_push(address[rp]);
    rp -= 1;
}

void inst_ju()
{
    ip = stack_pop() - 1;
}

void inst_ca()
{
    rp += 1;
    address[rp] = ip;
    ip = stack_pop() - 1;
}

void inst_cc()
{
    int a, b;
    a = stack_pop();  /* Target */
    b = stack_pop();  /* Flag   */
    if (b != 0)
    {
        rp += 1;
        address[rp] = ip;
        ip = a - 1;
    }
}

void inst_re()
{
    ip = address[rp];
    rp -= 1;
}

void inst_eq()
{
    data[sp - 1] = (data[sp - 1] == data[sp]) ? -1 : 0;
    inst_dr();
}

void inst_ne()
{
    data[sp - 1] = (data[sp - 1] != data[sp]) ? -1 : 0;
    inst_dr();
}

void inst_lt()
{
    data[sp - 1] = (data[sp - 1] < data[sp]) ? -1 : 0;
    inst_dr();
}

void inst_gt()
{
    data[sp - 1] = (data[sp - 1] > data[sp]) ? -1 : 0;
    inst_dr();
}

void inst_fe()
{
    switch (data[sp])
    {
        case -1: data[sp] = sp - 1; break;
        case -2: data[sp] = rp; break;
        case -3: data[sp] = IMAGE_SIZE; break;
        case -4: data[sp] = int_MIN; break;
        case -5: data[sp] = int_MAX; break;
        default: data[sp] = memory[data[sp]]; break;
    }
}

void inst_st()
{
    if (data[sp] <= IMAGE_SIZE && data[sp] >= 0)
    {
        memory[data[sp]] = data[sp - 1];
        inst_dr();
        inst_dr();
    }
    else
    {
        ip = IMAGE_SIZE;
    }
}

void inst_ad()
{
    data[sp - 1] += data[sp];
    inst_dr();
}

void inst_su()
{
    data[sp - 1] -= data[sp];
    inst_dr();
}

void inst_mu()
{
    data[sp - 1] *= data[sp];
    inst_dr();
}

void inst_di()
{
    int a = data[sp];
    int b = data[sp - 1];
    data[sp] = b / a;
    data[sp - 1] = b % a;
}

void inst_an()
{
    data[sp - 1] = data[sp] & data[sp - 1];
    inst_dr();
}

void inst_or()
{
    data[sp - 1] = data[sp] | data[sp - 1];
    inst_dr();
}

void inst_xo()
{
    data[sp - 1] = data[sp] ^ data[sp - 1];
    inst_dr();
}

void inst_sh()
{
    int y = data[sp];
    int x = data[sp - 1];
    if (data[sp] < 0)
        data[sp - 1] = data[sp - 1] << (0 - data[sp]);
    else
    {
        if (x < 0 && y > 0)
            data[sp - 1] = x >> y | ~(~0U >> y);
        else
            data[sp - 1] = x >> y;
    }
    inst_dr();
}

void inst_zr()
{
    if (data[sp] == 0)
    {
        inst_dr();
        ip = address[rp];
        rp -= 1;
    }
}

void inst_ha()
{
    ip = IMAGE_SIZE;
}

void inst_ie()
{
    stack_push(2);
}

void inst_iq()
{
    if (data[sp] == 0)
    {
        inst_dr();
        stack_push(0);
        stack_push(0);
    }
    else if (data[sp] == 1)
    {
        inst_dr();
        stack_push(1);
        stack_push(1);
    }
}

void inst_ii()
{
    int c;
    if (data[sp] == 0)
    {
        inst_dr();
        writef("%c", cast(char)stack_pop());
    }
    else if (data[sp] == 1)
    {
        c = getchar();
        if (c < 0) exit(0);
        inst_dr();
        stack_push(c);
    }
    else
    {
        inst_dr();
    }
}

void process(int opcode) {
    switch (opcode) {
        case 0: inst_no(); break;
        case 1: inst_li(); break;
        case 2: inst_du(); break;
        case 3: inst_dr(); break;
        case 4: inst_sw(); break;
        case 5: inst_pu(); break;
        case 6: inst_po(); break;
        case 7: inst_ju(); break;
        case 8: inst_ca(); break;
        case 9: inst_cc(); break;
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
        default: writef("\nERROR: %d\n", opcode); /* Handle unknown opcode */ exit(1); break;
    }
}

void process_bundle(int opcode) {
    process(opcode & 0xFF);
    process((opcode >> 8) & 0xFF);
    process((opcode >> 16) & 0xFF);
    process((opcode >> 24) & 0xFF);
}

void main(string[] args)
{
    prepare_vm();
    load_image();
    execute(0);
    while (sp > 0) { writef("%d ", stack_pop()); } writef("\n");
    exit(0);
}

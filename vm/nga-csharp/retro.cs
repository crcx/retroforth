/*
 * Nga Virtual Machine & Interface for RETRO 12
 *
 * Copyright (c) 2009-2011, Simon Waite
 * Copyright (c) 2009-2019, Charles Childers
*/

using System;
using System.IO;
using System.Text;

namespace Nga
{
  public class VM
  {

    /* Registers */
    int sp, rsp, ip;
    int[] data, address, memory;

    string request;
    static readonly int MAX_REQUEST_LENGTH = 1024;

    /* Opcodes recognized by the VM */
    enum OpCodes {
      VM_NOP,      VM_LIT,        VM_DUP,
      VM_DROP,     VM_SWAP,       VM_PUSH,
      VM_POP,      VM_JUMP,       VM_CALL,
      VM_CCALL,    VM_RETURN,     VM_EQ,
      VM_NEQ,      VM_LT,         VM_GT,
      VM_FETCH,    VM_STORE,      VM_ADD,
      VM_SUB,      VM_MUL,        VM_DIVMOD,
      VM_AND,      VM_OR,         VM_XOR,
      VM_SHIFT,    VM_ZRET,       VM_HALT,
      VM_IE,       VM_IQ,         VM_II
    }

    string rxGetString(int starting)
    {
        int i = 0;
        char[] requestTmp = new char[MAX_REQUEST_LENGTH];
        while (memory[starting] > 0 && i < MAX_REQUEST_LENGTH)
        {
            requestTmp[i++] = (char)memory[starting++];
        }
        request = new string(requestTmp);
        request = request.TrimEnd('\0');
        return request;
    }

    void ngaInjectString(string s, int starting)
    {
        int i = 0;
        char[] requestTmp = s.ToCharArray();
        for (i = 0; i < requestTmp.Length; i++)
        {
            memory[starting++] = (char)requestTmp[i];
        }
        memory[starting++] = 0;
    }

    void pushData(int i)
    {
        sp++; data[sp] = i;
    }

    /* Initialize the VM */
    public VM() {
      sp = 0;
      rsp = 0;
      ip = 0;
      data    = new int[128];
      address = new int[1024];
      memory  = new int[(524288 * 16) + 1];
      loadImage();
      if (memory[0] == 0) {
        Console.Write("Sorry, unable to find ngaImage\n");
        Environment.Exit(0);
      }
    }

    /* Load the 'ngaImage' into memory */
    public void loadImage() {
      int i = 0;
      if (!File.Exists("ngaImage"))
        return;
      BinaryReader binReader = new BinaryReader(File.Open("ngaImage", FileMode.Open));
      FileInfo f = new FileInfo("ngaImage");
      long s = f.Length / 4;
      try {
        while (i < s) { memory[i] = binReader.ReadInt32(); i++; }
      }
      catch(EndOfStreamException e) {
        Console.WriteLine("{0} caught and ignored." , e.GetType().Name);
      }
      finally {
        binReader.Close();
      }
    }

    /* Save the image */
    public void saveImage() {
      int i = 0, j = 1000000;
      BinaryWriter binWriter = new BinaryWriter(File.Open("ngaImage", FileMode.Create));
      try {
        while (i < j) { binWriter.Write(memory[i]); i++; }
      }
      catch(EndOfStreamException e) {
        Console.WriteLine("{0} caught and ignored." , e.GetType().Name);
      }
      finally {
        binWriter.Close();
      }
    }

    /* Read a key */
    public int read_key() {
      int a = 0;
        ConsoleKeyInfo cki = Console.ReadKey();
        a = (int)cki.KeyChar;
        if (cki.Key == ConsoleKey.Backspace) {
          a = 8;
          Console.Write((char)32);
        }
        if ( a >= 32)
          Console.Write((char)8);
      return a;
    }

    /* Process the current opcode */
    public void ngaProcessOpcode(int opcode) {
      int x, y;
      switch((OpCodes)opcode)
      {
        case OpCodes.VM_NOP:
          break;
        case OpCodes.VM_LIT:
          sp++; ip++; data[sp] = memory[ip];
          break;
        case OpCodes.VM_DUP:
          sp++; data[sp] = data[sp-1];
          break;
        case OpCodes.VM_DROP:
          data[sp] = 0; sp--;
          break;
        case OpCodes.VM_SWAP:
          x = data[sp];
          y = data[sp-1];
          data[sp] = y;
          data[sp-1] = x;
          break;
        case OpCodes.VM_PUSH:
          rsp++;
          address[rsp] = data[sp];
          sp--;
          break;
        case OpCodes.VM_POP:
          sp++;
          data[sp] = address[rsp];
          rsp--;
          break;
        case OpCodes.VM_CALL:
          rsp++;
          address[rsp] = ip;
          ip = data[sp] - 1;
          sp = sp - 1;
          break;
        case OpCodes.VM_CCALL:
          if (data[sp - 1] == -1) {
            rsp++;
            address[rsp] = ip;
            ip = data[sp] - 1;
          }
          sp = sp - 2;
          break;
        case OpCodes.VM_JUMP:
          ip = data[sp] - 1;
          sp = sp - 1;
          break;
        case OpCodes.VM_RETURN:
          ip = address[rsp]; rsp--;
          break;
        case OpCodes.VM_GT:
          if (data[sp-1] > data[sp])
            data[sp-1] = -1;
          else
            data[sp-1] = 0;
          sp = sp - 1;
          break;
        case OpCodes.VM_LT:
          if (data[sp-1] < data[sp])
            data[sp-1] = -1;
          else
            data[sp-1] = 0;
          sp = sp - 1;
          break;
        case OpCodes.VM_NEQ:
          if (data[sp-1] != data[sp])
            data[sp-1] = -1;
          else
            data[sp-1] = 0;
          sp = sp - 1;
          break;
        case OpCodes.VM_EQ:
          if (data[sp-1] == data[sp])
            data[sp-1] = -1;
          else
            data[sp-1] = 0;
          sp = sp - 1;
          break;
        case OpCodes.VM_FETCH:
          x = data[sp];
          if (x == -1)
            data[sp] = sp - 1;
          else if (x == -2)
            data[sp] = rsp - 1;
          else if (x == -3)
            data[sp] = 524288 * 16;
          else if (x == -4)
            data[sp] = -2147483648;
          else if (x == -5)
            data[sp] = 2147483647;
          else
            data[sp] = memory[x];
          break;
        case OpCodes.VM_STORE:
          memory[data[sp]] = data[sp-1];
          sp = sp - 2;
          break;
        case OpCodes.VM_ADD:
          data[sp-1] += data[sp]; data[sp] = 0; sp--;
          break;
        case OpCodes.VM_SUB:
          data[sp-1] -= data[sp]; data[sp] = 0; sp--;
          break;
        case OpCodes.VM_MUL:
          data[sp-1] *= data[sp]; data[sp] = 0; sp--;
          break;
        case OpCodes.VM_DIVMOD:
          x = data[sp];
          y = data[sp-1];
          data[sp] = y / x;
          data[sp-1] = y % x;
          break;
        case OpCodes.VM_AND:
          x = data[sp];
          y = data[sp-1];
          sp--;
          data[sp] = x & y;
          break;
        case OpCodes.VM_OR:
          x = data[sp];
          y = data[sp-1];
          sp--;
          data[sp] = x | y;
          break;
        case OpCodes.VM_XOR:
          x = data[sp];
          y = data[sp-1];
          sp--;
          data[sp] = x ^ y;
          break;
        case OpCodes.VM_SHIFT:
          x = data[sp];
          y = data[sp-1];
          sp--;
          if (x < 0)
            data[sp] = y << x;
          else
            data[sp] = y >>= x;
          break;
        case OpCodes.VM_ZRET:
          if (data[sp] == 0) {
            sp--;
            ip = address[rsp]; rsp--;
          }
          break;
        case OpCodes.VM_HALT:
          ip = 524288 * 16;
          break;
        case OpCodes.VM_IE:
          data[sp] = 1;
          break;
        case OpCodes.VM_IQ:
          data[sp++] = 0;
          data[sp] = 0;
          break;
        case OpCodes.VM_II:
          sp--;
          char c = (char)data[sp--];
          Console.Write(c);
          break;
        default:
          ip = 524288 * 16;
          break;
      }
    }

    public int ngaValidatePackedOpcodes(int opcode) {
      int raw = opcode;
      int current;
      int valid = -1;
      for (int i = 0; i < 4; i++) {
        current = raw & 0xFF;
        if (!(current >= 0 && current <= 29))
          valid = 0;
        raw = raw >> 8;
      }
      return valid;
    }

    void ngaProcessPackedOpcodes(int opcode) {
      int raw = opcode;
      for (int i = 0; i < 4; i++) {
        ngaProcessOpcode(raw & 0xFF);
        raw = raw >> 8;
      }
    }

    int d_lookup(string name) {
      int dt = 0;
      int i = memory[2]; // Dictionary
      string target;
      while (memory[i] != 0 && i != 0) {
        target = rxGetString(i + 3);
        if (name.Equals(target)) {
          dt = i;
          i = 0;
        } else {
          i = memory[i];
        }
      }
      return dt;
    }

public void executeFunction(int cell) {
  int opcode;
  rsp = 1;
  ip = cell;
  int notfound = memory[d_lookup("err:notfound") + 1];
  while (ip < 524288 * 16) {
    if (ip == notfound) {
      Console.Write("\n" + rxGetString(1471) + " ?\n");
    }
    opcode = memory[ip];
    if (ngaValidatePackedOpcodes(opcode) != 0) {
      ngaProcessPackedOpcodes(opcode);
    } else {
      ngaProcessOpcode(opcode);
    }
    ip++;
    if (rsp == 0)
      ip = 524288 * 16;
  }
}

    public void Execute() {
      for (ip = 0; ip < 524288 * 16; ip++) {
         Console.Write(ip + ":" + memory[ip] + "\n");
         ngaProcessPackedOpcodes(memory[ip]);
      }
    }

    /* Main entry point */
    /* Calls all the other stuff and process the command line */
    public static void Main(string [] args) {
      VM vm = new VM();
      for (int i = 0; i < args.Length; i++) {
        if (args[i] == "--about") {
          Console.Write("Nga [VM: C#, .NET]\n\n");
          Environment.Exit(0);
        }
      }

      Console.Write("RETRO 12 (rx-");
      Console.Write(vm.memory[4] / 100);
      Console.Write(".");
      Console.Write(vm.memory[4] % 100);
      Console.Write(")\n\n");

      while (true) {
        string input = Console.ReadLine();
        foreach (string word in input.Split(' ')) {
          if (word.Equals("bye")) Environment.Exit(0);
          vm.ngaInjectString(word, 1025);
          vm.pushData(1025);
          vm.executeFunction(vm.memory[vm.d_lookup("interpret") + 1]);
        }
      }
    }
  }
}


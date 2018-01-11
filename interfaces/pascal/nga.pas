// ********************************************************
//  Copyright (c) 2016 Rob Judd <judd@ob-wan.com>
//  Based on C version by Charles Childers et al
//  ISC License - see included file LICENSE
// ********************************************************

unit nga;

{$mode objfpc}{$H+}
{$macro on}

interface

{$include 'nga.inc'}

procedure ngaPrepare();
function ngaLoadImage(imageFile : string) : Cell;
function ngaValidatePackedOpcodes(opcode : Cell) : Integer;
procedure ngaProcessPackedOpcodes(opcode : Cell);
procedure ngaProcessOpcode(opcode : Cell);

var
  ip, ap, sp : Cell;                        // instruction, address & stack pointers
  data : array [0..STACK_DEPTH-1] of Cell;  // stack depth
  address : array [0..ADDRESSES-1] of Cell; // addresses
  memory : array [0..IMAGE_SIZE-1] of Cell; // image size

implementation

uses
  SysUtils;

function ngaLoadImage(imageFile : string) : Cell;
var
  f : File of Cell;
  sr : TSearchRec;
  fileLen : Cell;
  imageSize : Cell = 0;
begin
  if FindFirst(imageFile, faAnyFile-faDirectory, sr) = 0 then
  begin
    fileLen := sr.Size div sizeof(Cell);
    Assignfile(f, imageFile);
    Reset(f, SizeOf(Cell));
    try
      BlockRead(f, memory, fileLen, imageSize);
    finally
      CloseFile(f);
    end;
  end
  else
    writeln(format('Unable to find %s!', [imageFile]));
  FindClose(sr);
  result := imageSize;
end;

procedure ngaPrepare();
begin
  ip := 0;
  ap := 0;
  sp := 0;
  for ip := 0 to STACK_DEPTH - 1 do
    data[ip] := 0;                     // ord(VM_NOP);
  for ip := 0 to ADDRESSES - 1 do
    address[ip] := 0;
  for ip := 0 to IMAGE_SIZE - 1 do
    memory[ip] := 0;
end;

procedure inst_nop();
begin
  // Do nothing
end;

procedure inst_lit();
begin
  inc(ip);
  inc(sp);
  TOS := memory[ip];
end;

procedure inst_dup();
begin
  inc(sp);
  data[sp] := NOS;
end;

procedure inst_drop();
begin
  data[sp] := 0;
  dec(sp);
  if sp < 0 then
    ip := IMAGE_SIZE - 1;
end;

procedure inst_swap();
var a : Cell;
begin
  a := TOS;
  TOS := NOS;
  NOS := a;
end;

procedure inst_push();
begin
  inc(ap);
  TOA := TOS;
  inst_drop();
end;

procedure inst_pop();
begin
  inc(sp);
  TOS := TOA;
  dec(ap);
end;

procedure inst_jump();
begin
  ip := TOS - 1;
  inst_drop();
end;

procedure inst_call();
begin
  inc(ap);
  TOA := ip;
  ip := TOS - 1;
  inst_drop();
end;

procedure inst_ccall();
var
  a, b : Cell;
begin
  a := TOS;
  inst_drop();                         // false
  b := TOS;
  inst_drop();                         // flag
  if b <> 0 then
  begin
    inc(ap);
    TOA := ip;
    ip := a - 1;
  end;
end;

procedure inst_ret();
begin
  ip := TOA;
  dec(ap);
end;

procedure inst_eq();
begin
  if NOS = TOS then
    NOS := -1
  else
    NOS := 0;
  inst_drop();
end;

procedure inst_neq();
begin
  if NOS <> TOS then
    NOS := -1
  else
    NOS := 0;
  inst_drop();
end;

procedure inst_lt();
begin
  if NOS < TOS then
    NOS := -1
  else
    NOS := 0;
  inst_drop();
end;

procedure inst_gt();
begin
  if NOS > TOS then
    NOS := -1
  else
    NOS := 0;
  inst_drop();
end;

procedure inst_fetch();
begin
  case TOS of
    -1 : TOS := sp - 1;
    -2 : TOS := ap;
  else
    TOS := memory[TOS];
  end;
end;

procedure inst_store();
begin
  memory[TOS] := NOS;
  inst_drop();
  inst_drop();
end;

procedure inst_add();
begin
  NOS += TOS;
  inst_drop();
end;

procedure inst_sub();
begin
  NOS -= TOS;
  inst_drop();
end;

procedure inst_mul();
begin
  NOS *= TOS;
  inst_drop();
end;

procedure inst_divmod();
var
  a, b : Cell;
begin
  a := TOS;
  b := NOS;
  TOS := b div a;
  NOS := b mod a;
end;

procedure inst_and();
begin
  NOS := NOS and TOS;
  inst_drop();
end;

procedure inst_or();
begin
  NOS := NOS or TOS;
  inst_drop();
end;

procedure inst_xor();
begin
  NOS := NOS xor TOS;
  inst_drop();
end;

procedure inst_shift();
var
  x, y : Cell;
  z : Cell = 0;
begin
  x := NOS;
  y := TOS;
  if TOS < 0 then
    NOS := NOS shl (TOS * -1)
  else
  begin
    if (x < 0) and (y > 0) then
      NOS := x shr y or not(not z shr y)
    else
      NOS := x shr y;
  end;
  inst_drop();
end;

procedure inst_zret();
begin
  if TOS = 0 then
  begin
    inst_drop();
    ip := TOA;
    dec(ap);
  end;
end;

procedure inst_end();
begin
  ip := IMAGE_SIZE - 1;
end;

procedure ngaProcessOpcode(opcode : Cell);
begin
  case opcode of
    0  : inst_nop();
    1  : inst_lit();
    2  : inst_dup();
    3  : inst_drop();
    4  : inst_swap();
    5  : inst_push();
    6  : inst_pop();
    7  : inst_jump();
    8  : inst_call();
    9  : inst_ccall();
    10 : inst_ret();
    11 : inst_eq();
    12 : inst_neq();
    13 : inst_lt();
    14 : inst_gt();
    15 : inst_fetch();
    16 : inst_store();
    17 : inst_add();
    18 : inst_sub();
    19 : inst_mul();
    20 : inst_divmod();
    21 : inst_and();
    22 : inst_or();
    23 : inst_xor();
    24 : inst_shift();
    25 : inst_zret();
    26 : inst_end();
  end;
end;

function ngaValidatePackedOpcodes(opcode : Cell) : Integer;
var
  raw, current : Cell;
  i : Byte;
begin
  result := -1;                        // value for "true" in Unix-land
  raw := opcode;
  for i := 1 to 4 do
  begin
    current := raw and $FF;
    if ((current >= 0) and (current < NUM_OPS)) = false then
      result := 0;
    raw := raw shr 8;
  end;
end;

procedure ngaProcessPackedOpcodes(opcode : Cell);
var
  raw : Cell;
  i : Byte;
begin
  raw := opcode;
  for i := 1 to 4 do
  begin
    ngaProcessOpcode(raw and $FF);
    raw := raw shr 8;
  end;
end;
end.

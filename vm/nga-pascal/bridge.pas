// ********************************************************
//  Copyright (c) 2016 Rob Judd <judd@ob-wan.com>
//  Based on C version by Charles Childers et al
//  ISC License - see included file LICENSE
// ********************************************************

unit bridge;

{$mode objfpc}{$H+}
{$macro on}

interface

{$include 'nga.inc'}

const
  D_OFFSET_LINK  = 0;
  D_OFFSET_XT    = 1;
  D_OFFSET_CLASS = 2;
  D_OFFSET_NAME  = 4;
  TIB            = 1471;

var
  Dictionary, Heap, Compiler, notfound : Cell;
  string_data : array[0..8191] of Char;

function stack_pop() : Cell;
procedure stack_push(value : Cell);
procedure string_inject(str : PChar; buffer : Integer);
function string_extract(at : Cell) : PChar;
function d_link(dt : Cell) : Integer;
function d_xt(dt : Cell) : Integer;
function d_class(dt : Cell) : Integer;
function d_name(dt : Cell) : Integer;
function d_count_entries(Dictionary : Cell) : Cell;
function d_lookup(Dictionary : cell; name : PChar) : Cell;
function d_xt_for(Name : PChar; Dictionary : Cell) : Cell;
function d_class_for(Name : PChar; Dictionary : Cell) : Cell;
procedure execute(cel : Cell);
procedure update_rx();
procedure evaluate(s: PChar);
procedure read_token(var fil : File; token_buffer : PChar); overload;
procedure read_token(token_buffer : PChar); overload;

implementation

uses
  SysUtils, nga in 'nga.pas';

function stack_pop() : Cell;
begin
  dec(sp);
  result := data[sp + 1];
end;

procedure stack_push(value : Cell);
begin
  inc(sp);
  data[sp] := value;
end;

procedure string_inject(str : PChar; buffer : Integer);
var
  i : Integer = 0;
  m : Integer;
begin
  m := strlen(str);
  while m > 0 do
  begin
    memory[buffer + i] := Cell(str[i]);
    memory[buffer + i + 1] := 0;
    dec(m);
    inc(i);
  end;
end;

function string_extract(at : Cell) : PChar;
var
  i : Cell = 0;
  starting : Cell;
begin
  starting := at;
  while (memory[starting] <> 0) and (i < 8192) do
  begin
    string_data[i] := Char(memory[starting]);
    inc(i);
    inc(starting);
   end;
  string_data[i] := #0;
  result := string_data;
end;

function d_link(dt : Cell) : Integer;
begin
  result := dt + D_OFFSET_LINK;
end;

function d_xt(dt : Cell) : Integer;
begin
  result := dt + D_OFFSET_XT;
end;

function d_class(dt : Cell) : Integer;
begin
  result := dt + D_OFFSET_CLASS;
end;

function d_name(dt : Cell) : Integer;
begin
  result := dt + D_OFFSET_NAME;
end;

function d_count_entries(Dictionary : Cell) : Cell;
var
  count : Cell = 0;
  i : Cell;
begin
  i := Dictionary;
  while memory[i] <> 0 do
  begin
    inc(count);
    i := memory[i];
  end;
  result := count;
end;

function d_lookup(Dictionary : cell; name : PChar) : Cell;
var
  dt : Cell = 0;
  i : Cell;
  dname : PChar;
begin 
  i := Dictionary;
  while (memory[i] <> 0) and (i <> 0) do
  begin
    dname := string_extract(d_name(i));
    if strcomp(dname, name) = 0 then
    begin
      dt := i;
      i := 0;
    end
    else
      i := memory[i];
  end;
  result := dt;
end;

function d_xt_for(Name : PChar; Dictionary : Cell) : Cell;
begin
  result := memory[d_xt(d_lookup(Dictionary, Name))];
end;

function d_class_for(Name : PChar; Dictionary : Cell) : Cell;
begin
  result := memory[d_class(d_lookup(Dictionary, Name))];
end;

procedure execute(cel : Cell);
var
  opcode, i : Cell;
begin
  ap := 1;
  ip := cel;
  while ip < IMAGE_SIZE - 1 do
  begin
    if ip = notfound then
      writeln(format('%s ?', [string_extract(TIB)]));
    opcode := memory[ip];
    if ngaValidatePackedOpcodes(opcode) <> 0 then
      ngaProcessPackedOpcodes(opcode)
    else if (opcode >= 0) and (opcode < 27) then
      ngaProcessOpcode(opcode)
    else
      case opcode of
        1000:
        begin
          write(Char(data[sp]));
          dec(sp);
        end;
        1001:
        begin
          read(i);
          stack_push(i);
        end;
      else
      begin
        writeln('Invalid instruction!');
        writeln(format('  at %d, opcode %d', [ip, opcode]));
        halt();
      end;
    end;
    inc(ip);
    if ap = 0 then
      ip := IMAGE_SIZE - 1;
  end;
end;

procedure update_rx();
begin
  Dictionary := memory[2];
  Heap := memory[3];
  Compiler := d_xt_for('Compiler', Dictionary);
  notfound := d_xt_for('err:notfound', Dictionary);
end;

procedure evaluate(s: PChar);
var
  interpret : Cell;
begin
  if strlen(s) = 0 then
    exit();
  update_rx();
  interpret := d_xt_for('interpret', Dictionary);
  string_inject(s, TIB);
  stack_push(TIB);
  execute(interpret);
end;

// Read from file, assumed to be open already
procedure read_token(var fil : File; token_buffer : PChar); overload;
var
  count : Integer = 0;
  f : File of Char;
  ch : Char;
begin
  f := fil;
  repeat
  begin
    read(f, ch);
    token_buffer[count] := ch;
    inc(count);
  end;
  until (ch = #13) or (ch = #10) or (ch = ' ') or eof(f);
  token_buffer[count - 1] := #0;
end;

// Read from stdin
procedure read_token(token_buffer : PChar); overload;
var
  count : Integer = 0;
  ch : Char;
begin
  repeat
  begin
    read(ch);
    if (ch = #8) and (count <> 0) then
      dec(count)
    else
    begin
      token_buffer[count] := ch;
      inc(count);
    end;
  end;
  until (ch = #13) or (ch = #10) or (ch = ' ') or eof;
  token_buffer[count - 1] := #0;
end;
end.

// ********************************************************
//  Copyright (c) 2016 Rob Judd <judd@ob-wan.com>
//  Based on C version by Charles Childers et al
//  ISC License - see included file LICENSE
//  Minor adjustments by Charles Childers, 2018
// ********************************************************

program listener;

{$mode objfpc}{$H+}
{$macro on}

uses
  SysUtils, bridge in 'bridge.pas', nga in 'nga.pas';

{$include 'nga.inc'}

const
  TIB       = 1471;

//implementation

procedure include_file(fname : PChar);
var
  source : array[0..65535] of Char;
  fp : File of Char;
  f : THandle;
begin
  f := FileOpen(fname, fmOpenRead);
  if f <> THandle(-1) then
  begin
    writeln('+ load ', fname);
    Assign(fp, fname);
    Reset(fp);
    while not eof(fp) do
    begin
      read_token(fp, source);
      evaluate(source);
    end;
    Close(fp);
  end;
  FileClose(f);
end;

procedure dump_stack();
var
  i : Cell;
begin
  write('Stack: ');
  for i := 1 to sp do
    if i = sp then
      write(format('< %d >', [data[i]]))
    else
      write(format('%d ', [data[i]]));
  writeln();
end;

procedure prompt();
begin
  if memory[Compiler] = 0 then
    write(LineEnding, 'ok  ');
end;


// ********************************************************
//  Main program
// ********************************************************
var
  input : array[0..1023] of Char;
  i, n : Cell;
begin
  ngaPrepare();
  n := ngaLoadImage('ngaImage');
  if n = 0 then
    exit();
  update_rx();
  writeln(format('RETRO 12 (rx-%d.%d)', [memory[4] div 100, memory[4] mod 100]));
  //include_file('retro.forth');
  writeln(format('%d MAX, TIB @ %d, Heap @ %d', [IMAGE_SIZE, TIB, Heap]));
  writeln();
  while true do
  begin
    prompt();
    Dictionary := memory[2];
    read_token(input);
    if strcomp(input, 'bye') = 0 then
    begin
      exit();
    end
    else if strcomp(input, 'words') = 0 then
    begin
      i := Dictionary;
      while i <> 0 do
      begin
        string_extract(d_name(i));
        write(format('%s  ', [string_data]));
        i := memory[i];
      end;
      writeln(format('(%d entries)', [d_count_entries(Dictionary)]));
    end
    else if strcomp(input, '.p') = 0 then
      writeln(format('__%s__', [string_extract(data[sp])]))
    else if strcomp(input, '.s') = 0 then
      dump_stack()
    else
      evaluate(input);
  end;
end.


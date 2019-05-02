// ********************************************************
//  Copyright (c) 2016 Rob Judd <judd@ob-wan.com>
//  Copyright (c) 2018 Charles Childers <crc@forthworks.com>
//  Based on C version by Charles Childers et al
//  ISC License - see included file LICENSE
// ********************************************************

type
  Cell = Longint;
 
const
  STACK_DEPTH = 32;
  ADDRESSES   = 128;
  IMAGE_SIZE  = 524288;
  NUM_OPS     = 30;

{$define TOS          := data[sp]}
{$define NOS          := data[sp-1]}
{$define TOA          := address[ap]}


#!/usr/bin/env node

/* RETRO ------------------------------------------------------
   A Node.js implementation of the standard Nga assembler.
   It reads Muri code blocks and writes an image named ngaImage.
   ---------------------------------------------------------- */

"use strict";

const fs = require("fs");

const IMAGE_SIZE = 128 * 1024;
const INSTRUCTIONS = [
  "..", "li", "du", "dr", "sw", "pu", "po", "ju", "ca", "cc",
  "re", "eq", "ne", "lt", "gt", "fe", "st", "ad", "su", "mu",
  "di", "an", "or", "xo", "sh", "zr", "ha", "ie", "iq", "ii",
];
const IP_MODIFYING_INSTRUCTIONS = new Set(["ju", "ca", "cc", "re", "zr"]);

const labels = new Map();
const image = new Int32Array(IMAGE_SIZE);
let here = 0;
let lastDictionaryEntry = 0;
let dictionaryEntries = 0;

function stripComments(line) {
  if (line[0] === "s") {
    return line;
  }

  let hashPosition = line.indexOf("#");
  while (hashPosition !== -1) {
    if (hashPosition === 0 || line[hashPosition - 1] === " " || line[hashPosition - 1] === "\t") {
      return line.slice(0, hashPosition).replace(/[ \t]+$/, "");
    }
    hashPosition = line.indexOf("#", hashPosition + 1);
  }
  return line.replace(/[ \t]+$/, "");
}

function validateInstructionBundle(line, lineNumber) {
  if (!line.startsWith("i ")) {
    return;
  }

  const bundle = line.slice(2);
  if (bundle.length !== 8) {
    throw new Error(`Error on line ${lineNumber}: instruction bundle must contain exactly four two-character instructions`);
  }

  const instructions = [];
  for (let index = 0; index < 8; index += 2) {
    const instruction = bundle.slice(index, index + 2);
    if (!INSTRUCTIONS.includes(instruction)) {
      throw new Error(`Error on line ${lineNumber}: invalid Nga instruction '${instruction}' in bundle '${bundle}'`);
    }
    instructions.push(instruction);
  }

  for (let index = 0; index < 3; index++) {
    if (IP_MODIFYING_INSTRUCTIONS.has(instructions[index])) {
      for (let following = index + 1; following < 4; following++) {
        if (instructions[following] !== "..") {
          throw new Error(`Error on line ${lineNumber}: Nga instruction '${instructions[index]}' must be followed only by NOPs (..) in its bundle`);
        }
      }
    }
  }
}

function processSource(filename, handler) {
  const lines = fs.readFileSync(filename, "utf8").split("\n");
  let inBlock = false;

  lines.forEach((rawLine, index) => {
    const lineNumber = index + 1;
    if (rawLine === "~~~") {
      inBlock = !inBlock;
    } else if (inBlock) {
      const line = stripComments(rawLine);
      validateInstructionBundle(line, lineNumber);
      if (line.length > 0 && line[0] !== "c") {
        handler(line);
      }
    }
  });
}

function parseAtoi(value) {
  const match = value.match(/^[ \t]*([+-]?\d+)/);
  return match === null ? 0 : Number.parseInt(match[1], 10);
}

function parseDictionaryLine(line) {
  const fields = line.slice(2).split(/[ \t]+/).filter((field) => field.length > 0);
  return [fields[0] || "", fields[1] || "", fields[2] || ""];
}

function dictionaryEntrySize(name) {
  return 9 + Buffer.byteLength(name, "utf8") + 1;
}

function lookup(name) {
  return labels.has(name) ? labels.get(name) : -1;
}

function assemble(bundle) {
  const a = INSTRUCTIONS.indexOf(bundle.slice(0, 2));
  const b = INSTRUCTIONS.indexOf(bundle.slice(2, 4));
  const c = INSTRUCTIONS.indexOf(bundle.slice(4, 6));
  const d = INSTRUCTIONS.indexOf(bundle.slice(6, 8));
  return (a | (b << 8) | (c << 16) | (d << 24)) >>> 0;
}

function pass1(filename) {
  here = 0;
  processSource(filename, (line) => {
    const directive = line[0];
    if ("ir-d".includes(directive)) {
      here++;
    } else if (directive === "s") {
      here += Buffer.byteLength(line.slice(2), "utf8") + 1;
    } else if (directive === "o") {
      here = parseAtoi(line.slice(2));
    } else if (directive === "*") {
      here += parseAtoi(line.slice(2));
    } else if (directive === "D") {
      const [name] = parseDictionaryLine(line);
      here += dictionaryEntrySize(name);
    } else if (directive === ":") {
      const name = line.slice(2);
      if (labels.has(name)) {
        throw new Error(`Fatal error: ${name} already defined`);
      }
      labels.set(name, here);
    }
  });
}

function pass2(filename) {
  here = 0;
  processSource(filename, (line) => {
    const directive = line[0];
    if (directive === "i") {
      image[here++] = assemble(line.slice(2));
    } else if (directive === "o") {
      here = parseAtoi(line.slice(2));
    } else if (directive === "*") {
      here += parseAtoi(line.slice(2));
    } else if (directive === "D") {
      const [name] = parseDictionaryLine(line);
      here += dictionaryEntrySize(name);
    } else if ("-rd".includes(directive)) {
      here++;
    } else if (directive === "s") {
      here += Buffer.byteLength(line.slice(2), "utf8") + 1;
    }
  });
}

function pass3(filename) {
  here = 0;
  processSource(filename, (line) => {
    const directive = line[0];
    if ("ir-".includes(directive)) {
      here++;
    } else if (directive === "o") {
      here = parseAtoi(line.slice(2));
    } else if (directive === "*") {
      here += parseAtoi(line.slice(2));
    } else if (directive === "d") {
      image[here++] = parseAtoi(line.slice(2));
    } else if (directive === "D") {
      const [name] = parseDictionaryLine(line);
      here += dictionaryEntrySize(name);
    } else if (directive === "s") {
      here += Buffer.byteLength(line.slice(2), "utf8") + 1;
    }
  });
}

function pass4(filename) {
  here = 0;
  processSource(filename, (line) => {
    const directive = line[0];
    if ("ir-d".includes(directive)) {
      here++;
    } else if (directive === "o") {
      here = parseAtoi(line.slice(2));
    } else if (directive === "*") {
      here += parseAtoi(line.slice(2));
    } else if (directive === "s") {
      for (const byte of Buffer.from(line.slice(2), "utf8")) {
        image[here++] = byte;
      }
      image[here++] = 0;
    } else if (directive === "D") {
      const start = here;
      const [name] = parseDictionaryLine(line);
      const nameBytes = Buffer.from(name, "utf8");
      nameBytes.forEach((byte, index) => { image[start + 9 + index] = byte; });
      image[start + 9 + nameBytes.length] = 0;
      here += dictionaryEntrySize(name);
    }
  });
}

function pass5(filename) {
  here = 0;
  lastDictionaryEntry = 0;
  dictionaryEntries = 0;
  processSource(filename, (line) => {
    const directive = line[0];
    if (directive === "i") {
      here++;
    } else if (directive === "o") {
      here = parseAtoi(line.slice(2));
    } else if (directive === "*") {
      here += parseAtoi(line.slice(2));
    } else if ("r-".includes(directive)) {
      const name = line.slice(2);
      image[here] = lookup(name);
      if (image[here] === -1) {
        console.log(`Lookup failed: '${name}'`);
      }
      here++;
    } else if (directive === "d") {
      here++;
    } else if (directive === "s") {
      here += Buffer.byteLength(line.slice(2), "utf8") + 1;
    } else if (directive === "D") {
      const start = here;
      const [name, label, className] = parseDictionaryLine(line);
      const xt = lookup(label);
      const classHandler = lookup(className);
      if (xt === -1) {
        console.log(`Lookup failed: '${label}'`);
      }
      if (classHandler === -1) {
        console.log(`Lookup failed: '${className}'`);
      }
      image.set([
        dictionaryEntries === 0 ? 0 : lastDictionaryEntry,
        xt, classHandler, 0, 0, 0, 0, 0, 0,
      ], start);
      lastDictionaryEntry = start;
      dictionaryEntries++;
      here += dictionaryEntrySize(name);
    }
  });
}

function save(filename) {
  const output = Buffer.alloc(here * 4);
  for (let index = 0; index < here; index++) {
    output.writeInt32LE(image[index], index * 4);
  }
  fs.writeFileSync(filename, output);
}

function main() {
  if (process.argv.length < 3) {
    console.log("No file specified.");
    process.exitCode = 1;
    return;
  }
  const filename = process.argv[2];
  pass1(filename);
  pass2(filename);
  pass3(filename);
  pass4(filename);
  pass5(filename);
  save("ngaImage");
  console.log(`Wrote ${here} cells to ngaImage`);
}

try {
  main();
} catch (error) {
  console.log(error.message);
  process.exitCode = 1;
}

# RETRO 2019.12

This is the changelog for the development builds of Retro.

## Bug Fixes

- (all) strl* functions now only compiled if using GLIBC.
- (rre) `clock:year` corrected
- (rre) `clock:month` corrected
- (all) `d:add-header` is extended by retro.forth to remap spaces back to underscores
- (doc) fixed stack comments in glossary

## Build

- Merged Linux & BSD Makefiles

## Core Language

- rename `a:nth` to `a:th`
- rename `v:update-using` to `v:update`

## Documentation

- updated Linux build instructions
- updated Starting instructions

## Examples

- add sqlite3 wrapper
- add mandelbrot.forth
- add shell.forth

## General

- reorganized directory tree

## I/O

- added `clock:utc:` namespace

## Interfaces

- retro-compiler: runtime now supports scripting arguments

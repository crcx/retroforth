# RETRO 2019.12

This is the changelog for the development builds of Retro.
The version number is likely to change; I'm targetting a
July - September window for this release.

## Bug Fixes

- (all) strl* functions now only compiled if using GLIBC.
- (all) `d:add-header` is extended by retro.forth to remap spaces back to underscores
- (doc) fixed stack comments in glossary
- (ex ) fixed issue in mail.forth
- (rre) `clock:year` corrected
- (rre) `clock:month` corrected

## Build

- Merged Linux & BSD Makefiles

## Core Language

- rename: `a:nth` to `a:th`
- rename: `v:update-using` to `v:update`

## Documentation

- merged BSD, Linux, macOS build instructions
- updated Starting instructions
- added implementation notes on arrays

## Examples

- add bury.forth
- add compat.forth
- add gopher.forth
- add mandelbrot.forth
- add shell.forth
- add sqlite3 wrapper
- corrected an issue in mail.forth
- cleanup publish-examples.forth
- publish-examples.forth now uses `retro-document` to generate glossaries

## General

- reorganized directory tree

## I/O

- (rre) added `clock:utc:` namespace
- (rre) remove gopher downloader

## Interfaces

- retro-compiler: runtime now supports scripting arguments
- retro-unix: replaces earlier rre.c
- retro-windows: rre, adapted for windows

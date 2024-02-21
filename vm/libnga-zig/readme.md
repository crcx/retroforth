nga VM for retroforth, in Zig.

It's faster than the official C VM somehow.

## Building

This project is developed with Zig 0.11.0.

```
# Build and run
zig build run

# Install to ~/.local/bin/retro-zig
zig build -Doptimize=ReleaseFast -p ~/.local
```

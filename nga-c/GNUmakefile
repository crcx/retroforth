CFLAGS ?= -std=c89 -O2 -Werror -Wall -Wextra
EFLAGS ?= -lm -DSTRL
ENABLED ?=
ENABLED += -DENABLE_FLOATS
ENABLED += -DENABLE_SOCKETS
ENABLED += -DENABLE_FILES
ENABLED += -DENABLE_UNIX
ENABLED += -DENABLE_RNG
ENABLED += -DENABLE_CLOCK
ENABLED += -DENABLE_SCRIPTING

default: update vm kernel extend finish

update:
	cp ../image/* .

vm: nga.c
	$(CC) $(CFLAGS) $(EFLAGS) $(ENABLED) nga.c -o retro

kernel: retro.forth retro.muri assemble.retro
	./retro -u ngaImage -f retro.forth -f assemble.retro

extend: retro.forth devices.retro extend.retro
	./retro -u retro.nga -f retro.forth -f devices.retro -f extend.retro

finish:
	mv retro.nga ngaImage

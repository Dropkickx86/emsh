include config.mk

SRC = emsh.c helpers.c emgc.c environment.c
OBJ = emsh.o helpers.o emgc.o environment.o

GENERAL_SRC = input_read.c platform_init.c program_exec.c
GENERAL_OBJ = general/input_read.o general/platform_init.o general/program_exec.o

INTRINSIC_SRC = dir_change.c
INTRINSIC_OBJ = intrinsic/dir_change.o

all: emsh

emsh: $(OBJ) general intrinsic
	$(CC) $(OBJ) $(GENERAL_OBJ) $(INTRINSIC_OBJ) -o $@ $(LDFLAGS)

debug: clean general_debug intrinsic_debug
	$(CC) $(SRC) $(GENERAL_OBJ) $(INTRINSIC_OBJ) -o emsh.debug $(CFLAGS) $(CFLAGS_DEBUG) $(LDFLAGS)

general:
	cd general && $(CC) $(CFLAGS) -c $(GENERAL_SRC)

intrinsic:
	cd intrinsic && $(CC) $(CFLAGS) -c $(INTRINSIC_SRC)

general_debug:
	cd general && $(CC) $(CFLAGS) $(CFLAGS_DEBUG) -c $(GENERAL_SRC)

intrinsic_debug:
	cd intrinsic && $(CC) $(CFLAGS) $(CFLAGS_DEBUG) -c $(INTRINSIC_SRC)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(GENERAL_OBJ) $(INTRINSIC_OBJ) emsh.debug

rmbin:
	rm -f emsh

rebuild: clean rmbin emsh

.PHONY: all clean rebuild debug rmbin general intrinsic general_debug intrinsic_debug

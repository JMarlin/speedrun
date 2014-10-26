all: speedrun
	
speedrun:
	$(MAKE) cpu.o
	$(MAKE) gpu.o
	$(MAKE) interface.o
	$(MAKE) main.o
	$(MAKE) ram.o
	$(MAKE) rom.o
	gcc -o speedrun ram/ram.o rom/rom.o cpu/cpu.o gpu/gpu.o interface/interface.o main.o -lSDL
	
cpu.o:
	gcc -c -o cpu/cpu.o cpu/cpu.c
	
gpu.o:	
	gcc -c -o gpu/gpu.o gpu/gpu.c

interface.o:
	gcc -c -o interface/interface.o interface/interface.c

rom.o:
	gcc -c -o rom/rom.o rom/rom.c

ram.o:
	gcc -c -o ram/ram.o ram/ram.c
	
main.o:
	gcc -c -o main.o main.c
	

CC=gcc
ASMBIN=nasm

CFILE=erosion
ASMFILE=MiddleErosion
ASMFILE2=TwoWordsErosion
ASMFILE3=LeftEdgeErosion

all : asm cc link clean
asm : 
	$(ASMBIN) -o $(ASMFILE).o -f elf -l $(ASMFILE).lst $(ASMFILE).asm
	$(ASMBIN) -o $(ASMFILE2).o -f elf -l $(ASMFILE2).lst $(ASMFILE2).asm
	$(ASMBIN) -o $(ASMFILE3).o -f elf -l $(ASMFILE3).lst $(ASMFILE3).asm
cc :
	$(CC) -m32 -c -g -O0 -fpack-struct $(CFILE).c
link :
	$(CC) -m32 -o $(CFILE) $(CFILE).o $(ASMFILE).o $(ASMFILE2).o $(ASMFILE3).o
clean :
	rm *.o
	rm $(ASMFILE).lst
	rm $(ASMFILE2).lst
	rm $(ASMFILE3).lst

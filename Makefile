CPP = clang++

all : cs492_pa.exe

cs492_pa.exe : cs492_pa.o aes.o
	$(CPP) -o cs492_pa.exe cs492_pa.o aes.o

cs492_pa.o : cs492_pa.cpp cs492_pa.h aes.h
	$(CPP) -c cs492_pa.cpp

aes.o : aes.cpp aes.h
	$(CPP) -c aes.cpp

clean :
	-del *.o *.exe

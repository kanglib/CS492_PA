set KEY=AngGimuttiHahAHA

nmake
cs492_pa.exe c test.zip message.txt %KEY%
cs492_pa.exe x test.zip.out message.txt.out %KEY%
fc message.txt message.txt.out

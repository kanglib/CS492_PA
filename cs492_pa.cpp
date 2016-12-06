#define _CRT_SECURE_NO_WARNINGS
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "cs492_pa.h"
#include "aes.h"

int encrypt(char** args);
int decrypt(char** args);
void read_dest(char* dest);
void patch_dest();
char* encrypt_cbc(char* src, char* key);
int decrypt_cbc(char* src, char* key, char* payload);

char* Buffer;
size_t Size;
LocalFileHeader** Lfh;
CentralFileHeader** Cfh;
EndOfCentralDirectoryRecord* Eocd;

uint8_t Iv[16] = {
    0x46, 0xb8, 0xf3, 0x06, 0x00, 0x55, 0x18, 0x31,
    0xb1, 0x4f, 0xbd, 0x31, 0x9f, 0x05, 0x5a, 0xa5
};

int main(int argc, char** argv)
{
    puts("OVERVIEW: CS492(A) Project - 20150031 Kang Hyeonwoo");
    if (argc != 5) {
        puts("\nUSAGE: cs492_pa.exe [cx] zipfile message key\n");
        return 1;
    }

    if (!strcmp(argv[1], "c")) {
        return encrypt(argv + 2);
    } else if (!strcmp(argv[1], "x")) {
        return decrypt(argv + 2);
    } else {
        puts("\nUSAGE: cs492_pa.exe [cx] zipfile message key\n");
        return 1;
    }
}

int encrypt(char** args)
{
    system("rd /s /q %TEMP%\\cs492_pa");
    system("del %TEMP%\\cs492_pa.zip");
    char command[BUFSIZ] = "7z x -o%TEMP%\\cs492_pa ";
    strcat(command, args[0]);
    system(command);

    char* payload;
    if (!(payload = encrypt_cbc(args[1], args[2]))) {
        puts("\nkey is not 128-bit (16-char)");
        return 1;
    }

    char filename[BUFSIZ];
    strcpy(filename, getenv("TEMP"));
    strcat(filename, "\\cs492_pa\\~~~dropship~~~");
    FILE* dropship = fopen(filename, "wb");
    fwrite(payload + 4, 1, *(uint32_t*) payload, dropship);
    fclose(dropship);

    system("7z a %TEMP%\\cs492_pa.zip %TEMP%\\.\\cs492_pa\\*");
    strcpy(filename, getenv("TEMP"));
    strcat(filename, "\\cs492_pa.zip");

    read_dest(filename);
    patch_dest();

    strcpy(filename, args[0]);
    strcat(filename, ".out");
    FILE* dest = fopen(filename, "wb");
    fwrite(Buffer, 1, Size, dest);
    fclose(dest);

    delete []payload;
    delete []Lfh;
    delete []Cfh;
    delete Buffer;
    return 0;
}

int decrypt(char** args)
{
    read_dest(args[0]);
    for (int i = 0; i < Eocd->EntriesTotal; i++) {
        if (!Cfh[i]->FileName[0]) {
            memcpy(Cfh[i]->FileName, "~~~dropship~~~", 14);
            memcpy(Lfh[i]->FileName, "~~~dropship~~~", 14);
            break;
        }
    }

    system("rd /s /q %TEMP%\\cs492_pa");
    system("del %TEMP%\\cs492_pa.zip");
    char filename[BUFSIZ];
    strcpy(filename, getenv("TEMP"));
    strcat(filename, "\\cs492_pa.zip");
    FILE* dest = fopen(filename, "wb");
    fwrite(Buffer, 1, Size, dest);
    fclose(dest);

    system("rd /s /q %TEMP%\\cs492_pa");
    char command[BUFSIZ] = "7z x -o%TEMP%\\cs492_pa %TEMP%\\cs492_pa.zip";
    system(command);

    strcpy(filename, getenv("TEMP"));
    strcat(filename, "\\cs492_pa\\~~~dropship~~~");
    FILE* file = fopen(filename, "rb");
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    char* payload = new char[BUFSIZ];
    memset(payload, 0, BUFSIZ);
    rewind(file);
    fread(payload + 4, 1, size, file);
    fclose(file);

    *(uint32_t*) payload = size;
    if (decrypt_cbc(args[1], args[2], payload)) {
        puts("\nkey is not 128-bit (16-char)");

        delete []payload;
        delete []Lfh;
        delete []Cfh;
        delete Buffer;
        return 1;
    }

    delete []payload;
    delete []Lfh;
    delete []Cfh;
    delete Buffer;
    return 0;
}

void read_dest(char* dest)
{
    FILE* file = fopen(dest, "rb");
    fseek(file, 0, SEEK_END);
    Size = ftell(file);
    Buffer = new char[Size];
    rewind(file);
    fread(Buffer, 1, Size, file);
    fclose(file);

    Eocd = (EndOfCentralDirectoryRecord*) &Buffer[Size - 22];
    while (Eocd->Signature != 0x06054b50)
        Eocd = (EndOfCentralDirectoryRecord*) ((char*) Eocd - 1);

    Cfh = new CentralFileHeader*[Eocd->EntriesTotal];
    Lfh = new LocalFileHeader*[Eocd->EntriesTotal];
    char* offset = &Buffer[Eocd->Offset];
    for (int i = 0; i < Eocd->EntriesTotal; i++) {
        Cfh[i] = (CentralFileHeader*) offset;
        Lfh[i] = (LocalFileHeader*) &Buffer[Cfh[i]->LocalHeaderOffset];
        offset += 46;
        offset += Cfh[i]->FilenameLength;
        offset += Cfh[i]->ExtraFieldLength;
        offset += Cfh[i]->CommentLength;
    }
}

void patch_dest()
{
    for (int i = 0; i < Eocd->EntriesTotal; i++) {
        char filename[BUFSIZ];
        memcpy(filename, Cfh[i]->FileName, Cfh[i]->FilenameLength);
        filename[Cfh[i]->FilenameLength] = '\0';
        if (!strcmp(filename, "~~~dropship~~~")) {
            for (int j = 0; j < 14; j++)
                Cfh[i]->FileName[j] = Lfh[i]->FileName[j] = '\0';
            break;
        }
    }
}

char* encrypt_cbc(char* src, char* key)
{
    if (strlen(key) != 16)
        return NULL;

    FILE* file = fopen(src, "rb");
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    uint8_t* input = new uint8_t[size];
    rewind(file);
    fread(input, 1, size, file);
    fclose(file);

    uint8_t* buffer = new uint8_t[BUFSIZ];
    memset(buffer, 0, BUFSIZ);
    *(uint32_t*) buffer = size;
    AES128_CBC_encrypt_buffer(buffer + 4, input, size, (uint8_t*) key, Iv);
    delete []input;
    return (char*) buffer;
}

int decrypt_cbc(char* src, char* key, char* payload)
{
    if (strlen(key) != 16)
        return -1;

    int size = *(uint32_t*) payload;
    uint8_t output[BUFSIZ];
    for (int i = 0; i < size; i += 16) {
        AES128_CBC_decrypt_buffer(
                output + i,
                (uint8_t*) payload + i + 4,
                16,
                i ? NULL : (uint8_t*) key,
                i ? NULL : Iv);
    }

    FILE* file = fopen(src, "wb");
    fwrite(output, 1, size, file);
    fclose(file);
    return 0;
}

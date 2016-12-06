#pragma once

#include <cstdint>

#pragma pack(push, 1)

struct LocalFileHeader {
    uint32_t Signature;
    uint16_t ExtractVersion;
    uint16_t GeneralPurposeFlagBits;
    uint16_t CompressionMethod;
    uint16_t LastModTime;
    uint16_t LastModDate;
    uint32_t Crc;
    uint32_t CompressedSize;
    uint32_t UncompressedSize;
    uint16_t FilenameLength;
    uint16_t ExtraFieldLength;
    char     FileName[1];
};

struct CentralFileHeader {
    uint32_t Signature;
    uint16_t CreateVersion;
    uint16_t ExtractVersion;
    uint16_t FlagBits;
    uint16_t CompressType;
    uint16_t Time;
    uint16_t Date;
    uint32_t Crc;
    uint32_t CompressedSize;
    uint32_t UncompressedSize;
    uint16_t FilenameLength;
    uint16_t ExtraFieldLength;
    uint16_t CommentLength;
    uint16_t DiskNumberStart;
    uint16_t InternalFileAttributes;
    uint32_t ExternalFileAttributes;
    uint32_t LocalHeaderOffset;
    char     FileName[1];
};

struct EndOfCentralDirectoryRecord {
    uint32_t Signature;
    uint16_t DiskNumber;
    uint16_t DiskStart;
    uint16_t EntriesThisDisk;
    uint16_t EntriesTotal;
    uint32_t Size;
    uint32_t Offset;
    uint16_t CommentSize;
};

#pragma pack(pop)

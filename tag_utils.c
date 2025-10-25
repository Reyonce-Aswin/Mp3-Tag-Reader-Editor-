#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "tag.h"

// Reads a 4-byte big-endian integer (used for ID3v2.3 frame sizes)
int read_big_endian_int(unsigned char *bytes)
{
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

// Writes a 4-byte big-endian integer (used for ID3v2.3 frame sizes)
void write_big_endian_int(int value, unsigned char *bytes)
{
    bytes[0] = (value >> 24) & 0xFF;
    bytes[1] = (value >> 16) & 0xFF;
    bytes[2] = (value >> 8) & 0xFF;
    bytes[3] = value & 0xFF;
}

// Reads a 4-byte synchsafe integer (used for ID3v2.4 frame/header sizes)
int read_synchsafe_int(unsigned char *bytes)
{
    return (bytes[0] << 21) | (bytes[1] << 14) | (bytes[2] << 7) | bytes[3];
}

// Writes a 4-byte synchsafe integer (used for ID3v2.4 frame/header sizes)
void write_synchsafe_int(int value, unsigned char *bytes)
{
    bytes[0] = (value >> 21) & 0x7F;
    bytes[1] = (value >> 14) & 0x7F;
    bytes[2] = (value >> 7) & 0x7F;
    bytes[3] = value & 0x7F;
}

// Helper function to get the ID3 version from the file header
int get_mp3_version(FILE *fp)
{
    unsigned char header[10];
    long original_pos = ftell(fp);
    rewind(fp);

    if (fread(header, 1, 10, fp) != 10)
    {
        fseek(fp, original_pos, SEEK_SET);
        return -1;
    }
    fseek(fp, original_pos, SEEK_SET);

    if (strncmp((char *)header, "ID3", 3) != 0)
    {
        return -1;
    }
    
    int version = header[3];
    // Check for v2.2 (2), v2.3 (3), or v2.4 (4)
    if (version >= 2 && version <= 4)
    {
        return version;
    }
    return -1;
}

// Utility to check for .mp3 extension
int mp3_extn(const char *filename)
{
    size_t len = strlen(filename);
    if (len < 4) return 0;
    
    const char *ext = filename + len - 4;
    return (strcmp(ext, ".mp3") == 0 || strcmp(ext, ".MP3") == 0);
}

// Utility to validate year format
int valid_year(const char *str)
{
    if (strlen(str) != 4) return 0;
    for (int i = 0; i < 4; i++)
    {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}
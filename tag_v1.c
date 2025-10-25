#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tag.h"

// Helper to remove null/space padding from fixed-length ID3v1 strings
static void cleanup_v1_string(char *dest, const unsigned char *src, int len)
{
    int i = len - 1;
    // Find the last non-space, non-null character
    while (i >= 0 && (src[i] == '\0' || src[i] == ' '))
    {
        i--;
    }
    int copy_len = i + 1;
    if (copy_len > 0)
    {
        strncpy(dest, (const char *)src, copy_len);
        dest[copy_len] = '\0';
    }
    else
    {
        dest[0] = '\0';
    }
}

// Reads an ID3v1/v1.1 tag from the end of the file
Status read_mp3_tag_v1(const char *filename, ID3v2Tag *tag)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        perror("Error opening file for v1 read");
        return FAILURE;
    }

    // ID3v1 tag is 128 bytes at the end of the file
    if (fseek(fp, -128, SEEK_END) != 0)
    {
        fclose(fp);
        return FAILURE;
    }
    
    unsigned char buffer[128];

    if (fread(buffer, 1, 128, fp) != 128)
    {
        fclose(fp);
        return FAILURE;
    }

    fclose(fp);

    // Check for "TAG" identifier
    if (strncmp((char *)buffer, "TAG", 3) != 0)
    {
        return FAILURE;
    }
    
    // Fill the tag structure fields
    
    cleanup_v1_string(tag->title, &buffer[3], 30);
    cleanup_v1_string(tag->artist, &buffer[33], 30);
    cleanup_v1_string(tag->album, &buffer[63], 30);
    cleanup_v1_string(tag->year, &buffer[93], 4);
    
    // ID3v1.1 check: If the 30th comment byte (pos 125) is 0
    if (buffer[125] == 0 && buffer[126] != 0) 
    {
        // ID3v1.1 (Track at pos 126)
        cleanup_v1_string(tag->comment, &buffer[97], 28);
        snprintf(tag->track, sizeof(tag->track), "%d", buffer[126]);
    }
    else
    {
        // ID3v1.0 (Comment is 30 bytes, no reliable track)
        cleanup_v1_string(tag->comment, &buffer[97], 30);
        tag->track[0] = '\0';
    }

    // Genre (1 byte, pos 127) - Storing the index
    snprintf(tag->content_type, sizeof(tag->content_type), "%d", buffer[127]); 

    return SUCCESS;
}
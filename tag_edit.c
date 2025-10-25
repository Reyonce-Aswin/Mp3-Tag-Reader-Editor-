#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include "tag.h"
#include "colour.h"

Status edit_mp3_tag(const char *filename, const char *frame_id_in, const char *new_value)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        perror(RED "Error opening original file" RESET);
        return FAILURE;
    }

    int version = get_mp3_version(fp);
    if (version == -1)
    {
        printf(RED "Error: Could not determine valid ID3v2 version for editing (v2.3/v2.4 required).\n" RESET);
        return FAILURE;
    }

    // Determine Search IDs and Prepare Payload
    const char *search_id_1 = frame_id_in; 
    const char *search_id_2 = NULL;     
    int is_comment = 0;

    if (strcmp(frame_id_in, "TYER") == 0) 
    {
        // Search for TDRC as well if TYER is requested
        search_id_1 = "TYER";
        search_id_2 = "TDRC"; 
    }
    else if (strcmp(frame_id_in, "COMM") == 0)
    {
        is_comment = 1;
    }
    
    char frame_content[256 + 5] = {0}; 
    int new_len_text = strlen(new_value);
    int new_frame_payload_size = 0;

    // Build the new frame payload
    if (is_comment)
    {
        // [Encoding (1)] + [Language (3, 'eng')] + [Description (1, \0)] + [Text]
        unsigned char comm_prefix[5] = {0x00, 'e', 'n', 'g', 0x00}; 
        int prefix_len = 5;

        memcpy(frame_content, comm_prefix, prefix_len);
        memcpy(frame_content + prefix_len, new_value, new_len_text);
        
        new_frame_payload_size = prefix_len + new_len_text;
    }
    else // All T*** frames (TIT2, TPE1, TYER, TDRC, TRCK, etc.)
    {
        // [Encoding (1)] + [Text]
        frame_content[0] = 0x00; // Encoding 0x00 (ISO-8859-1)
        memcpy(frame_content + 1, new_value, new_len_text);
        new_frame_payload_size = 1 + new_len_text;
    }
    
    // Load entire file into memory
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    unsigned char *buffer = (unsigned char *)malloc(file_size);
    if (!buffer) { fclose(fp); return FAILURE; }
    fread(buffer, 1, file_size, fp);
    fclose(fp);

    int tag_size = read_synchsafe_int(&buffer[6]);
    long pos = 10; 

    // Loop through frames
    while (pos + 10 < file_size && pos < tag_size + 10)
    {
        char id[5] = {0};
        memcpy(id, &buffer[pos], 4);

        int frame_size = 0;
        if (version == 3)
        {
            frame_size = read_big_endian_int(&buffer[pos + 4]);
        }
        else // version == 4
        {
            frame_size = read_synchsafe_int(&buffer[pos + 4]);
        }

        if (frame_size <= 0 || frame_size > 1024 * 1024)
            break;

        // --- Frame Match Check (Fixed for TYER/TDRC) ---
        int is_match = 0;
        if (strcmp(id, search_id_1) == 0) { is_match = 1; }
        else if (search_id_2 && strcmp(id, search_id_2) == 0) { is_match = 1; }
        
        if (is_match)
        {
            // Found the frame to edit. 'id' holds the ID we are replacing (e.g., TDRC).
            
            int diff = new_frame_payload_size - frame_size;
            int new_tag_size = tag_size + diff;

            long new_file_size = file_size + diff;
            unsigned char *new_buf = (unsigned char *)malloc(new_file_size);
            if (!new_buf) { 
                free(buffer); 
                return FAILURE; 
            }
            // 1. Copy everything up to the frame's ID/Header
            memcpy(new_buf, buffer, pos); 
            
            // 2. Copy the Frame Header (ID + new Size + Flags)
            memcpy(new_buf + pos, &buffer[pos], 10); // Copy 10 bytes: ID, Size, Flags

            // 3. Copy the NEW frame content (including prefix for COMM, encoding for T***)
            memcpy(new_buf + pos + 10, frame_content, new_frame_payload_size);

            // 4. Copy the rest of the file
            memcpy(&new_buf[pos + 10 + new_frame_payload_size], 
                   &buffer[pos + 10 + frame_size], 
                   file_size - (pos + 10 + frame_size));

            // 5. Update frame size in the new buffer
            if (version == 3)
            {
                write_big_endian_int(new_frame_payload_size, &new_buf[pos + 4]);
            }
            else // version == 4
            {
                write_synchsafe_int(new_frame_payload_size, &new_buf[pos + 4]);
            }

            // 6. Update ID3 header size (always synchsafe)
            new_tag_size = new_tag_size & 0x0FFFFFFF;
            write_synchsafe_int(new_tag_size, &new_buf[6]);

            // Write to temp file, rename, and clean up
            char temp_file[256];
            snprintf(temp_file, sizeof(temp_file), "%s_temp.mp3", filename);
            FILE *out = fopen(temp_file, "wb");
            if (!out) { 
                perror(RED "Error writing temp file" RESET); free(buffer); free(new_buf); 
                return FAILURE; 
            }
            fwrite(new_buf, 1, new_file_size, out);
            fclose(out);

            free(buffer);
            free(new_buf);

            if (remove(filename) != 0) { 
                perror(RED "Error removing original file" RESET); 
                remove(temp_file); return FAILURE; 
            }
            if (rename(temp_file, filename) != 0) { 
                perror(RED "Error renaming temp file" RESET); 
                return FAILURE; 
            }
            return SUCCESS;
        }
        pos += 10 + frame_size;
    }

    free(buffer);
    printf(YELLOW "Error: Frame ID '%s' not found or tag version unsupported for editing.\n" RESET, frame_id_in);
    return FAILURE;
}
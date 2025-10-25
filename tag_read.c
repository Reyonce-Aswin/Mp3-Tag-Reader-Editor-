#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tag.h"
#include "colour.h"

// The main function to try reading ID3v2 first, then ID3v1
Status read_mp3_tag(const char *filename, ID3v2Tag *tag)
{
    // Initialize fields
    memset(tag, 0, sizeof(ID3v2Tag));

    // Try to read ID3v2 tag (at the start)
    if (read_mp3_tag_v2(filename, tag) == SUCCESS)
    {
        return SUCCESS;
    }

    // If ID3v2 tag not found, try to read ID3v1 tag (at the end)
    if (read_mp3_tag_v1(filename, tag) == SUCCESS)
    {
        tag->major_version = 1; // Mark as v1 tag
        return SUCCESS;
    }

    printf("Error: ID3 tag not found.\n");
    return FAILURE;
}


// ID3v2 Reader (Function Definition)
Status read_mp3_tag_v2(const char *filename, ID3v2Tag *tag)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) { perror("Error opening file"); return FAILURE; }

    unsigned char header[10];
    if (fread(header, 1, 10, fp) != 10) { fclose(fp); return FAILURE; }

    if (strncmp((char *)header, "ID3", 3) != 0) { fclose(fp); return FAILURE; }

    tag->major_version = header[3];

    if (tag->major_version < 2 || tag->major_version > 4)
    {
        printf("Error: Unsupported ID3v2 version: v2.%d\n", tag->major_version);
        fclose(fp);
        return FAILURE;
    }

    // You will need a forward declaration for read_synchsafe_int or ensure tag_utils.h is included
    // Assuming tag_utils is included via tag.h, we proceed:
    int tag_size = read_synchsafe_int(&header[6]);
    long end_pos = 10 + tag_size;
    long pos = 10; 

    while (pos < end_pos)
    {
        unsigned char frame_header[10];
        if (fread(frame_header, 1, 10, fp) != 10) 
            break;

        char frame_id[5] = {0};
        strncpy(frame_id, (char *)frame_header, 4);

        int frame_size = 0;
        if (frame_id[0] == 0) 
            break; // Padding
        
        if (tag->major_version == 3)
            frame_size = read_big_endian_int(&frame_header[4]);
        else // version 4
            frame_size = read_synchsafe_int(&frame_header[4]);

        if (frame_size < 1) 
            break;

        pos += 10; // Header length
        
        // --- TEXT FRAMES (T***) ---
        if (frame_id[0] == 'T' && strcmp(frame_id, "TXX") != 0)
        {
            fgetc(fp); // Skip encoding byte
            char buffer[1024] = {0};
            int data_len = frame_size - 1;
            
            if (data_len > 0)
            {
                if (fread(buffer, 1, data_len, fp) != data_len) break;
            }

            if (strcmp(frame_id, "TIT2") == 0) strncpy(tag->title, buffer, sizeof(tag->title) - 1);
            else if (strcmp(frame_id, "TPE1") == 0) strncpy(tag->artist, buffer, sizeof(tag->artist) - 1);
            else if (strcmp(frame_id, "TALB") == 0) strncpy(tag->album, buffer, sizeof(tag->album) - 1);
            else if (strcmp(frame_id, "TYER") == 0) strncpy(tag->year, buffer, 4); // v2.3 Year
            else if (strcmp(frame_id, "TDRC") == 0) strncpy(tag->year, buffer, 4); // v2.4 Year
            else if (strcmp(frame_id, "TCOM") == 0) strncpy(tag->composer, buffer, sizeof(tag->composer) - 1);
            else if (strcmp(frame_id, "TCON") == 0) strncpy(tag->content_type, buffer, sizeof(tag->content_type) - 1);
            else if (strcmp(frame_id, "TRCK") == 0) strncpy(tag->track, buffer, sizeof(tag->track) - 1);
        }
        // --- COMMENT FRAME (COMM) ---
        else if (strcmp(frame_id, "COMM") == 0)
        {
            fgetc(fp); // Skip encoding byte
            fseek(fp, 3, SEEK_CUR); // Skip language code
            
            int bytes_skipped = 4;
            int remaining_frame_size = frame_size - bytes_skipped;
            
            // Skip Short Content Description (null-terminated)
            char description_byte;
            int desc_len = 0;
            
            while (desc_len < remaining_frame_size && (description_byte = fgetc(fp)) != '\0')
            {
                desc_len++;
            }
            
            int total_skipped = bytes_skipped + desc_len + 1; 
            remaining_frame_size = frame_size - total_skipped;
            
            // Read the Actual Comment Text
            if (remaining_frame_size > 0)
            {
                if (fread(tag->comment, 1, remaining_frame_size, fp) != remaining_frame_size) break;
                tag->comment[remaining_frame_size] = '\0';
            }
        }
        // ATTACHED PICTURE FRAME (APIC)
        else if (strcmp(frame_id, "APIC") == 0) 
        {
            snprintf(tag->image_details, sizeof(tag->image_details), 
                     "Embedded Image found (Size: %d bytes)", frame_size);
            fseek(fp, frame_size, SEEK_CUR); // Skip content
        }
        else
        {
            // Skip unhandled frame content
            fseek(fp, frame_size, SEEK_CUR);
        }

        pos += frame_size;
    }

    fclose(fp);
    return SUCCESS;
}

// Updated print_tag to use ANSI colors and table formatting
void print_tag(const ID3v2Tag *tag)
{
    // Macro for printing a row (Label, Value, Color)
    #define PRINT_ROW(label, value, color) \
        printf("%s|%s %-12s %s|%s %-65s %s|\n", \
               BLUE, BOLD, label, RESET, color, value, RESET);

    // --- Header ---
    printf("\n%s%s====================================================================================\n", BOLD, BLUE);
    if (tag->major_version == 1) {
        printf("|| %sMP3 TAG METADATA (ID3v1)                                                        %s||\n", BOLD, BLUE);
    } else {
        printf("|| %sMP3 TAG METADATA (ID3v2.%d)                                                     %s||\n", BOLD, tag->major_version, BLUE);
    }
    printf("====================================================================================%s\n", RESET);

    // Data Table
    PRINT_ROW("Title", tag->title, GREEN);
    PRINT_ROW("Artist", tag->artist, YELLOW);
    PRINT_ROW("Album", tag->album, GREEN);
    PRINT_ROW("Year", tag->year, MAGENTA);
    PRINT_ROW("Composer", tag->composer, CYAN);
    PRINT_ROW("Genre", tag->content_type, YELLOW);
    PRINT_ROW("Track", tag->track, MAGENTA);
    PRINT_ROW("Comment", tag->comment, CYAN);

    // Image Details
    if (tag->image_details[0] != '\0') {
        printf("%s|%s %-12s %s|%s %-65s %s|\n", 
               BLUE, BOLD, "Image", RESET, WHITE, tag->image_details, RESET);
    }

    // Footer
    printf(BLUE"%s====================================================================================%s\n\n", BOLD, RESET);
    
    #undef PRINT_ROW
}
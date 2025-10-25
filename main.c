#include <stdio.h>
#include <string.h>
#include "tag.h"
#include <stdlib.h> // for exit

// Show usage info
void print_usage(const char *program)
{
    printf("Usage:\n");
    printf("  %s -v <mp3_filename>\n", program);
    printf("  %s -e -<option> <new_value> <mp3_filename>\n", program);
    printf("  %s --help / -h\n", program); 
    printf("Options for -e:\n");
    printf("  -t   Edit Title\n");
    printf("  -a   Edit Artist\n");
    printf("  -A   Edit Album\n");
    printf("  -y   Edit Year (4 digits only)\n");
    printf("  -c   Edit Composer\n");
    printf("  -g   Edit Genre\n");
    printf("  -T   Edit Track\n"); 
    printf("  -C   Edit Comment\n");
}

int main(int argc, char *argv[])
{
    // Handle Help Option (Restored)
    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0))
    {
        print_usage(argv[0]);
        return SUCCESS;
    }
    
    // View tags
    if (argc == 3 && strcmp(argv[1], "-v") == 0)
    {
        if (!mp3_extn(argv[2]))
        {
            printf("Error: Only .mp3 files are allowed!\n");
            return FAILURE;
        }

        ID3v2Tag tag = {0};
        if (read_mp3_tag(argv[2], &tag) == SUCCESS)
        {
            print_tag(&tag);
        }
        else
        {
            printf("Failed to read tags from: %s\n", argv[2]);
            return FAILURE;
        }
    }
    // Edit tag
    else if (argc == 5 && strcmp(argv[1], "-e") == 0)
    {
        const char *edit_flag = argv[2];
        const char *new_value = argv[3];
        const char *filename = argv[4];
        const char *frame_id = NULL; // ID3v2 compatible ID

        if (!mp3_extn(filename))
        {
            printf("Error: Only .mp3 files are allowed!\n");
            return FAILURE;
        }

        const char *field_name = NULL;

        if (strcmp(edit_flag, "-t") == 0) { frame_id = "TIT2"; field_name = "Title"; }
        else if (strcmp(edit_flag, "-a") == 0) { frame_id = "TPE1"; field_name = "Artist"; }
        else if (strcmp(edit_flag, "-A") == 0) { frame_id = "TALB"; field_name = "Album"; }
        else if (strcmp(edit_flag, "-y") == 0)
        {
            if (!valid_year(new_value))
            {
                printf("Error: Year must be 4 digits.\n");
                return FAILURE;
            }
            frame_id = "TYER"; // TYER is the primary search ID. tag_edit.c handles TDRC.
            field_name = "Year";
        }
        else if (strcmp(edit_flag, "-c") == 0) { frame_id = "TCOM"; field_name = "Composer"; }
        else if (strcmp(edit_flag, "-g") == 0) { frame_id = "TCON"; field_name = "Genre"; }
        else if (strcmp(edit_flag, "-T") == 0) { frame_id = "TRCK"; field_name = "Track"; } // Added
        else if (strcmp(edit_flag, "-C") == 0) { frame_id = "COMM"; field_name = "Comment"; } // Added
        else
        {
            printf("Unknown edit flag: %s\n", edit_flag);
            print_usage(argv[0]);
            return FAILURE;
        }

        printf("------------------------ Selected %s change option ---------------------\n", field_name);

        if (edit_mp3_tag(filename, frame_id, new_value) == SUCCESS)
        {
            printf("%s        : %s\n", field_name, new_value);
            printf("--------------------------%s changed successfully--------------------------\n", field_name);
        }
        else
        {
            printf("Failed to update tag.\n");
            return FAILURE;
        }
    }
    else
    {
        print_usage(argv[0]);
        return FAILURE;
    }

    return SUCCESS;
}
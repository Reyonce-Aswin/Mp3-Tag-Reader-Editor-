#ifndef TAG_H
#define TAG_H

#include <stdio.h> // For FILE*
#include <ctype.h> // For isdigit()
#include <string.h> // For strcmp

// Status enum for success/failure
typedef enum
{
    SUCCESS = 1,
    FAILURE = 0
} Status;

// Struct to hold common MP3 tag metadata
typedef struct
{
    int major_version;      // 1 for ID3v1, 2 for v2.2, 3 for v2.3, 4 for v2.4
    char title[128];
    char artist[128];
    char album[128];
    char year[5];           // 4 characters for year + null terminator
    char composer[128];
    char content_type[64];  // genre
    char track[8];          
    char comment[128];      
    char image_details[128]; 
} ID3v2Tag;

// Prototypes from tag_read.c
Status read_mp3_tag(const char *filename, ID3v2Tag *tag);
void print_tag(const ID3v2Tag *tag);
Status read_mp3_tag_v2(const char *filename, ID3v2Tag *tag); // <--- ADDED PROTOTYPE

// Prototypes from tag_v1.c
Status read_mp3_tag_v1(const char *filename, ID3v2Tag *tag);

// Prototypes from tag_edit.c
Status edit_mp3_tag(const char *filename, const char *frame_id_in, const char *new_value);

// Prototypes from tag_utils.c
int read_big_endian_int(unsigned char *bytes);
void write_big_endian_int(int value, unsigned char *bytes);
int read_synchsafe_int(unsigned char *bytes);
void write_synchsafe_int(int value, unsigned char *bytes);
int get_mp3_version(FILE *fp);
int mp3_extn(const char *filename);
int valid_year(const char *str);

#endif // TAG_H
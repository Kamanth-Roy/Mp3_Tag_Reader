#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 1
#define FAILURE 0

/* This structure stores the command line arguments.*/
typedef struct
{
    char *operation;       /* -v or -e */
    char *edit_option;     /* -t, -a, -A, -y, -m or -c */
    char *value;           /* New tag value */
    char *filename;        /* MP3 file name */
} MP3;

/* menu.c */
void print_usage(void);
void print_help(void);

/* view.c */
int view_mp3(char *filename);
unsigned int get_frame_size(unsigned char *size);
void print_text(unsigned char *data, unsigned int size);
void print_comment(unsigned char *data, unsigned int size);

/* edit.c */
int edit_title(char *filename, char *new_value);
int edit_artist(char *filename, char *new_value);
int edit_album(char *filename, char *new_value);
int edit_year(char *filename, char *new_value);
int edit_content(char *filename, char *new_value);
int edit_comment(char *filename, char *new_value);

#endif

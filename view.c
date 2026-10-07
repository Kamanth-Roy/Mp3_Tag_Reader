#include "header.h"
/* Convert the 4 frame-size bytes into one integer.
 * ID3v2.3 stores frame size in big-endian format. */
unsigned int get_frame_size(unsigned char *size)
{
    unsigned int value;

    value = ((unsigned int)size[0] << 24) |
            ((unsigned int)size[1] << 16) |
            ((unsigned int)size[2] << 8)  |
            size[3];

    return value;
}

void print_text(unsigned char *data, unsigned int size)
{
    unsigned int i;

    if (size <= 1)
    {
        printf("\n");
        return;
    }

    for (i = 1; i < size; i++)
    {
        if (data[i] == '\0')
            break;

        if (data[i] >= 32 && data[i] <= 126)
            printf("%c", data[i]);
    }

    printf("\n");
}

void print_comment(unsigned char *data, unsigned int size)
{
    unsigned int i;

    if (size <= 4)
    {
        printf("\n");
        return;
    }
    /* Skip encoding byte and 3-byte language. */
    i = 4;
    /* Skip the short description. */
    while (i < size && data[i] != '\0')
        i++;
    /* Skip the null byte after the description. */
    if (i < size)
        i++;
    while (i < size)
    {
        if (data[i] >= 32 && data[i] <= 126)
            printf("%c", data[i]);

        i++;
    }
    printf("\n");
}
/* Read the MP3 file and display its ID3v2.3 tag information.*/
int view_mp3(char *filename)
{
    FILE *fp;
    unsigned char header[10];
    unsigned char frame_header[10];
    unsigned char *frame_data;
    char frame_id[5];

    unsigned int tag_size;
    unsigned int frame_size;
    unsigned int total_read = 0;

    long file_size;

    fp = fopen(filename, "rb");

    if (fp == NULL)
    {
        printf("\nERROR: Unable to open MP3 file\n");
        return FAILURE;
    }
    /* Get the actual file size. */
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        printf("\nERROR: Unable to read file size\n");
        fclose(fp);
        return FAILURE;
    }

    file_size = ftell(fp);

    if (file_size < 10)
    {
        printf("\nERROR: Invalid MP3 file\n");
        fclose(fp);
        return FAILURE;
    }
    if (fseek(fp, 0, SEEK_SET) != 0)
    {
        printf("\nERROR: Unable to read MP3 file\n");
        fclose(fp);
        return FAILURE;
    }
    /* Read the 10-byte ID3 header. */
    if (fread(header, 1, 10, fp) != 10)
    {
        printf("\nERROR: Unable to read ID3 header\n");
        fclose(fp);
        return FAILURE;
    }
    /* Check for ID3 tag. */
    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3')
    {
        printf("\nERROR: ID3 tag not found\n");
        fclose(fp);
        return FAILURE;
    }
    /* This project supports ID3v2.3 only. */
    if (header[3] != 3)
    {
        printf("\nERROR: Only ID3v2.3 is supported\n");
        fclose(fp);
        return FAILURE;
    }
    /* Convert the 4 synchsafe tag-size bytes into an integer. */
    tag_size = ((unsigned int)(header[6] & 0x7F) << 21) |
               ((unsigned int)(header[7] & 0x7F) << 14) |
               ((unsigned int)(header[8] & 0x7F) << 7)  |
               (header[9] & 0x7F);

    if ((long)tag_size > file_size - 10)
    {
        printf("\nERROR: Invalid ID3 tag size\n");
        fclose(fp);
        return FAILURE;
    }
    printf("\n------------------------------------------------------------\n");
    printf("                 MP3 TAG READER / EDITOR\n");
    printf("------------------------------------------------------------\n");
   // printf("\n------------------------------------------------------------\n");
    printf("ID3 VERSION : 2.%d.%d\n", header[3], header[4]);
    printf("TAG SIZE    : %u bytes\n", tag_size);
    printf("------------------------------------------------------------\n");

    printf("TAG DETAILS\n");
    printf("------------------------------------------------------------\n");
    /* Read all frames inside the ID3 tag. */
    while (total_read + 10 <= tag_size)
    {
        if (fread(frame_header, 1, 10, fp) != 10)
        {
            printf("\nERROR: Unable to read frame header\n");
            fclose(fp);
            return FAILURE;
        }
        /* A zero byte means the remaining area is padding. */
        if (frame_header[0] == 0)
            break;

        frame_size = get_frame_size(&frame_header[4]);

        if (frame_size > tag_size - total_read - 10)
        {
            printf("\nERROR: Invalid frame size\n");
            fclose(fp);
            return FAILURE;
        }
        frame_id[0] = frame_header[0];
        frame_id[1] = frame_header[1];
        frame_id[2] = frame_header[2];
        frame_id[3] = frame_header[3];
        frame_id[4] = '\0';

        if (frame_size == 0)
        {
            total_read += 10;
            continue;
        }

        frame_data = malloc(frame_size);

        if (frame_data == NULL)
        {
            printf("\nERROR: Memory allocation failed\n");
            fclose(fp);
            return FAILURE;
        }

        if (fread(frame_data, 1, frame_size, fp) != frame_size)
        {
            printf("\nERROR: Unable to read frame data\n");
            free(frame_data);
            fclose(fp);
            return FAILURE;
        }

        if (strcmp(frame_id, "TIT2") == 0)
        {
            printf("TITLE   : ");
            print_text(frame_data, frame_size);
        }
        else if (strcmp(frame_id, "TPE1") == 0)
        {
            printf("ARTIST  : ");
            print_text(frame_data, frame_size);
        }
        else if (strcmp(frame_id, "TALB") == 0)
        {
            printf("ALBUM   : ");
            print_text(frame_data, frame_size);
        }
        else if (strcmp(frame_id, "TYER") == 0)
        {
            printf("YEAR    : ");
            print_text(frame_data, frame_size);
        }
        else if (strcmp(frame_id, "TCON") == 0)
        {
            printf("MUSIC   : ");
            print_text(frame_data, frame_size);
        }
        else if (strcmp(frame_id, "COMM") == 0)
        {
            printf("COMMENT : ");
            print_comment(frame_data, frame_size);
        }
        free(frame_data);
        total_read += 10 + frame_size;
    }
    printf("------------------------------------------------------------\n");
    fclose(fp);
    return SUCCESS;
}
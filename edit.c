#include "header.h"

/* Copy the new value into the frame.
* The value is copied exactly as the user gives it.*/
static void copy_value(unsigned char *data, char *value)
{
    unsigned int i;

    for (i = 0; value[i] != '\0'; i++)
        data[i + 1] = value[i];
}

static int edit_tag(char *filename, char *tag, char *new_value)
{
    FILE *fp;
    unsigned char header[10];
    unsigned char frame_header[10];
    unsigned char *data;

    char frame_id[5];

    unsigned int tag_size;
    unsigned int frame_size;
    unsigned int total_read = 0;
    unsigned int required_size;

    long data_position;
    long file_size;

    fp = fopen(filename, "r+b");

    if (fp == NULL)
    {
        printf("\nERROR: Unable to open MP3 file\n");
        return FAILURE;
    }
    /* Get file size. */
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
        fclose(fp);
        return FAILURE;
    }
    /* Read ID3 header. */
    if (fread(header, 1, 10, fp) != 10)
    {
        printf("\nERROR: Unable to read ID3 header\n");
        fclose(fp);
        return FAILURE;
    }

    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3')
    {
        printf("\nERROR: ID3 tag not found\n");
        fclose(fp);
        return FAILURE;
    }

    if (header[3] != 3)
    {
        printf("\nERROR: Only ID3v2.3 is supported\n");
        fclose(fp);
        return FAILURE;
    }

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
    /* Search for the required frame. */
    while (total_read + 10 <= tag_size)
    {
        if (fread(frame_header, 1, 10, fp) != 10)
        {
            printf("\nERROR: Unable to read frame header\n");
            fclose(fp);
            return FAILURE;
        }

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

        data_position = ftell(fp);

        if (strcmp(frame_id, tag) == 0)
        {
            if (frame_size == 0)
            {
                printf("\nERROR: Empty frame cannot be edited\n");
                fclose(fp);
                return FAILURE;
            }

            /* One byte is used for encoding. */
            required_size = 1 + strlen(new_value);

            if (required_size > frame_size)
            {
                printf("\nERROR: New value is too long\n");
                printf("Existing frame size : %u\n", frame_size);
                printf("Required size       : %u\n", required_size);
                fclose(fp);
                return FAILURE;
            }

            data = calloc(1, frame_size);

            if (data == NULL)
            {
                printf("\nERROR: Memory allocation failed\n");
                fclose(fp);
                return FAILURE;
            }

            /* Encoding 0 = ISO-8859-1. */
            data[0] = 0;
            copy_value(data, new_value);

            if (fseek(fp, data_position, SEEK_SET) != 0)
            {
                printf("\nERROR: Unable to move to frame data\n");
                free(data);
                fclose(fp);
                return FAILURE;
            }

            if (fwrite(data, 1, frame_size, fp) != frame_size)
            {
                printf("\nERROR: Unable to write new value\n");
                free(data);
                fclose(fp);
                return FAILURE;
            }
            free(data);
            fclose(fp);
            return SUCCESS;
        }

        if (fseek(fp, frame_size, SEEK_CUR) != 0)
        {
            printf("\nERROR: Unable to move to next frame\n");
            fclose(fp);
            return FAILURE;
        }

        total_read += 10 + frame_size;
    }
    printf("\nERROR: Tag %s not found\n", tag);

    fclose(fp);
    return FAILURE;
}

int edit_title(char *filename, char *new_value)
{
    if (edit_tag(filename, "TIT2", new_value) == FAILURE)
        return FAILURE;

    printf("\nTITLE updated successfully\n");
    return SUCCESS;
}

int edit_artist(char *filename, char *new_value)
{
    if (edit_tag(filename, "TPE1", new_value) == FAILURE)
        return FAILURE;

    printf("\nARTIST updated successfully\n");
    return SUCCESS;
}

int edit_album(char *filename, char *new_value)
{
    if (edit_tag(filename, "TALB", new_value) == FAILURE)
        return FAILURE;

    printf("\nALBUM updated successfully\n");
    return SUCCESS;
}

int edit_year(char *filename, char *new_value)
{
    if (edit_tag(filename, "TYER", new_value) == FAILURE)
        return FAILURE;

    printf("\nYEAR updated successfully\n");
    return SUCCESS;
}

int edit_content(char *filename, char *new_value)
{
    if (edit_tag(filename, "TCON", new_value) == FAILURE)
        return FAILURE;

    printf("\nMUSIC updated successfully\n");
    return SUCCESS;
}

int edit_comment(char *filename, char *new_value)
{
    FILE *fp;
    unsigned char header[10];
    unsigned char frame_header[10];
    unsigned char *data;

    char frame_id[5];

    unsigned int tag_size;
    unsigned int frame_size;
    unsigned int total_read = 0;
    unsigned int required_size;
    unsigned int i;

    long data_position;
    long file_size;

    fp = fopen(filename, "r+b");

    if (fp == NULL)
    {
        printf("\nERROR: Unable to open MP3 file\n");
        return FAILURE;
    }
    /* Get file size. */
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
        fclose(fp);
        return FAILURE;
    }

    if (fread(header, 1, 10, fp) != 10)
    {
        printf("\nERROR: Unable to read ID3 header\n");
        fclose(fp);
        return FAILURE;
    }

    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3')
    {
        printf("\nERROR: ID3 tag not found\n");
        fclose(fp);
        return FAILURE;
    }

    if (header[3] != 3)
    {
        printf("\nERROR: Only ID3v2.3 is supported\n");
        fclose(fp);
        return FAILURE;
    }

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
    /* Search for COMM frame. */
    while (total_read + 10 <= tag_size)
    {
        if (fread(frame_header, 1, 10, fp) != 10)
        {
            printf("\nERROR: Unable to read frame header\n");
            fclose(fp);
            return FAILURE;
        }

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

        data_position = ftell(fp);

        if (strcmp(frame_id, "COMM") == 0)
        {
            /*
             * COMM data:
             * 1 byte encoding
             * 3 bytes language
             * 1 byte empty description
             * comment text
             */
            required_size = 5 + strlen(new_value);

            if (required_size > frame_size)
            {
                printf("\nERROR: New comment is too long\n");
                printf("Existing frame size : %u\n", frame_size);
                printf("Required size       : %u\n", required_size);
                fclose(fp);
                return FAILURE;
            }

            data = calloc(1, frame_size);

            if (data == NULL)
            {
                printf("\nERROR: Memory allocation failed\n");
                fclose(fp);
                return FAILURE;
            }

            /* Encoding = 0 */
            data[0] = 0;

            /* Language = English */
            data[1] = 'e';
            data[2] = 'n';
            data[3] = 'g';

            /* Empty short description */
            data[4] = 0;

            /* Copy comment exactly as entered. */
            for (i = 0; new_value[i] != '\0'; i++)
                data[5 + i] = new_value[i];

            if (fseek(fp, data_position, SEEK_SET) != 0)
            {
                printf("\nERROR: Unable to move to comment data\n");
                free(data);
                fclose(fp);
                return FAILURE;
            }

            if (fwrite(data, 1, frame_size, fp) != frame_size)
            {
                printf("\nERROR: Unable to write comment\n");
                free(data);
                fclose(fp);
                return FAILURE;
            }

            free(data);
            fclose(fp);

            printf("\nCOMMENT updated successfully\n");
            return SUCCESS;
        }

        if (fseek(fp, frame_size, SEEK_CUR) != 0)
        {
            printf("\nERROR: Unable to move to next frame\n");
            fclose(fp);
            return FAILURE;
        }
        total_read += 10 + frame_size;
    }
    printf("\nERROR: COMM tag not found\n");

    fclose(fp);
    return FAILURE;
}

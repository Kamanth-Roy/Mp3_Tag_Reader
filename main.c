#include "header.h"

/*
 * Join the value arguments into one string.
 *
 * Example:
 * ./a.out -e -t Vikram Title Track sample.mp3
 *
 * The value becomes:
 * "Vikram Title Track"
 */
char *get_value(int argc, char *argv[])
{
    int i;
    unsigned int length = 0;
    unsigned int position = 0;
    char *value;

    for (i = 3; i < argc - 1; i++)
        length += strlen(argv[i]);

    /* Add one space between words. */
    if (argc > 5)
        length += argc - 5;

    value = malloc(length + 1);

    if (value == NULL)
        return NULL;

    for (i = 3; i < argc - 1; i++)
    {
        strcpy(&value[position], argv[i]);
        position += strlen(argv[i]);

        if (i < argc - 2)
        {
            value[position] = ' ';
            position++;
        }
    }

    value[position] = '\0';

    return value;
}

int main(int argc, char *argv[])
{
    MP3 mp3;
    int result;

    /* Check whether any command line argument is given. */
    if (argc < 2)
    {
        printf("\nERROR: Command line arguments not passed\n");
        print_usage();
        return 1;
    }

    /* Display help. */
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        print_help();
        return 0;
    }

    /* View operation: ./a.out -v sample.mp3 */
    if (strcmp(argv[1], "-v") == 0)
    {
        if (argc != 3)
        {
            printf("\nERROR: Invalid arguments for view\n");
            print_usage();
            return 1;
        }

        mp3.operation = argv[1];
        mp3.filename = argv[2];

        result = view_mp3(mp3.filename);

        if (result == SUCCESS)
            return 0;

        return 1;
    }

    /* Edit operation:
     * ./a.out -e -t Vikram Title Track sample.mp3
     */
    if (strcmp(argv[1], "-e") == 0)
    {
        if (argc < 5)
        {
            printf("\nERROR: Invalid arguments for edit\n");
            print_usage();
            return 1;
        }

        mp3.operation = argv[1];
        mp3.edit_option = argv[2];
        mp3.filename = argv[argc - 1];

        mp3.value = get_value(argc, argv);

        if (mp3.value == NULL)
        {
            printf("\nERROR: Memory allocation failed\n");
            return 1;
        }

        if (strlen(mp3.value) == 0)
        {
            printf("\nERROR: Tag value cannot be empty\n");
            free(mp3.value);
            return 1;
        }

        if (strcmp(mp3.edit_option, "-t") == 0)
        {
            result = edit_title(mp3.filename, mp3.value);
        }
        else if (strcmp(mp3.edit_option, "-a") == 0)
        {
            result = edit_artist(mp3.filename, mp3.value);
        }
        else if (strcmp(mp3.edit_option, "-A") == 0)
        {
            result = edit_album(mp3.filename, mp3.value);
        }
        else if (strcmp(mp3.edit_option, "-y") == 0)
        {
            result = edit_year(mp3.filename, mp3.value);
        }
        else if (strcmp(mp3.edit_option, "-m") == 0)
        {
            result = edit_content(mp3.filename, mp3.value);
        }
        else if (strcmp(mp3.edit_option, "-c") == 0)
        {
            result = edit_comment(mp3.filename, mp3.value);
        }
        else
        {
            printf("\nERROR: Invalid edit option\n");
            print_help();
            free(mp3.value);
            return 1;
        }

        free(mp3.value);

        if (result == SUCCESS)
            return 0;

        return 1;
    }

    printf("\nERROR: Invalid command line arguments\n");
    print_usage();

    return 1;
}

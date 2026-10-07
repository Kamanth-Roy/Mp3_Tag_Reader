#include "header.h"

/*
 * Print the basic command line syntax.
 */
void print_usage(void)
{
    printf("\n------------------------------------------------------------\n");
    printf("USAGE\n");
    printf("------------------------------------------------------------\n");

    printf("\nView MP3:\n");
    printf("./a.out -v sample.mp3\n");

    printf("\nEdit MP3:\n");
    printf("./a.out -e -t Vikram Title Track sample.mp3\n");

    printf("\nHelp:\n");
    printf("./a.out -h\n");
}

/*
 * Print all supported operations.
 */
void print_help(void)
{
    printf("\n------------------------------------------------------------\n");
    printf("                 MP3 TAG READER / EDITOR\n");
    printf("------------------------------------------------------------\n");

    printf("\nVIEW:\n");
    printf("  -v                 View MP3 tags\n");

    printf("\nEDIT:\n");
    printf("  -t                 Edit Title\n");
    printf("  -a                 Edit Artist\n");
    printf("  -A                 Edit Album\n");
    printf("  -y                 Edit Year\n");
    printf("  -m                 Edit Music / Genre\n");
    printf("  -c                 Edit Comment\n");

    printf("\nEXAMPLES:\n");
    printf("  ./a.out -v sample.mp3\n");
    printf("  ./a.out -e -t Vikram Title Track sample.mp3\n");
    printf("  ./a.out -e -a Anirudh Ravichander sample.mp3\n");
    printf("  ./a.out -e -A Vikram 2022 sample.mp3\n");
    printf("  ./a.out -e -y 2022 sample.mp3\n");
    printf("  ./a.out -e -m Tamil sample.mp3\n");
    printf("  ./a.out -e -c Elivation sample.mp3\n");

    printf("\nNOTE: This project supports ID3v2.3 tags.\n");
}

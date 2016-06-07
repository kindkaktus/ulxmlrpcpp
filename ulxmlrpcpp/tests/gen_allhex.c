#include <stdio.h>

int main (int argc, char **argv)
{
    if (argc < 2)
    {
        printf ("Bad command args\n");
        return 1;
    }

    printf ("Creating %s\n", argv[1]);
    FILE *hex = fopen(argv[1], "wb");
    unsigned  i;
    unsigned  j;
    for (j = 0; j <= 255; ++j)
        for (i = 0; i <= 255; ++i)
            fputc(i, hex);

    fclose(hex);
    return 0;
}

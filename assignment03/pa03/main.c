#include <stdio.h>
#include <string.h>

#include "file_ops.h"

static void usage(const char *program_name)
{
    fprintf(stderr, "usage: %s info PATH\n"
                    "       %s copy SOURCE DEST\n",
            program_name, program_name);
}

int main(int argc, char *argv[])
{
    if (argc == 3 && strcmp(argv[1], "info") == 0) {
        return file_info(argv[2]);
    }
    if (argc == 4 && strcmp(argv[1], "copy") == 0) {
        return file_copy(argv[2], argv[3]);
    }
    usage(argv[0]);
    return 2;
}

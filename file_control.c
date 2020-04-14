#include <stdio.h>

// we use fseeko() and fwrite() to create a file.
void create_file(char *filename, file_bytes size) {
    FILE *file = fopen(filename, "wb");
    fseeko(file, size - 1, SEEK_SET);
    fwrite("\0", 1, 1, file);
    fclose(file);
}
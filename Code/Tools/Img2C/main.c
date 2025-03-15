// Author: Jake Rieger
// Created: 3/15/2025.
//

/**
 * Converts a collection of image files to byte arrays and exports the file as a C header
 */

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BYTES_PER_LINE 16
#define MAX_PATH_LEN 256

void create_identifier(const char* filename, char* identifier) {
    const char* base_name = strrchr(filename, '\\');
    if (base_name) {
        base_name++;
    } else {
        base_name = filename;
    }

    int j = 0;
    for (int i = 0; base_name[i] && base_name[i] != '.'; i++) {
        if (isalnum(base_name[i])) {
            identifier[j++] = toupper(base_name[i]);
        } else {
            identifier[j++] = '_';
        }
    }
    identifier[j] = '\0';
}

void write_bytes_array(unsigned char* input_bytes, size_t input_size, FILE* output, const char* identifier) {
    unsigned char buffer[BYTES_PER_LINE];

    fprintf(output, "const unsigned char %s_BYTES[] = {\n    ", identifier);

    size_t bytes_remaining = input_size;
    size_t offset          = 0;
    while (bytes_remaining > 0) {
        size_t bytes_to_read = (bytes_remaining < BYTES_PER_LINE) ? bytes_remaining : BYTES_PER_LINE;
        memcpy(buffer, input_bytes + offset, bytes_to_read);

        for (size_t i = 0; i < bytes_to_read; i++) {
            fprintf(output, "0x%02x", buffer[i]);

            if (offset + i + 1 < input_size) {
                fprintf(output, ", ");
                if ((offset + i + 1) % BYTES_PER_LINE == 0) { fprintf(output, "\n    "); }
            }
        }

        offset += bytes_to_read;
        bytes_remaining -= bytes_to_read;
    }

    fprintf(output, "\n};\n\n");
    fprintf(output, "const unsigned long %s_SIZE = %llu;\n\n", identifier, input_size);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s output.h image1 [image2 ...]\n", argv[0]);
        return 1;
    }

    FILE* output = fopen(argv[1], "w");
    if (!output) {
        fprintf(stderr, "Error: Cannot create output file %s\n", argv[1]);
        return 1;
    }

    char header_guard[MAX_PATH_LEN];
    create_identifier(argv[1], header_guard);
    for (int i = 0; header_guard[i]; i++) {
        header_guard[i] = toupper(header_guard[i]);
    }

    fprintf(output, "#ifndef %s_H\n", header_guard);
    fprintf(output, "#define %s_H\n\n", header_guard);

    for (int i = 2; i < argc; i++) {
        int x, y, channels;
        unsigned char* image_data = stbi_load(argv[i], &x, &y, &channels, 4);
        if (!image_data) { fprintf(stderr, "Error: Cannot load image file %s\n", argv[i]); }
        size_t image_size = x * y * channels;

        printf("Loading image: %s\n", argv[i]);
        printf("Image size: %i x %i x %i\n", x, y, channels);

        char identifier[MAX_PATH_LEN];
        create_identifier(argv[i], identifier);

        write_bytes_array(image_data, image_size, output, identifier);
    }

    fprintf(output, "#endif // %s_H\n", header_guard);
    fclose(output);

    return 0;
}
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NCON_VERSION "1.0.0"
#define NUMBER_BUFFER_SIZE 68

typedef enum {
    PARSE_OK,
    PARSE_INVALID,
    PARSE_OUT_OF_RANGE
} ParseStatus;

typedef struct {
    int64_t value;
    char decimal[NUMBER_BUFFER_SIZE];
    char octal[NUMBER_BUFFER_SIZE];
    char hexadecimal[NUMBER_BUFFER_SIZE];
    char binary[NUMBER_BUFFER_SIZE];
} Row;

typedef struct {
    bool uppercase;
    bool prefix;
} Options;

static ParseStatus to_int64(const char *s, int64_t *result) {
    if (s == NULL || result == NULL || *s == '\0' || isspace((unsigned char)*s)) {
        return PARSE_INVALID;
    }

    char *end;
    errno = 0;

    intmax_t value = strtoimax(s, &end, 10);

    if (end == s || *end != '\0') {
        return PARSE_INVALID;
    }

    if (errno == ERANGE || value < INT64_MIN || value > INT64_MAX) {
        return PARSE_OUT_OF_RANGE;
    }

    *result = (int64_t)value;
    return PARSE_OK;
}

static bool format_number(int64_t value, unsigned int base, bool uppercase,
                          bool prefix, char *result, size_t result_size) {
    const char *digits = uppercase
        ? "0123456789ABCDEF"
        : "0123456789abcdef";
    char reversed[65];
    size_t digit_count = 0;
    size_t prefix_length = prefix && base != 10 ? 2 : 0;
    size_t sign_length = value < 0 ? 1 : 0;
    uint64_t magnitude = value < 0
        ? (uint64_t)(-(value + 1)) + 1
        : (uint64_t)value;

    if (base < 2 || base > 16 || result == NULL) {
        return false;
    }

    do {
        reversed[digit_count++] = digits[magnitude % base];
        magnitude /= base;
    } while (magnitude != 0);

    if (sign_length + prefix_length + digit_count + 1 > result_size) {
        return false;
    }

    size_t position = 0;

    if (value < 0) {
        result[position++] = '-';
    }

    if (prefix_length != 0) {
        result[position++] = '0';
        if (base == 2) {
            result[position++] = uppercase ? 'B' : 'b';
        } else if (base == 8) {
            result[position++] = uppercase ? 'O' : 'o';
        } else {
            result[position++] = uppercase ? 'X' : 'x';
        }
    }

    while (digit_count != 0) {
        result[position++] = reversed[--digit_count];
    }

    result[position] = '\0';
    return true;
}

static bool format_row(Row *row, const Options *options) {
    return format_number(row->value, 10, false, false,
                         row->decimal, sizeof(row->decimal))
        && format_number(row->value, 8, false, options->prefix,
                         row->octal, sizeof(row->octal))
        && format_number(row->value, 16, options->uppercase, options->prefix,
                         row->hexadecimal, sizeof(row->hexadecimal))
        && format_number(row->value, 2, false, options->prefix,
                         row->binary, sizeof(row->binary));
}

static void update_width(size_t *width, const char *text) {
    size_t length = strlen(text);

    if (length > *width) {
        *width = length;
    }
}

static void print_border(const size_t widths[4]) {
    for (size_t column = 0; column < 4; ++column) {
        putchar('+');
        for (size_t i = 0; i < widths[column] + 2; ++i) {
            putchar('-');
        }
    }
    puts("+");
}

static void print_table(const Row *rows, size_t row_count) {
    size_t widths[4] = {9, 9, 9, 18};

    for (size_t i = 0; i < row_count; ++i) {
        update_width(&widths[0], rows[i].decimal);
        update_width(&widths[1], rows[i].octal);
        update_width(&widths[2], rows[i].hexadecimal);
        update_width(&widths[3], rows[i].binary);
    }

    print_border(widths);
    printf("| %-*s | %-*s | %-*s | %-*s |\n",
           (int)widths[0], "DEC",
           (int)widths[1], "OCT",
           (int)widths[2], "HEX",
           (int)widths[3], "BIN");
    print_border(widths);

    for (size_t i = 0; i < row_count; ++i) {
        printf("| %*s | %*s | %*s | %*s |\n",
               (int)widths[0], rows[i].decimal,
               (int)widths[1], rows[i].octal,
               (int)widths[2], rows[i].hexadecimal,
               (int)widths[3], rows[i].binary);
        print_border(widths);
    }
}

static void print_usage(FILE *stream) {
    fprintf(stream,
            "Usage: ncon [OPTIONS] <NUMBER>...\n"
            "Convert signed decimal integers to octal, hexadecimal, and binary.\n"
            "\n"
            "Options:\n"
            "  -u, --uppercase  use uppercase hexadecimal digits\n"
            "  -p, --prefix     add 0o, 0x, and 0b prefixes\n"
            "  -h, --help       display this help and exit\n"
            "  -V, --version    display version information and exit\n"
            "  --               stop processing options\n");
}

int main(int argc, char *argv[]) {
    Options options = {false, false};
    bool parse_options = true;
    size_t row_count = 0;
    size_t row_capacity = argc > 1 ? (size_t)argc - 1 : 0;
    Row *rows = NULL;

    if (row_capacity > SIZE_MAX / sizeof(*rows)) {
        fprintf(stderr, "ncon: too many number operands\n");
        return EXIT_FAILURE;
    }

    if (row_capacity != 0) {
        rows = malloc(row_capacity * sizeof(*rows));
        if (rows == NULL) {
            fprintf(stderr, "ncon: unable to allocate memory\n");
            return EXIT_FAILURE;
        }
    }

    for (int i = 1; i < argc; ++i) {
        const char *argument = argv[i];

        if (parse_options && strcmp(argument, "--") == 0) {
            parse_options = false;
            continue;
        }

        if (parse_options && (strcmp(argument, "-h") == 0
                              || strcmp(argument, "--help") == 0)) {
            print_usage(stdout);
            free(rows);
            return EXIT_SUCCESS;
        }

        if (parse_options && (strcmp(argument, "-V") == 0
                              || strcmp(argument, "--version") == 0)) {
            printf("ncon %s\n", NCON_VERSION);
            free(rows);
            return EXIT_SUCCESS;
        }

        if (parse_options && (strcmp(argument, "-u") == 0
                              || strcmp(argument, "--uppercase") == 0)) {
            options.uppercase = true;
            continue;
        }

        if (parse_options && (strcmp(argument, "-p") == 0
                              || strcmp(argument, "--prefix") == 0)) {
            options.prefix = true;
            continue;
        }

        if (parse_options && argument[0] == '-' && argument[1] != '\0'
            && !isdigit((unsigned char)argument[1])) {
            fprintf(stderr, "ncon: unknown option '%s'\n", argument);
            fprintf(stderr, "Try 'ncon --help' for more information.\n");
            free(rows);
            return EXIT_FAILURE;
        }

        ParseStatus status = to_int64(argument, &rows[row_count].value);

        if (status == PARSE_INVALID) {
            fprintf(stderr, "ncon: invalid decimal integer '%s'\n", argument);
            free(rows);
            return EXIT_FAILURE;
        }

        if (status == PARSE_OUT_OF_RANGE) {
            fprintf(stderr, "ncon: integer out of range '%s'\n", argument);
            free(rows);
            return EXIT_FAILURE;
        }

        ++row_count;
    }

    if (row_count == 0) {
        fprintf(stderr, "ncon: missing number operand\n");
        fprintf(stderr, "Try 'ncon --help' for more information.\n");
        free(rows);
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < row_count; ++i) {
        if (!format_row(&rows[i], &options)) {
            fprintf(stderr, "ncon: failed to format a number\n");
            free(rows);
            return EXIT_FAILURE;
        }
    }

    print_table(rows, row_count);
    free(rows);

    if (fflush(stdout) == EOF) {
        fprintf(stderr, "ncon: failed to write output\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

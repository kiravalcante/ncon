#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

bool to_int(const char *s, int *result) {
    if (s == NULL || result == NULL || *s == '\0') {
        return false;
    }

    char *end;
    errno = 0;

    long value = strtol(s, &end, 10);

    if (end == s)
        return false;

    if (*end != '\0')
        return false;

    if (errno == ERANGE || value < INT_MIN || value > INT_MAX)
        return false;

    *result = (int)value;
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ncon <NUMBER_1> <NUMBER_2> (...)\n");
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        int n;
        if (!to_int(argv[i], &n)) {
            printf("Error: failed to parse \"%s\"\n", argv[i]);
            return 1;
        }
    }

    printf("+-----------+-----------+-----------+--------------------+\n");
    printf("| DEC       | OCT       | HEX       | BIN                |\n");
    printf("+-----------+-----------+-----------+--------------------+\n");

    for (int i = 1; i < argc; ++i) {
        int n;
        to_int(argv[i], &n);

        printf("| %9d ", n);  // DEC
        printf("| %9o ", n);  // OCT
        printf("| %9x ", n);  // HEX
        printf("| %18b ", n); // BIN

        printf("|\n");

        printf("+-----------+-----------+-----------+--------------------+\n");
    }

    return 0;
}

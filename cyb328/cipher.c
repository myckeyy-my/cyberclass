/*
 * Extended Caesar Cipher
 *
 * Encrypts/decrypts all printable ASCII characters from '!' (33) through
 * '~' (126). Characters outside that range are left unchanged.
 * Key values must be in the range [0, 93].
 *
 * Usage:
 *   ./cipher -i <input_file> -o <output_file> -k <key> -p <encrypt|decrypt>
 *
 * Compile:
 *   gcc -std=c11 cipher.c -o cipher
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>   /* getopt */

/* Printable ASCII range: '!' (33) .. '~' (126) => 94 characters */
#define RANGE_START 33   /* '!' */
#define RANGE_END   126  /* '~' */
#define RANGE_SIZE  94   /* 126 - 33 + 1 */

/* ---------------------------------------------------------------------------
 * Print program usage information to stderr.
 * --------------------------------------------------------------------------- */
static void print_usage(const char *prog_name)
{
    fprintf(stderr,
            "Usage: %s -i <input_file> -o <output_file> -k <key> -p <encrypt|decrypt>\n"
            "  -i  Input file path\n"
            "  -o  Output file path\n"
            "  -k  Key value (integer 0..93)\n"
            "  -p  Process mode: encrypt or decrypt\n",
            prog_name);
}

/* ---------------------------------------------------------------------------
 * Shift a single character using modular (wrap-around) arithmetic.
 *
 * direction > 0  => encrypt (shift forward by key)
 * direction < 0  => decrypt (shift backward by key)
 *
 * Characters outside ASCII 33-126 are returned unchanged.
 * --------------------------------------------------------------------------- */
static char shift_char(char ch, int key, int direction)
{
    unsigned char c = (unsigned char)ch; //convert character to unsigned char
    int offset;  //offset of the character
    int shifted; //shifted character

    /* Leave non-printable / out-of-range characters alone */
    if (c < RANGE_START || c > RANGE_END) {
        return ch;
    }

    /* Map character into [0, 93], apply signed shift, wrap, map back */
    offset = (int)c - RANGE_START;  //map character into [0, 93]
    shifted = offset + (direction * key); //apply signed shift

    /* Positive modulo so the result is always in [0, RANGE_SIZE) */
    shifted = ((shifted % RANGE_SIZE) + RANGE_SIZE) % RANGE_SIZE; //wrap around the character

    return (char)(shifted + RANGE_START);
}

/* ---------------------------------------------------------------------------
 * Encrypt or decrypt an entire buffer into a separate buffer.
 * mode_encrypt != 0 => encrypt; otherwise decrypt.
 * --------------------------------------------------------------------------- */
static void process_buffer(const char *input, char *output, size_t length,
                           int key, int mode_encrypt)
{
    size_t i;   //loop counter
    int direction = mode_encrypt ? 1 : -1;  //check if encrypt or decrypt
    for (i = 0; i < length; i++) {
        output[i] = shift_char(input[i], key, direction); //call shift_char function to shift the character
    }
}

/* ---------------------------------------------------------------------------
 * Read the entire contents of a file into a dynamically allocated buffer.
 * If success, *out_buf points to the data and *out_len is the byte count.
 * --------------------------------------------------------------------------- */
static int read_entire_file(const char *path, char **out_buf, size_t *out_len)
{
    FILE *fp;
    long size;
    char *buf;
    size_t nread;

    fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "Error: could not open input file '%s': %s\n",
                path, strerror(errno));
        return -1;
    }

    /* Determine file size */
    if (fseek(fp, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error: could not seek in input file '%s'\n", path);
        fclose(fp);
        return -1;
    }

    size = ftell(fp);
    if (size < 0) {
        fprintf(stderr, "Error: could not determine size of '%s'\n", path);
        fclose(fp);
        return -1;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Error: could not rewind input file '%s'\n", path);
        fclose(fp);
        return -1;
    }

    buf = (char *)malloc((size_t)size + 1);  //allocate memory for the buffer
    if (!buf) {  //check if the buffer is allocated/not null
        fprintf(stderr, "Error: out of memory reading '%s'\n", path);
        fclose(fp);
        return -1;
    }

    nread = fread(buf, 1, (size_t)size, fp);  //read the file into the buffer   
    if (nread != (size_t)size) {  //check if the file is read completely
        fprintf(stderr, "Error: failed to read entire input file '%s'\n", path);
        free(buf);
        fclose(fp);
        return -1;
    }

    buf[nread] = '\0'; // null-terminate
    fclose(fp);  //close the file

    *out_buf = buf;  //point to the buffer
    *out_len = nread;  //set the length of the buffer
    return 0;
}

/* ---------------------------------------------------------------------------
 * Write a buffer of the given length to an output file.
 * Returns 0 on success, -1 on failure.
 * --------------------------------------------------------------------------- */
static int write_entire_file(const char *path, const char *buf, size_t length)
{
    FILE *fp;
    size_t nwritten;

    fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "Error: could not open output file '%s': %s\n",
                path, strerror(errno));
        return -1;
    }

    nwritten = fwrite(buf, 1, length, fp);  //write the buffer to the file
    if (nwritten != length) {  //check if the file is written completely
        fprintf(stderr, "Error: failed to write entire output file '%s'\n", path);
        fclose(fp);
        return -1;
    }

    if (fclose(fp) != 0) {  //close the file
        fprintf(stderr, "Error: failed to close output file '%s': %s\n",
                path, strerror(errno));
        return -1;
    }

    return 0;
}

/* ---------------------------------------------------------------------------
 * Parse and validate the key string. Must be an integer in [0, 93].
 * Returns 0 on success (stores value in *out_key), -1 on failure.
 * --------------------------------------------------------------------------- */
static int parse_key(const char *arg, int *out_key)
{
    char *endptr = NULL;
    long val;

    errno = 0;
    val = strtol(arg, &endptr, 10);  //convert the string to a long integer

    if (endptr == arg || *endptr != '\0' || errno == ERANGE) {  //check if the string is a valid integer
        fprintf(stderr, "Error: -k requires an integer, got '%s'\n", arg);
        return -1;
    }

    if (val < 0 || val > 93) {  //check if the integer is in the range [0, 93]
        fprintf(stderr, "Error: -k value %ld out of range [0, 93]\n", val);
        return -1;
    }

    *out_key = (int)val;  //store the integer in the output key
    return 0;
}

/* ---------------------------------------------------------------------------
 * Validate that process mode is exactly "encrypt" or "decrypt".
 * Returns 1 for encrypt, 0 for decrypt, -1 for invalid.
 * --------------------------------------------------------------------------- */
static int parse_mode(const char *arg)
{
    if (strcmp(arg, "encrypt") == 0) {  //check if the mode is encrypt
        return 1;
    }
    if (strcmp(arg, "decrypt") == 0) {  //check if the mode is decrypt
        return 0;
    }

    fprintf(stderr,
            "Error: -p must be 'encrypt' or 'decrypt', got '%s'\n", arg);  //print the error message if the mode is invalid
    return -1;
}

/* ---------------------------------------------------------------------------
 * main: parse CLI args, read input, process, write output.
 * --------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
    char *input_path = NULL;
    char *output_path = NULL;
    char *mode_arg = NULL;
    char *input_buf = NULL;
    char *output_buf = NULL;
    size_t input_len = 0;
    int key = 0;
    int key_provided = 0;
    int mode_encrypt;
    int c;

    opterr = 0; //we handle option errors ourselves

    while ((c = getopt(argc, argv, "i:o:k:p:")) != -1) {
        switch (c) {
            case 'i':
                input_path = optarg;
                break;

            case 'o':
                output_path = optarg;
                break;

            case 'k':
                if (parse_key(optarg, &key) != 0) {
                    return 1;
                }
                key_provided = 1;
                break;

            case 'p':
                mode_arg = optarg;
                break;

            case '?':
                if (strchr("iopk", optopt) != NULL) {
                    fprintf(stderr, "Error: option -%c requires an argument.\n",
                            optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Error: unknown option '-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Error: unknown option character '\\x%x'.\n",
                            optopt);
                }
                print_usage(argv[0]);
                return 1;

            default:
                abort();
        }
    }

    /* Validate that all required options were supplied */
    if (!input_path || !output_path || !key_provided || !mode_arg) {
        fprintf(stderr, "Error: missing required argument(s).\n");
        print_usage(argv[0]);
        return 1;
    }

    mode_encrypt = parse_mode(mode_arg);
    if (mode_encrypt < 0) {
        print_usage(argv[0]);
        return 1;
    }

    /* Read the entire input file */
    if (read_entire_file(input_path, &input_buf, &input_len) != 0) {
        return 1;
    }

    /* Allocate output buffer and process every character */
    output_buf = (char *)malloc(input_len + 1);
    if (!output_buf) {
        fprintf(stderr, "Error: out of memory for output buffer\n");
        free(input_buf);
        return 1;
    }

    process_buffer(input_buf, output_buf, input_len, key, mode_encrypt);  //call the process_buffer function to process the buffer

    /* Write results to the output file */
    if (write_entire_file(output_path, output_buf, input_len) != 0) {
        free(input_buf);
        free(output_buf);
        return 1;
    }

    free(input_buf);  //free the input buffer
    free(output_buf);  //free the output buffer
    return 0;
}

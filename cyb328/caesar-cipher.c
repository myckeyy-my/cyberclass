#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

int main(int argc, char **argv) {
    char *ivalue = NULL;
    char *ovalue = NULL; 
    int kvalue = 0; 
    char *pvalue = NULL;
    int c;
    char *input = NULL;

    opterr = 0;

    while ((c = getopt (argc, argv, "i:o:k:p:")) != -1)
        switch (c) {
            case 'i':
                ivalue = optarg;
                break;
            case 'o':
                ovalue = optarg;
                break;
            case 'k':
                errno = 0;
                char *endptr;
                long val = strtol(optarg, &endptr, 10);

                if (endptr == optarg || *endptr != '\0' || errno == ERANGE) {
                    fprintf(stderr, "Error: -k requires an integer, got '%s'\n", optarg);
                    return 1;
                }
                if (val < 0 || val > 93) {
                    fprintf(stderr, "Error: -k value %ld out of range [0, 93]\n", val);
                    return 1;
                }
                kvalue = (int)val;
                break;
            case 'p':
                pvalue = optarg;
                break;
            case '?':
                if (strchr("iopk", optopt) != NULL)
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option '-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
                return 1;
            default:
                abort();
        }

    printf ("ivalue = %s, ovalue = %s, kvalue = %d, pvalue = %s", ivalue, ovalue, kvalue, pvalue);
    
    FILE *fp = fopen(ivalue, "r");
    if (!fp) {
        fprintf(stderr, "Could not open '%s': %s\n", ivalue, strerror(errno));
        return 1;
    }
    while (fgets(input, sizeof(input), fp) != NULL) {
        printf("%s", input);
    }
    fclose(fp);
    return 0;
}
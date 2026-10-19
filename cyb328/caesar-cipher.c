#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

int main(int argc, char **argv) {
    char *ivalue = NULL;
    char *ovalue = NULL; 
    char *kvalue = NULL; 
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
                kvalue = optarg;
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

    printf ("ivalue = %s, ovalue = %s, kvalue = %s, pvalue = %s", ivalue, ovalue, kvalue, pvalue);
    
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void okp_decompress(void *src, void *dst);
int main(int argc, char **argv){
    const char *orig = argv[1], *packed = argv[2];
    long offset = atol(argv[3]);
    FILE *f = fopen(orig,"rb"); fseek(f,0,SEEK_END); long on = ftell(f); rewind(f);
    unsigned char *o = malloc(on); fread(o,1,on,f); fclose(f);
    f = fopen(packed,"rb"); fseek(f,0,SEEK_END); long pn = ftell(f); rewind(f);
    unsigned char *p = malloc(pn+64); memset(p,0,pn+64); fread(p,1,pn,f); fclose(f);

    const int MARGIN = 13, CAP = 262144;
    unsigned char *dst = calloc(1, CAP);
    printf("original %ld bytes, packed %ld bytes, offset %ld\n", on, pn, offset);
    okp_decompress(p + offset, dst + MARGIN);

    long bad = 0, first = -1;
    for (long i = 0; i < on; i++)
        if (dst[MARGIN+i] != o[i]) { if (first < 0) first = i; bad++; }
    if (!bad) printf("ROUND TRIP OK - all %ld bytes match\n", on);
    else printf("MISMATCH: %ld of %ld bytes differ, first at %ld (%02x vs %02x)\n",
                bad, on, first, dst[MARGIN+first], o[first]);
    return bad != 0;
}

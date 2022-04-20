#include <string.h>
#include <stdlib.h>
#include "xyutils.h"
#include "liteopt.h"
#include "book.h"
#include "fragcov.h"

unsigned char *flat_memory;

int main(int argc, char **argv)
{
    if(argc == 0) // Interactive mode
    {
        puts("Not implemented yet.");
    }
    
    flat_memory = (unsigned char *)calloc(0x20000000, sizeof(unsigned char));
}

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "r5900.h"
#include "xyutils.h"
#include "liteopt.h"
#include "book.h"
#include "fragcov.h"
#include "EISystem.h"

unsigned char *flat_memory;

int loadelf(byte_t *target, const char *filename) {
    int ret;
    FILE* elf = fopen(filename, "rb");
    struct elf_header *header = (struct elf_header*)malloc(sizeof(struct elf_header));
    if((ret = fread(header, sizeof(struct elf_header), 1, elf)) != 1) {
        fprintf(stderr, "ERROR: Can't read elf file (%d): %s.\n", ret, strerror(errno));
        free(header);
        fclose(elf);
        exit(-1);
    };

    struct program_header *phs = (struct program_header *)malloc(sizeof(struct program_header) * header->phnum);
    fseek(elf, header->phoff, SEEK_SET);
    fread(phs, sizeof(struct program_header), header->phnum, elf);

    for (int i = 0; i < header->phnum; ++i) {
        if (phs[i].type != 1) continue; // non PT_LOAD, skip
        fseek(elf, phs[i].offset, SEEK_SET);
        fread(target + phs[i].paddr, phs[i].filesz, 1, elf);
    }

    fclose(elf);
    
    ret = header->entry;
    free(header);
    free(phs);
    printf("Entry point is %08X\n", ret);
    return ret;
}

extern cfg_emu_anal_ignore_bmerr;

int main(int argc, char **argv)
{
    if(argc == 1) // Interactive mode
    {
        puts("Not implemented yet.");
        exit(-1);
    }
    cfg_emu_anal_ignore_bmerr = 1;
    
    flat_memory = (unsigned char *)calloc(0x20000000, sizeof(unsigned char));

    struct book *lib = book_init(argv[1]);
    lib->entry_point = loadelf(flat_memory, argv[1]);
    struct emu_status *sta = emu_init();
    emu_anal(sta, lib, flat_memory);

    book_close(lib);
}

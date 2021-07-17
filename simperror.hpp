#pragma once
#ifndef SIMPERROR_HPP
#define SIMPERROR_HPP
#include <stdio.h>
#include <stdlib.h>

static struct err_t {
    long code; // hi-byte: 0 - msg, 1 - warning, 2 - error, 4 - fatal
    const char* msg;
} errs[] = {
    {0x00000000, "Completed nornally."},
    {0x00000001, "Information generated."},
    {0x02000000, "Incorrect bin num format."},
    {0x02000001, "Incorrect oct num format."},
    {0x02010000, "Unexpected determined num starter. Expected 'x', 'o', 'd' or 'b'."},
    {0x04000000, "Reflect output file is not specified."},
    {0x04000001, "Asmbler part routine output file is not specified."},
    {0x04000002, "IO string template output file is not specified."},
    {0x04000003, "Macro defination output file is not specified."},
    {0x04000004, "Execution emulation definition output file is not specified."},
    {0x04000005, "Disasmbler part routine output file is not specified."},
    {0x04000100, "Input file is not specified."},
    {0x040F7FFF, "Uknown error code."}
};

static void fatal (long code)
{
    for(int i = 0; i < sizeof(errs); ++i)
    {
        if(errs[i].code == code) {
            printf("F %X: %s\n", errs[i].code, errs[i].msg);
            exit(code);
        }
    }
    fatal(0x040F7FFF);
    exit(code);
}

static void error (long code)
{
    for(int i = 0; i < sizeof(errs); ++i)
    {
        if(errs[i].code == code) {
            printf("E %X: %s\n", errs[i].code, errs[i].msg);
            return;
        }
    }
    fatal(0x040F7FFF);
}

static void error_hint(long code, const char *inst)
{
    printf("When parsing %s:\n", inst);
    error(code);
}

#endif
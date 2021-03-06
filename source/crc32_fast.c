/* Standard CRC32 checksum: fast public domain implementation for
 * little-endian architectures.  Written for compilation with an
 * optimizer set to perform loop unwinding.  Outputs the checksum for
 * each file given as a command line argument.  Invalid file names and
 * files that cause errors are silently skipped.  The program reads
 * from stdin if it is called with no arguments. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "crc32_fast.h"

u32 crc32_for_byte(u32 r)
{
    for(int j = 0; j < 8; ++j) r = (r & 1? 0: (u32)0xEDB88320L) ^ r >> 1;
    return r ^ (u32)0xFF000000L;
}

/* Any unsigned integer type with at least 32 bits may be used as
 * accumulator type for fast crc32-calulation, but unsigned long is
 * probably the optimal choice for most systems. */
typedef unsigned long accum_t;

void init_tables(u32* table, u32* wtable)
{
    for(u64 i = 0; i < 0x100; ++i) table[i] = crc32_for_byte(i);
    
    for(u64 k = 0; k < sizeof(accum_t); ++k)
    {
        for(u64 w, i = 0; i < 0x100; ++i)
        {
            for(u64 j = w = 0; j < sizeof(accum_t); ++j) w = table[(u8)(j == k? w ^ i: w)] ^ w >> 8;
            wtable[(k << 8) + i] = w ^ (k? wtable[0]: 0);
        }
    }
}

void crc32(const void* data, u64 n_bytes, u32* crc)
{
    static u32 table[0x100], wtable[0x100*sizeof(accum_t)];
    u64 n_accum = n_bytes / sizeof(accum_t);
    
    if (!*table) init_tables(table, wtable);
    for(u64 i = 0; i < n_accum; ++i)
    {
        accum_t a = *crc ^ ((accum_t*)data)[i];
        for(u64 j = *crc = 0; j < sizeof(accum_t); ++j) *crc ^= wtable[(j << 8) + (u8)(a >> 8*j)];
    }
    
    for(u64 i = n_accum*sizeof(accum_t); i < n_bytes; ++i) *crc = table[(u8)*crc ^ ((u8*)data)[i]] ^ *crc >> 8;
}

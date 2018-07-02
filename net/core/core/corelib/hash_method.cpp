#include "hash_method.h"
/* -------------------------------------------------------------------------
    Name:       djb_hash.cpp
    Title:      Daniel J. Bernstein's Hash Function
    Package:    Server Platform Library Project

    Written:    Eric Lee <hahalee@163.com>

    $Id: djb_hash.cpp,v 1.2 2003/10/28 10:47:55 lch Exp $
   ------------------------------------------------------------------------- */


#include		<string.h>

//static char rcsid[] =  "$Id: djb_hash.cpp,v 1.2 2003/10/28 10:47:55 lch Exp $";

/* -------------------------------------------------------------------------
    This is Daniel J. Bernstein's popular `times 33' hash function as
    posted by him years ago on comp.lang.c. It basically uses a function
    like ``hash(i) = hash(i-1) * 33 + string[i]''. This is one of the
    best hashing functions for strings. Because it is both computed very
    fast and distributes very well.
   ------------------------------------------------------------------------- */
uint32_t djb_hash(const char *key, size_t len)
{
    uint32_t hash = 5381;

    /* the hash unrolled eight times */
    for (; len >= 8; len -= 8) {
        hash = ((hash << 5) + hash) + (uint8_t)*key++;
        hash = ((hash << 5) + hash) + (uint8_t)*key++;
        hash = ((hash << 5) + hash) + (uint8_t)*key++;
        hash = ((hash << 5) + hash) + (uint8_t)*key++;
        hash = ((hash << 5) + hash) + (uint8_t)*key++;
        hash = ((hash << 5) + hash) + (uint8_t)*key++;
        hash = ((hash << 5) + hash) + (uint8_t)*key++;
        hash = ((hash << 5) + hash) + (uint8_t)*key++;
    }
    switch (len) {
        case 7: hash = ((hash << 5) + hash) + (uint8_t)*key++; // fallthrough...
        case 6: hash = ((hash << 5) + hash) + (uint8_t)*key++; // fallthrough...
        case 5: hash = ((hash << 5) + hash) + (uint8_t)*key++; // fallthrough...
        case 4: hash = ((hash << 5) + hash) + (uint8_t)*key++; // fallthrough...
        case 3: hash = ((hash << 5) + hash) + (uint8_t)*key++; // fallthrough...
        case 2: hash = ((hash << 5) + hash) + (uint8_t)*key++; // fallthrough...
        case 1: hash = ((hash << 5) + hash) + (uint8_t)*key++; break;
        default: /* case 0: */ break;
    }
    return hash;
}

/*
    vim: set et ts=4 sts=4 syn=cpp :
 */

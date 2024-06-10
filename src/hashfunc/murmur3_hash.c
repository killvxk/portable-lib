/* vim: expandtab:sw=4:ts=4:notextmode:tw=82:
 *
 * murmur3_hash.c - Austin Appleby's MurMurHash3 implementation.
 *
 * Licensing Terms: GPLv2
 *
 * If you need a commercial license for this work, please contact
 * the author.
 *
 * This software does not come with any express or implied
 * warranty; it is provided "as is". No claim  is made to its
 * suitability for any purpose.
 *
 * This is Austin Appleby's super fast MurMurHash3 implementation.
 *  http://code.google.com/p/smhasher/
 *
 * The original code is written in C++ and is placed in the public
 * domain.
 *
 * C translation is (c) Sudhi Herle <sw at herle.net> and is
 * licensed under GPLv2
 *
 * Synopsis:
 *     - Faster than Hsieh Hash, with better distribution (mixing)
 *     - Comes in 3 variants:
 *          o 32-bit for use in hash tables
 *          o 128-bit for use in 32-bit machines
 *          o 128-bit for use in 64-bit machines
 *       The 128-bit variants can be used for generating unique
 *       identifiers from long blocks of data
 */


/*
 * MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code.
 *
 * Note - The x86 and x64 versions do _not_ produce the same results, as the
 * algorithms are optimized for their respective platforms. You can still
 * compile and run any of them on any platform, but your performance with the
 * non-native version will be less than optimal.
 *
 * The C implementation is written by Sudhi Herle and is licensed
 * under GPLv2.
 */

#include "utils/hashfunc.h"


#if defined(_MSC_VER)
#define FORCE_INLINE    __forceinline
#include <stdlib.h>
#define ROTL32(x,y) _rotl(x,y)
#define ROTL64(x,y) _rotl64(x,y)

#define BIG_CONSTANT(x) (x)

#else   // defined(_MSC_VER)
#define FORCE_INLINE inline
static inline uint32_t
rotl32(uint32_t x, int8_t r)
{
  return (x << r) | (x >> (32 - r));
}

static inline uint64_t
rotl64(uint64_t x, int8_t r)
{
  return (x << r) | (x >> (64 - r));
}

#define ROTL32(x,y) rotl32(x,y)
#define ROTL64(x,y) rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

#if 0
//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

FORCE_INLINE uint32_t getblock ( const uint32_t * p, int i )
{
  return p[i];
}

FORCE_INLINE uint64_t getblock ( const uint64_t * p, int i )
{
  return p[i];
}
#else

#define getblock(p, i)      p[i]

#endif

/* Finalization mix - force all bits of a hash block to avalanche */

static FORCE_INLINE uint32_t
fmix(uint32_t h)
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}


static FORCE_INLINE uint64_t
fmix64(uint64_t k)
{
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xff51afd7ed558ccd);
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
  k ^= k >> 33;

  return k;
}



/*
 * 32 bit hash func for use in hash tables.
 */
uint32_t
murmur3_hash_32(const void* key, size_t len, uint32_t seed)
{
  const uint8_t * data = (const uint8_t*)key;
  const int nblocks    = len / 4;

  uint32_t k1 = 0;
  uint32_t h1 = seed;
  uint32_t c1 = 0xcc9e2d51;
  uint32_t c2 = 0x1b873593;

  const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);
  const uint8_t  * tail   = (const uint8_t *) (data + nblocks*4);
  int i;

  for(i = -nblocks; i; i++)
  {
      k1 = getblock(blocks,i);

      k1 *= c1;
      k1 = ROTL32(k1,15);
      k1 *= c2;

      h1 ^= k1;
      h1 = ROTL32(h1,13);
      h1 = h1*5+0xe6546b64;
  }

  /* tail */


  switch(len & 3)
  {
      case 3: k1 ^= tail[2] << 16; // fallthrough
      case 2: k1 ^= tail[1] << 8; // fallthrough
      case 1: k1 ^= tail[0];
              k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
  };

  /* finalization */

  h1 ^= len;

  return fmix(h1);
}



/*
 * 128-bit hash value for 32-bit arch. This is good for representing
 * long blocks of data with a unique value.
 */
uint_128_t
murmur3_hash_128(const void* key, const size_t len, uint32_t seed)
{
    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 16;

    uint32_t h1 = seed;
    uint32_t h2 = seed;
    uint32_t h3 = seed;
    uint32_t h4 = seed;

    uint32_t k1 = 0;
    uint32_t k2 = 0;
    uint32_t k3 = 0;
    uint32_t k4 = 0;

    uint32_t c1 = 0x239b961b;
    uint32_t c2 = 0xab0e9789;
    uint32_t c3 = 0x38b34ae5;
    uint32_t c4 = 0xa1e38b93;
    const uint32_t * blocks = (const uint32_t *)(data + nblocks*16);
    const uint8_t * tail    = (const uint8_t*)(data + nblocks*16);
    int i;


    for(i = -nblocks; i; i++)
    {
        k1 = getblock(blocks,i*4+0);
        k2 = getblock(blocks,i*4+1);
        k3 = getblock(blocks,i*4+2);
        k4 = getblock(blocks,i*4+3);

        k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1;

        h1 = ROTL32(h1,19); h1 += h2; h1 = h1*5+0x561ccd1b;

        k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2;

        h2 = ROTL32(h2,17); h2 += h3; h2 = h2*5+0x0bcaa747;

        k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3;

        h3 = ROTL32(h3,15); h3 += h4; h3 = h3*5+0x96cd1c35;

        k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4;

        h4 = ROTL32(h4,13); h4 += h1; h4 = h4*5+0x32ac3b17;
    }

    /* tail */


    switch(len & 15)
    {
        case 15: k4 ^= tail[14] << 16; // fallthrough
        case 14: k4 ^= tail[13] << 8; // fallthrough
        case 13: k4 ^= tail[12] << 0;
                 k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4; // fallthrough

        case 12: k3 ^= tail[11] << 24; // fallthrough
        case 11: k3 ^= tail[10] << 16; // fallthrough
        case 10: k3 ^= tail[ 9] << 8; // fallthrough
        case  9: k3 ^= tail[ 8] << 0;
                 k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3; // fallthrough

        case  8: k2 ^= tail[ 7] << 24; // fallthrough
        case  7: k2 ^= tail[ 6] << 16; // fallthrough
        case  6: k2 ^= tail[ 5] << 8; // fallthrough
        case  5: k2 ^= tail[ 4] << 0;
                 k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2; // fallthrough

        case  4: k1 ^= tail[ 3] << 24; // fallthrough
        case  3: k1 ^= tail[ 2] << 16; // fallthrough
        case  2: k1 ^= tail[ 1] << 8; // fallthrough
        case  1: k1 ^= tail[ 0] << 0;
                 k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1; // fallthrough
    };

    /* finalization */

    h1 ^= len; h2 ^= len; h3 ^= len; h4 ^= len;

    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    h1 = fmix(h1);
    h2 = fmix(h2);
    h3 = fmix(h3);
    h4 = fmix(h4);

    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    {
        uint_128_t r;

        r.v[0] = h1; r.v[0] <<= 32; r.v[0] |= h2;
        r.v[1] = h1; r.v[1] <<= 32; r.v[1] |= h2;
        return r;
    }
}



/*
 * 128-bit hash value for 64-bit arch. This is good for representing
 * long blocks of data with a unique value.
 */
uint_128_t
murmur3_hash64_128(const void* key, size_t len, uint32_t seed)
{
    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 16;

    uint64_t h1 = seed;
    uint64_t h2 = seed;

    uint64_t k1 = 0;
    uint64_t k2 = 0;

    uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
    uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);

    const uint64_t * blocks = (const uint64_t *)(data);
    const uint8_t * tail    = (const uint8_t*)(data + nblocks*16);
    int i;

    for(i = 0; i < nblocks; i++)
    {
        k1 = getblock(blocks,i*2+0);
        k2 = getblock(blocks,i*2+1);

        k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;

        h1 = ROTL64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;

        k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;

        h2 = ROTL64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
    }

    /* tail */

    switch(len & 15)
    {
        case 15: k2 ^= ((uint64_t)(tail[14])) << 48; // fallthrough
        case 14: k2 ^= ((uint64_t)(tail[13])) << 40; // fallthrough
        case 13: k2 ^= ((uint64_t)(tail[12])) << 32; // fallthrough
        case 12: k2 ^= ((uint64_t)(tail[11])) << 24; // fallthrough
        case 11: k2 ^= ((uint64_t)(tail[10])) << 16; // fallthrough
        case 10: k2 ^= ((uint64_t)(tail[ 9])) << 8; // fallthrough
        case  9: k2 ^= ((uint64_t)(tail[ 8])) << 0;
                 k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2; // fallthrough

        case  8: k1 ^= ((uint64_t)(tail[ 7])) << 56; // fallthrough
        case  7: k1 ^= ((uint64_t)(tail[ 6])) << 48; // fallthrough
        case  6: k1 ^= ((uint64_t)(tail[ 5])) << 40; // fallthrough
        case  5: k1 ^= ((uint64_t)(tail[ 4])) << 32; // fallthrough
        case  4: k1 ^= ((uint64_t)(tail[ 3])) << 24; // fallthrough
        case  3: k1 ^= ((uint64_t)(tail[ 2])) << 16; // fallthrough
        case  2: k1 ^= ((uint64_t)(tail[ 1])) << 8; // fallthrough
        case  1: k1 ^= ((uint64_t)(tail[ 0])) << 0;
                 k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1; // fallthrough
    };

    /* finalization */

    h1 ^= len; h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 = fmix64(h1);
    h2 = fmix64(h2);

    h1 += h2;
    h2 += h1;

    {
        uint_128_t r;

        r.v[0] = h1;
        r.v[1] = h2;
        return r;
    }
}

/* EOF */

#ifndef RNG_H_INCLUDED
#define RNG_H_INCLUDED

#include "util.h" // SWAP

// RNG functions
static struct TINYMT64_T mt;
#define rand64() tinymt64_generate_uint64(&mt)
#define srand64(seed) tinymt64_init(&mt, seed)
void shuffle_u8(uint8_t *array, size_t n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand64() / (UINT64_MAX / (n - i) + 1);
          SWAP(array[j], array[i], uint8_t);
        }
    }
}
void shuffle_u8_pairs(uint8_t *array, size_t n/*pairs*/)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand64() / (UINT64_MAX / (n - i) + 1);
          SWAP(array[j*2],   array[i*2],   uint8_t);
          SWAP(array[j*2+1], array[i*2+1], uint8_t);
        }
    }
}
void shuffle_u16(uint16_t *array, size_t n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand64() / (UINT64_MAX / (n - i) + 1);
          SWAP(array[j], array[i], uint16_t);
        }
    }
}
uint8_t rand_u8(uint8_t min, uint8_t max)
{
    if (max<=min) { rand64(); return min; }
    return (min+(uint8_t)(rand64()%(max-min+1)));
}
uint8_t rand_u8_except(uint8_t min, uint8_t max, uint8_t except)
{
    uint8_t res = rand_u8(min, max-1);
    if (res>=except) res++;
    return res;
}
uint8_t rand_amount(uint8_t min, uint8_t max, int cur_minus_expected_times_100)
{
    // cur_minus_expected >0: decrease max
    // cur_minus_expected <0: increase min
    // NOTE: a probability curve would better than just setting limits here.
    if (cur_minus_expected_times_100<-300) return rand_u8(min+1, max);
    if (cur_minus_expected_times_100> 300) return rand_u8(min, max-1);
    return rand_u8(min, max);
}
uint16_t rand_u16(uint16_t min, uint16_t max)
{
    if (max<=min) return min;
    return (min+(uint16_t)(rand64()%(max-min+1)));
}

#endif // RNG_H_INCLUDED

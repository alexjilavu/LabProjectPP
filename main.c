#include <stdio.h>

void xorShift(unsigned int seed, int size)
{
    unsigned int r = seed;
    int k;
    for(k = 0; k < size; k++)
    {
        r = r ^ r << 13;
        r = r ^ r >> 17;
        r = r ^ r << 5;
        printf("%u\n", r);
    }
}

int main()
{
    xorShift(1003210, 100);
    return 0;
}
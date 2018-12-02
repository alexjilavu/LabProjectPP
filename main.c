#include <stdio.h>
#include <stdlib.h>
// definim o structura de date care permite memorarea unui pixel

typedef struct pixel
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};


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

struct pixel* loadBMP(char *fileName, struct pixel *p) {
    FILE *filePointer;
    filePointer = fopen(fileName, "rb");
    if (filePointer == NULL) {
        printf("There was an error while opening the file");
        return NULL;
    }
    printf("AM AJUNS");
    unsigned int imageWidth, imageHeight;
    unsigned int padding, i, j;
    unsigned char curentByte;

    // citim cele doua dimensiuni ale imaginii din header
    printf("AM AJUNS");
    fseek(filePointer, 18, SEEK_SET);
    printf("AM AJUNS");
    fread(&imageWidth, sizeof(unsigned int), 1, filePointer);
    fread(&imageHeight, sizeof(unsigned int), 1, filePointer);
    printf("%d %d", imageWidth, imageHeight);
    // verificam daca exista biti pentru padding

    if (imageWidth % 4 == 0)
        padding = 0;
    else
        padding = 4 - (imageWidth % 4);

    // trecem de primii 54 biti de header

    fseek(filePointer, 54, SEEK_SET);
    // declaram tabloul in care vom stoca imaginea liniarizata

    p = malloc(imageHeight * imageWidth * sizeof(struct pixel));

    for (i = 0; i < imageHeight; i++) {
        for (j = 0; j < imageWidth; j++) {
            fread(&p[i * j].b, sizeof(unsigned char), 1, filePointer);
            fread(&p[i * j].g, sizeof(unsigned char), 1, filePointer);
            fread(&p[i * j].r, sizeof(unsigned char), 1, filePointer);
        }
        fseek(filePointer, padding, SEEK_CUR);
    }
    return p;
}
int main()
{
    //xorShift(1003210, 100);
    struct pixel *p = loadBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\peppers.bmp", p);
    return 0;
}
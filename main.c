#include <stdio.h>
#include <stdlib.h>
// definim o structura de date care permite memorarea unui pixel

struct pixel
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct image
{
    unsigned char *header;
    struct pixel *content;
    unsigned int width, height, padding;
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
struct image loadBmp(char *fileName)
{
    FILE *filePointer;
    filePointer = fopen(fileName, "rb");
    if(filePointer == NULL) {
        printf("There was an error while opening the file");
    }
    int i, j;
    struct image image;
    image.header = malloc(54 * sizeof(unsigned char));
    fread(&image.header[i], sizeof(unsigned char), 53, filePointer);

    fseek(filePointer, 18, SEEK_SET);

    fread(&image.width, sizeof(unsigned int), 1, filePointer);
    fread(&image.height, sizeof(unsigned int), 1, filePointer);
    if(image.width % 4 == 0)
        image.padding = 0;
    else
        image.padding = 4 - (image.width % 4);

    fseek(filePointer, 54, SEEK_SET);
    image.content = malloc(image.width * image.height * sizeof(struct pixel));

    for(i = 0; i < image.height; i++) {
        for (j = 0; j < image.width; j++) {
            fread(&image.content[i * j].b, sizeof(unsigned char), 1, filePointer);
            fread(&image.content[i * j].g, sizeof(unsigned char), 1, filePointer);
            fread(&image.content[i * j].r, sizeof(unsigned char), 1, filePointer);
        }
        fseek(filePointer, image.padding, SEEK_CUR);
    }
    return image;

}



int main()
{
    int imageWidth, imageHeight;
    struct image image = loadBmp("E:\\INFO\\FMI\\ProgProced\\ProectLab\\peppers.bmp");
    printf("%d %d", image.width, image.height);


    return 0;
}
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
struct image loadBMP(char *fileName)
{
    FILE *filePointer;
    filePointer = fopen(fileName, "rb");
    struct image nul;
    nul.width = nul.height = 0;
    if(filePointer == NULL) {
        printf("There was an error while opening the file");
        return nul;
    }
    int i, j;
    struct image image;
    image.header = malloc(54 * sizeof(unsigned char));
    for(i = 0; i < 53; i++)
        fread(&image.header[i], sizeof(unsigned char), 1, filePointer);

    fseek(filePointer, 18, SEEK_SET);

    fread(&image.width, sizeof(unsigned int), 1, filePointer);
    fread(&image.height, sizeof(unsigned int), 1, filePointer);
    printf("%d %d", image.width, image.height);
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
    fclose(filePointer);
    return image;

}

void createBMP(char *fileName, struct image image)
{
    FILE* filePointer;
    filePointer = fopen(fileName, "wb");
    if(filePointer == NULL){
        printf("There was an error while opening the file");
        return ;}
    int i, j;
    for(i = 0; i < 54; i++)
        fwrite(&image.header[i], sizeof(unsigned char), 1, filePointer);
    for(i = 0; i < image.height; i++){
        for(j = 0; j < image.width; j++){
            fwrite(&image.content[i * j].b, sizeof(unsigned char), 1, filePointer);
            fwrite(&image.content[i * j].g, sizeof(unsigned char), 1, filePointer);
            fwrite(&image.content[i * j].r, sizeof(unsigned char), 1, filePointer);
        }
        fwrite(0, sizeof(unsigned char), image.padding, filePointer);
    }
    fclose(filePointer);
}


int main()
{
    struct image image = loadBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\peppers.bmp");
    createBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\testpeppers.bmp", image);


    return 0;
}
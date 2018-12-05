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
    unsigned int width, height, padding, contentSize;
};

struct array
{
    unsigned int *randSequence, sizeOfSeq;
};

void swap (int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

struct array xorShift(unsigned int seed, int size)
{
    unsigned int r = seed;
    int k;
    struct array rand;
    rand.randSequence = malloc(size * sizeof(unsigned int));
    rand.sizeOfSeq = 0;
    for(k = 0; k < size; k++)
    {
        r = r ^ r << 13;
        r = r ^ r >> 17;
        r = r ^ r << 5;
        rand.randSequence[rand.sizeOfSeq] = r;
        rand.sizeOfSeq++;
    }
    return rand;
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
    for(i = 0; i < 54; i++)
        fread(&image.header[i], sizeof(unsigned char), 1, filePointer);

    fseek(filePointer, 18, SEEK_SET);

    fread(&image.width, sizeof(unsigned int), 1, filePointer);
    fread(&image.height, sizeof(unsigned int), 1, filePointer);
    if(image.width % 4 == 0)
        image.padding = 0;
    else
        image.padding = 4 - (image.width % 4);

    fseek(filePointer, 54, SEEK_SET);
    image.content = malloc(image.width * image.height * sizeof(struct pixel));

    image.contentSize = 0;
    for(i = 0; i < image.height; i++) {
        for (j = 0; j < image.width; j++) {
            fread(&image.content[image.contentSize].b, sizeof(unsigned char), 1, filePointer);
            fread(&image.content[image.contentSize].g, sizeof(unsigned char), 1, filePointer);
            fread(&image.content[image.contentSize].r, sizeof(unsigned char), 1, filePointer);
            image.contentSize++;
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
    int curentPosition = 0;
    for(i = 0; i < image.height; i++){
        for(j = 0; j < image.width; j++){
            fwrite(&image.content[curentPosition].b, sizeof(unsigned char), 1, filePointer);
            fwrite(&image.content[curentPosition].g, sizeof(unsigned char), 1, filePointer);
            fwrite(&image.content[curentPosition].r, sizeof(unsigned char), 1, filePointer);
            curentPosition++;
        }
        fwrite(0, sizeof(unsigned char), image.padding, filePointer);
    }
    fclose(filePointer);
}

void cripBMP(char *fileSource, char *fileDestination, char *fileKey)
{
    FILE* sourcePointer = fopen(fileSource, "rb");
    if(sourcePointer == NULL)
    {
        printf("Calea imaginii sursa este incorecta");
        return ;
    }
    FILE* destinationPointer = fopen(fileDestination, "wb");
    if(destinationPointer == NULL)
    {
        printf("Calea imaginii destinatie este incorecta");
        return ;
    }
    FILE* fin = fopen(fileKey, "r");
    if(fin == NULL)
    {
        printf("Calea fisierului in care se afla cheia este incorecta");
        return ;
    }
    struct image image = loadBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\peppers.bmp");
    unsigned int randomSeed, startingValue;
    fscanf(fin, "%d %d", &randomSeed, &startingValue);
    struct array rand = xorShift(randomSeed, image.width * image.height * 2);
    int i, j = 0, k;
    struct image criptedImage = image;
    criptedImage.content = malloc(criptedImage.width * criptedImage.height * sizeof(struct pixel));
    struct array perm;
    perm.sizeOfSeq = rand.sizeOfSeq;
    perm.randSequence = malloc(image.height * image.width * sizeof(unsigned int));
    for(i = 0; i < image.width * image.height; i++)
        perm.randSequence[i] = i;
    for(i = image.height * image.width - 1; i > 0; i--)
    {
        j = rand.randSequence[i] % (i + 1);
        swap(&perm.randSequence[i], &perm.randSequence[j]);
    }

    for(k = 0; k < image.width * image.height - 1; k++)
    {
        criptedImage.content[perm.randSequence[k]] = image.content[k];
    }
    union{
        int x;
        unsigned char byteRepr[4];
    } lastKey, random, curentPerm;
    lastKey.x = startingValue;
    random.x = rand.randSequence[criptedImage.height * criptedImage.width];
    curentPerm.x = perm.randSequence[0];
    criptedImage.content[0].r = lastKey.byteRepr[2] ^ curentPerm.byteRepr[2] ^ random.byteRepr[2];
    criptedImage.content[0].g = lastKey.byteRepr[1] ^ curentPerm.byteRepr[1] ^ random.byteRepr[1];
    criptedImage.content[0].b = lastKey.byteRepr[0] ^ curentPerm.byteRepr[0] ^ random.byteRepr[0];

    for(k = 1; k < criptedImage.width * criptedImage.height - 1; k++)
    {
        curentPerm.x = perm.randSequence[k];
        random.x = rand.randSequence[criptedImage.height * criptedImage.width + k];
        criptedImage.content[k].r = criptedImage.content[k - 1].r ^ curentPerm.byteRepr[2] ^ random.byteRepr[2];
        criptedImage.content[k].g = criptedImage.content[k - 1].g ^ curentPerm.byteRepr[1] ^ random.byteRepr[1];
        criptedImage.content[k].b = criptedImage.content[k - 1].b ^ curentPerm.byteRepr[0] ^ random.byteRepr[0];
    }

    createBMP(fileDestination, criptedImage);

}


int main()
{
    //struct image image = loadBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\peppers.bmp");
    //createBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\testpeppers.bmp", image);
    //struct random rand = xorShift(1000, 1000);
    cripBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\peppers.bmp",
            "E:\\INFO\\FMI\\ProgProced\\ProiectLab\\cript.bmp",
            "E:\\INFO\\FMI\\ProgProced\\ProiectLab\\secret_key.txt");
    return 0;
}
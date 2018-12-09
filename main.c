#include <stdio.h>
#include <stdlib.h>
// definim o structura de date care permite memorarea unui pixel

struct pixel
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};
union binarInt{
    int x;
    unsigned char byteRepr[4];
};

struct image
{
    unsigned char *header;
    struct pixel *content;
    unsigned int width, height, padding, contentSize;
};

void swap (int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

unsigned int* xorShift(int seed, int size)
{
    unsigned int r = seed;
    int k;
    unsigned int* randomSequence = malloc(size * sizeof(unsigned int));
    int sizeOfSeq = 1;
    randomSequence[0] = seed;
    for(k = 1; k < size; k++)
    {
        r = r ^ r << 13;
        r = r ^ r >> 17;
        r = r ^ r << 5;
        randomSequence[sizeOfSeq] = r;
        sizeOfSeq++;
    }

    return randomSequence;
}
struct image loadBMP(char *fileName)
{
    FILE *filePointer;
    filePointer = fopen(fileName, "rb");
    struct image nul;
    nul.width = nul.height = 0;
    if(filePointer == NULL) {
        printf("There was an error while opening the file");
        fclose(filePointer);
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
        fclose(filePointer);
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
void criptBMP(char *fileSource, char *fileDestination, char *fileKey)
{
    FILE *fin = fopen(fileKey, "r");
    if(fin == NULL)
    {
        printf("A aparut o eroare la deschiderea fisierului cu cheia");
        fclose(fin);
        return ;
    }
    int randomSeed, startingValue;
    fscanf(fin, "%d %d", &randomSeed, &startingValue);
    fclose(fin);
    struct image image = loadBMP(fileSource);
    struct image cripImage = image;
    int pixelSize = image.width * image.height;
    cripImage.content = malloc(pixelSize * sizeof(struct pixel));
    unsigned int* randomSequence = xorShift(randomSeed, pixelSize * 2);
    unsigned int* permutation = malloc(pixelSize * sizeof(unsigned int));

    int i, j;
    for(i = 0; i < pixelSize; i++)
        permutation[i] = i;
    unsigned int aux;
    for(i = pixelSize - 1; i > 0; i--)
    {
        j = randomSequence[i] % (i + 1);
        swap(&permutation[i], &permutation[j]);
    }
    for(i = 0; i < pixelSize; i++)
        cripImage.content[permutation[i]] = image.content[i];
    //createBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\cript.bmp", cripImage);
    union binarInt key, random;
    key.x = startingValue;
    random.x = randomSequence[pixelSize];
    cripImage.content[0].r = key.byteRepr[2] ^ cripImage.content[0].r ^ random.byteRepr[2];
    cripImage.content[0].g = key.byteRepr[1] ^ cripImage.content[0].g ^ random.byteRepr[1];
    cripImage.content[0].b = key.byteRepr[0] ^ cripImage.content[0].b ^ random.byteRepr[0];
    for(i = 1; i < pixelSize; i++)
    {
        random.x = randomSequence[pixelSize + i];
        cripImage.content[i].r = cripImage.content[i - 1].r ^ cripImage.content[i].r ^ random.byteRepr[2];
        cripImage.content[i].g = cripImage.content[i - 1].g ^ cripImage.content[i].g ^ random.byteRepr[1];
        cripImage.content[i].b = cripImage.content[i - 1].b ^ cripImage.content[i].b ^ random.byteRepr[0];
    }
    createBMP(fileDestination, cripImage);
    free(cripImage.content);
    free(image.content);
    free(randomSequence);
    free(permutation);
}

void decriptBMP(char* fileSource, char* fileCripted, char* fileKey)
{
    FILE* fin = fopen(fileKey, "r");
    if(fin == NULL)
    {
        printf("Eroare la deschiderea fisierului cu cheia secreta");
        fclose(fin);
        return ;
    }
    struct image criptedImage = loadBMP(fileCripted);
    struct image finalImage = criptedImage;
    int startingValue, randomSeed;
    int pixelSize = criptedImage.width * criptedImage.height;
    finalImage.content = malloc(pixelSize * sizeof(struct pixel));
    fscanf(fin, "%d %d", &randomSeed, &startingValue);
    fclose(fin);
    unsigned int *randomSequence = xorShift(randomSeed, 2 * pixelSize);
    int i, j;
    unsigned int *permutation = malloc(pixelSize * sizeof(unsigned int));
    for(i = 0; i < pixelSize; i++)
        permutation[i] = i;
    for(i = pixelSize - 1; i > 0; i--)
    {
        j = randomSequence[i] % (i + 1);
        swap(&permutation[i], &permutation[j]);
    }
    unsigned int* inversPerm = malloc(pixelSize * sizeof(unsigned int));

    for(i = 0; i < pixelSize; i++)
        inversPerm[permutation[i]] = i;
    union{
        int x;
        unsigned char byteRepr[4];
    }key, random;
    key.x = startingValue;
    random.x = randomSequence[pixelSize];
    finalImage.content[0].r = key.byteRepr[2] ^ criptedImage.content[0].r ^ random.byteRepr[2];
    finalImage.content[0].g = key.byteRepr[1] ^ criptedImage.content[0].g ^ random.byteRepr[1];
    finalImage.content[0].b = key.byteRepr[0] ^ criptedImage.content[0].b ^ random.byteRepr[0];
    int k;
    for(k = 1; k < pixelSize; k++)
    {
        random.x = randomSequence[pixelSize + k];
        finalImage.content[k].r = criptedImage.content[k].r ^ criptedImage.content[k - 1].r ^ random.byteRepr[2];
        finalImage.content[k].g = criptedImage.content[k].g ^ criptedImage.content[k - 1].g ^ random.byteRepr[1];
        finalImage.content[k].b = criptedImage.content[k].b ^ criptedImage.content[k - 1].b ^ random.byteRepr[0];
    }
    struct image decriptedImage = criptedImage;
    decriptedImage.content = malloc(pixelSize * sizeof(struct pixel));
    for(i = 0; i < pixelSize; i++)
        decriptedImage.content[inversPerm[i]] = finalImage.content[i];
    createBMP(fileSource, decriptedImage);
    free(finalImage.content);
    free(inversPerm);
    free(permutation);
    free(criptedImage.content);
    free(decriptedImage.content);
}

void chiSquaredValues(char* filePath) {
    struct image image = loadBMP(filePath);
    int pixelSize = image.width * image.height;
    float fValue = (image.width * image.height) / 256.0;
    printf("%f\n", fValue);
    float *frecvRed, *frecvGreen, *frecvBlue;
    frecvRed = malloc(256 * sizeof(float));
    frecvBlue = malloc(256 * sizeof(float));
    frecvGreen = malloc(256 * sizeof(float));
    int i, j;
    for (i = 0; i < pixelSize; i++) {
        frecvRed[image.content[i].r]++;
        frecvGreen[image.content[i].g]++;
        frecvBlue[image.content[i].b]++;
    }
    float chiRed = 0, chiGreen = 0, chiBlue = 0;
    for (i = 0; i < 256; i++) {
        chiRed += ((frecvRed[i] - fValue) * (frecvRed[i] - fValue)) / fValue;
        chiGreen += ((frecvGreen[i] - fValue) * (frecvGreen[i] - fValue)) / fValue;
        chiBlue += ((frecvBlue[i] - fValue) * (frecvBlue[i] - fValue)) / fValue;
    }
    printf("chi-squared-red = %.2f \n chi-squared-green = %.2f \n chi-squared-blue = %.2f", chiRed, chiGreen, chiBlue);
}
int main()
{
    struct image image = loadBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\peppers.bmp");

    criptBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\peppers.bmp",
            "E:\\INFO\\FMI\\ProgProced\\ProiectLab\\cript.bmp",
            "E:\\INFO\\FMI\\ProgProced\\ProiectLab\\secret_key.txt");

     decriptBMP("E:\\INFO\\FMI\\ProgProc""d\\ProiectLab\\DecriptPeppers.bmp",
            "E:\\INFO\\FMI\\ProgProced\\ProiectLab\\cript.bmp",
            "E:\\INFO\\FMI\\ProgProced\\ProiectLab\\secret_key.txt");
    chiSquaredValues("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\enc_peppers_ok.bmp");
    return 0;
}
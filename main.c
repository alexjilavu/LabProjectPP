#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

struct pairPoint
{
    int x, y;
    float corelation;
    int nrSablon;
};

struct arrayPairPoint
{
    struct pairPoint* arr;
    int size;
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
        image.padding = 4 - (3 * image.width) % 4;
    fflush(filePointer);
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
    fflush(filePointer);
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
    unsigned char pad = 0;
    for(i = 0; i < image.height; i++){
        for(j = 0; j < image.width; j++){
            fwrite(&image.content[curentPosition].b, sizeof(unsigned char), 1, filePointer);
            fwrite(&image.content[curentPosition].g, sizeof(unsigned char), 1, filePointer);
            fwrite(&image.content[curentPosition].r, sizeof(unsigned char), 1, filePointer);
            curentPosition++;
        }
        fwrite(&pad, sizeof(unsigned char), image.padding, filePointer);
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
    float *frecvRed, *frecvGreen, *frecvBlue;
    frecvRed = malloc(256 * sizeof(float));
    frecvBlue = malloc(256 * sizeof(float));
    frecvGreen = malloc(256 * sizeof(float));
    int i;
    for(i = 0; i < 256; i++)
    {
        frecvBlue[i] = 0;
        frecvGreen[i] = 0;
        frecvRed[i] = 0;
    }
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
    free(frecvBlue);
    free(frecvRed);
    free(frecvGreen);
    printf("chi-squared-red = %.2f \n chi-squared-green = %.2f \n chi-squared-blue = %.2f", chiRed, chiGreen, chiBlue);
}

float corr(int x, int y, float SPrime, float deviationS, struct image sablon, struct image image)
{
    int i, j;
    if((x + sablon.height > image.height) || (y + sablon.width > image.width))
        return -1;
    float fIPrime = 0, corelation = 0;
    for(i = x; i < x + sablon.height; i++)
        for(j = y; j < y + sablon.width; j++) {
            fIPrime += image.content[i * image.width + j].r;
        }
    fIPrime = fIPrime / (sablon.height * sablon.width);
    float deviationFI = 0;
    for(i = x; i < x + sablon.height; i++)
        for(j = y; j < y + sablon.width; j++) {
            deviationFI += (image.content[i * image.width + j].r - fIPrime) * (image.content[i * image.width + j].r - fIPrime);
        }
    deviationFI = sqrtf(deviationFI / (sablon.height * sablon.width - 1));
    int curentImagePoz = 0, curentSablonPoz = 0;
    for(i = 0; i < sablon.height; i++)
        for(j = 0; j < sablon.width; j++)
        {
            curentImagePoz = (x + i) * image.width + y + j;
            curentSablonPoz = i * sablon.width + j;
            corelation += ((image.content[curentImagePoz].r - fIPrime) * (sablon.content[curentSablonPoz].r - SPrime))
                          / (deviationFI * deviationS);
        }
    float n = sablon.width * sablon.height;
    corelation = corelation / n;
    return corelation;
}
struct image grayscale(char* fileSource)
{
    struct image image = loadBMP(fileSource);
    int i;
    unsigned char aux;
    for(i = 0; i < image.width * image.height; i++) {
        aux = 0.299 * image.content[i].r + 0.587 * image.content[i].g + 0.114 * image.content[i].b;
        image.content[i].r = image.content[i].g = image.content[i].b = aux;
    }
    return image;
}

struct arrayPairPoint templateMatching(struct image image, struct image sablon, float ps)
{
    int i, j, curent = 0;
    float SPrime = 0;
    // (*) Calculam S barat
    for(i = 0; i < sablon.height; i++)
        for(j = 0; j < sablon.width; j++)
        {
            SPrime += sablon.content[curent].r;
            curent++;
        }
    int pixelDimSablon = sablon.width * sablon.height;
    SPrime = SPrime / pixelDimSablon;
    // (*)

    // (**) Calculam deviatia standard din sablonul S
    double deviationS = 0;
    curent = 0;
    for(i = 0; i < sablon.height; i++)
        for(j = 0; j < sablon.width; j++)
        {
            deviationS += (sablon.content[curent].r - SPrime) * (sablon.content[curent].r - SPrime);
            curent++;
        }
    deviationS = deviationS / (pixelDimSablon - 1);
    deviationS = sqrtf(deviationS);
    // (**)
    curent = 0;
    struct arrayPairPoint result;
    result.arr = malloc(sizeof(struct pairPoint));
    struct pairPoint *aux;
    int x, y, size = 1;
    float curentCorelation;
    for(x = 0; x < image.height; x++)
        for(y = 0; y < image.width; y++) {
            curentCorelation = corr(x, y, SPrime, deviationS, sablon, image);
            if (curentCorelation > ps)
            {
                aux = realloc(result.arr, sizeof(struct pairPoint) * size);
                if(aux == NULL)
                    printf("MEMORIE INSUFICIENTA");
                else
                    result.arr = aux;
                result.arr[size - 1].x = x;
                result.arr[size - 1].y = y;
                result.arr[size - 1].corelation = curentCorelation;
                size++;
            }
        }
    result.size = size;
    return result;
}
struct image drawBorder(struct image image, struct pairPoint f, struct pixel color, struct image sablon)
{
    if(f.x > image.height || f.x > image.width || f.x < 0 || f.y < 0)
        return image;
    if((f.x + sablon.height > image.height) || (f.y + sablon.width) > image.width)
        return image;
    int i, j;
    int curentPoz = f.x * image.width + f.y;
    // Ne deplasam spre dreapta

    for(i = 0; i < sablon.width; i++) {
        image.content[curentPoz] = color;
        curentPoz++;
    }
    // Ne deplasam in sus

    for(j = 0; j < sablon.height; j++)
    {
        image.content[curentPoz] = color;
        curentPoz += image.width;
    }
    // Ne deplasam in stanga

    for(i = 0; i < sablon.width; i++)
    {
        image.content[curentPoz] = color;
        curentPoz--;
    }

    // Ne deplasam in jos

    for(j = 0; j < sablon.height; j++)
    {
        image.content[curentPoz] = color;
        curentPoz -= image.width;
    }
    return image;
}
struct pixel chooseColor(int i)
{
    struct pixel color;
    color.b = color.r = color.g = 0;
    switch (i)
    {
        case 0:
            color.r = 255;
            break;
        case 1:
            color.r = color.g = 255;
            break;
        case 2:
            color.g = 255;
            break;
        case 3:
            color.g = color.b = 255;
            break;
        case 4:
            color.r = color.b = 255;
            break;
        case 5:
            color.b = 255;
            break;
        case 6:
            color.r = color.b = color.g = 192;
            break;
        case 7:
            color.r = 255;
            color.g = 140;
            break;
        case 8:
            color.r = color.b = 128;
            break;
        case 9:
            color.r = 128;
            break;
    }
    return color;
}

struct arrayPairPoint getCorelations(struct image grayscaleImage, const char *sabloane)
{
    FILE* fin = fopen(sabloane, "r");
    struct arrayPairPoint nul;
    if(fin == NULL)
    {
        printf("Fisierul cu sabloane nu a putut fi deschis\n");
        return nul;
    }
    int numarSabloane, i, j;
    char fileName[100];
    struct image sablon;
    struct pixel color;
    struct arrayPairPoint curentArr, corelationTable;
    corelationTable.size = 0;
    corelationTable.arr = malloc(sizeof(struct pairPoint));
    int previousSize = 0, index = 0;
    struct pairPoint *aux;
    fscanf(fin, "%d", &numarSabloane);
    for(i = 0; i < numarSabloane; i++)
    {
        fscanf(fin, "%s", &fileName);
        sablon = grayscale(fileName);
        curentArr = templateMatching(grayscaleImage, sablon, 0.5);
        aux = realloc(corelationTable.arr, sizeof(struct pairPoint) * (curentArr.size + corelationTable.size));
        if (aux == NULL) {
            printf("MEMORIE INSUFICIENTA");
            return nul;
        }
        previousSize = corelationTable.size;
        corelationTable.arr = aux;
        corelationTable.size = previousSize + curentArr.size;
        index = 0;
        for(j = previousSize; j < corelationTable.size; j++)
        {
            corelationTable.arr[j] = curentArr.arr[j - previousSize];
            corelationTable.arr[j].nrSablon = i;
            index++;
        }
    }
    return corelationTable;
}

int cmp(const void *a, const void *b)
{
    struct pairPoint *pairA = (struct pairPoint *) a;
    struct pairPoint *pairB = (struct pairPoint *) b;
    if(pairA->corelation < pairB->corelation)
        return 1;
    if(pairA->corelation > pairB->corelation)
        return -1;
}

int min(int a, int b)
{
    if(a > b)
        return b;
    return a;
}

int max(int a, int b)
{
    if(a < b)
        return b;
    return a;
}

void deleteElement(struct pairPoint** array, int* size, int j)
{
    int i;
    for(i = j; i < (*size) - 1; i++)
        (*array)[i] = (*array)[i + 1];
    (*size)--;
}
int overlap(struct pairPoint p1, struct pairPoint p2, struct image sablon)
{
    struct pairPoint l1, r1, l2, r2;
    l1.x = p1.x; l1.y = p1.y + sablon.height;
    r1.y = p1.y; r1.x = p1.x + sablon.width;
    l2.x = p2.x; l2.y = p2.y + sablon.height;
    r2.y = p2.y; r2.x = p2.x + sablon.width;

    if(l1.x > r2.x || l2.x > r1.x)
        return 0;
    if(l1.y < r2.y || l2.y < r1.y)
        return 0;
    return 1;
}
int overlapingArea(struct pairPoint p1, struct pairPoint p2, struct image sablon)
{
    // (*) Folosim colturile din stanga jos si dreapta sus pentru a calcula aria suprapusa
    // l1 = coltul stanga jos al dreptunghiului 1
    // r1 = coltul dreapta sus al dreptunghiului 1

    struct pairPoint l1, r1, l2, r2;
    l1 = p1;
    r1.x = p1.x + sablon.width; r1.y = p1.y + sablon.height;
    l2 = p2;
    r2.x = p2.x + sablon.width; r2.y = p2.y + sablon.height;
    return (min(r1.x, r2.x) - max(l1.x, l2.x)) * (min(r1.y, r2.y) - max(l1.y, l2.y));
}
struct arrayPairPoint nonMaxElimination(struct arrayPairPoint corelationTable, struct image sablon)
{
    int i, j, nr = 0;
    float areaOfTwoRectangles = 2 * sablon.width * sablon.height;
    for(i = 0; i < corelationTable.size - 1; i++)
        for(j = i + 1; j < corelationTable.size; j++)
        {
            if(corelationTable.arr[i].corelation > corelationTable.arr[j].corelation &&
                    overlap(corelationTable.arr[i], corelationTable.arr[j], sablon))
            {
                float overlapArea = overlapingArea(corelationTable.arr[i], corelationTable.arr[j], sablon);
                float suprapunere = overlapArea / (areaOfTwoRectangles - overlapArea);
                if(suprapunere > 0.2) {
                    deleteElement(&corelationTable.arr, &corelationTable.size, j);
                }
                //printf("%.2f\n", suprapunere);
            }
        }
    return corelationTable;
}

void drawFinalImage(struct image originalImage, struct image sablon, struct arrayPairPoint corelationTable)
{
    int i, j;
    char fileName[100];
    struct pixel color;
    for(i = 0; i < corelationTable.size; i++)
    {
        color = chooseColor(corelationTable.arr[i].nrSablon);
        originalImage = drawBorder(originalImage, corelationTable.arr[i], color, sablon);
    }
    createBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\Sabloane\\imagineFinala.bmp", originalImage);

}

int main()
{
   /* criptBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\peppers.bmp",
            "E:\\INFO\\FMI\\ProgProced\\ProiectLab\\cript.bmp",
            "E:\\INFO\\FMI\\ProgProced\\ProiectLab\\secret_key.txt");

     decriptBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\DecriptPeppers.bmp",
            "E:\\INFO\\FMI\\ProgProced\\ProiectLab\\cript.bmp",
            "E:\\INFO\\FMI\\ProgProced\\ProiectLab\\secret_key.txt");
    chiSquaredValues("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\enc_peppers_ok.bmp");
    */
    struct image sablon = grayscale("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\Sabloane\\cifra7.bmp");
    struct image grayscaleImage = grayscale("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\test.bmp");
    struct image originalImage = loadBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\test.bmp");

   /* struct arrayPairPoint result = templateMatching(grayscaleImage, sablon, 0.7);
    struct pixel color; color.r = 250; color.b = 200; color.g = 0;
    int i;
    struct pairPoint f;
    f.x = 387;
    f.y = 184;
    //image = drawBorder(image, f, color, sablon);
    for(i = 0; i < result.size; i++){
        originalImage = drawBorder(originalImage, result.arr[i], color, sablon);}
    createBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\Sabloane\\test7.bmp", originalImage);
*/

    struct arrayPairPoint corelationTable = getCorelations(grayscaleImage, "E:\\INFO\\FMI\\ProgProced\\ProiectLab\\sabloane.txt");

    int i;
    qsort(corelationTable.arr, corelationTable.size, sizeof(struct pairPoint), cmp);
    corelationTable = nonMaxElimination(corelationTable, sablon);
    printf("%d\n", corelationTable.size);
    /*struct pixel color; color.r = 250; color.b = 200; color.g = 0;
    for(i = 0; i < corelationTable.size; i++)
        originalImage = drawBorder(originalImage, corelationTable.arr[i], color, sablon);
    createBMP("E:\\INFO\\FMI\\ProgProced\\ProiectLab\\Sabloane\\imagineFinala.bmp", originalImage);
     */
    drawFinalImage(originalImage, sablon, corelationTable);
     return 0;
}

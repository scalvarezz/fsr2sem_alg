#include "lodepng.c"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define epsilon 40


typedef struct point{
    int x, y;
} Point;


char* load_png_file(const char *filename, int *width, int *height) {
    unsigned char *image = NULL;
    int error = lodepng_decode32_file(&image, width, height, filename);
    if (error) {
        printf("error %u: %s\n", error, lodepng_error_text(error));
        return NULL;
    }

    return (image);
}

void floodFill(unsigned char* image, int x, int y, int newColor1, int newColor2, int newColor3, int oldColor, int width, int height) {
    int dx[] = {-1, 0, 1, 0};
    int dy[] = {0, 1, 0, -1};
    Point* ar = malloc(width * height * 4 * sizeof(Point));
    long top = 0;
    ar[top++] = (Point){x, y};
    while(top > 0) {
        Point p = ar[--top];
        if(p.x < 0 || p.x >= width || p.y < 0 || p.y >= height)
            continue;
        int resultIndex = (p.y * width + p.x) * 4;
        if(image[resultIndex] > oldColor)
            continue;
        image[resultIndex] = newColor1;
        image[resultIndex + 1] = newColor2;
        image[resultIndex + 2] = newColor3;
        for(int i = 0; i < 4; i++) {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];
            int nIndex = (ny * width + nx) * 4;
            if(nx > 0 && nx < width && ny > 0 && ny < height && image[nIndex] <= oldColor) {
                ar[top++] = (Point){nx, ny};
            }
        }
    }
    free(ar);
}

void ScharrFilter(unsigned char *image, int width, int height) {
    int x, y, dy, dx, i;
    int gx[3][3] = {{3, 0, -3},
                    {10, 0, -10},
                    {3, 0, -3}};
    int gy[3][3] = {{3,  10,  3},
                    {0,  0,  0},
                    {-3, -10, -3}};
    unsigned char *result = malloc(width * height * 4 * sizeof(unsigned char));

    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            int sumX = 0, sumY = 0;
            for (dy = -1; dy <= 1; dy++) {
                for (dx = -1; dx <= 1; dx++) {
                    int index = ((y + dy) * width + (x + dx)) * 4;
                    int gray = (image[index] + image[index + 1] + image[index + 2]) / 10;
                    sumX += gx[dy + 1][dx + 1] * gray;
                    sumY += gy[dy + 1][dx + 1] * gray;
                }
            }
            int mg = sqrt(sumX * sumX + sumY * sumY);
            if (mg > 255) mg = 255;
            if (mg < 0) mg = 0;

            int resultIndex = (y * width + x) * 4;
            result[resultIndex] = (unsigned char) mg;
            result[resultIndex + 1] = (unsigned char) mg;
            result[resultIndex + 2] = (unsigned char) mg;
            result[resultIndex + 3] = image[resultIndex + 3];
        }
    }

    for (i = 0; i < width * height * 4; i++) {
        image[i] = result[i];
    }
    free(result);
}



void colorComponents(unsigned char *image, int width, int height) {
    int x, y;
    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            if (image[4 * (y * width + x)] < epsilon) {
                floodFill(image, x, y, rand() % (255 - epsilon * 2) + epsilon * 2, \
                rand() % (255 - epsilon * 2) + epsilon * 2, \
                rand() % (255 - epsilon * 2) + epsilon * 2, epsilon, width, height);

            }
        }
    }
    char *output_filename = "skull_filter_applied.png";
    lodepng_encode32_file(output_filename, image, width, height);
}


int main() {
    int width = 0, height = 0, x, y, dy, dx;
    char *filename = "skull.png";
    char *image = load_png_file(filename, &width, &height);

    ScharrFilter(image, width, height);
    colorComponents(image, width, height);

    free(image);
    return 0;
}

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lin.h"

//g++ --std=c++11 lintest.cpp -o lintest

void printMatrix(const float* m) {
    printf("%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n\n",
        m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);
}

int main(int argc, char **argv) {

    //float a = argc > 1 ? atof(argv[1]) : 42.f;
    //float b = argc > 2 ? atof(argv[2]) : 43.f;
    //float c = argc > 3 ? atof(argv[3]) : 44.f;
    //float d = argc > 4 ? atof(argv[4]) : 45.f;

    lVec4f vector(2,4,6,1);

    //printf("%.1f, %.1f, %.1f, %.1f\n", vector.v[0], vector.v[1], vector.v[2], vector.v[3]);

    lMatrix4f mat;
    mat.SetScale(2);

    lMatrix4f mat1;
    mat1.SetTranslate(2,5,6);

    const lMatrix4f out = mat * mat1;

    printMatrix(&out.lM[0]);

    return 0;
}
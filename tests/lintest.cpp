#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lin.h"
#include "linear.h"

using namespace r3;

//g++ --std=c++11 lintest.cpp -o lintest

void printVec4(const float* v) {
    printf("%.3f, %.3f, %.3f : %.3f\n\n",
        v[0], v[1], v[2], v[3]);
}

void printMatrix4(const float* m) {
    printf("%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n\n",
        m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);
}

int main(int argc, char **argv) {

    lVec4f vector(2,4,6);

    lMatrix4f mat0;
    mat0.SetScale(2);

    lMatrix4f mat1;
    mat1.SetRotation(2, ToRadians(90.0));

    const lVec4f out = mat1 * vector;

    printVec4(&out.v[0]);

    return 0;
}
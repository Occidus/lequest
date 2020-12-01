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

Vec4f ToR3(const lVec4f & lvec) {
    return *reinterpret_cast<const Vec4f*>(&lvec);
}

int main(int argc, char **argv) {



    lVec4f vector(0,1,0);
    lVec4f axis(0,.5,0.5);
    float angleDeg = 39;

    lMatrix4f mat0;
    mat0.SetRotation(axis, lToRadians(angleDeg));

    printMatrix4(&mat0.lM[0]);

    const lVec4f out = mat0 * vector;

    printVec4(&out.x);


    Quaternionf rotate(Homogenize(ToR3(axis)), ToRadians(angleDeg));

    Matrix4f mat1 = rotate.GetMatrix4().Transposed();

    printMatrix4(&mat1.el(0,0));

    mat1 = mat1.Transposed();

    const Vec4f out1 = mat1 * ToR3(vector);

    printVec4(&out1.x);

    return 0;
}
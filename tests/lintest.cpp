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



    lVec4f vector(1,0,1);
    Vec4f r3Vector(1,0,1,1);

    lMatrix4f mat0;
    mat0.SetRotation(lVec4f(1,1,1), lToRadians(39.0));

    const lMatrix4f out0 = mat0;

    printMatrix4(&out0.lM[0]);

    const lVec4f out = mat0 * vector;

    printVec4(&out.x);


    Quaternionf rotate(Vec3f(1,1,1), ToRadians(39.0f));

    const Matrix4f matl = rotate.GetMatrix4();

    printMatrix4(&matl.el(0,0));

    const Vec4f out1 = rotate.GetMatrix4() * r3Vector;

    printVec4(&out1.x);

    return 0;
}
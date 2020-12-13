#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lin.h"
#include "linear.h"

using namespace r3;

// g++ --std=c++11 lintest.cpp -o lintest

void printVec4(const float *v) {
  printf("%.3f, %.3f, %.3f : %.3f\n\n", v[0], v[1], v[2], v[3]);
}

void printMatrix4(const float *m) {
  printf("%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, "
         "%.3f\n%.3f, %.3f, %.3f, %.3f\n\n",
         m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10],
         m[11], m[12], m[13], m[14], m[15]);
}

Vec4f ToR3(const lVec4f &lvec) {
  return *reinterpret_cast<const Vec4f *>(&lvec);
}

Matrix4f ToR3(const lMatrix4f &lmat) {
  return *reinterpret_cast<const Matrix4f *>(&lmat);
}

int main(int argc, char **argv) {

  lMatrix4f testMat0(
    1.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);

  lMatrix4f testMat1(
    1.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);

  lMatrix4f testMat2(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);

  const lMatrix4f lTestMat0 = testMat0.Inverted();
  const lMatrix4f lTestMat1 = testMat1.Inverted();
  const lMatrix4f lTestMat2 = testMat2.Inverted();

  printMatrix4(&lTestMat0.lM[0]);
  printMatrix4(&lTestMat1.lM[0]);
  printMatrix4(&lTestMat2.lM[0]);

  printf("\n");

  const Matrix4f r3TestMat0 = ToR3(testMat0);
  const Matrix4f r3TestMat1 = ToR3(testMat1);
  const Matrix4f r3TestMat2 = ToR3(testMat2);

  printMatrix4(&r3TestMat0.Inverted().el(0,0));
  printMatrix4(&r3TestMat1.Inverted().el(0,0));
  printMatrix4(&r3TestMat2.Inverted().el(0,0));

  return 0;
}

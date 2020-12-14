#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lin.h"
#include "randmat.h"
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

  lMatrix4f identity;
  lMatrix4f testMat;
  float max = 0;

  srand(485214521);

  for(int i=0;i<1000000;i++){
    randmat(&testMat.lM[0]);
    lMatrix4f lTestMat = testMat.Inverted();
    lMatrix4f mult = lTestMat * testMat;
    bool eq = true;
    for(int r=0;r<4;r++) {
        for(int c=0;c<4;c++) {
            int element = mult.el(r,c) - identity.el(r,c);
            element = element * element;
            if(element > max) {
                max = element;
            }
            if(element > 0.000000001) {
                eq = false;
            }
        }
    }
    if(eq == false) {//} || i==95360 || i==549817 || i==669538) {
        printf("Original Matrix: %i\n",i);
        printMatrix4(&testMat.lM[0]);
        printf("Lin Inverted Matrix:\n");
        printMatrix4(&lTestMat.lM[0]);
        printf("Mult Matrix:\n");
        printMatrix4(&mult.lM[0]);
        printf("\n");
        Matrix4f r3Mat = ToR3(testMat).Inverted();
        Matrix4f r3Mult = ToR3(testMat) * r3Mat;
        printf("Linear Inverted Matrix:\n");
        printMatrix4(&r3Mat.el(0,0));
        printf("Mult Matrix:\n");
        printMatrix4(&r3Mult.el(0,0));
        printf("Max: %.20f\n\n\n",max);
    }
  }  

  //printMatrix4(&lTestMat0.lM[0]);
  //printMatrix4(&lTestMat1.lM[0]);
  //printMatrix4(&lTestMat2.lM[0]);

  //bool test mat

  //print

  /*
  printf("\n");

  const Matrix4f r3TestMat0 = ToR3(testMat0);
  const Matrix4f r3TestMat1 = ToR3(testMat1);
  const Matrix4f r3TestMat2 = ToR3(testMat2);

  printMatrix4(&r3TestMat0.Inverted().el(0,0));
  printMatrix4(&r3TestMat1.Inverted().el(0,0));
  printMatrix4(&r3TestMat2.Inverted().el(0,0));
  */

  return 0;
}

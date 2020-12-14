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
  Matrix4f r3Identity = ToR3(identity);
  lMatrix4f testMat;
  int messedUp = 0;
  int r3MessedUp = 0;
  int uninvertable = 0;
  bool eq;

  srand(485214521);

  for(int i=0;i<1000000;i++){
    randmat(&testMat.lM[0]);
    lMatrix4f lTestMat = testMat.Inverted();
    lMatrix4f mult = lTestMat * testMat;
    eq = true;
    for(int r=0;r<4;r++) {
        for(int c=0;c<4;c++) {
            float element = fabs(mult.el(r,c) - identity.el(r,c));
            if(element > 0.01) {
                eq = false;
            }
        }
    }
    if(!eq) {
        float determinant = Mat4Det(testMat);
        if(determinant < 0.01 && determinant > -0.01){
            uninvertable++;
        } else {
            messedUp++;
            //lTestMat = testMat.Inverted(true);
            printf("Original Matrix: %i\n",i);
            printMatrix4(&testMat.lM[0]);
            printf("Lin Inverted Matrix:\n");
            printMatrix4(&lTestMat.lM[0]);
            printf("Mult Matrix:\n");
            printMatrix4(&mult.lM[0]);

            Matrix4f r3TestMat = ToR3(testMat).Inverted();
            Matrix4f r3Mult = ToR3(testMat) * r3TestMat;
            bool r3Pass = true;
            for(int r=0;r<4;r++) {
                for(int c=0;c<4;c++) {
                    float element = fabs(r3Mult.el(r,c) - r3Identity.el(r,c));
                    if(element > 0.01) {
                        r3Pass = false;
                    }
                }
            }
            if(!r3Pass) {
                r3MessedUp++;
                printf("Linear Inverted Matrix:\n");
                printMatrix4(&r3TestMat.el(0,0));
                printf("Mult Matrix:\n");
                printMatrix4(&r3Mult.el(0,0));
            }
            printf("Determinant: %.20f\n\n\n", determinant);
        }
    }
  }

  printf("Num Messed Up: %i\n", messedUp);
  printf("Num r3 Messed Up: %i\n", r3MessedUp);
  printf("Num Uninvertable: %i\n", uninvertable);

  return 0;
}

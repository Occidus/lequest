#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lin.h"
#include "linear.h"
#include "randmat.h"

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

Vec3f ToR3(const lVec3f &lvec) {
  return *reinterpret_cast<const Vec3f *>(&lvec);
}

Vec4f ToR3(const lVec4f &lvec) {
  return *reinterpret_cast<const Vec4f *>(&lvec);
}

Matrix4f ToR3(const lMatrix4f &lmat) {
  return *reinterpret_cast<const Matrix4f *>(&lmat);
}

int main(int argc, char **argv) {

  /*
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
      printf("Determinant: %.20f\n\n\n", determinant);
    }
  }

  printf("Num Messed Up: %i\n", messedUp);
  printf("Num r3 Messed Up: %i\n", r3MessedUp);
  printf("Num Uninvertable: %i\n", uninvertable);
  */

  float angle0 = 35.0f;
  float angle1 = 73.0f;
  lVec3f axis0(1.0f, 1.0f, 0.0f);
  lVec3f axis1(1.0f, 0.0f, 1.0f);
  // lMatrix4f lRotMat;
  // lRotMat = lRotMat.Rotate(axis1, lToRadians(angle1));
  // Matrix4f r3RotMat = Quaternionf(ToR3(axis1),
  // lToRadians(angle1)).GetMatrix4();
  lVec3f lPoint0(2.0f, 3.0f, 5.0f);
  lVec3f lPoint1(7.0f, 1.0f, 6.0f);
  Vec3f r3Point0(ToR3(lPoint0));
  Vec3f r3Point1(ToR3(lPoint1));
  printf("Point 0| %.3f, %.3f, %.3f\n", lPoint0.x, lPoint0.y, lPoint0.z);
  printf("Point 1| %.3f, %.3f, %.3f\n", lPoint1.x, lPoint1.y, lPoint1.z);
  printf("Quat 0 Axis and Angle| %.3f, %.3f, %.3f : %.3f\n", axis0.x, axis0.y,
         axis0.z, angle0);
  printf("Quat 1 Axis and Angle| %.3f, %.3f, %.3f : %.3f\n\n", axis1.x, axis1.y,
         axis1.z, angle1);
  // printf("Quat 1 Matrix4|\n");
  // printMatrix4(&lRotMat.lM[0]);
  // printf("\n");

  lQuaternionf lQuat0(axis0, lToRadians(angle0));
  lQuaternionf lQuat1(axis1, lToRadians(angle1));
  // lQuaternionf lQuat1(lRotMat);
  lQuaternionf lMultQuat = lQuat0 * lQuat1;
  Quaternionf r3Quat0(ToR3(axis0), ToRadians(angle0));
  Quaternionf r3Quat1(ToR3(axis1), ToRadians(angle1));
  // Quaternionf r3Quat1(r3RotMat);
  Quaternionf r3MultQuat = r3Quat0 * r3Quat1;
  lPoint0 = lQuat0 * lPoint0;
  lPoint1 = lQuat1 * lPoint1;
  r3Point0 = r3Quat0 * r3Point0;
  r3Point1 = r3Quat1 * r3Point1;

  printf(" l Quat 0| %.3f, %.3f, %.3f : %.3f\n", lQuat0.x, lQuat0.y, lQuat0.z,
         lQuat0.w);
  printf("r3 Quat 0| %.3f, %.3f, %.3f : %.3f\n", r3Quat0.x, r3Quat0.y,
         r3Quat0.z, r3Quat0.w);
  printf(" l Quat 1| %.3f, %.3f, %.3f : %.3f\n", lQuat1.x, lQuat1.y, lQuat1.z,
         lQuat1.w);
  printf("r3 Quat 1| %.3f, %.3f, %.3f : %.3f\n\n", r3Quat1.x, r3Quat1.y,
         r3Quat1.z, r3Quat1.w);
  printf(" l Mult Quat| %.3f, %.3f, %.3f : %.3f\n", lMultQuat.x, lMultQuat.y,
         lMultQuat.z, lMultQuat.w);
  printf("r3 Mult Quat| %.3f, %.3f, %.3f : %.3f\n\n", r3MultQuat.x,
         r3MultQuat.y, r3MultQuat.z, r3MultQuat.w);
  printf(" l Point 0 Transformed| %.3f, %.3f, %.3f\n", lPoint0.x, lPoint0.y,
         lPoint0.z);
  printf("r3 Point 0 Transformed| %.3f, %.3f, %.3f\n", r3Point0.x, r3Point0.y,
         r3Point0.z);
  printf(" l Point 1 Transformed| %.3f, %.3f, %.3f\n", lPoint1.x, lPoint1.y,
         lPoint1.z);
  printf("r3 Point 1 Transformed| %.3f, %.3f, %.3f\n", r3Point1.x, r3Point1.y,
         r3Point1.z);

  return 0;
}

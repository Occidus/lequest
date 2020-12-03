#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float lToRadians(float degrees) { return degrees * (M_PI / 180.0f); }

float lToDegrees(float radians) { return radians * (180.0f / M_PI); }

class lVec4f {
public:
  union {
    float v[4];
    struct {
      float x, y, z, w;
    };
  };

  lVec4f() {
    v[0] = 0.0f;
    v[1] = 0.0f;
    v[2] = 0.0f;
    v[3] = 1.0f;
  }

  lVec4f(float a, float b, float c) {
    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = 1.0f;
  }

  lVec4f(float a, float b, float c, float d) {
    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;
  }

  void Normalize() {
    float l = sqrt((x * x) + (y * y) + (z * z));
    x /= l;
    y /= l;
    z /= l;
  }

  lVec4f Normalized() {
    float l = sqrt((x * x) + (y * y) + (z * z));
    return lVec4f((x / l), (y / l), (z / l));
  }
};

float Dot(lVec4f a, lVec4f b) {
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

enum ElAxis { AXIS_X, AXIS_Y, AXIS_Z };

class lMatrix4f;

lVec4f Mult(const lMatrix4f &m, const lVec4f &v);

lVec4f operator*(const lMatrix4f &m, const lVec4f &v);

lVec4f Mult(const lVec4f &v, const lMatrix4f &m);

lVec4f operator*(const lVec4f &v, const lMatrix4f &m);

lMatrix4f Mult(const lMatrix4f &m0, const lMatrix4f &m1);

lMatrix4f operator*(const lMatrix4f &m0, const lMatrix4f &m1);

lVec4f Mult(const lVec4f &v, const float &f);
lVec4f Divi(const lVec4f &v, const float &f);
lVec4f add(const lVec4f &v0, const lVec4f &v1);
lVec4f min(const lVec4f &v0, const lVec4f &v1);

lVec4f operator*(const lVec4f &v, const float &f);
lVec4f operator/(const lVec4f &v, const float &f);
lVec4f operator+(const lVec4f &v0, const lVec4f &v1);
lVec4f operator-(const lVec4f &v0, const lVec4f &v1);

void printOp(float *m0, float *m1);

class lMatrix4f {
public:
  float lM[16];

  lMatrix4f() { MakeIdentity(); }

  lMatrix4f(float f00, float f01, float f02, float f03, float f10, float f11,
            float f12, float f13, float f20, float f21, float f22, float f23,
            float f30, float f31, float f32, float f33) {
    el(0, 0) = f00;
    el(0, 1) = f01;
    el(0, 2) = f02;
    el(0, 3) = f03;

    el(1, 0) = f10;
    el(1, 1) = f11;
    el(1, 2) = f12;
    el(1, 3) = f13;

    el(2, 0) = f20;
    el(2, 1) = f21;
    el(2, 2) = f22;
    el(2, 3) = f23;

    el(3, 0) = f30;
    el(3, 1) = f31;
    el(3, 2) = f32;
    el(3, 3) = f33;
  }

  lMatrix4f(const lVec4f &r0, const lVec4f &r1, const lVec4f &r2,
            const lVec4f &r3) {
    Row(0, r0);
    Row(1, r1);
    Row(2, r2);
    Row(3, r3);
  }

  void MakeIdentity() {
    el(0, 0) = 1.0;
    el(0, 1) = 0.0;
    el(0, 2) = 0.0;
    el(0, 3) = 0.0;

    el(1, 0) = 0.0;
    el(1, 1) = 1.0;
    el(1, 2) = 0.0;
    el(1, 3) = 0.0;

    el(2, 0) = 0.0;
    el(2, 1) = 0.0;
    el(2, 2) = 1.0;
    el(2, 3) = 0.0;

    el(3, 0) = 0.0;
    el(3, 1) = 0.0;
    el(3, 2) = 0.0;
    el(3, 3) = 1.0;
  }

  lVec4f Row(int i) const {
    return lVec4f(el(i, 0), el(i, 1), el(i, 2), el(i, 3));
  }

  void Row(int i, const lVec4f &r) {
    el(i, 0) = r.x;
    el(i, 1) = r.y;
    el(i, 2) = r.z;
    el(i, 3) = r.w;
  }

  lVec4f Col(int i) const {
    return lVec4f(el(0, i), el(1, i), el(2, i), el(3, i));
  }

  lMatrix4f Inverted() {
    lMatrix4f mat = *this;
    mat.Invert();
    return mat;
  }

  static lMatrix4f Rotate(ElAxis a, double angle) {
    lMatrix4f mat;
    mat.SetRotation(a, angle);
    return mat;
  }

  static lMatrix4f Rotate(lVec4f a, double angle) {
    lMatrix4f mat;
    mat.SetRotation(a, angle);
    return mat;
  }

  static lMatrix4f Scale(float s) {
    lMatrix4f mat;
    mat.SetScale(s);
    return mat;
  }

  static lMatrix4f Translate(float x, float y, float z) {
    lMatrix4f mat;
    mat.SetTranslate(x, y, z);
    return mat;
  }

  void Invert() {
    lMatrix4f out;
    printOp(&el(0,0), &out.el(0,0));
    for(int r=0;r<4;r++) {
      if(Row(r).v[r]!=1.0f) { // Sets diagonal to 1.0f
        printf("R[%i] / %f\n\n", r, Row(r).v[r]);
        out.Row(r, out.Row(r) / Row(r).v[r]);
        Row(r, Row(r) / Row(r).v[r]);
        printOp(&el(0,0), &out.el(0,0));
      }
      for(int i=r+1;i<4;i++) { // Works numbers below to 0.0f
        printf("R[%i] - %fR[%i]\n\n", i, Row(i).v[r], r);
        out.Row(i, (out.Row(i)-(out.Row(r)*Row(i).v[r])));
        Row(i, (Row(i)-(Row(r)*Row(i).v[r])));
        printOp(&el(0,0), &out.el(0,0));
      }
    }
    for(int r=3;r>0;r--) {
      //lVec4f  = (Row(r)*Row(i).v[r]);
      for(int i=r-1;i>=0;i--) { // Works numbers above to 0.0f
        printf("R[%i] - %fR[%i]\n\n", i, Row(i).v[r], r);
        out.Row(i, (out.Row(i)-(out.Row(r)*Row(i).v[r])));
        Row(i, (Row(i)-(Row(r)*Row(i).v[r])));
        printOp(&el(0,0), &out.el(0,0));
      }
    }

    printOp(&el(0,0), &out.el(0,0));
    *this = out;
  }

  void SetRotation(ElAxis a, double angle) { // Rotates about x, y, or z axis
    MakeIdentity();
    switch (a) {
    case AXIS_X: { // x-axis
      el(1, 1) = cos(angle);
      el(1, 2) = -sin(angle);

      el(2, 1) = sin(angle);
      el(2, 2) = cos(angle);
      break;
    }
    case AXIS_Y: { // y-axis
      el(0, 0) = cos(angle);
      el(0, 2) = sin(angle);

      el(2, 0) = -sin(angle);
      el(2, 2) = cos(angle);
      break;
    }
    case AXIS_Z: { // z-axis
      el(0, 0) = cos(angle);
      el(0, 1) = -sin(angle);

      el(1, 0) = sin(angle);
      el(1, 1) = cos(angle);
      break;
    }
    default: {
      break;
    }
    }
  }

  void SetRotation(lVec4f a, double angle) { // Rotates about any abitrary axis
    MakeIdentity();
    a.Normalize();
    lMatrix4f toXyPlane, fromXyPlane, toXAxis, fromXAxis;
    float xRot = 0.0f;
    float zRot = 0.0f;

    if (a.z != 0) {
      xRot = atan2(a.z, a.y);
      toXyPlane = lMatrix4f::Rotate(AXIS_X, -xRot);
      fromXyPlane = lMatrix4f::Rotate(AXIS_X, xRot);
    }
    if (a.y != 0) {
      zRot = atan2(sqrt(1.0f - a.x * a.x), a.x);
      toXAxis = lMatrix4f::Rotate(AXIS_Z, -zRot);
      fromXAxis = lMatrix4f::Rotate(AXIS_Z, zRot);
    }

    *this = fromXyPlane * fromXAxis * lMatrix4f::Rotate(AXIS_X, angle) *
            toXAxis * toXyPlane;
  }

  void SetScale(float s) {
    el(0, 0) = s;
    el(1, 1) = s;
    el(2, 2) = s;
  }

  void SetTranslate(float x, float y, float z) {
    el(0, 3) = x;
    el(1, 3) = y;
    el(2, 3) = z;
  }

  float &el(int row, int col) { return lM[(row * 4) + col]; }

  float el(int row, int col) const { return lM[(row * 4) + col]; }
};

lVec4f Mult(const lMatrix4f &m, const lVec4f &v) {
  return lVec4f(Dot(m.Row(0), v), Dot(m.Row(1), v), Dot(m.Row(2), v),
                Dot(m.Row(3), v));
}

lVec4f operator*(const lMatrix4f &m, const lVec4f &v) { return Mult(m, v); }

lVec4f Mult(const lVec4f &v, const lMatrix4f &m) {
  return lVec4f(Dot(m.Col(0), v), Dot(m.Col(1), v), Dot(m.Col(2), v),
                Dot(m.Col(3), v));
}

lVec4f operator*(const lVec4f &v, const lMatrix4f &m) { return Mult(v, m); }

lMatrix4f Mult(const lMatrix4f &m0, const lMatrix4f &m1) {
  return lMatrix4f((m0.Row(0) * m1), (m0.Row(1) * m1), (m0.Row(2) * m1),
                   (m0.Row(3) * m1));
}

lMatrix4f operator*(const lMatrix4f &m0, const lMatrix4f &m1) {
  return Mult(m0, m1);
}

lVec4f Mult(const lVec4f &v, const float &f) { return lVec4f(v.x*f, v.y*f, v.z*f, v.w*f); }
lVec4f Divi(const lVec4f &v, const float &f) { return lVec4f(v.x/f, v.y/f, v.z/f, v.w/f); }
lVec4f add(const lVec4f &v0, const lVec4f &v1) { return lVec4f(v0.x+v1.x, v0.y+v1.y, v0.z+v1.z, v0.w+v1.w); }
lVec4f min(const lVec4f &v0, const lVec4f &v1) { return lVec4f(v0.x-v1.x, v0.y-v1.y, v0.z-v1.z, v0.w-v1.w); }

lVec4f operator*(const lVec4f &v, const float &f) { return Mult(v, f); }
lVec4f operator/(const lVec4f &v, const float &f) { return Divi(v, f); }
lVec4f operator+(const lVec4f &v0, const lVec4f &v1) { return add(v0, v1); }
lVec4f operator-(const lVec4f &v0, const lVec4f &v1) { return min(v0, v1); }

void printOp(float *m0, float *m1) {
  printf("%.3f, %.3f, %.3f, %.3f\t%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\t%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\t%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\t%.3f, %.3f, %.3f, %.3f\n\n",
    m0[0], m0[1], m0[2], m0[3],      m1[0], m1[1], m1[2], m1[3],
    m0[4], m0[5], m0[6], m0[7],      m1[4], m1[5], m1[6], m1[7],
    m0[8], m0[9], m0[10], m0[11],    m1[8], m1[9], m1[10], m1[11],
    m0[12], m0[13], m0[14], m0[15],  m1[12], m1[13], m1[14], m1[15]);
}

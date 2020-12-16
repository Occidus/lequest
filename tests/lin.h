#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float lToRadians(float degrees) { return degrees * (M_PI / 180.0f); }

float lToDegrees(float radians) { return radians * (180.0f / M_PI); }

class lVec3f {
public:
  union {
    float v[3];
    struct {
      float x, y, z;
    };
  };

  lVec3f() {
    v[0] = 0.0f;
    v[1] = 0.0f;
    v[2] = 0.0f;
  }

  lVec3f(float v0, float v1, float v2) {
    v[0] = v0;
    v[1] = v1;
    v[2] = v2;
    v[3] = 1.0f;
  }

  void Normalize() {
    float l = sqrt((x * x) + (y * y) + (z * z));
    x /= l;
    y /= l;
    z /= l;
  }

  lVec3f Normalized() {
    float l = sqrt((x * x) + (y * y) + (z * z));
    return lVec3f((x / l), (y / l), (z / l));
  }
};

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

float Dot(lVec3f a, lVec3f b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z); }
float Dot(lVec4f a, lVec4f b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w); }

lVec3f Cross(lVec3f v0, lVec3f v1) { return lVec3f((v0.y*v1.z - v0.z*v1.y), (v0.z*v1.x - v0.x*v1.z), (v0.x*v1.y - v0.y*v1.x)); }

enum ElAxis { AXIS_X, AXIS_Y, AXIS_Z };

class lMatrix3f;
class lMatrix4f;

lVec3f Mult(const lMatrix3f &m, const lVec3f &v);
lVec3f Mult(const lVec3f &v, const lMatrix3f &m);
lMatrix3f Mult(const lMatrix3f &m0, const lMatrix3f &m1);

lVec3f operator*(const lMatrix3f &m, const lVec3f &v);
lVec3f operator*(const lVec3f &v, const lMatrix3f &m);
lMatrix3f operator*(const lMatrix3f &m0, const lMatrix3f &m1);


lVec4f Mult(const lMatrix4f &m, const lVec4f &v);
lVec4f Mult(const lVec4f &v, const lMatrix4f &m);
lMatrix4f Mult(const lMatrix4f &m0, const lMatrix4f &m1);
bool Equals(const lMatrix4f &m0, const lMatrix4f &m1);

lVec4f operator*(const lMatrix4f &m, const lVec4f &v);
lVec4f operator*(const lVec4f &v, const lMatrix4f &m);
lMatrix4f operator*(const lMatrix4f &m0, const lMatrix4f &m1);
bool operator==(const lMatrix4f &m0, const lMatrix4f &m1);


lVec3f Mult(const lVec3f &v, const float &f);
lVec3f Divi(const lVec3f &v, const float &f);
lVec3f add(const lVec3f &v0, const lVec3f &v1);
lVec3f min(const lVec3f &v0, const lVec3f &v1);
bool Equals(const lMatrix3f &m0, const lMatrix3f &m1);

lVec3f operator*(const lVec3f &v, const float &f);
lVec3f operator/(const lVec3f &v, const float &f);
lVec3f operator+(const lVec3f &v0, const lVec3f &v1);
lVec3f operator-(const lVec3f &v0, const lVec3f &v1);
bool operator==(const lMatrix3f &m0, const lMatrix3f &m1);


lVec4f Mult(const lVec4f &v, const float &f);
lVec4f Divi(const lVec4f &v, const float &f);
lVec4f add(const lVec4f &v0, const lVec4f &v1);
lVec4f min(const lVec4f &v0, const lVec4f &v1);

lVec4f operator*(const lVec4f &v, const float &f);
lVec4f operator/(const lVec4f &v, const float &f);
lVec4f operator+(const lVec4f &v0, const lVec4f &v1);
lVec4f operator-(const lVec4f &v0, const lVec4f &v1);

void printOp(float *m0, float *m1);

class lMatrix3f {
public:
  float lM[9];

  lMatrix3f() { MakeIdentity(); }

  lMatrix3f(float f00, float f01, float f02, float f10, float f11, float f12, float f20, float f21, float f22) {
    el(0, 0) = f00;
    el(0, 1) = f01;
    el(0, 2) = f02;

    el(1, 0) = f10;
    el(1, 1) = f11;
    el(1, 2) = f12;

    el(2, 0) = f20;
    el(2, 1) = f21;
    el(2, 2) = f22;
  }

  lMatrix3f(const lVec3f &r0, const lVec3f &r1, const lVec3f &r2) {
    Row(0, r0);
    Row(1, r1);
    Row(2, r2);
  }

  void MakeIdentity() {
    el(0, 0) = 1.0;
    el(0, 1) = 0.0;
    el(0, 2) = 0.0;

    el(1, 0) = 0.0;
    el(1, 1) = 1.0;
    el(1, 2) = 0.0;

    el(2, 0) = 0.0;
    el(2, 1) = 0.0;
    el(2, 2) = 1.0;
  }

  lVec3f Row(int i) const {
    return lVec3f(el(i, 0), el(i, 1), el(i, 2));
  }

  void Row(int i, const lVec3f &r) {
    el(i, 0) = r.x;
    el(i, 1) = r.y;
    el(i, 2) = r.z;
  }

  void SwapRow(int i, int j) {
    lVec3f stowRow = Row(i);
    Row(i, Row(j));
    Row(j, stowRow);
  }

  lVec3f Col(int i) const {
    return lVec3f(el(0, i), el(1, i), el(2, i));
  }

  void Col(int i, const lVec3f &c) {
    el(0, i) = c.x;
    el(1, i) = c.y;
    el(2, i) = c.z;
  }

  /*lMatrix4f Inverted(bool print = false) {
    lMatrix4f mat = *this;
    mat.Invert(print);
    return mat;
  }*/

  static lMatrix3f Rotate(ElAxis a, double angle) {
    lMatrix3f mat;
    mat.SetRotation(a, angle);
    return mat;
  }

  static lMatrix3f Rotate(lVec3f a, double angle) {
    lMatrix3f mat;
    mat.SetRotation(a, angle);
    return mat;
  }

  static lMatrix3f Scale(float s) {
    lMatrix3f mat;
    mat.SetScale(s);
    return mat;
  }

  /*void Invert(bool print = false) {
    lMatrix4f out;
    if(print) { printOp(&el(0,0), &out.el(0,0)); }
    /* // Old invert method
    for(int r=0;r<4;r++) {
      if(Row(r).v[r]!=1.0f) { // Sets diagonal to 1.0f
        if(print) { printf("R[%i] / %f\n\n", r, Row(r).v[r]); }
        out.Row(r, out.Row(r) / Row(r).v[r]);
        Row(r, Row(r) / Row(r).v[r]);
        if(print) { printOp(&el(0,0), &out.el(0,0)); }
      }
      for(int i=r+1;i<4;i++) { // Works numbers below to 0.0f
        if(print) { printf("R[%i] - %fR[%i]\n\n", i, Row(i).v[r], r); }
        out.Row(i, (out.Row(i)-(out.Row(r)*Row(i).v[r])));
        Row(i, (Row(i)-(Row(r)*Row(i).v[r])));
        if(print) { printOp(&el(0,0), &out.el(0,0)); }
      }
    }
    for(int r=3;r>0;r--) {
      for(int i=r-1;i>=0;i--) { // Works numbers above to 0.0f
        if(print) { printf("R[%i] - %fR[%i]\n\n", i, Row(i).v[r], r); }
        out.Row(i, (out.Row(i)-(out.Row(r)*Row(i).v[r])));
        Row(i, (Row(i)-(Row(r)*Row(i).v[r])));
        if(print) { printOp(&el(0,0), &out.el(0,0)); }
      }
    }
    
    for(int i=0;i<3;i++) { // Rearranged in acending order
      
      
    }
    if(print) {
      printf("Rearranged diagonally dominant:\n\n");
      printOp(&el(0,0), &out.el(0,0));
    }
    //
    for(int r=0;r<4;r++) { // Pivots
      float rowEl = fabs(Row(r).v[r]);
      for(int j=r+1;j<4;j++) {
        if(fabs(Row(j).v[r]) > rowEl){
          out.SwapRow(r, j);
          SwapRow(r, j);
        }
      }
      float pivotEl = Row(r).v[r];
      for(int i=0;i<4;i++) {
        if(i==r) {
          continue;
        }
        float rowEl = Col(r).v[i];
        if(print) { printf("R[%i] = %.3f*R[%i] - %.3f*R[%i]\n\n", i, pivotEl, i, rowEl, r); }
        out.Row(i, ( (out.Row(i)*pivotEl) - (out.Row(r)*rowEl) ) );
        Row(i, ( (Row(i)*pivotEl) - (Row(r)*rowEl) ) );
        if(print) { printOp(&el(0,0), &out.el(0,0)); }
      }
    }
    for(int r=0;r<4;r++) { // Divides diagonals
      float element = Row(r).v[r];
      if(print) { printf("R[%i] = R[%i] / %f\n\n", r, r, element); }
      out.Row(r, (out.Row(r)/element));
      Row(r, (Row(r)/element));
      if(print) { printOp(&el(0,0), &out.el(0,0)); }
    }
    *this = out;
  }*/

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

  void SetRotation(lVec3f a, double angle) { // Rotates about any abitrary axis
    MakeIdentity();
    a.Normalize();
    lMatrix3f toXyPlane, fromXyPlane, toXAxis, fromXAxis;
    float xRot = 0.0f;
    float zRot = 0.0f;

    if (a.z != 0) {
      xRot = atan2(a.z, a.y);
      toXyPlane = lMatrix3f::Rotate(AXIS_X, -xRot);
      fromXyPlane = lMatrix3f::Rotate(AXIS_X, xRot);
    }
    if (a.y != 0) {
      zRot = atan2(sqrt(1.0f - a.x * a.x), a.x);
      toXAxis = lMatrix3f::Rotate(AXIS_Z, -zRot);
      fromXAxis = lMatrix3f::Rotate(AXIS_Z, zRot);
    }

    *this = fromXyPlane * fromXAxis * lMatrix3f::Rotate(AXIS_X, angle) *
            toXAxis * toXyPlane;
  }

  void SetScale(float s) {
    el(0, 0) = s;
    el(1, 1) = s;
    el(2, 2) = s;
  }

  float &el(int row, int col) { return lM[(row * 3) + col]; }
  float el(int row, int col) const { return lM[(row * 3) + col]; }
};

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

  lMatrix4f(const lVec4f &r0, const lVec4f &r1, const lVec4f &r2, const lVec4f &r3) {
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

  void SwapRow(int i, int j) {
    lVec4f stowRow = Row(i);
    Row(i, Row(j));
    Row(j, stowRow);
  }

  lVec4f Col(int i) const {
    return lVec4f(el(0, i), el(1, i), el(2, i), el(3, i));
  }

  void Col(int i, const lVec4f &c) {
    el(0, i) = c.x;
    el(1, i) = c.y;
    el(2, i) = c.z;
    el(3, i) = c.w;
  }

  lMatrix4f Inverted(bool print = false) {
    lMatrix4f mat = *this;
    mat.Invert(print);
    return mat;
  }

  static lMatrix4f Rotate(ElAxis a, double angle) {
    lMatrix4f mat;
    mat.SetRotation(a, angle);
    return mat;
  }

  static lMatrix4f Rotate(lVec3f a, double angle) {
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

  void Invert(bool print = false) {
    lMatrix4f out;
    if(print) { printOp(&el(0,0), &out.el(0,0)); }
    for(int r=0;r<4;r++) { // Pivots
      float rowEl = fabs(Row(r).v[r]);
      for(int j=r+1;j<4;j++) {
        if(fabs(Row(j).v[r]) > rowEl){
          out.SwapRow(r, j);
          SwapRow(r, j);
        }
      }
      float pivotEl = Row(r).v[r];
      for(int i=0;i<4;i++) {
        if(i==r) {
          continue;
        }
        float rowEl = Col(r).v[i];
        if(print) { printf("R[%i] = %.3f*R[%i] - %.3f*R[%i]\n\n", i, pivotEl, i, rowEl, r); }
        out.Row(i, ( (out.Row(i)*pivotEl) - (out.Row(r)*rowEl) ) );
        Row(i, ( (Row(i)*pivotEl) - (Row(r)*rowEl) ) );
        if(print) { printOp(&el(0,0), &out.el(0,0)); }
      }
    }
    for(int r=0;r<4;r++) { // Divides diagonals
      float element = Row(r).v[r];
      if(print) { printf("R[%i] = R[%i] / %f\n\n", r, r, element); }
      out.Row(r, (out.Row(r)/element));
      Row(r, (Row(r)/element));
      if(print) { printOp(&el(0,0), &out.el(0,0)); }
    }
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

  void SetRotation(lVec3f a, double angle) { // Rotates about any abitrary axis
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

lMatrix4f ToMatrix4(lMatrix3f &mat3) {
  lMatrix4f mat4;
  mat4.el(0,0) = mat3.el(0,0);
  mat4.el(0,1) = mat3.el(0,1);
  mat4.el(0,2) = mat3.el(0,2);
  mat4.el(1,0) = mat3.el(1,0);
  mat4.el(1,1) = mat3.el(1,1);
  mat4.el(1,2) = mat3.el(1,2);
  mat4.el(2,0) = mat3.el(2,0);
  mat4.el(2,1) = mat3.el(2,1);
  mat4.el(2,2) = mat3.el(2,2);
  mat4.el(0,3) = 0.0f;
  mat4.el(1,3) = 0.0f;
  mat4.el(2,3) = 0.0f;
  mat4.el(3,0) = 0.0f;
  mat4.el(3,1) = 0.0f;
  mat4.el(3,2) = 0.0f;
  mat4.el(3,3) = 1.0f;
  return mat4;
}

lMatrix3f ToMatrix3(lMatrix4f &mat4) {
  lMatrix3f mat3;
  mat3.el(0,0) = mat4.el(0,0);
  mat3.el(0,1) = mat4.el(0,1);
  mat3.el(0,2) = mat4.el(0,2);
  mat3.el(1,0) = mat4.el(1,0);
  mat3.el(1,1) = mat4.el(1,1);
  mat3.el(1,2) = mat4.el(1,2);
  mat3.el(2,0) = mat4.el(2,0);
  mat3.el(2,1) = mat4.el(2,1);
  mat3.el(2,2) = mat4.el(2,2);
  return mat3;
}

class lQuaternionf {
public:
  union {
    float q[4];
    struct {
      float w, x, y, z;
    };
  };

  lQuaternionf() { Identity(); }

  lQuaternionf(float v[4]) { SetValue(v); }

  lQuaternionf(float q0, float q1, float q2, float q3) { SetValue(q0, q1, q2, q3); }

  lQuaternionf(lMatrix4f &m) { SetValue(m); }

  lQuaternionf(lVec3f axis, float radians) { SetValue(axis, radians); }

  /*
  lQuaternionf(lVec3f &rotateFrom, lVec3f &rotateTo) {
    SetValue(rotateFrom, rotateTo);
  }

  lQuaternionf(lVec3f &fromLook, lVec3f &fromUp, lVec3f &toLook, lVec3f &toUp) {
    SetValue(fromLook, fromUp, toLook, toUp);
  }
  */

  void Identity() {
    q[1] = q[1] = q[3] = 0.0f;
    q[0] = 1.0f;
  }

  void SetValue(float q0, float q1, float q2, float q3) {
    q[0] = q0;
    q[1] = q1;
    q[2] = q2;
    q[3] = q3;
  }

  void SetValue(float *ql) {
    q[0] = ql[0];
    q[1] = ql[1];
    q[2] = ql[2];
    q[3] = ql[3];
  }

  void SetValue(lMatrix4f m) {
    float t,k;
    if(m.el(2,2) < 0) {
      if(m.el(0,0) > m.el(1,1)) { // X-form
        t = 1 + m.el(0,0) - m.el(1,1) - m.el(2,2);
        k = 0.5 / sqrt(t);
        SetValue( t*k, (m.el(0,1)+m.el(1,0))*k, (m.el(2,0)+m.el(0,2))*k, (m.el(1,2)-m.el(2,1))*k );
      } else {                    // Y-form
        t = 1 - m.el(0,0) + m.el(1,1) - m.el(2,2);
        k = 0.5 / sqrt(t);
        SetValue( (m.el(0,1)+m.el(1,0))*k, t*k, (m.el(1,2)+m.el(2,1))*k, (m.el(2,0)-m.el(0,2))*k );
      }
    } else {
      if(m.el(0,0) < -m.el(1,1)) {  // Z-form
        t = 1 - m.el(0,0) - m.el(1,1) + m.el(2,2);
        k = 0.5 / sqrt(t);
        SetValue( (m.el(2,0)+m.el(0,2))*k, (m.el(1,2)+m.el(2,1))*k, t*k, (m.el(0,1)-m.el(1,0))*k );
      } else {                      // W-form
        t = 1 + m.el(0,0) + m.el(1,1) + m.el(2,2);
        k = 0.5 / sqrt(t);
        SetValue( (m.el(1,2)-m.el(2,1))*k, (m.el(2,0)-m.el(0,2))*k, (m.el(0,1)-m.el(1,0))*k, t*k );
      }
    }
  }

  void SetValue(lVec3f axis, float theta) {
    theta *= 0.5;
    float sinTheta = sin(theta);
    w = cos(theta);
    x = sinTheta * axis.x;
    y = sinTheta * axis.y;
    z = sinTheta * axis.z;
  }

  void SetValue(lVec3f rotFrom, lVec3f rotTo) {
    lVec3f p1 = rotFrom.Normalized();
    lVec3f p2 = rotTo.Normalized();
    float alpha = p1.Dot(p2);

    if(alpha > 1.0f) {
      Identity();
    } if(alpha < -1.0f) {
      lVec3f v;
      if(p1.x != p1.y) {
        v = lVec3f(p1.y, p1.x, p1.z);
      } else {
        v = lVec3f(p1.z, p1.y, p1.x);
      }
      v -= p1 * p1.Dot(v);
      v.Normalize();
      SetValue(v, M_PI);
    } else {
      p1 = p1.Cross(p2);
      p1.Normalize();
      SetValue(p1, T(acos(alpha)));
    }
  }

  float *GetValue() {
    return &q[0];
  }

  void GetValue(lVec3f &axis, float &radians) {
    radians = acos(q[0]) * 2.0f;
    if (radians == 0.0f)
      axis = lVec3f(0.0, 0.0, 1.0);
    else {
      axis.x = q[1];
      axis.y = q[2];
      axis.z = q[3];
      axis.Normalize();
    }
  }

  void GetValue(lMatrix3f &m) {
    float s,xs,ys,zs,wx,wy,wz,xx,xy,xz,yy,yz,zz;
    float norm = q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3];
    s = (norm == 0.0f) ? 0.0f : (2.0f/norm);

    xs = q[0]*s;
    ys = q[1]*s;
    zs = q[2]*s;
    wx = q[3]*xs;
    wy = q[3]*s;
    wz = q[3]*zs;
    xx = q[0]*xs;
    xy = q[0]*ys;
    xz = q[0]*zs;
    yy = q[1]*ys;
    yz = q[1]*zs;
    zz = q[2]*zs;

    m.el(0,0) = 1.0f-(yy+zz);
    m.el(1,0) = xy+wz;
    m.el(2,0) = xz-wy;
    m.el(0,1) = xy-wz;
    m.el(1,1) = 1.0f-(xx+zz);
    m.el(2,1) = yz+wx;
    m.el(0,2) = xz+wy;
    m.el(1,2) = yz-wx;
    m.el(2,2) = 1.0f-(xx+yy);
  }

  void GetValue(lMatrix4f &m) {
    lMatrix3f mat3;
    GetValue(mat3);
    m = ToMatrix4(mat3);
  }

  lMatrix3f GetMatrix3() {
    lMatrix3f mat3;
    GetValue(mat3);
    return mat3;
  }

  lMatrix4f GetMatrix4() {
    lMatrix4f mat4;
    GetValue(mat4);
    return mat4;
  }

};

lVec3f Mult(const lMatrix3f &m, const lVec3f &v) { return lVec3f(Dot(m.Row(0), v), Dot(m.Row(1), v), Dot(m.Row(2), v)); }
lVec3f Mult(const lVec3f &v, const lMatrix3f &m) { return lVec3f(Dot(m.Col(0), v), Dot(m.Col(1), v), Dot(m.Col(2), v)); }
lMatrix3f Mult(const lMatrix3f &m0, const lMatrix3f &m1) { return lMatrix3f((m0.Row(0) * m1), (m0.Row(1) * m1), (m0.Row(2) * m1)); }

lVec3f operator*(const lMatrix3f &m, const lVec3f &v) { return Mult(m, v); }
lVec3f operator*(const lVec3f &v, const lMatrix3f &m) { return Mult(v, m); }
lMatrix3f operator*(const lMatrix3f &m0, const lMatrix3f &m1) { return Mult(m0, m1); }


lVec4f Mult(const lMatrix4f &m, const lVec4f &v) { return lVec4f(Dot(m.Row(0), v), Dot(m.Row(1), v), Dot(m.Row(2), v), Dot(m.Row(3), v)); }
lVec4f Mult(const lVec4f &v, const lMatrix4f &m) { return lVec4f(Dot(m.Col(0), v), Dot(m.Col(1), v), Dot(m.Col(2), v), Dot(m.Col(3), v)); }
lMatrix4f Mult(const lMatrix4f &m0, const lMatrix4f &m1) { return lMatrix4f((m0.Row(0) * m1), (m0.Row(1) * m1), (m0.Row(2) * m1), (m0.Row(3) * m1)); }

lVec4f operator*(const lMatrix4f &m, const lVec4f &v) { return Mult(m, v); }
lVec4f operator*(const lVec4f &v, const lMatrix4f &m) { return Mult(v, m); }
lMatrix4f operator*(const lMatrix4f &m0, const lMatrix4f &m1) { return Mult(m0, m1); }


lVec3f Mult(const lVec3f &v, const float &f) { return lVec3f(v.x*f, v.y*f, v.z*f); }
lVec3f Divi(const lVec3f &v, const float &f) { return lVec3f(v.x/f, v.y/f, v.z/f); }
lVec3f add(const lVec3f &v0, const lVec3f &v1) { return lVec3f(v0.x+v1.x, v0.y+v1.y, v0.z+v1.z); }
lVec3f min(const lVec3f &v0, const lVec3f &v1) { return lVec3f(v0.x-v1.x, v0.y-v1.y, v0.z-v1.z); }
bool Equals(const lMatrix3f &m0, const lMatrix3f &m1) {
  for(int i=0;i<3;i++) {
    for(int j=0;j<3;j++) {
      if(m0.el(i,j) != m1.el(i,j)) {
        return false;
      }
    }
  }
  return true;
}

lVec3f operator*(const lVec3f &v, const float &f) { return Mult(v, f); }
lVec3f operator*(const float &f, const lVec3f &v) { return Mult(v, f); }
lVec3f operator/(const lVec3f &v, const float &f) { return Divi(v, f); }
lVec3f operator+(const lVec3f &v0, const lVec3f &v1) { return add(v0, v1); }
lVec3f operator-(const lVec3f &v0, const lVec3f &v1) { return min(v0, v1); }
bool operator==(const lMatrix3f &m0, const lMatrix3f &m1) { return Equals(m0, m1); }


lVec4f Mult(const lVec4f &v, const float &f) { return lVec4f(v.x*f, v.y*f, v.z*f, v.w*f); }
lVec4f Divi(const lVec4f &v, const float &f) { return lVec4f(v.x/f, v.y/f, v.z/f, v.w/f); }
lVec4f add(const lVec4f &v0, const lVec4f &v1) { return lVec4f(v0.x+v1.x, v0.y+v1.y, v0.z+v1.z, v0.w+v1.w); }
lVec4f min(const lVec4f &v0, const lVec4f &v1) { return lVec4f(v0.x-v1.x, v0.y-v1.y, v0.z-v1.z, v0.w-v1.w); }
bool Equals(const lMatrix4f &m0, const lMatrix4f &m1) {
  for(int i=0;i<4;i++) {
    for(int j=0;j<4;j++) {
      if(m0.el(i,j) != m1.el(i,j)) {
        return false;
      }
    }
  }
  return true;
}

lVec4f operator*(const lVec4f &v, const float &f) { return Mult(v, f); }
lVec4f operator/(const lVec4f &v, const float &f) { return Divi(v, f); }
lVec4f operator+(const lVec4f &v0, const lVec4f &v1) { return add(v0, v1); }
lVec4f operator-(const lVec4f &v0, const lVec4f &v1) { return min(v0, v1); }
bool operator==(const lMatrix4f &m0, const lMatrix4f &m1) { return Equals(m0, m1); }


lQuaternionf Mult(lQuaternionf q0, lQuaternionf q1) {
  lVec3f q0axis, q1axis;
  float q0real, q1real;
  q0.GetValue(q0axis, q0real);
  q1.GetValue(q1axis, q1real);
  return lQuaternionf((Cross(q0axis,q1axis)+(q0real*q1axis)+(q1real*q0axis)),((q0real*q1real)-(Dot(q0axis,q1axis))));
}
lVec3f Mult(lQuaternionf q, lVec3f v) {
  float vCo = (q.w*q.w) - (q.x*q.x) - (q.y*q.y) - (q.z*q.z);
  float uCo = 2.0f*(v.x*q.x + v.y*q.y + v.z*q.z);
  float cCo = 2.0f*q.w;
  return lVec3f((vCo*v.x + uCo*q.x + cCo*(q.y*v.z - q.z*v.y)),(vCo*v.y + uCo*q.y + cCo*(q.z*v.x - q.x*v.z)),(vCo*v.z + uCo*q.z + cCo*(q.x*v.y - q.y*v.x)));
}

lQuaternionf operator*(lQuaternionf q0, lQuaternionf q1) { Mult(q0,q1); }
lVec3f operator*(lQuaternionf q, lVec3f v) { Mult(q,v); }

void printOp(float *m0, float *m1) {
  printf("%.3f, %.3f, %.3f, %.3f\t%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\t%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\t%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\t%.3f, %.3f, %.3f, %.3f\n\n",
    m0[0], m0[1], m0[2], m0[3],      m1[0], m1[1], m1[2], m1[3],
    m0[4], m0[5], m0[6], m0[7],      m1[4], m1[5], m1[6], m1[7],
    m0[8], m0[9], m0[10], m0[11],    m1[8], m1[9], m1[10], m1[11],
    m0[12], m0[13], m0[14], m0[15],  m1[12], m1[13], m1[14], m1[15]);
}

float Mat2Det(float a, float b, float c, float d) {
  return (a*d) - (b*c);
}
float Mat3Det(lMatrix3f m) {
  return (m.lM[0]*Mat2Det(m.lM[4],m.lM[5],m.lM[8],m.lM[7])) - 
  (m.lM[1]*Mat2Det(m.lM[3],m.lM[5],m.lM[6],m.lM[7])) + 
  (m.lM[2]*Mat2Det(m.lM[3],m.lM[4],m.lM[6],m.lM[8]));
}
float Mat4Det(lMatrix4f m) {
  lMatrix3f a(m.lM[5], m.lM[6], m.lM[7], m.lM[9], m.lM[10], m.lM[11], m.lM[13], m.lM[14], m.lM[15]);
  lMatrix3f b(m.lM[4], m.lM[6], m.lM[7], m.lM[8], m.lM[10], m.lM[11], m.lM[12], m.lM[14], m.lM[15]);
  lMatrix3f c(m.lM[4], m.lM[5], m.lM[7], m.lM[8], m.lM[9], m.lM[11], m.lM[12], m.lM[13], m.lM[15]);
  lMatrix3f d(m.lM[4], m.lM[5], m.lM[6], m.lM[8], m.lM[9], m.lM[10], m.lM[12], m.lM[13], m.lM[14]);
  return (m.lM[0]*Mat3Det(a)) - (m.lM[1]*Mat3Det(b)) + (m.lM[2]*Mat3Det(c)) - (m.lM[3]*Mat3Det(d));
}

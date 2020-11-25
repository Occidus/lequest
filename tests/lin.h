#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

class lVec4f {
public:
    union {
        float v[4];
        struct
        {
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

};

float Dot(lVec4f a, lVec4f b) {
    return (a.v[0] * b.v[0]) + (a.v[1] * b.v[1]) + (a.v[2] * b.v[2]) + (a.v[3] * b.v[3]);
}

class lMatrix4f {
public:
    float lM[16];

    lMatrix4f() {
        MakeIdentity();
    }

    lMatrix4f(float f00, float f01, float f02, float f03, float f10, float f11, float f12, float f13, float f20, float f21, float f22, float f23, float f30, float f31, float f32, float f33) {
        el(0,0) = f00;
        el(0,1) = f01;
        el(0,2) = f02;
        el(0,3) = f03;

        el(1,0) = f10;
        el(1,1) = f11;
        el(1,2) = f12;
        el(1,3) = f13;

        el(2,0) = f20;
        el(2,1) = f21;
        el(2,2) = f22;
        el(2,3) = f23;

        el(3,0) = f30;
        el(3,1) = f31;
        el(3,2) = f32;
        el(3,3) = f33;
    }

    lMatrix4f(const lVec4f& r0, const lVec4f& r1, const lVec4f& r2, const lVec4f& r3) {
        Row(0, r0);
        Row(1, r1);
        Row(2, r2);
        Row(3, r3);
    }

    void MakeIdentity() {
        el(0,0) = 1.0;
        el(0,1) = 0.0;
        el(0,2) = 0.0;
        el(0,3) = 0.0;

        el(1,0) = 0.0;
        el(1,1) = 1.0;
        el(1,2) = 0.0;
        el(1,3) = 0.0;

        el(2,0) = 0.0;
        el(2,1) = 0.0;
        el(2,2) = 1.0;
        el(2,3) = 0.0;

        el(3,0) = 0.0;
        el(3,1) = 0.0;
        el(3,2) = 0.0;
        el(3,3) = 1.0;
    }

    lVec4f Row(int i) const {
        return lVec4f(el(i,0), el(i,1), el(i,2), el(i,3));
    }

    void Row(int i, const lVec4f& r){
        el(i,0) = r.x;
        el(i,1) = r.y;
        el(i,2) = r.z;
        el(i,3) = r.w;
    }

    lVec4f Col(int i) const {
        return lVec4f(el(0,i), el(1,i), el(2,i), el(3,i));
    }

    static lMatrix4f Scale(float s) {
        lMatrix4f mat;
        mat.SetScale(s);
        return mat;
    }

    static lMatrix4f Translate(float x, float y, float z) {
        lMatrix4f mat;
        mat.SetTranslate(x,y,z);
        return mat;
    }

    void SetScale(float s) {
        el(0,0) = s;
        el(1,1) = s;
        el(2,2) = s;
    }

    void SetTranslate(float x, float y, float z) {
        el(0, 3) = x;
        el(1, 3) = y;
        el(2, 3) = z;
    }

    float& el(int row, int col){
        return lM[(row * 4) + col];
    }

    float el(int row, int col) const {
        return lM[(row * 4) + col];
    }

    /*lVec4f operator*(const lVec4f vec) {
        float x = (el(0,0)*vec.v[0])+(el(1,0)*vec.v[0])+(el(2,0)*vec.v[0])+(el(3,0)*vec.v[0]);
        float y = (el(0,1)*vec.v[1])+(el(1,1)*vec.v[1])+(el(2,1)*vec.v[1])+(el(3,1)*vec.v[1]);
        float z = (el(0,2)*vec.v[2])+(el(1,2)*vec.v[2])+(el(2,2)*vec.v[2])+(el(3,2)*vec.v[2]);
        float w = (el(0,3)*vec.v[3])+(el(1,3)*vec.v[3])+(el(2,3)*vec.v[3])+(el(3,3)*vec.v[3]);
        return lVec4f(x,y,z,w);
    }
    */
};

lVec4f Mult(const lMatrix4f& m, const lVec4f& v) {
    return lVec4f(Dot(m.Row(0), v), Dot(m.Row(1), v), Dot(m.Row(2), v), Dot(m.Row(3), v));
}

lVec4f operator*(const lMatrix4f& m, const lVec4f& v){
    return Mult(m,v);
}

lVec4f Mult(const lVec4f& v, const lMatrix4f& m) {
    return lVec4f(Dot(m.Col(0), v), Dot(m.Col(1), v), Dot(m.Col(2), v), Dot(m.Col(3), v));
}

lVec4f operator*(const lVec4f& v, const lMatrix4f& m){
    return Mult(v,m);
}

lMatrix4f Mult(const lMatrix4f& m0, const lMatrix4f& m1) {
    return lMatrix4f((m0.Row(0) * m1), (m0.Row(1) * m1), (m0.Row(2) * m1), (m0.Row(3) * m1));
}

lMatrix4f operator*(const lMatrix4f& m0, const lMatrix4f& m1) {
    return Mult(m0,m1);
}
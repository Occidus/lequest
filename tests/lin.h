#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

class lVec4f {
public:
    float v[4];

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

    static lMatrix4f Scale(float s) {
        lMatrix4f mat;
        mat.SetScale(s);
        return mat;
    }

    void SetScale(float s) {
        el(0,0) = s;
        el(1,1) = s;
        el(2,2) = s;
    }

    float& el(int row, int col) {
        return lM[(row * 4) + col];
    }

    lVec4f operator*(const lVec4f vec) {
        float x = (el(0,0)*vec.v[0])+(el(1,0)*vec.v[0])+(el(2,0)*vec.v[0])+(el(3,0)*vec.v[0]);
        float y = (el(0,1)*vec.v[1])+(el(1,1)*vec.v[1])+(el(2,1)*vec.v[1])+(el(3,1)*vec.v[1]);
        float z = (el(0,2)*vec.v[2])+(el(1,2)*vec.v[2])+(el(2,2)*vec.v[2])+(el(3,2)*vec.v[2]);
        float w = (el(0,3)*vec.v[3])+(el(1,3)*vec.v[3])+(el(2,3)*vec.v[3])+(el(3,3)*vec.v[3]);
        return lVec4f(x,y,z,w);
    }

    /*
    lMatrix4f operator*(const lMatrix4f mat) {
    }
    */
};
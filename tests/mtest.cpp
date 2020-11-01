#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "linear.h"
#include "VrApi_Helpers.h"

using namespace r3;

//g++ --std=c++11 -I ../vrapp/Src -I ../VrApi/Include/  mtest.cpp -o mtest

const Matrix4f& ToR3(const ovrMatrix4f & om) {
    return *reinterpret_cast<const Matrix4f*>(&om);
}

const ovrMatrix4f& ToOvr(const Matrix4f & op) {
    return *reinterpret_cast<const ovrMatrix4f*>(&op);
}

void printMatrix(const float* m) {
    printf("%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n\n",
        m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);
}

int main(int argc, char **argv) {

    float a = argc > 1 ? atof(argv[1]) : 42.f;
    float b = argc > 2 ? atof(argv[2]) : 43.f;
    float c = argc > 3 ? atof(argv[3]) : 44.f;
    float d = argc > 4 ? atof(argv[4]) : 45.f;

	const ovrMatrix4f ovrProjMat = ovrMatrix4f_CreateProjectionAsymmetricFov(a, b, c, d, 0.1f, 10.0f);
	ovrMatrix4f ovrTangledMat = ovrMatrix4f_TanAngleMatrixFromProjection(&ovrProjMat);

	ovrMatrix4f ovrUntangledMat = {
        {{ovrTangledMat.M[0][0] / 0.5f, 0.0f, (ovrTangledMat.M[0][2] + 0.5f) / 0.5f, 0.0f},
         {0.0f, ovrTangledMat.M[1][1] / 0.5f, (ovrTangledMat.M[1][2] + 0.5f) / 0.5f, 0.0f},
         {0.0f, 0.0f, ovrTangledMat.M[3][0], ovrTangledMat.M[3][1]},
         {0.0f, 0.0f, ovrTangledMat.M[3][2], 0.0f}}};
    Matrix4f untangledMat = ToR3(ovrUntangledMat);

    float leftDegrees;
    float rightDegrees;
    float upDegrees;
    float downDegrees;
    ovrMatrix4f_ExtractFov(&ovrUntangledMat, &leftDegrees, &rightDegrees, &upDegrees, &downDegrees);

    printf("%.1f, %.1f, %.1f, %.1f.\n", leftDegrees, rightDegrees, upDegrees, downDegrees);
    return 0;
}
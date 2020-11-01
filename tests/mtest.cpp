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

int main(int argc, char **argv) {

	ovrMatrix4f ovrProjMat = ovrMatrix4f_CreateProjectionAsymmetricFov(-45.0f, 45.0f, -45.0f, 45.0f, 0.1f, 10.0f);
	ovrMatrix4f ovrTangledMat = ovrMatrix4f_TanAngleMatrixFromProjection(ovrProjMat);
	Matrix4f tangledMat = ToR3(ovrTangledMat);

	Matrix4f untangledMat = {
        {{tangledMat.el(0,0) / 0.5f, 0.0f, (tangledMat.el(0,2) + 0.5f) / 0.5f, 0.0f},
         {0.0f, tangledMat.el(1,1) / 0.5f, (tangledMat.el(1,2) + 0.5f) / 0.5f, 0.0f},
         {0.0f, 0.0f, -1.0f, 0.0f},
         // Store the values to convert a clip-Z to a linear depth in the unused matrix elements.
         {tangledMat.el(3,0), tangledMat.el(3,1), tangledMat.el(3,2), 1.0f}}};
    ovrMatrix4f ovrUntangledMat = ToOvr(tangledMat);
    float leftDegrees;
    float rightDegrees;
    float upDegrees;
    float downDegrees;
    ovrMatrix4f_ExtractFov(ovrUntangledMat, leftDegrees, rightDegrees, upDegrees, downDegrees);

    printf("%.1f, %.1f, %.1f, %.1f.\n", leftDegrees, rightDegrees, upDegrees, downDegrees);
    return 0;
}
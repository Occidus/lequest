// generate some random matrices to test inversion

#include <stdlib.h>

namespace {
	float randfloat(float lower, float upper) {
		return (float(rand()) / RAND_MAX) * (upper - lower) - lower;
	}

}

// pass a pointer to 16 floats
void randmat(float *m) {
	for (int i=0; i < 16; i++) {
		m[i] = randfloat(-2.0f, 2.0f);
	}
}

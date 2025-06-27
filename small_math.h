#ifndef SMALL_MATH_H
#define SMALL_MATH_H

// Simple Taylor series for sine and cosine (radians, float)
static float my_sinf(float x) {
    float x2 = x * x;
    float res = x;
    float term = x;
    term *= -x2 / (2*3);
    res += term;
    term *= -x2 / (4*5);
    res += term;
    term *= -x2 / (6*7);
    res += term;
    term *= -x2 / (8*9);
    res += term;
    return res;
}
static float my_cosf(float x) {
    float x2 = x * x;
    float res = 1.0f;
    float term = 1.0f;
    term *= -x2 / (1*2);
    res += term;
    term *= -x2 / (3*4);
    res += term;
    term *= -x2 / (5*6);
    res += term;
    term *= -x2 / (7*8);
    res += term;
    return res;
}

#endif

#ifndef _SPLINES_H_
#define _SPLINES_H_
#define VERSION 1

#include "exception.h"

void logspline1
    (
        double* yy,
        const double* y,
        const double* w,
        int n,
        double p
    ) throw(math::exception);

void logspline3
    (
        double* yy,
        const double* y,
        const double* w,
        int n,
        double p
    ) throw(math::exception);

#endif // _SPLINES_H_

#include "splines.h"
#include "smoothing_splines.h"

void logspline1
    (double* yy, const double* y, const double* w, int n, double p) throw(math::exception)
{
    if(p<0)
    {
        THROW("Smooth parameter should be >= 0! \n")
    }

    //copy input data to temporary arrays
    double* y1 = new double[n];
    double* w1 = new double[n];

    for(int i = 0; i < n; i++)
    {
        if(y[i] < 1.0)
        {
            THROW("Only values that are >= 1.0 are allowed in the input data array!\n")
        }
        else
            if(w[i] <= 0)
            {
                THROW("Only non-negative weightings are allowed!\n")
            }
            else
            {
                y1[i] = y[i];
                w1[i] = w[i];
            }
    }

    math::log_fst_order_smoothing_spline_eq<double>(n,y1,w1,p,yy);

    //Free memory
    delete [] y1;
    delete [] w1;
}

void logspline3
    (double* yy, const double* y, const double* w, int n, double p) throw(math::exception)
{
    if(p<0)
    {
        THROW("Smooth parameter should be >= 0! \n")
    }

    //copy input data to temporary arrays
    double* y1 = new double[n];
    double* w1 = new double[n];

    for(int i = 0; i < n; i++)
    {
        if(y[i] < 1.0)
        {
            THROW("Only values that are >= 1.0 are allowed in the input data array!\n")
        }
        else
            if(w[i] <= 0)
            {
                THROW("Only non-negative weightings are allowed!\n")
            }
            else
            {
                y1[i] = y[i];
                w1[i] = w[i];
            }
    }

    math::log_third_order_smoothing_spline_eq<double>(n,y1,w1,p,yy);

    //Free memory
    delete [] y1;
    delete [] w1;
}

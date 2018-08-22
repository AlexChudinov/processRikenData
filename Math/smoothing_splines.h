#ifndef _SMOOTHING_SPLINES_
#define _SMOOTHING_SPLINES_

#include "Solvers.h"
#include "cmath"


namespace math
{
/*Linear splines:*/
    template <class DataType>
    int fst_order_smoothing_spline_eq
    //smooth data using linear interpolation on equally spaced mesh (h = 1)
        (
            int n, //number of data points
            DataType* y, //initial data array
            DataType* w, //weightings
            DataType lambda, //smoothing parameter
            DataType* yy //smoothed data
        )
    {
        DataType* r = new DataType[n]; //allocate memory to save initial data
        for(int i = 0; i < n; i++) r[i] = y[i];

        //allocate matrix coefficients
        DataType* a = new DataType[n-1];
        DataType* b = new DataType[n];
        DataType* c = new DataType[n-1];

        //Estimate matrix coefficients
        b[0] = 1.0 + lambda*w[0];
        for(int i = 0; i < n-1; i++)
        {
            a[i] = -lambda*w[i+1];
            b[i+1] = 1.0 + 2.0*lambda*w[i+1];
            c[i] = -lambda*w[i];
        }
        b[n-1] = b[n-1] - lambda*w[n-1]; //correct last value

        //Estimate smoothed data
        int res_flag = tridiagonalsolve(n,a,b,c,r,yy);

        delete [] r;
        delete [] a;
        delete [] b;
        delete [] c;
        return res_flag;
    }

    template <class DataType>
    int third_order_smoothing_spline_eq
    //smooth data using linear interpolation on equally spaced mesh (h = 1)
        (
            int n, //number of data points
            DataType* y, //initial data array
            DataType* w, //weightings
            DataType lambda, //smoothing parameter
            DataType* yy //smoothed data
        )
    {
        DataType* bb= new DataType[n]; //vector of spline quadratic coefficients
        DataType* r = new DataType[n]; //right-hand side values
        r[0]  = 0.0;
        r[n-1]= 0.0;
        w[0]  = lambda*w[0];
        w[n-1]= lambda*w[n-1];
        for(int i = 1; i<n-1; i++)
        {
            r[i] = y[i-1] - 2.0 *y[i] + y[i+1];
            w[i] = lambda*w[i];
        }

        //Allocate coefficients for a linear equations system
        DataType* a = new DataType[n-2];
        DataType* b = new DataType[n-1];
        DataType* c = new DataType[n];
        DataType* d = new DataType[n-1];
        DataType* e = new DataType[n-2];
        //Boundary coefficients
        b[0] = -w[0] - 2*w[1] + 1.0;
        c[0] = 1.0;
        d[0] = 0.0;
        e[0] = 0.0;
        //
        a[n-3] = 0.0;
        b[n-2] = 0.0;
        c[n-1] = 1.0;
        d[n-2] = -2*w[n-2] - w[n-1] + 1.0;
        //
        for(int i = 0; i < n-3; i++)
        {
            a[i]   = w[i+1];
            b[i+1] = -2.0*w[i+1] - 2.0*w[i+2] + 1.0;
            c[i+1] = w[i] + 4.0*w[i+1] + w[i+2] + 4.0;
            d[i+1] = -2.0*w[i+1] - 2.0*w[i+2] + 1.0;
            e[i+1] = w[i+2];
        }
        c[n-2] = w[n-3] + 4.0*w[n-2] + w[n-1] + 4.0; //set last one value at main diagonal

        int res_flag = fivediagonalsolve(n,a,b,c,d,e,r,bb); //estimate quadratic spline coefficient

        if( res_flag != 0 ) return res_flag; //some errors occur during estimation

        yy[0]  = y[0] -  (bb[1]  - bb[0])  * w[0];
        yy[n-1]= y[n-1]- (bb[n-2]- bb[n-1])* w[n-1];
        for(int i = 1; i<n-1; i++) yy[i] = y[i] - (bb[i-1] - 2.0*bb[i] + bb[i+1])*w[i];

        delete [] bb;
        delete [] r;
        delete [] a;
        delete [] b;
        delete [] c;
        delete [] d;
        delete [] e;
        return(0);
    }

/*Logarithmic splines*/
    template <class DataType>
    int log_fst_order_smoothing_spline_eq
    //smooth data using logarithm scale
    //all values in y array should be greater than 1 to aware about infinite logarithms
    //if one of y values is not, then function returns 1
        (
            int n, //number of data points
            DataType* y, //initial data array
            DataType* w, //weightings
            DataType lambda, //smoothness parameter
            DataType* yy //smoothed data
        )
    {

        //Estimate logarithm values
        DataType* logy = new DataType[n];
        DataType* logw = new DataType[n];
        for(int i = 0; i < n; i++)
            if(y[i] < 1.0)
            {
                return 1;
            }
            else
            {
                logy[i] = ::log(y[i]);
                logw[i] = w[i]/(y[i]*y[i]);
            }

        //Estimate smoothed logarithms
        int res_flag = fst_order_smoothing_spline_eq(n,logy,logw,lambda,yy);
        if(res_flag != 0) return res_flag; //some errors occurred

        //Transform data back to initial scale
        for(int i = 0; i<n; i++) yy[i] = ::exp(yy[i]);

        delete [] logy;
        delete [] logw;
        return 0;
    }

    template <class DataType>
    int log_third_order_smoothing_spline_eq
    //smooth data using logarithm scale
    //all values in y array should be greater than 1 to aware about infinite logarithms
    //if one of y values is not, then function returns 1
        (
            int n, //number of data points
            DataType* y, //initial data array
            DataType* w, //weightings
            DataType lambda, //smoothness parameter
            DataType* yy //smoothed data
        )
    {

        //Estimate logarithm values
        DataType* logy = new DataType[n];
        DataType* logw = new DataType[n];
        for(int i = 0; i < n; i++)
            if(y[i] < 1.0)
            {
                return 1;
            }
            else
            {
                logy[i] = ::log(y[i]);
                logw[i] = w[i]/(y[i]*y[i]);
            }

        //Estimate smoothed logarithms
        int res_flag = third_order_smoothing_spline_eq(n,logy,logw,lambda,yy);
        if(res_flag != 0) return res_flag; //some errors occurred

        //Transform data back to initial scale
        for(int i = 0; i<n; i++) yy[i] = ::exp(yy[i]);

        delete [] logy;
        delete [] logw;
        return 0;
    }
}

#endif // _SMOOTHING_SPLINES_

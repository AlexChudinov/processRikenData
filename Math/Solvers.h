#ifndef SOLVERS_H
#define SOLVERS_H

#include "exception.h"

#include <sstream>

namespace math
{
    /*System of equations with symmetric diagonal matrix*/
    template<class DataType> int tridiagonalsolve //tridiagonal bound matrix
        (
            int n,          //number of equations
            DataType* a,
            DataType* b,    //main diagonal
            DataType* c,
            DataType* r,    //right-hand part
            DataType* x     //solution
        ) throw();

    template<class DataType> int fivediagonalsolve //five diagonal bound matrix
        (
            int n,          //number of equations
            DataType* a,
            DataType* b,
            DataType* c,    //main diagonal
            DataType* d,
            DataType* e,
            DataType* r,    //right-hand part
            DataType* x     //solution
        ) throw();

    template <class DataType>
    int tridiagonalsolve
        (
            int n, //number of equations
            DataType* a,  //down diagonal
            DataType* b,  //main diagonal
            DataType* c,  //upper diagonal
            DataType* r,  //right-hand part
            DataType* x   //solution
        ) throw()
    //solve Ax=b where A is a tridiagonal matrix, returns 0 if it is ok
    //It changes inserted data array b and r. It was taken from
    //https://vk.com/pages?oid=-30481242&p=%D0%9C%D0%B5%D1%82%D0%BE%D0%B4_%D0%BF%D1%80%D0%BE%D0%B3%D0%BE%D0%BD%D0%BA%D0%B8
    {
        for (int i = 0; i < n-1; i++)
        {
            DataType m = a[i] / b[i];
            b[i+1] = b[i+1] - m * c[i];
            r[i+1] = r[i+1] - m * r[i];
        }
        x[n - 1] = r[n - 1] / b[n - 1];
        for (int i = n - 2; i >= 0; i--)
                x[i] = (r[i] - c[i] * x[i + 1]) / b[i];
        return 0;
    }

    template<class DataType> int fivediagonalsolve
        (
            int n,          //number of equations
            DataType* a,
            DataType* b,
            DataType* c,    //main diagonal
            DataType* d,
            DataType* e,
            DataType* r,    //right-hand part
            DataType* x     //solution
        ) throw()
    {
        for(int i = 0; i < n-2; i++)
        {
            DataType m1 = b[i]/c[i];
            DataType m2 = a[i]/c[i];
            c[i+1] = c[i+1] - m1*d[i];
            d[i+1] = d[i+1] - m1*e[i];
            b[i+1] = b[i+1] - m2*d[i];
            c[i+2] = c[i+2] - m2*e[i];
            r[i+1] = r[i+1] - m1*r[i];
            r[i+2] = r[i+2] - m2*r[i];
        }
        DataType m3 = b[n-2]/c[n-2];
        c[n-1] = c[n-1] - m3*d[n-2];
        r[n-1] = r[n-1] - m3*r[n-2];
        x[n-1] = r[n-1] / c[n-1];
        x[n-2] = (r[n-2] - d[n-2]*x[n-1]) / c[n-2];

        for(int i = n-3; i >= 0; i--)
            x[i] = (r[i] - d[i]*x[i+1] - e[i]*x[i+2]) / c[i];

        return 0;
    }

    /**
     * simple bisection solver
     */
    template<class Fun>
    double froot(Fun fun, double a, double b)
    {
        if(fun(a) > fun(b)) std::swap(a, b);
        if(fun(a) * fun(b) > 0.0)
            return std::fabs(fun(a)) > std::fabs(fun(b)) ? b : a;
        while(std::fabs((b - a) / (b + a)) > 1e-12)
        {
            double m = .5 * (a + b);
            if(fun(m) == 0.0) return m;
            else if(fun(m) > 0.0) b = m;
            else a = m;
        }
        return .5 * (a + b);
    }
}

#endif // SOLVERS_H

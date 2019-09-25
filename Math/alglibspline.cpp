#include "alglibspline.h"
#include "Solvers.h"
#include "Math/alglib/interpolation.h"

AlglibSpline::AlglibSpline(const QVariantMap& pars)
    :
      Smoother (pars, paramsTemplate()),
      mM(paramPtr<int>("Spline nodes:"))
{
}

Smoother::Type AlglibSpline::type() const
{
    return Smoother::AlglibSplineType;
}

void AlglibSpline::run(VectorDouble &yOut, const VectorDouble &yIn)
{
    if(inputCheck(yOut, yIn))
    {
        alglib::ae_int_t N = static_cast<alglib::ae_int_t>(yIn.size());
        alglib::real_1d_array xx, yy, ww;
        xx.setlength(N);
        yy.setlength(N);
        ww.setlength(N);
        double x0 = 0.;
        for(int i = 0; i < N; ++i)
        {
            ww[i] = std::sqrt(yIn[i] + 1.);
            yy[i] = std::log(yIn[i] + 1.);
            xx[i] = x0++;
        }
        alglib::spline1dinterpolant s;
        auto chiSquare = [&](double rho)->double
        {
            alglib::ae_int_t info;
            alglib::spline1dfitreport rep;
            alglib::spline1dfitpenalizedw(xx, yy, ww, N, *mM, rho, info, s, rep);
            int NN = 0;
            double sig = 0.;
            for(int i = 0; i < N; ++i)
            {
                const double y = std::exp(alglib::spline1dcalc(s, xx[i])) - 1.;
                if(y >= 1.)
                {
                    double dy = yIn[i] - y;
                    sig += dy * dy / y;
                    NN++;
                }
            }
            return sig/NN - 1.;
        };

        double rho = math::froot(chiSquare, -8., 8.);

        alglib::ae_int_t info;
        alglib::spline1dfitreport rep;
        alglib::spline1dfitpenalizedw(xx, yy, ww, N, *mM, rho, info, s, rep);

        for(int i = 0; i < N; ++i)
        {
            yOut[i] = std::exp(spline1dcalc(s, xx[i])) - 1.;
        }
    }
}

QVariantMap AlglibSpline::paramsTemplate() const
{
    return QVariantMap{{"Spline nodes:", QVariant(4)}};
}

void AlglibSpline::setParams(const QVariantMap &params)
{
    Smoother::setParams(params);
    mM = paramPtr<int>("Spline nodes:");
}

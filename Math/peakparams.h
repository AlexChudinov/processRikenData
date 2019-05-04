#ifndef PEAKPARAMS_H
#define PEAKPARAMS_H

/**
 * @brief The PeakParams class defines interface to obtain peak
 * parameters from any fitting procedure
 */
class PeakParams
{
public:
    PeakParams();
    virtual ~PeakParams();

    virtual double peakPosition() const = 0;
    virtual double peakPositionUncertainty() const = 0;
};

#endif // PEAKPARAMS_H

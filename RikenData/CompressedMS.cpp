#include <random>
#include <cmath>

#include "CompressedMS.h"
#include "Math/Smoother.h"
#include "Math/ParSplineCalc.h"

std::mt19937_64 CompressedMS::s_gen;

CompressedMS::CompressedMS(const CompressedMS::VectorInt &vVals,
                           size_t tMin, Interpolator::InterpType type)
{
    size_t x0 = 0, x1 = 1;
    while(x1 < vVals.size() && vVals[x1] == 0) x0 = x1++;
    CompressedMS::Map tab;
    bool bPeakBack = false;
    while(x1 < vVals.size())
    {
        //If it is equal to zero, then check previous value
        if(vVals[x1] == 0)
        {
            //...ok, it is not zero, then write it
            if(vVals[x0] != 0)
            {
                bPeakBack = true;
                tab[x1 + tMin] = vVals[x1];
            }
            else bPeakBack = false;
            //...otherwise skip it
        }
        else //write just not zero values
        {
            //If previous value was equal to zero and it was not directly behind previous peak
            //...then write it too
            if(vVals[x0] == 0 && !bPeakBack) tab[x0 + tMin] = 0;
            tab[x1 + tMin] = vVals[x1];
        }
        x0 = x1++;
    }
    m_pInterpolator.reset(Interpolator::create(type, std::move(tab)).release());
}

CompressedMS::CompressedMS
(
    const CompressedMS::VectorDouble &vVals,
    size_t tMin,
    Interpolator::InterpType type
)
{
    VectorInt vValsInt(vVals.size());
    for(size_t i = 0; i < vVals.size(); ++i)
    {
        vValsInt[i] = static_cast<size_t>(std::round(vVals[i]));
    }
    *this = CompressedMS(vValsInt, tMin, type);
}

CompressedMS::CompressedMS(const Map& data, Interpolator::InterpType type)
    :
      m_pInterpolator(Interpolator::create(type, data))
{
}

CompressedMS::CompressedMS(Map &&data, Interpolator::InterpType type)
    :
      m_pInterpolator(Interpolator::create(type, data))
{
}

CompressedMS::CompressedMS(const CompressedMS &ms)
    :
      m_pInterpolator(Interpolator::create(ms.interp()->type(), ms.interp()->table()))
{
	m_pInterpolator->xFactor(ms.interp()->xFactor());
	m_pInterpolator->yFactor(ms.interp()->yFactor());
}

CompressedMS::CompressedMS(CompressedMS&& ms)
    :
      m_pInterpolator(Interpolator::create(ms.interp()->type(), std::move(ms.interp()->table())))
{
	m_pInterpolator->xFactor(ms.interp()->xFactor());
	m_pInterpolator->yFactor(ms.interp()->yFactor());
}

CompressedMS &CompressedMS::operator=(const CompressedMS &ms)
{
    m_pInterpolator.reset(Interpolator::create(ms.interp()->type(), ms.interp()->table()).release());
    return *this;
}

CompressedMS &CompressedMS::operator=(CompressedMS&& ms)
{
    m_pInterpolator.reset
    (
        Interpolator::create(ms.interp()->type(), std::move(ms.interp()->table())).release()
    );
    return *this;
}


CompressedMS::~CompressedMS()
{

}

void CompressedMS::squeezeXScale(double s)
{
    if(s>-1.0 && s<1.0) m_pInterpolator->xFactor(1. + s);
}

double CompressedMS::match(const CompressedMS &msRef) const
{
    const Map& tabRef = msRef.interp()->table();
    double res = 0;

    for(Map::const_reference e : tabRef)
        res += e.second * interp()->interpolate(e.first);

    return res;
}

double CompressedMS::bestMatch(const CompressedMS &msRef, size_t nMaxTime, size_t nTimeInterval) const
{
    VectorDouble vMatchVals(2*nTimeInterval + 1);
    VectorInt vIdx(vMatchVals.size());
    std::iota(vIdx.begin(), vIdx.end(), 0);
    VectorDouble sVals(vMatchVals.size());

    QtConcurrent::map<VectorInt>(vIdx, [&](size_t i)->void
    {
        CompressedMS temp(*this);
        sVals[i] = (double(i) - double(nTimeInterval)) / double(nMaxTime);
        temp.squeezeXScale(sVals[i]);
        vMatchVals[i] = temp.match(msRef);
    }).waitForFinished();

	return sVals
		[
			std::distance
			(
				vMatchVals.begin(),
				std::max_element(vMatchVals.begin(), vMatchVals.end())
			)
		];
}

void CompressedMS::addToAcc(CompressedMS &msAcc) const
{
    const Map& tab = interp()->table();
    Map& tabAcc = msAcc.interp()->table();
    for(const auto& e : tab)
    {
        Map::iterator it = tabAcc.find(e.first);
        if(it != tabAcc.end()) it->second += e.second;
        else tabAcc[e.first] = e.second;
    }
}

void CompressedMS::rescale()
{
    uint32_t tMin = static_cast<uint32_t>(interp()->minX());
    uint32_t tMax = static_cast<uint32_t>(interp()->maxX()) + 1;
    VectorInt vVals(tMax - tMin + 1);
    QtConcurrent::map<VectorInt>(vVals, [&](VectorInt::reference val)->void
    {
        const size_t i = &val - &vVals[0];
        val = static_cast<size_t>(std::round(interp()->interpolate(i+tMin)));
    }).waitForFinished();

    *this = CompressedMS(vVals, tMin, interp()->type());
}

Peak::PeakCollection CompressedMS::smooth(Smoother *s)
{
    Peak::PeakCollection res;
    if(s)
    {
        if(interp()->xFactor() != 1.0) rescale();
        VectorDouble yOut;
        s->run(yOut, transformToVector());
        const size_t tMin = static_cast<size_t>(interp()->minX());
        const size_t n = yOut.size();
        QVariantMap::ConstIterator it = s->params().find(NOISE_LEVEL);
        const double noise = it != s->params().end() ?
                    it.value().toDouble() : 1.0;

        QMutex mutex;
        QtConcurrent::map<VectorDouble::const_iterator>
        (
            std::next(yOut.begin()),
            std::prev(yOut.end()),
            [&](VectorDouble::const_reference yRef)->void
        {
			const size_t i = &yRef - &yOut[0];

            if(yOut[i-1] < yOut[i] && yOut[i+1] < yOut[i])
            {
                double left, right;
                Peak peak;
                peak.parabolicMaximum
                (
                    static_cast<double>(i) + tMin,
                    yOut[i-1],
                    yOut[i],
                    yOut[i+1]
                );
                //look for right and left peak sides
                size_t lhi = i, rhi = i;
                while(lhi != 0 && yOut[lhi] > peak.height()/2.) --lhi;
                while (rhi != n-1 && yOut[rhi] > peak.height()/2.) ++rhi;
                left = static_cast<double>(lhi);
                right = static_cast<double>(rhi);
                if(lhi != 0)
                    left += (peak.height()/2. - yOut[lhi])/(yOut[lhi+1] - yOut[lhi]);
                if(rhi != n-1)
                    right += (peak.height()/2. - yOut[rhi])/(yOut[rhi-1] - yOut[rhi]);
                peak.setLeft(left+tMin);
                peak.setRight(right+tMin);
                //Write down new peak
                if(yOut[i] > 1.0 && right - left > 2.0)
                {
					peak.setHeight(peak.height() * noise);
                    QMutexLocker lock(&mutex);
                    res.insert(peak);
                }
            }
        }
        ).waitForFinished();

        *this = CompressedMS(yOut, tMin, interp()->type());
		this->interp()->yFactor(noise);
    }
    return res;
}

Peak::PeakCollection CompressedMS::getPeaksWithErrors
(
    Smoother *s,
    size_t sigmaFactor
)
{
    Peak::PeakCollection res = this->smooth(s);
	VectorDouble stdevs(res.size());

    for (size_t i = 0; i < 20; ++i)
	{
		CompressedMS randMS = this->genRandMS();
		Peak::PeakCollection randPeaks = randMS.smooth(s);
		size_t j = 0;
		for (const Peak& p : res)
		{
			Peak::PeakCollection::const_iterator it
				= randPeaks.lower_bound(p);
			if (
				it != randPeaks.begin() && 
				(
					it == randPeaks.end() ||
					it->center() - p.center() > p.center() - std::prev(it)->center()
					)
				)
			{
				--it;
			}
			double d = it->center() - p.center();
			stdevs[j] += d * d;
			j++;
		}
	}

	size_t j = 0;
	for (const Peak& p : res)
	{
        p.setDisp(sigmaFactor * stdevs[j++]/20.);
	}

    return res;
}

CompressedMS::uint64_t CompressedMS::sumSqDev(const CompressedMS &ms) const
{
    CompressedMS::uint64_t res = 0;
    for(const auto& e : ms.interp()->table()) res += e.second * e.second;
    for(const auto& e : interp()->table())
    {
        CompressedMS::Map::const_iterator it = ms.interp()->table().find(e.first);
        if (it != ms.interp()->table().end())
        {
            res -= it->second * it->second;
            res += (it->second - e.second) * (it->second - e.second);
        }
        else
            res += e.second * e.second;
    }
    return res;
}

CompressedMS::uint64_t CompressedMS::totalIonCount() const
{
    uint64_t res = 0;
    for(const auto& e : interp()->table()) res += e.second;
    return res;
}

CompressedMS CompressedMS::cutRange(double tMin, double tMax) const
{
    const Map& tab = interp()->table();
    Map::const_iterator
            it1 = tab.lower_bound(static_cast<size_t>(tMin)),
            it2 = tab.upper_bound(static_cast<size_t>(tMax));
    while(it1 != tab.cbegin() && it1->second != 0) --it1;
    while(it2 != tab.cend() && std::prev(it2)->second != 0) ++it2;

    Map tabNew(it1, it2);

    return CompressedMS(tabNew, interp()->type());
}

CompressedMS CompressedMS::genRandMS() const
{
	CompressedMS res(*this);

	for (Map::reference& e : res.interp()->table())
	{
		e.second = e.second == 0.0 ? e.second
			: std::poisson_distribution<int>(e.second)(s_gen);
	}

    return res;
}

CompressedMS::VectorDouble CompressedMS::transformToVector() const
{
    size_t
            tMin = static_cast<size_t>(interp()->minX()),
            tMax = static_cast<size_t>(interp()->maxX());
    VectorDouble res(tMax - tMin + 1);
    for(const auto& e : interp()->table())
        res[e.first - tMin] = e.second;
    return res;
}


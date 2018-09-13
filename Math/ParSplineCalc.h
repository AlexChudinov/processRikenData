#ifndef PARSPLINECALC_H
#define PARSPLINECALC_H

#include <vector>
#include "Base\ThreadPool.h"

/**
 * @brief The ParSplineCalc class calculates spline in parallel using
 * optimized memory consumption. Error checking is minimal
 */
class ParSplineCalc : public BaseObject
{
public:
	using VectorDouble = std::vector<double>;

private:
	//Vectors to store matrix values
	VectorDouble a, b, c, d, e, bb, r;

	Q_DISABLE_COPY(ParSplineCalc)
	
	BASE_OBJECT_DUMMY_STATE

	//Only one thread can work with
	static ThreadPool::Mutex s_mutex;
	static size_t s_typeId;

	ThreadPool * m_threadPool;

public:
    friend class InstanceLocker;
	/**
	* @brief The InstanceLocker class locks and unlocks ParSplineCalc instance
	* automatically
	*/
	class InstanceLocker
	{
		ParSplineCalc * m_instance;
		bool m_clearMemory;
	public:
		InstanceLocker(ParSplineCalc * instance = nullptr, bool clearMemory = true);

		~InstanceLocker();

		ParSplineCalc * operator -> () { return m_instance; }

		operator bool() const { return m_instance; }
	};

	ParSplineCalc();

    /**
     * @brief getInstance locks mutex
     * @return
     */
    static InstanceLocker lockInstance(bool clearMemory = true);

    /**
     * @brief freeInstance releases mutex
     */
    static void freeInstance();

    /**
     * @brief clear frees allocated memory
     */
    void clear();

    /**
     * @brief logSpline calculates smoothed y-values using log scale spline
     * yIn values will be accepted
     * @param yOut - array of smoothed data
     * @param yIn - array of not smoothed data
     * @param p - smoothness parameter
     */
    void logSplinePoissonWeights
    (
        VectorDouble& yOut,
        const VectorDouble& yIn,
        double p
    );

	virtual size_t typeId() const;
};

#endif // PARSPLINECALC_H

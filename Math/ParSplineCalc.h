#ifndef PARSPLINECALC_H
#define PARSPLINECALC_H

#include <vector>
#include <QObject>

class QMutex;

/**
 * @brief The ParSplineCalc class calculates spline in parallel using
 * optimized memory consumption. Error checking is minimal
 */
class ParSplineCalc : public QObject
{
    Q_OBJECT

public:
    using VectorDouble = std::vector<double>;
    using VectorDoublePtr = QScopedPointer<VectorDouble>;

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

    ParSplineCalc(QObject * parent = nullptr);

    ~ParSplineCalc();

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

signals:
    /**
     * @brief sendTextMessage sends outside text messages in emergency cases
     * @param msg
     */
    void sendTextMessage(QString msg);

private:
    //Vectors to store matrix values
    VectorDoublePtr a, b, c, d, e, bb, r;

    /**
     * @brief s_mutex blocks current instance,
     * because it has shared resources
     */
    static QMutex s_mutex;
};

#endif // PARSPLINECALC_H

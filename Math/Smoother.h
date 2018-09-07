#ifndef SMOOTHER_H
#define SMOOTHER_H

#include <vector>
#include <memory>
#include <QVariant>

//smoothing params
const QString SMOOTH_PARAM = "Smooth. param";
const QString PEAK_COUNT = "Peak count";
const QString NOISE_LEVEL = "Noise level";

/**
 * @brief The Smoother class provides basic interface for smoothing
 */
class Smoother
{
public:
    using VectorDouble = std::vector<double>;
    using Pointer = std::unique_ptr<Smoother>;

    Smoother(const QVariantMap& pars, QVariantMap&& parsTemp);
    virtual ~Smoother();

    /**
     * @brief The Type enum types of smoothers
     */
    enum Type
    {
        LogSplinePoissonWeightType,
        LogSplinePoissonWeightPoissonNoiseType,
        LogSplinePoissonWeightOnePeakType
    };

    static Pointer create(Type type, const QVariantMap& pars = QVariantMap());

    /**
     * @brief type
     * @return type of current smoother
     */
    virtual Type type() const = 0;

    /**
     * @brief run calculates smoothing
     * @param yOut is a vector of output data
     * @param yIn is a vector of input data
     */
    virtual void run
    (
        VectorDouble& yOut,
        const VectorDouble& yIn
    ) = 0;

    /**
     * @brief paramsTemplate returns parameters table, using which parameters
     * can be supplied to smoothing model
     * @return
     */
    virtual QVariantMap paramsTemplate() const = 0;

    /**
     * @brief params
     * @return pointer to params staff in memory
     */
    inline const QVariantMap& params() const { return m_params; }
    virtual void setParams(const QVariantMap &params);

    static inline bool inputCheck
    (
        VectorDouble& yOut,
        const VectorDouble& yIn
    )
    {
        yOut.resize(yIn.size());
        return std::any_of
        (
            yIn.cbegin(),
            yIn.cend(),
            [](double d)->bool{ return d != 0.0; }
        );
    }

    static inline const QMap<Type, QString>& registry()
    {
        return s_registry;
    }

protected:
    template <class T>
    inline T* paramPtr(const QString& key)
    {
        QVariantMap::Iterator it = m_params.find(key);
        if(it != m_params.end())
            return reinterpret_cast<T*>(it.value().data());
        else return nullptr;
    }

private:
    QVariantMap m_params;
    static QMap<Type, QString> s_registry;
};

#endif // SMOOTHER_H

#ifndef SMOOTHER_H
#define SMOOTHER_H

#include <vector>
#include <memory>
#include <QVariant>

/**
 * @brief The Smoother class provides basic interface for smoothing
 */
class Smoother
{
public:
    using VectorDouble = std::vector<double>;
    using Pointer = std::unique_ptr<Smoother>;

    Smoother(const QVariantMap& pars);
    virtual ~Smoother();

    /**
     * @brief The Type enum types of smoothers
     */
    enum Type
    {
        LogSplinePoissonWeightType,
        LogSplinePoissonWeightPoissonNoiseType
    };

    static Pointer create(Type type, const QVariantMap& pars);

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
     * @brief params
     * @return pointer to params staff in memory
     */
    inline const QVariantMap& params() const { return m_params; }
    inline QVariantMap& params() { return m_params; }

    static inline bool inputCheck
    (
        VectorDouble& yOut,
        const VectorDouble& yIn
    )
    {
        yOut.resize(yIn.size());
        return !yIn.empty();
    }

private:
    QVariantMap m_params;
};

#endif // SMOOTHER_H

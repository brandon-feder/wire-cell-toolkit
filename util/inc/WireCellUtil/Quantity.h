#ifndef WIRECELL_QUANTITY
#define WIRECELL_QUANTITY

#include <cmath>
#include <ostream>

namespace WireCell {

    // This is heavily cribbed from Boost's units/example/measurement.hpp

    /** Provide a quantity with a value and an uncertainty and all the
     * little math operators.
     *
     *  Beware this assumes all quantities are uncorrelated!
     */
    class Quantity {
       public:
        Quantity()
          : m_mean(0.0)
          , m_sigma(0.0)
        {
        }
        Quantity(const double& mean, const double& sigma)
          : m_mean(mean)
          , m_sigma(sigma)
        {
        }
        Quantity(const double& mean)
          : m_mean(mean)
          , m_sigma(0.0)
        {
        }
        Quantity(const int& imean)
          : m_mean((double) imean)
          , m_sigma(0.0)
        {
        }
        Quantity(const Quantity& other)
          : m_mean(other.m_mean)
          , m_sigma(other.m_sigma)
        {
        }

        ~Quantity() {}

        Quantity& operator=(const Quantity& other)
        {
            if (this == &other) return *this;
            m_mean = other.m_mean;
            m_sigma = other.m_sigma;
            return *this;
        }

        operator double() const { return m_mean; }

        double mean() const { return m_mean; }
        double sigma() const { return m_sigma; }

        Quantity& operator+=(const double& exact)
        {
            m_mean += exact;
            return *this;
        }

        Quantity& operator-=(const double& exact)
        {
            m_mean -= exact;
            return *this;
        }

        Quantity& operator*=(const double& exact)
        {
            m_mean *= exact;
            m_sigma *= exact;
            return *this;
        }

        Quantity& operator/=(const double& exact)
        {
            m_mean /= exact;
            m_sigma /= exact;
            return *this;
        }

        Quantity& operator*=(const Quantity& other)
        {
            // relative uncertainties
            const double mrs = m_sigma / m_mean;
            const double ors = other.m_sigma / other.m_mean;

            m_mean *= other.m_mean;
            m_sigma = m_mean * std::hypot(mrs, ors);

            return *this;
        }

        Quantity& operator/=(const Quantity& other)
        {
            // relative uncertainties
            const double mrs = m_sigma / m_mean;
            const double ors = other.m_sigma / other.m_mean;

            m_mean /= other.m_mean;
            m_sigma = m_mean * std::hypot(mrs, ors);

            return *this;
        }

        Quantity& operator-=(const Quantity& other)
        {
            m_mean -= other.m_mean;
            m_sigma = std::hypot(m_sigma, other.m_sigma);
            return *this;
        }
        Quantity& operator+=(const Quantity& other)
        {
            m_mean += other.m_mean;
            m_sigma = std::hypot(m_sigma, other.m_sigma);
            return *this;
        }

        bool operator<(const Quantity& other) const { return m_mean < other.m_mean; }
        bool operator>(const Quantity& other) const { return m_mean > other.m_mean; }
        bool operator==(const Quantity& other) const { return m_mean == other.m_mean; }
        bool operator!=(const Quantity& other) const { return m_mean != other.m_mean; }

       private:
        double m_mean, m_sigma;
    };

    inline std::ostream& operator<<(std::ostream& os, const WireCell::Quantity& q)
    {
        os << "(" << q.mean() << " +/- " << q.sigma() << ")";
        return os;
    }

    inline WireCell::Quantity operator-(const WireCell::Quantity& other)
    {
        return WireCell::Quantity(-1 * other.mean(), other.sigma());
    }

    inline WireCell::Quantity operator*(const WireCell::Quantity& lhs, const WireCell::Quantity& rhs)
    {
        WireCell::Quantity res = lhs;
        res *= rhs;
        return res;
    }
    inline WireCell::Quantity operator/(const WireCell::Quantity& lhs, const WireCell::Quantity& rhs)
    {
        WireCell::Quantity res = lhs;
        res /= rhs;
        return res;
    }
    inline WireCell::Quantity operator+(const WireCell::Quantity& lhs, const WireCell::Quantity& rhs)
    {
        WireCell::Quantity res = lhs;
        res += rhs;
        return res;
    }
    inline WireCell::Quantity operator-(const WireCell::Quantity& lhs, const WireCell::Quantity& rhs)
    {
        WireCell::Quantity res = lhs;
        res -= rhs;
        return res;
    }

    // some comparisons with other, scalar types

    // fixme: *should* this be true:  should exact values be considered equal?
    inline bool operator==(const WireCell::Quantity& lhs, const double& scalar) { return lhs.mean() == scalar; }

    inline bool operator!=(const WireCell::Quantity& lhs, const double& scalar) { return lhs.mean() != scalar; }

    inline bool operator<(const WireCell::Quantity& lhs, const double& scalar) { return lhs.mean() < scalar; }

    inline bool operator>(const WireCell::Quantity& lhs, const double& scalar) { return lhs.mean() > scalar; }

    inline bool operator==(const double& scalar, const WireCell::Quantity& rhs) { return scalar == rhs.mean(); }

    inline bool operator!=(const double& scalar, const WireCell::Quantity& rhs) { return scalar != rhs.mean(); }

    inline bool operator<(const double& scalar, const WireCell::Quantity& rhs) { return scalar < rhs.mean(); }

    inline bool operator>(const double& scalar, const WireCell::Quantity& rhs) { return scalar > rhs.mean(); }

}  // namespace WireCell

#endif

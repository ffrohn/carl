#pragma once

#include <carl-common/config.h>
#ifdef USE_LIBPOLY

#include "../real_roots_common.h"
#include <carl-common/config.h>
#include <carl/core/Variable.h>
#include <carl/poly/umvpoly/UnivariatePolynomial.h>

#include <carl-logging/carl-logging.h>
#include <carl/converter/LibpolyConverter.h>

#include <carl/poly/lp/LPPolynomial.h>

#include "ran_libpoly.h"

namespace carl::ran::libpoly {

template<typename Number>
real_roots_result<RealAlgebraicNumberLibpoly<Number>> real_roots_libpoly(
    const LPPolynomial& polynomial,
    const Interval<Number>& interval = Interval<Number>::unbounded_interval()) {
    CARL_LOG_DEBUG("carl.ran.libpoly", " Real roots of " << polynomial << " within " << interval);

    assert(polynomial.isUnivariateRepresented());

    // Easy checks
    if (carl::is_zero(polynomial)) {
        CARL_LOG_TRACE("carl.ran.libpoly", "poly is 0 -> nullified");
        return real_roots_result<RealAlgebraicNumberLibpoly<Number>>::nullified_response();
    } else if (carl::is_constant(polynomial)) {
        CARL_LOG_TRACE("carl.ran.libpoly", "poly is constant but not zero -> no root");
        return real_roots_result<RealAlgebraicNumberLibpoly<Number>>::no_roots_response();
    }

    poly::Interval inter_poly = to_libpoly_interval(interval);

    // actual calculations
    std::vector<poly::AlgebraicNumber> roots = poly::isolate_real_roots(poly::to_univariate(polynomial.getPolynomial()));

    if (roots.empty()) {
        CARL_LOG_DEBUG("carl.ran.libpoly", "Poly has no roots");
        return real_roots_result<RealAlgebraicNumberLibpoly<Number>>::no_roots_response();
    }

    // sort roots in ascending order
    std::sort(roots.begin(), roots.end(), std::less<poly::AlgebraicNumber>());

    // turn into real_roots_result
    std::vector<RealAlgebraicNumberLibpoly<Number>> res;
    for (const auto& val : roots) {
        auto tmp = RealAlgebraicNumberLibpoly<Number>(val);
        // filter out roots not in interval
        if (poly::contains(inter_poly, poly::Value(val))) {
            CARL_LOG_DEBUG("carl.ran.libpoly", " Found Root " << val);
            res.emplace_back(tmp);
        }
    }

    return real_roots_result<RealAlgebraicNumberLibpoly<Number>>::roots_response(std::move(res));
}

template<typename Number>
real_roots_result<RealAlgebraicNumberLibpoly<Number>> real_roots_libpoly(
    const LPPolynomial& polynomial,
    const std::map<Variable, RealAlgebraicNumberLibpoly<Number>>& m,
    const Interval<Number>& interval = Interval<Number>::unbounded_interval()) {
    CARL_LOG_DEBUG("carl.ran.libpoly", polynomial << " " << m << " " << interval);

    if (polynomial.isUnivariateRepresented()) {
        return real_roots_libpoly(polynomial, interval);
    }

    // easy checks
    if (carl::is_zero(polynomial)) {
        CARL_LOG_TRACE("carl.ran.libpoly", "poly is 0 -> nullified");
        return real_roots_result<RealAlgebraicNumberLibpoly<Number>>::nullified_response();
    } else if (carl::is_constant(polynomial)) {
        CARL_LOG_TRACE("carl.ran.libpoly", "poly is constant but not zero -> no root");
        return real_roots_result<RealAlgebraicNumberLibpoly<Number>>::no_roots_response();
    }

    poly::Interval inter_poly = to_libpoly_interval(interval);

    // Multivariate Polynomial
    // build the assignment
    poly::Assignment assignment;
    Variable mainVar = polynomial.mainVar();
    for (Variable& var : carl::variables(polynomial)) {
        if (var == mainVar) continue;
        // We convert numbers to libpoly values and add to assignment so we can substitute them later using libpoly
        lp_value_t val;
        // Turn into value
        lp_value_construct(&val, lp_value_type_t::LP_VALUE_ALGEBRAIC, m.at(var).get_internal());
        // That copies the value into the assignment
        assignment.set(VariableMapper::getInstance().getLibpolyVariable(var), poly::Value(&val));
        // Destroy the value, but dont free the algebraic number!
        lp_value_destruct(&val);
    }

    std::vector<poly::Value> roots = poly::isolate_real_roots(polynomial.getPolynomial(), assignment);

    if (roots.empty()) {
        CARL_LOG_DEBUG("carl.ran.libpoly", " Checking for nullification -> Evaluation at " << mainVar << "= 1");
        assignment.set(VariableMapper::getInstance().getLibpolyVariable(mainVar), poly::Value(long(1)));
        poly::Value eval_val = poly::evaluate(polynomial.getPolynomial(), assignment);
        CARL_LOG_DEBUG("carl.ran.libpoly", " Got eval_val " << eval_val);

        if (eval_val == poly::Value(long(0))) {
            CARL_LOG_DEBUG("carl.ran.libpoly", "poly is 0 after substituting rational assignments -> nullified");
            return real_roots_result<RealAlgebraicNumberLibpoly<Number>>::nullified_response();
        } else {
            CARL_LOG_DEBUG("carl.ran.libpoly", "Poly has no roots");
            return real_roots_result<RealAlgebraicNumberLibpoly<Number>>::no_roots_response();
        }
    }

    std::sort(roots.begin(), roots.end(), std::less<poly::Value>());

    // turn result into real_roots_result
    std::vector<RealAlgebraicNumberLibpoly<Number>> res;
    for (const poly::Value& val : roots) {
        auto tmp = RealAlgebraicNumberLibpoly<Number>::create_from_value(val.get_internal());
        // filter out roots not in interval
        if (poly::contains(inter_poly, val)) {
            CARL_LOG_DEBUG("carl.ran.libpoly", " Found root " << val);
            res.emplace_back(tmp);
        }
    }

    return real_roots_result<RealAlgebraicNumberLibpoly<Number>>::roots_response(std::move(res));
}

} // namespace carl::ran::libpoly

#endif
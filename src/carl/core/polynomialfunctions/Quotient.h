#pragma once

#include "to_univariate_polynomial.h"

#include "../MultivariatePolynomial.h"
#include "../UnivariatePolynomial.h"

namespace carl {

template<typename C, typename O, typename P>
MultivariatePolynomial<C,O,P> quotient(const MultivariatePolynomial<C,O,P>& dividend, const MultivariatePolynomial<C,O,P>& divisor) {
	assert(!carl::isZero(divisor));
	if (dividend == divisor) {
		return MultivariatePolynomial<C,O,P>(1);
	}
	if (carl::isOne(divisor)) {
		return dividend;
	}
	//static_assert(is_field<C>::value, "Division only defined for field coefficients");
	MultivariatePolynomial<C,O,P> p(dividend);
	auto& tam = MultivariatePolynomial<C,O,P>::mTermAdditionManager;
	auto id = tam.getId(p.nrTerms());
	while(!carl::isZero(p))
	{
		Term<C> factor;
		if (p.lterm().divide(divisor.lterm(), factor)) {
			//p -= factor * divisor;
			p.subtractProduct(factor, divisor);
			tam.template addTerm<true>(id, factor);
		}
		else
		{
			p.stripLT();
		}
	}
	MultivariatePolynomial<C,O,P> result;
	tam.readTerms(id, result.getTerms());
	result.reset_ordered();
	result.template makeMinimallyOrdered<false, true>();
	assert(result.isConsistent());
	assert(dividend.isConsistent());
	return result;
}

}

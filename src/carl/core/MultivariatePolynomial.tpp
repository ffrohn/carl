#pragma once
#include <memory>
#include <list>
#include "MultivariatePolynomial.h"
#include "logging.h"
#include "numbers.h"

namespace carl
{


template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>::MultivariatePolynomial(const Coeff& c) :
mTerms(1,std::make_shared<const Term<Coeff>>(c))
{
    
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>::MultivariatePolynomial(Variable::Arg v) :
mTerms(1,std::make_shared<const Term<Coeff>>(v))
{
    
}
template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>::MultivariatePolynomial(const Monomial& m) :
mTerms(1,std::make_shared<const Term<Coeff>>(m))
{
    
}
template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>::MultivariatePolynomial(const Term<Coeff>& t) :
mTerms(1,std::make_shared<const Term<Coeff>>(t))
{
    
}
template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>::MultivariatePolynomial(std::shared_ptr<const Monomial> m) :
mTerms(1,std::make_shared<const Term<Coeff>>(m))
{

}
template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>::MultivariatePolynomial(std::shared_ptr<const Term<Coeff>> t) :
mTerms(1,t)
{
    
}
template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>::MultivariatePolynomial(const UnivariatePolynomial<MultivariatePolynomial<Coeff, Ordering, Policy>>& pol)
{
    LOG_NOTIMPLEMENTED();
}
template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>::MultivariatePolynomial(const UnivariatePolynomial<Coeff>& pol)
{
    LOG_NOTIMPLEMENTED();
}

template<typename Coeff, typename Ordering, typename Policy>
template<typename InputIterator>
MultivariatePolynomial<Coeff,Ordering,Policy>::MultivariatePolynomial(InputIterator begin, InputIterator end, bool duplicates, bool sorted)
{
	mTerms.assign(begin, end);
	if(!sorted)
	{
		sortTerms();
	}
	if(duplicates)
	{
		// We now iterate over the terms to find equal monomials.
		for(typename TermsType::iterator it=mTerms.begin(); it != mTerms.end(); )
		{
			// look ahead for equal monomials
			typename TermsType::iterator jt=it;
			Coeff c = (*it)->coeff();
			for(++jt; jt != mTerms.end(); ++jt)
			{
				if( *((**jt).monomial()) == *((**it).monomial()) ) 
				{
					c += (*jt)->coeff();
					// We do not yet remove the term as this would cause multiple movements
					// over the whole operation. Instead, we write a zero and clear these zeros later on.
					// TODO make a global shared ptr for zero terms.
					*jt = std::make_shared<const Term<Coeff>>((Coeff)0);
				}
				else
				{
					break;
				}
			}
			if(c != (*it)->coeff())
			{
				if(c == (Coeff)0)
				{
					*it = std::make_shared<const Term<Coeff>>((Coeff)0);
				}
				else
				{
					*it = std::make_shared<const Term<Coeff>>(c, (**it).monomial());
				}
				// We need to update this iterator.
			}
			
			// Go on where the look ahead stopped.
			it = jt;
		}
		// Now we have to remove zeros.
		for(typename TermsType::iterator it=mTerms.begin(); it != mTerms.end(); )
		{
			if(**it == (Coeff)0)
			{
				it = mTerms.erase(it);
			}
			else
			{
				++it;
			}
		}
		
	}
	
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>::MultivariatePolynomial(const std::initializer_list<Term<Coeff>>& terms)
{
	for(Term<Coeff> term : terms)
	{
		mTerms.push_back(std::make_shared<const Term<Coeff>>(term));
	}
	sortTerms();
}


template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>::MultivariatePolynomial(const std::initializer_list<Variable>& terms)
{
	for(Variable term : terms)
	{
		mTerms.push_back(std::make_shared<const Term<Coeff>>(term));
	}
	sortTerms();
}
    
template<typename Coeff, typename Ordering, typename Policy>
std::shared_ptr<const Monomial> MultivariatePolynomial<Coeff,Ordering,Policy>::lmon() const
{
    return lterm()->monomial();
}
template<typename Coeff, typename Ordering, typename Policy>
std::shared_ptr<const Term<Coeff>> MultivariatePolynomial<Coeff,Ordering,Policy>::lterm() const
{
    LOG_ASSERT(!isZero(), "Leading term undefined on zero polynomials.");
	return mTerms.back();
}

template<typename Coeff, typename Ordering, typename Policy>
std::shared_ptr<const Term<Coeff>> MultivariatePolynomial<Coeff,Ordering,Policy>::trailingTerm() const
{
    LOG_ASSERT(!isZero(), "Trailing term undefined on zero polynomials.");
	return mTerms.front();
}

template<typename Coeff, typename Ordering, typename Policy>
Coeff MultivariatePolynomial<Coeff,Ordering,Policy>::lcoeff() const
{
    return lterm()->coeff();
}

template<typename Coeff, typename Ordering, typename Policy>
exponent MultivariatePolynomial<Coeff,Ordering,Policy>::highestDegree() const
{
    if(mTerms.size == 0) return 0;
    if(Ordering::degreeOrder)
    {
        return mTerms.back()->tdeg();
    }
    else 
    {
        LOG_NOTIMPLEMENTED();
    }
}

template<typename Coeff, typename Ordering, typename Policy>
bool MultivariatePolynomial<Coeff,Ordering,Policy>::isZero() const
{
    return mTerms.empty();
}
template<typename Coeff, typename Ordering, typename Policy>
bool MultivariatePolynomial<Coeff,Ordering,Policy>::isConstant() const
{
    return (mTerms.size() == 0) || (mTerms.size() == 1 && mTerms.front()->isConstant());
}
template<typename Coeff, typename Ordering, typename Policy>
bool MultivariatePolynomial<Coeff,Ordering,Policy>::isLinear() const
{
    if(mTerms.size() == 0) return true;
    if(Ordering::degreeOrder)
    {
        return mTerms.back()->isLinear();
    }
    else 
    {
        LOG_NOTIMPLEMENTED();
    }
}

template<typename Coeff, typename Ordering, typename Policy>
bool MultivariatePolynomial<Coeff,Ordering,Policy>::hasConstantTerm() const
{
    if(nrTerms() == 0) return false;
	else
	{
		return trailingTerm()->isConstant();
	}
}

template<typename Coeff, typename Ordering, typename Policy>
const std::shared_ptr<const Term<Coeff>>& MultivariatePolynomial<Coeff,Ordering,Policy>::operator[](int index) const
{
	assert(index < nrTerms());
	return mTerms.at(index);
}




template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy> MultivariatePolynomial<Coeff,Ordering,Policy>::tail() const
{
	assert(mTerms.size() != 0);
	if (mTerms.size() == 1) return MultivariatePolynomial();
    MultivariatePolynomial tail;
	tail.mTerms.reserve(mTerms.size()-1);
    tail.mTerms.insert(tail.mTerms.begin(), mTerms.begin(), --mTerms.end());
    return tail;
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>& MultivariatePolynomial<Coeff,Ordering,Policy>::stripLT()
{
    assert(!isZero());
    mTerms.pop_back();
    return *this;
}

template<typename Coeff, typename Ordering, typename Policy>
bool MultivariatePolynomial<Coeff,Ordering,Policy>::isTsos() const
{
    //LOG_NOTIMPLEMENTED();
    return false;
}

template<typename Coeff, typename Ordering, typename Policy>
bool MultivariatePolynomial<Coeff,Ordering,Policy>::isReducibleIdentity() const
{
    //LOG_NOTIMPLEMENTED();
    return false;
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy> MultivariatePolynomial<Coeff,Ordering,Policy>::substitute(const std::map<Variable,Coeff>& substitutions)
{
	MultivariatePolynomial result;
	for(auto term : mTerms)
	{
		Term<Coeff>* t = term->substitute(substitutions);
		if(t->coeff() != 0)
		{
			result.mTerms.push_back(std::shared_ptr<const Term<Coeff>>(t));
		}
	}
	result.sortTerms();
	return result;
}

template<typename Coeff, typename Ordering, typename Policy>
Coeff MultivariatePolynomial<Coeff,Ordering,Policy>::evaluate(const std::map<Variable,Coeff>& substitutions)
{
	// We do not have to construct polynomials all the time.
	LOG_INEFFICIENT();
	MultivariatePolynomial result = substitute(substitutions);
	assert(result.isConstant());
	return result.constantPart();
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy> MultivariatePolynomial<Coeff,Ordering,Policy>::coprimeCoefficients() const
{
	assert(nrTerms() != 0);
	if(nrTerms() == 1) return *this;
	typename TermsType::const_iterator it = mTerms.begin();
	typename IntegralT<Coeff>::type num = getNum((*it)->coeff());
	typename IntegralT<Coeff>::type den = getDenom((*it)->coeff());
	for(++it; it != mTerms.end(); ++it)
	{
		num = gcd(num, getNum((*it)->coeff()));
		den = lcm(den, getDenom((*it)->coeff()));
	}
	Coeff factor = den/num;
	// Notice that even if factor is 1, we create a new polynomial
	MultivariatePolynomial<Coeff, Ordering, Policy> result;
	result.mTerms.reserve(mTerms.size());
	for(const typename std::shared_ptr<const TermType> term : mTerms)
	{
		result.mTerms.push_back(std::make_shared<const TermType>(*term * factor));
	}
	return result;
	
	
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy> MultivariatePolynomial<Coeff,Ordering,Policy>::normalize() const
{
	MultivariatePolynomial result;
	result.mTerms.reserve(mTerms.size());
	for(typename TermsType::const_iterator it = mTerms.begin(); it != mTerms.end(); ++it)
	{
		result.mTerms.emplace_back((*it)->dividedBy(lcoeff()));
	}
	return result;
	
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy> MultivariatePolynomial<Coeff,Ordering,Policy>::derivative(Variable::Arg v, unsigned nth) const
{
	assert(!isZero());
	// TODO n > 1 not yet implemented!
	assert(nth == 1);
	TermsType tmpTerms;
	for(std::shared_ptr<const Term<Coeff>> t : mTerms)
	{
		tmpTerms.emplace_back(t->derivative(v));
	}
	return MultivariatePolynomial(tmpTerms.begin(), tmpTerms.end(), true, false);
}


template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy> MultivariatePolynomial<Coeff,Ordering,Policy>::SPolynomial(
																const MultivariatePolynomial<Coeff, Ordering, Policy>& p,
																const MultivariatePolynomial<Coeff, Ordering, Policy>& q)
{
	assert(p.nrTerms() != 0);
	assert(q.nrTerms() != 0);

	
	if( p.nrTerms() == 1 && q.nrTerms() == 1 )
	{
		return MultivariatePolynomial();
	}
	else if( p.nrTerms() == 1 )
	{
		return -(p.lterm()->calcLcmAndDivideBy( *q.lmon() ) * q.tail());
	}
	else if( q.nrTerms() == 1 )
	{
		return (q.lterm()->calcLcmAndDivideBy( *p.lmon() ) * p.tail());
	}
	else
	{
		return (p.tail() * q.lterm()->calcLcmAndDivideBy(*p.lmon())) - (q.tail() * p.lterm()->calcLcmAndDivideBy( *q.lmon() ));
	}
	
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy> MultivariatePolynomial<Coeff,Ordering,Policy>::pow(unsigned exp) const
{
	if(exp == 0)
	{
		return MultivariatePolynomial((Coeff)1);
	}
	
	LOG_INEFFICIENT();
	MultivariatePolynomial<Coeff,Ordering,Policy> res(*this);
	for(unsigned i = 1; i < exp; i++)
	{
		res *= *this;
	}
	return res;	
}


template<typename Coeff, typename Ordering, typename Policy>
void MultivariatePolynomial<Coeff,Ordering,Policy>::gatherVariables(std::set<Variable>& vars) const
{
	for(std::shared_ptr<const Term<Coeff>> t : mTerms)
	{
		t->gatherVariables(vars);
	}
}

template<typename Coeff, typename Ordering, typename Policy>
std::set<Variable> MultivariatePolynomial<Coeff,Ordering,Policy>::gatherVariables() const
{
	std::set<Variable> vars;
	gatherVariables(vars);
	return vars;
}

template<typename Coeff, typename Ordering, typename Policy>
unsigned MultivariatePolynomial<Coeff,Ordering,Policy>::hash() const
{
    return mTerms.size << 16 || (lcoeff() ^ lterm()->getNrVariables());
}



template<typename C, typename O, typename P>
bool operator==( const MultivariatePolynomial<C,O,P>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    if(lhs.mTerms.size() != rhs.mTerms.size()) return false;
    // Compare vector entries. We cannot use std::vector== as we not only want to compare the pointers.
    return std::equal(lhs.mTerms.begin(), lhs.mTerms.end(), rhs.mTerms.begin(),
                    [](const std::shared_ptr<const Term<C>>& lterm, const std::shared_ptr<const Term<C>>& rterm) 
                    -> bool 
                    {
                        return lterm == rterm || *lterm == *rterm;
                    });
}
template<typename C, typename O, typename P>
bool operator==(const UnivariatePolynomial<C>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    LOG_NOTIMPLEMENTED();
    return false;
}
template<typename C, typename O, typename P>
bool operator==(const MultivariatePolynomial<C,O,P>& lhs, const UnivariatePolynomial<C>& rhs)
{
    return rhs == lhs;
}
template<typename C, typename O, typename P>
bool operator==(const UnivariatePolynomial<MultivariatePolynomial<C>>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    LOG_NOTIMPLEMENTED();
    return false;
}
template<typename C, typename O, typename P>
bool operator==(const MultivariatePolynomial<C,O,P>& lhs, const UnivariatePolynomial<MultivariatePolynomial<C>>& rhs)
{
    return rhs == lhs;
}
template<typename C, typename O, typename P>
bool operator==(const MultivariatePolynomial<C,O,P>& lhs, const Term<C>& rhs)
{
    if(lhs.mTerms.empty() && rhs.coeff() == 0) return true;
    if(lhs.mTerms.size() > 1) return false;
    return *(lhs.mTerms.front()) == rhs;
}
template<typename C, typename O, typename P>
bool operator==(const Term<C>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs == lhs;
}
template<typename C, typename O, typename P>
bool operator==(const MultivariatePolynomial<C,O,P>& lhs, const Monomial& rhs)
{
    if(lhs.mTerms.size() != 1) return false;
    return *(lhs.mTerms.front()->monomial) == rhs;
}
template<typename C, typename O, typename P>
bool operator==(const Monomial& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs == lhs;
}
template<typename C, typename O, typename P>
bool operator==(const MultivariatePolynomial<C,O,P>& lhs, const C& rhs)
{
    if(lhs.mTerms.empty() && rhs == 0) return true;
    if(lhs.mTerms.size() > 1) return false;
    return (lhs.mTerms.front()->coeff()) == rhs;
}
template<typename C, typename O, typename P>
bool operator==(const C& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs == lhs;
}
template<typename C, typename O, typename P>
bool operator==(const MultivariatePolynomial<C,O,P>& lhs, Variable::Arg rhs)
{
    if(lhs.mTerms.size() != 1) return false;
    return *(lhs.mTerms.front()->monomial()) == rhs;
}
template<typename C, typename O, typename P>
bool operator==(Variable::Arg lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs == lhs;
}

template<typename C, typename O, typename P>
bool operator!=( const MultivariatePolynomial<C,O,P>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(const UnivariatePolynomial<C>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(const MultivariatePolynomial<C,O,P>& lhs, const UnivariatePolynomial<C>& rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(const UnivariatePolynomial<MultivariatePolynomial<C>>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(const MultivariatePolynomial<C,O,P>& lhs, const UnivariatePolynomial<MultivariatePolynomial<C>>& rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(const MultivariatePolynomial<C,O,P>& lhs, const Term<C>& rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(const Term<C>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(const MultivariatePolynomial<C,O,P>& lhs, const Monomial& rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(const Monomial& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(const MultivariatePolynomial<C,O,P>& lhs, const C& rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(const C& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(const MultivariatePolynomial<C,O,P>& lhs, Variable::Arg rhs)
{
    return lhs != rhs;
}
template<typename C, typename O, typename P>
bool operator!=(Variable::Arg lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return lhs != rhs;
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>& MultivariatePolynomial<Coeff, Ordering, Policy>::operator+=(const MultivariatePolynomial& rhs)
{
    if(mTerms.size() == 0) mTerms = rhs.mTerms;
    if(rhs.mTerms.size() == 0) return *this;
    
    TermsType newTerms;
    newTerms.reserve(mTerms.size() + rhs.mTerms.size());
    typename TermsType::const_iterator lhsIt(mTerms.begin());
    typename TermsType::const_iterator rhsIt(rhs.mTerms.begin());
    while(true)
    {
        CompareResult cmpres(Ordering::compare(**lhsIt, **rhsIt));
        if(cmpres == CompareResult::LESS)
        {
            newTerms.push_back(*lhsIt);
            if(++lhsIt != mTerms.end()) break;
        }
        else if(cmpres == CompareResult::GREATER)
        {
            newTerms.push_back(*rhsIt);
            if(++rhsIt != rhs.mTerms.end()) break;
        }
        else 
        {
            if( (**lhsIt).coeff() != -(**rhsIt).coeff() )
            {
                newTerms.push_back(std::make_shared<const Term<Coeff>>( (**lhsIt).coeff() + (**rhsIt).coeff(), (**lhsIt).monomial() ));
            }
            ++lhsIt;
            ++rhsIt;
            if(lhsIt == mTerms.end() || rhsIt == rhs.mTerms.end() ) break;
        }
    }
    newTerms.insert(newTerms.end(), lhsIt, mTerms.cend());
    newTerms.insert(newTerms.end(), rhsIt, rhs.mTerms.cend());
    
    mTerms = std::move(newTerms);
    return *this;
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>& MultivariatePolynomial<Coeff, Ordering, Policy>::operator+=(const Term<Coeff>& rhs)
{
    if(rhs.coeff() == 0) return *this;
    if(Policy::searchLinear) 
    {
        typename TermsType::iterator it(mTerms.begin());
        while(it != mTerms.end())
        {
            // TODO consider comparing the shared pointers.
            CompareResult cmpres(Ordering::compare((**it), rhs));
            if( cmpres == CompareResult::GREATER ) break;
            if( cmpres == CompareResult::EQUAL )
            {
                // new coefficient would be zero, simply removing is enough.
                if((**it).coeff() == -rhs.coeff())
                {
                    mTerms.erase(it);
                }
                // we have to create a new term object. 
                else
                {
                    *it = std::make_shared<const Term<Coeff>>((**it).coeff()+rhs.coeff(), (**it).monomial());
                }
                return *this;
            }
            ++it;    
        }
        // no equal monomial does occur. We can simply insert.
        mTerms.insert(it,std::make_shared<const Term<Coeff>>(rhs));
        return *this;
    }
    else 
    {
        LOG_NOTIMPLEMENTED();
    }
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>& MultivariatePolynomial<Coeff, Ordering, Policy>::operator+=(const Monomial& rhs)
{
    if(rhs.tdeg() == 0) return *this;
    if(Policy::searchLinear) 
    {
        typename TermsType::iterator it(mTerms.begin());
        while(it != mTerms.end())
        {
            CompareResult cmpres(Ordering::compare(*(**it).monomial(), rhs));
            if( cmpres == CompareResult::GREATER ) break;
            if( cmpres == CompareResult::EQUAL )
            {
                // new coefficient would be zero, simply removing is enough.
                if((**it).coeff() == -1)
                {
                    mTerms.erase(it);
                }
                // we have to create a new term object. 
                else
                {
                    *it = std::make_shared<const Term<Coeff>>((**it).coeff()+1, (**it).monomial());
                }
                return *this;
            }
            ++it;    
        }
        // no eq ual monomial does occur. We can simply insert.
        mTerms.insert(it,std::make_shared<const Term<Coeff>>(rhs));
        return *this;
    }
    else 
    {
        LOG_NOTIMPLEMENTED();
    }
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>& MultivariatePolynomial<Coeff, Ordering, Policy>::operator+=(const Variable::Arg rhs)
{
    if(Policy::searchLinear) 
    {
        typename TermsType::iterator it(mTerms.begin());
        while(it != mTerms.end())
        {
            CompareResult cmpres(Ordering::compare(*(**it).monomial(), rhs));
            if( cmpres == CompareResult::GREATER ) break;
            if( cmpres == CompareResult::EQUAL )
            {
                // new coefficient would be zero, simply removing is enough.
                if((**it).coeff() == -1)
                {
                    mTerms.erase(it);
                }
                // we have to create a new term object. 
                else
                {
                    *it = std::make_shared<const Term<Coeff>>((**it).coeff()+1, (**it).monomial());
                }
                return *this;
            }
            ++it;    
        }
        // no eq ual monomial does occur. We can simply insert.
        mTerms.insert(it,std::make_shared<const Term<Coeff>>(rhs));
    }
    else 
    {
        LOG_NOTIMPLEMENTED();
        
    }
    return *this;
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>& MultivariatePolynomial<Coeff, Ordering, Policy>::operator+=(const Coeff& c)
{
    if(c == 0) return *this;
    if(Ordering::degreeOrder)
    {
        if(mTerms.size() > 0 && mTerms.front()->isConstant())
        {
            Coeff newConstantPart(mTerms.front()->coeff() + c);
            
            if(newConstantPart != 0)
            {
                mTerms.front() = (std::make_shared<Term<Coeff>>(newConstantPart));
            }
            else
            {
                mTerms.erase(mTerms.begin());
            }
        }
        else
        {
            mTerms.emplace(mTerms.begin(),std::make_shared<Term<Coeff>>(c));
        }
    }
    else
    {
        LOG_NOTIMPLEMENTED();
        
    }
    return *this;
}

template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+( const MultivariatePolynomial<C,O,P>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    // TODO write dedicated add
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result += rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(const UnivariatePolynomial<C>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    LOG_NOTIMPLEMENTED();
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(const MultivariatePolynomial<C,O,P>& lhs, const UnivariatePolynomial<C>& rhs)
{
    LOG_NOTIMPLEMENTED();
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(const UnivariatePolynomial<MultivariatePolynomial<C>>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    LOG_NOTIMPLEMENTED();
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(const MultivariatePolynomial<C,O,P>& lhs, const UnivariatePolynomial<MultivariatePolynomial<C>>& rhs)
{
	LOG_NOTIMPLEMENTED();
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(const MultivariatePolynomial<C,O,P>& lhs, const Term<C>& rhs)
{
    // TODO write dedicated add
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result += rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(const Term<C>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs + lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(const MultivariatePolynomial<C,O,P>& lhs, const Monomial& rhs)
{
    // TODO write dedicated add
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result += rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(const Monomial& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs + lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(const MultivariatePolynomial<C,O,P>& lhs, const C& rhs)
{
    // TODO write dedicated add
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result += rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(const C& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs + lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(const MultivariatePolynomial<C,O,P>& lhs, Variable::Arg rhs)
{
	// TODO write dedicated add
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result += rhs;
	return result;
	
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator+(Variable::Arg lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs + lhs;
}



template<typename Coeff, typename Ordering, typename Policy>
const MultivariatePolynomial<Coeff, Ordering, Policy> MultivariatePolynomial<Coeff,Ordering,Policy>::operator -() const
{
    MultivariatePolynomial<Coeff, Ordering, Policy> negation;
    negation.mTerms.reserve(mTerms.size());
    for(auto term : mTerms)
    {
        negation.mTerms.push_back(std::make_shared<const Term<Coeff>>(-*term));
    }
    return negation;
}


template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>& MultivariatePolynomial<Coeff, Ordering, Policy>::operator-=(const MultivariatePolynomial& rhs)
{
    if(mTerms.size() == 0) mTerms = rhs.mTerms;
    if(rhs.mTerms.size() == 0) return *this;
    
    TermsType newTerms;
    newTerms.reserve(mTerms.size() + rhs.mTerms.size());
    typename TermsType::const_iterator lhsIt(mTerms.begin());
    typename TermsType::const_iterator rhsIt(rhs.mTerms.begin());
    while(true)
    {
        CompareResult cmpres(Ordering::compare(**lhsIt, **rhsIt));
        if(cmpres == CompareResult::LESS)
        {
            newTerms.push_back(std::make_shared<const Term<Coeff>>(-(**lhsIt)));
            if(++lhsIt == mTerms.end()) break;
        }
        else if(cmpres == CompareResult::GREATER)
        {
            newTerms.push_back(std::make_shared<const Term<Coeff>>(-(**rhsIt)));
            if(++rhsIt == rhs.mTerms.end()) break;
        }
        else 
        {
			assert(cmpres == CompareResult::EQUAL);
            if( (**lhsIt).coeff() != (**rhsIt).coeff() )
            {
                newTerms.push_back(std::make_shared<const Term<Coeff>>( (**lhsIt).coeff() - (**rhsIt).coeff(), (**lhsIt).monomial() ));
            }
            ++lhsIt;
            ++rhsIt;
            if(lhsIt == mTerms.end() || rhsIt == rhs.mTerms.end() ) break;
        }
    }
    while(lhsIt != mTerms.end())
    {
        newTerms.push_back(std::make_shared<const Term<Coeff>>(-(**lhsIt)));
        ++lhsIt;
    }
    
    while(rhsIt != rhs.mTerms.end())
    {
        newTerms.push_back(std::make_shared<const Term<Coeff>>(-(**rhsIt)));
        ++rhsIt;
    }
    
    mTerms = std::move(newTerms);
    return *this;
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>& MultivariatePolynomial<Coeff, Ordering, Policy>::operator-=(const Term<Coeff>& rhs)
{
    if(rhs.coeff() == 0) return *this;
    if(Policy::searchLinear) 
    {
        typename TermsType::iterator it(mTerms.begin());
        while(it != mTerms.end())
        {
            // TODO consider comparing the shared pointers.
            if( (**it).isConstant() ) break;
            CompareResult cmpres(Ordering::compare((**it), rhs));
            if( cmpres == CompareResult::GREATER ) break;
            if( cmpres == CompareResult::EQUAL )
            {
                // new coefficient would be zero, simply removing is enough.
                if((**it).coeff() == rhs.coeff())
                {
                    mTerms.erase(it);
                }
                // we have to create a new term object. 
                else
                {
                    *it = std::make_shared<const Term<Coeff>>((**it).coeff()-rhs.coeff(), (**it).monomial());
                }
                return *this;
            }
            ++it;    
        }
        // no equal monomial does occur. We can simply insert.
        mTerms.insert(it,std::make_shared<const Term<Coeff>>(-rhs));
        return *this;
    }
    else 
    {
        LOG_NOTIMPLEMENTED();
    }
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>& MultivariatePolynomial<Coeff, Ordering, Policy>::operator-=(const Monomial& rhs)
{
    if(rhs.tdeg() == 0) return *this;
    if(Policy::searchLinear) 
    {
        typename TermsType::iterator it(mTerms.begin());
        while(it != mTerms.end())
        {
            if( (**it).isConstant() ) break;
            CompareResult cmpres(Ordering::compare(*(**it).monomial(), rhs));
            if( cmpres == CompareResult::GREATER ) break;
            if( cmpres == CompareResult::EQUAL )
            {
                // new coefficient would be zero, simply removing is enough.
                if((**it).coeff() == 1)
                {
                    mTerms.erase(it);
                }
                // we have to create a new term object. 
                else
                {
                    *it = std::make_shared<const Term<Coeff>>((**it).coeff()-1, (**it).monomial());
                }
                return *this;
            }
            ++it;    
        }
        // no eq ual monomial does occur. We can simply insert.
        mTerms.insert(it,std::make_shared<const Term<Coeff>>(-1, rhs));
        return *this;
    }
    else 
    {
        LOG_NOTIMPLEMENTED();
    }
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>& MultivariatePolynomial<Coeff, Ordering, Policy>::operator-=(const Variable::Arg rhs)
{
    if(Policy::searchLinear) 
    {
        typename TermsType::iterator it(mTerms.begin());
        while(it != mTerms.end())
        {
            CompareResult cmpres(Ordering::compare(*(**it).monomial(), rhs));
            if( cmpres == CompareResult::GREATER ) break;
            if( cmpres == CompareResult::EQUAL )
            {
                // new coefficient would be zero, simply removing is enough.
                if((**it).coeff() == 1)
                {
                    mTerms.erase(it);
                }
                // we have to create a new term object. 
                else
                {
                    *it = std::make_shared<const Term<Coeff>>((**it).coeff()-1, (**it).monomial());
                }
                return *this;
            }
            ++it;    
        }
        // no eq ual monomial does occur. We can simply insert.
        mTerms.insert(it,std::make_shared<const Term<Coeff>>(-1, rhs));
    }
    else 
    {
        LOG_NOTIMPLEMENTED();
        
    }
    return *this;
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff, Ordering, Policy>& MultivariatePolynomial<Coeff, Ordering, Policy>::operator-=(const Coeff& c)
{
    if(c == 0) return *this;
    if(Ordering::degreeOrder)
    {
        if(mTerms.size() > 0 && mTerms.front()->isConstant())
        {
            Coeff newConstantPart(mTerms.front()->coeff() - c);
            
            if(newConstantPart != 0)
            {
                mTerms.front() = (std::make_shared<Term<Coeff>>(newConstantPart));
            }
            else
            {
                mTerms.erase(mTerms.begin());
            }
        }
        else
        {
            mTerms.emplace(mTerms.begin(),std::make_shared<Term<Coeff>>(-c));
        }
    }
    else
    {
        LOG_NOTIMPLEMENTED();
        
    }
    return *this;
}

template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-( const MultivariatePolynomial<C,O,P>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    // TODO write dedicated add
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result -= rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(const UnivariatePolynomial<C>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    LOG_NOTIMPLEMENTED();
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(const MultivariatePolynomial<C,O,P>& lhs, const UnivariatePolynomial<C>& rhs)
{
	return rhs - lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(const UnivariatePolynomial<MultivariatePolynomial<C>>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    LOG_NOTIMPLEMENTED();
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(const MultivariatePolynomial<C,O,P>& lhs, const UnivariatePolynomial<MultivariatePolynomial<C>>& rhs)
{
	return rhs - lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(const MultivariatePolynomial<C,O,P>& lhs, const Term<C>& rhs)
{
    // TODO write dedicated add
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result -= rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(const Term<C>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs - lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(const MultivariatePolynomial<C,O,P>& lhs, const Monomial& rhs)
{
    // TODO write dedicated add
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result -= rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(const Monomial& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs - lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(const MultivariatePolynomial<C,O,P>& lhs, const C& rhs)
{
    // TODO write dedicated add
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result -= rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(const C& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
	return rhs - lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(const MultivariatePolynomial<C,O,P>& lhs, Variable::Arg rhs)
{
    // TODO write dedicated add
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result -= rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator-(Variable::Arg lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs - lhs;
}

template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>& MultivariatePolynomial<Coeff,Ordering,Policy>::operator*=(const MultivariatePolynomial<Coeff,Ordering,Policy>& rhs)
{
    TermsType newTerms;
    newTerms.reserve(mTerms.size() * rhs.mTerms.size());
    for(auto termLhs : mTerms)
    {
        for(auto termRhs : rhs.mTerms)
        {
            newTerms.push_back(std::make_shared<Term<Coeff>>(*termLhs * *termRhs));
        }
    }
    mTerms.clear();
    // Sort the entries from newterms.
    // As automatic template deduction will not work (Ordering::less is overloaded), we give an explicit function pointer cast.
    std::sort(newTerms.begin(), newTerms.end(), (bool (&)(std::shared_ptr<const Term<Coeff>> const&, std::shared_ptr<const Term<Coeff>> const&))Ordering::less);
    // remove duplicates by adding their coefficients.
    // list.unique() fails because it does not handle coefficient updates.
    std::shared_ptr<const Term<Coeff>> frontTerm = newTerms.front();
    Coeff frontCoeff(frontTerm->coeff());
    
    for(auto it = ++newTerms.begin(); it != newTerms.end(); ++it)
    {
        if(Ordering::compare(*frontTerm, **it) == CompareResult::EQUAL)
        {
            // Do not add yet, but simply add the coefficient.
            frontCoeff += (*it)->coeff();
        }
        else
        {
            if(frontCoeff == frontTerm->coeff())
            {
                mTerms.push_back(frontTerm);
            }
            else
            {
                mTerms.emplace_back(std::make_shared<const Term<Coeff>>(frontCoeff, frontTerm->monomial()));
            }
            frontTerm = *it;
            frontCoeff = (*it)->coeff();
        }
    }
    
    if(frontCoeff == frontTerm->coeff())
    {
        mTerms.push_back(frontTerm);
    }
    else
    {
        mTerms.emplace_back(std::make_shared<const Term<Coeff>>(frontCoeff, frontTerm->monomial()));
    }
    
    return *this;
    
}
template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>& MultivariatePolynomial<Coeff,Ordering,Policy>::operator*=(const Term<Coeff>& rhs)
{
    TermsType newTerms;
    newTerms.reserve(mTerms.size());
    for(auto term : mTerms)
    {
        newTerms.push_back(std::make_shared<Term<Coeff>>(*term * rhs));
    }
    mTerms = std::move(newTerms);
    return *this;
}
template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>& MultivariatePolynomial<Coeff,Ordering,Policy>::operator*=(const Monomial& rhs)
{
    TermsType newTerms;
    newTerms.reserve(mTerms.size());
    for(auto term : mTerms)
    {
        newTerms.push_back(std::make_shared<Term<Coeff>>(*term * rhs));
    }
    mTerms = std::move(newTerms);
    return *this;
}
template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>& MultivariatePolynomial<Coeff,Ordering,Policy>::operator*=(const Variable::Arg rhs)
{
    TermsType newTerms;
    newTerms.reserve(mTerms.size());
    for(auto term : mTerms)
    {
        newTerms.push_back(std::make_shared<Term<Coeff>>(*term * rhs));
    }
    mTerms = std::move(newTerms);
    return *this;
}
template<typename Coeff, typename Ordering, typename Policy>
MultivariatePolynomial<Coeff,Ordering,Policy>& MultivariatePolynomial<Coeff,Ordering,Policy>::operator*=(const Coeff& c)
{
    if(c == 1) return *this;
    if(c == 0) 
    {
        mTerms.clear();
        return *this;
    }
    TermsType newTerms;
    newTerms.reserve(mTerms.size());
    for(auto term : mTerms)
    {
        newTerms.push_back(std::make_shared<Term<Coeff>>(*term * c));
    }
    mTerms = std::move(newTerms);
    return *this;
}

template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*( const MultivariatePolynomial<C,O,P>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    // TODO write dedicated mult
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result *= rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(const UnivariatePolynomial<C>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    LOG_NOTIMPLEMENTED();
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(const MultivariatePolynomial<C,O,P>& lhs, const UnivariatePolynomial<C>& rhs)
{
    return rhs * lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(const UnivariatePolynomial<MultivariatePolynomial<C>>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    LOG_NOTIMPLEMENTED();
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(const MultivariatePolynomial<C,O,P>& lhs, const UnivariatePolynomial<MultivariatePolynomial<C>>& rhs)
{
    return rhs * lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(const MultivariatePolynomial<C,O,P>& lhs, const Term<C>& rhs)
{
    // TODO write dedicated mult
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result *= rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(const Term<C>& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs * lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(const MultivariatePolynomial<C,O,P>& lhs, const Monomial& rhs)
{
    // TODO write dedicated mult
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result *= rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(const Monomial& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs * lhs;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(const MultivariatePolynomial<C,O,P>& lhs, const C& rhs)
{
    // TODO write dedicated mult
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result *= rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(const C& lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
	return rhs * lhs;
}


template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(const MultivariatePolynomial<C,O,P>& lhs, Variable::Arg rhs)
{
    // TODO write dedicated mult
    LOG_INEFFICIENT();
	MultivariatePolynomial<C,O,P> result(lhs);
	result *= rhs;
	return result;
}
template<typename C, typename O, typename P>
const MultivariatePolynomial<C,O,P> operator*(Variable::Arg lhs, const MultivariatePolynomial<C,O,P>& rhs)
{
    return rhs * lhs;
}

template<typename C, typename O, typename P>
std::ostream& operator <<( std::ostream& os, const MultivariatePolynomial<C,O,P>& rhs )
{
    const typename MultivariatePolynomial<C,O,P>::TermsType& terms(rhs.mTerms);
    typename MultivariatePolynomial<C,O,P>::TermsType::const_reverse_iterator term = terms.rbegin();
    if(terms.size() == 0) return os << "0";
    if(terms.size() == 1) return os << **term;
    os << **term;
    
    for(++term; term != terms.rend(); ++term)
    {
        os << " + " << **term;
    }
    return os;
}

template<typename Coeff, typename Ordering, typename Policy>
void MultivariatePolynomial<Coeff, Ordering, Policy>::sortTerms()
{
    std::sort(mTerms.begin(), mTerms.end(), (bool (*)(const std::shared_ptr<const Term<Coeff>>&, const std::shared_ptr<const Term<Coeff>>& ))Ordering::less);
}

}
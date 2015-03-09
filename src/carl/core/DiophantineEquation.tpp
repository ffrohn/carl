/**
 * @file DiophantineEquation.tpp
 * @author Tobias Winkler
 * 
 */

#pragma once

#include "DiophantineEquation.h"

namespace carl {

template<typename T>
std::vector<T> solveDiophantine(MultivariatePolynomial<T>& p) {
	static_assert(carl::is_integer<T>::value, "Diophantine equations are build from integral coefficients only!");
	std::vector<T> res;
        res = std::vector<T>();
	
        // use an algorithm for linear equations only
        // todo: it is possible that p has several terms with the same variable e.g. p= 2*x_1+3*x_1
        // p might have a constant term 0, thats not good
        if(p.isLinear()) {
            res = solveLinearDiophantine(p);
              
        }
            
	return res;
}

// Finds a single solution of a non-trivial linear diophantine Equation
template<typename T>
std::vector<T> solveLinearDiophantine(MultivariatePolynomial<T>& equation) {
    assert(equation.isLinear());
    assert(equation.hatConstantTerm()); // change this later
    
    T const_part = equation.constantPart();
    
    // initialize coefficient vector
    std::vector<carl::Term<T>> terms = equation.getTerms();
    std::vector<T> coeffs = std::vector<T>();
    for(int i = 1; i < terms.size(); i++ ) {
        coeffs.push_back(terms[i].coeff());
    }
    
    std::vector<T> fromExtendedGcd = std::vector<T>(1, 1);
    T currGcd = coeffs[0];
    std::vector<T> res;
    
    for(int i = 0; i < coeffs.size(); i++) {
        if(i == 0) {
            if(carl::mod(const_part, coeffs[0]) == 0) {
                res = std::vector<T>(coeffs.size(), 0);
                res[0] = -carl::div(const_part, coeffs[0]);
                return res;
            }
        }
        else {
            T s;
            T t;
            currGcd = extended_gcd_integer(currGcd, coeffs[i], s, t);
            for(auto& r : fromExtendedGcd) {
                r *= s;
            }
            fromExtendedGcd.push_back(t);
            if(carl::mod(const_part, currGcd) == 0) {
                T factor = -carl::div(const_part, currGcd);
                for(auto& r : fromExtendedGcd) {
                    r *= factor;
                }
                for(int j = coeffs.size() - fromExtendedGcd.size(); j < coeffs.size(); j++) {
                    fromExtendedGcd.push_back(0);
                }
                return fromExtendedGcd;
            }
            else {
                if(i == coeffs.size() - 1) {
                    // no solutions exist
                    res = std::vector<T>(coeffs.size(), 0); // change this later
                    return res;
                }
            }
        }
    }
}

template<typename T>
T extended_gcd_integer(T a, T b, T& s, T& t) {
    static_assert(carl::is_integer<T>::value, "extended gcd algorithmn is for integral types only!");
    
    if(a == 0 && b == 0) {
        s = 0; t = 0;
        return 0;
    }
    
    s = 0;
    T old_s;
    old_s = 1;
    
    t = 1;
    T old_t;
    old_t = 0;
    
    T r;
    r = b;
    T old_r;
    old_r = a;
    
    T temp;
    T quotient;
    
    while(r != 0) {
        quotient = carl::quotient(old_r, r);
        
        temp = r;
        r = old_r - quotient * r;
        old_r = temp;
        
        temp = s;
        s = old_s - quotient * s;
        old_s = temp;
        
        temp = t;
        t = old_t - quotient * t;
        old_t = temp;
    }
    // normalize so that the gcd is always positive
    if(old_r < 0) {
        old_s = -old_s;
        old_t = -old_t;
        old_r = -old_r;
    }
    s = old_s;
    t = old_t;
    return old_r;
}

}
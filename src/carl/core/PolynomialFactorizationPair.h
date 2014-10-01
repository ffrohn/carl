/* 
 * File:   PolynomialFactorizationPair.h
 * Author: Florian Corzilius
 *
 * Created on September 5, 2014, 3:57 PM
 */

#pragma once

#include <map>
#include <mutex>

namespace carl
{
    template<typename P>
    class FactorizedPolynomial;
    
    template<typename P>
    using Factorization = std::map<FactorizedPolynomial<P>, size_t>;
    
    template <typename P>
    std::ostream& operator<<( std::ostream& _out, const Factorization<P>& _factorization );
    
    template<typename P>
    bool factorizationsEqual( const Factorization<P>& _factorizationA, const Factorization<P>& _factorizationB );
    
    template<typename P>
    class PolynomialFactorizationPair
    {
        friend const Factorization<P>& FactorizedPolynomial<P>::factorization() const;
        //TODO (matthias) friend needed?
        friend FactorizedPolynomial<P>::FactorizedPolynomial( const P& _polynomial, Cache<PolynomialFactorizationPair<P>>& _cache );

    private:
        // Members
        
        /**
         * The hash of this polynomial factorization pair.
         */
        mutable size_t mHash;
        
        /**
         * A mutex for situation where any member is changed.
         */
        mutable std::recursive_mutex mMutex;
        
        /**
         * A factorization (not necessarily the prime factorization) of the polynomial.
         */
        mutable Factorization<P> mFactorization;
        
        /**
         * A pointer to a polynomial. This pointer might be set to nullptr, if the factorization has not yet been expanded.
         */
        mutable P* mpPolynomial;

        /**
         * Compute the polynomial from the factorization.
         * @param _fFactorization The factorization.
         * @return The polynomial.
         */
        //TODO (matthias) return reference?
        template<typename P1>
        friend P1 computePolynomial( const Factorization<P1>& _fFactorization );

        /**
         * Turn (possible) tree structure of factorization into linear list of factors.
         */
        void flattenFactorization() const;

        inline void assertFactorization() const
        {
            assert( mpPolynomial == nullptr || computePolynomial( mFactorization ) == *mpPolynomial );
        }

        /**
         * Get the flattened factorization.
         * @return The factorization of this polynomial factorization pair
         */
        const Factorization<P>& factorization() const
        {
            flattenFactorization();
            return mFactorization;
        }

        /**
         * Set new factorization for polynomial as two factors
         * @param _fpolyA First polynomial.
         * @param exponentA Exponent of first polynomial.
         * @param _fpolyB Second polynomial.
         * @param exponentB Exponent of second polynomial.
         */
        void setNewFactors( const FactorizedPolynomial<P>& _fpolyA, size_t exponentA, const FactorizedPolynomial<P>& _fpolyB, size_t exponentB ) const;

    public:
        // Constructor.
        PolynomialFactorizationPair(); // no implementation
        /*
         * @param _factorization The factorization. Every factor must be not constant.
         * @param _polynomial Polynomial with Polynomial = Factorization * Coefficient
         */
        PolynomialFactorizationPair( Factorization<P>&& _factorization, P* _polynomial = nullptr );
        PolynomialFactorizationPair( const PolynomialFactorizationPair& ); // no implementation
        ~PolynomialFactorizationPair();
        
        /**
         * @return The hash of this polynomial factorization pair.
         */
        size_t getHash() const
        {
            return mHash;
        }
        
        /**
         * @param _polyFactA The first polynomial factorization pair to compare.
         * @param _polyFactB The second polynomial factorization pair to compare.
         * @return true, if the two given polynomial factorization pairs are equal.
         */
        template<typename P1>
        friend bool operator==( const PolynomialFactorizationPair<P1>& _polyFactA, const PolynomialFactorizationPair<P1>& _polyFactB );
        
        /**
         * @param _polyFactA The first polynomial factorization pair to compare.
         * @param _polyFactB The second polynomial factorization pair to compare.
         * @return true, if the first given polynomial factorization pair is less than the second given polynomial factorization pair.
         */
        template<typename P1>
        friend bool operator<( const PolynomialFactorizationPair<P1>& _polyFactA, const PolynomialFactorizationPair<P1>& _polyFactB );
    
        /**
         * @param _toUpdate The polynomial factorization pair to be checked for the possibility to be updated.
         * @param _updateWith The polynomial factorization pair used to update the first given one.
         * @return true, if the first polynomial factorization pair can be updated with the second one.
         */
        template<typename P1>
        friend bool canBeUpdated( const PolynomialFactorizationPair<P1>& _toUpdate, const PolynomialFactorizationPair<P1>& _updateWith );
    
        /**
         * Updates the first given polynomial factorization pair with the information stored in the second given polynomial factorization pair.
         * @param _toUpdate The polynomial factorization pair to update with the second given one.
         * @param _updateWith The polynomial factorization pair used to update the first given one.
         */
        template<typename P1>
        friend void update( const PolynomialFactorizationPair<P1>& _toUpdate, const PolynomialFactorizationPair<P1>& _updateWith );
    
        /**
         * Calculates the factorization of the gcd of the polynomial represented by the two given polynomial factorization pairs. 
         * As a side effect the factorizations of these pairs can be refined. (c.f. Accelerating Parametric Probabilistic Verification, Algorithm 2)
         * @param _factA The first polynomial factorization pair to calculate the gcd with.
         * @param _factB The second polynomial factorization pair to calculate the gcd with.
         * @param _restA The remaining factorization of the first polynomial without the gcd.
         * @param _restB The remaining factorization of the second polynomial without the gcd.
         * @param _factARefined A bool which is set to true, if the factorization of the first given polynomial factorization pair has been refined.
         * @param _factBRefined A bool which is set to true, if the factorization of the second given polynomial factorization pair has been refined.
         * @return The factorization of the gcd of the polynomial represented by the two given polynomial factorization pairs. 
         */
        template<typename P1>
        friend Factorization<P1> gcd( const PolynomialFactorizationPair<P1>& _pfPairA, const PolynomialFactorizationPair<P1>& _pfPairB, Factorization<P1>& _restA, Factorization<P1>& _rest2B, bool& _pfPairARefined, bool& _pfPairBRefined );
        
        /**
         * Prints the given polynomial-factorization pair on the given output stream.
         * @param _out The stream to print on.
         * @param _pfPair The polynomial-factorization pair to print.
         * @return The output stream after inserting the output.
         */
        template <typename P1>
        friend std::ostream& operator<<( std::ostream& _out, const PolynomialFactorizationPair<P1>& _pfPair );
        
        /**
         * Updates the hash.
         */
        void rehash() const;
        
    };
} // namespace carl

namespace std
{
    template<typename P>
    struct hash<carl::PolynomialFactorizationPair<P>>
    {
        size_t operator()( const carl::PolynomialFactorizationPair<P>& _pfp ) const 
        {
            return _pfp.getHash();
        }
    };
} // namespace std

#include "PolynomialFactorizationPair.tpp"
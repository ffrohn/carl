/**
 * @file FormulaPool.h
 *
 * @author Florian Corzilius<corzilius@cs.rwth-aachen.de>
 * @version 2014-05-08
 */

#pragma once

#include "Formula.h"
#include "VariableNamePool.h"
#include "ConstraintPool.h"
#include "../util/Singleton.h"
#include <mutex>
#include <boost/variant.hpp>

#define SIMPLIFY_FORMULA

namespace carl
{
    
    template<typename Pol>
    class FormulaPool : public Singleton<FormulaPool<Pol>>
    {
            friend Singleton<FormulaPool>;
        
            ///
            typedef FormulaContent<Pol> Element;
            ///
            typedef std::shared_ptr<Element> ElementSPtr;
            ///
            typedef std::shared_ptr<const Element> ConstElementSPtr;
        
        private:
            
            // Members:
            /// id allocator
            unsigned mIdAllocator;
            /// The unique formula representing true.
            ElementSPtr mpTrue;
            /// The unique formula representing false.
            ElementSPtr mpFalse;
            /// The formula pool.
            FastSharedPointerSet<Element> mPool;
            /// Mutex to avoid multiple access to the pool
            mutable std::mutex mMutexPool;
            
            #define FORMULA_POOL_LOCK_GUARD std::lock_guard<std::mutex> lock( mMutexPool );
            #define FORMULA_POOL_LOCK mMutexPool.lock();
            #define FORMULA_POOL_UNLOCK mMutexPool.unlock();
            
        protected:
            
            /**
             * Constructor of the formula pool.
             * @param _capacity Expected necessary capacity of the pool.
             */
            FormulaPool( unsigned _capacity = 10000 );
            
        public:
            
            void print() const
            {
                std::cout << "Formula pool contains:" << std::endl;
                for( const auto& ele : mPool )
                {
                    std::cout << "id " << ele->mId << ":  " << *ele << std::endl;
                }
                std::cout << std::endl;
            }
    
            ConstElementSPtr trueFormula()
            {
                return mpTrue;
            }

            ConstElementSPtr falseFormula()
            {
                return mpFalse;
            }

            /**
             * @param _booleanVar The Boolean variable wrapped by this formula.
             * @return A formula with wrapping the given Boolean variable.
             */
            ConstElementSPtr create( Variable::Arg _booleanVar )
            {
                return add( std::move( ElementSPtr( new Element( _booleanVar ) ) ) );
            }
            
            /**
             * @param _constraint The constraint wrapped by this formula.
             * @return A formula with wrapping the given constraint.
             */
            ConstElementSPtr create( const Constraint<Pol>* _constraint )
            {
				assert(_constraint != nullptr);
                #ifdef SIMPLIFY_FORMULA
                if( _constraint == constraintPool<Pol>().consistentConstraint() )
                    return mpTrue;
                if( _constraint == constraintPool<Pol>().inconsistentConstraint() )
                    return mpFalse;
                #endif
                return add( std::move( ElementSPtr( new Element( _constraint ) ) ) );
            }
            
            /**
             * 
             * @param _subFormula
             * @return 
             */
            ConstElementSPtr createNegation( const Formula<Pol>& _subFormula )
            {
                #ifdef SIMPLIFY_FORMULA
                if( _subFormula.mpContent == mpTrue )
                    return mpFalse;
                if( _subFormula.mpContent == mpFalse )
                    return mpTrue;
				if (_subFormula.getType() == FormulaType::NOT)
					return _subFormula.subformula().mpContent;
                #endif
                // TODO: Actually we know that this formula does not begin with NOT and is already in the pool. Use this for optimization purposes.
                return add( std::move( ElementSPtr( new Element( _subFormula ) ) ) );
            }
    
            /**
             * 
             * @param _premise
             * @param _conclusion
             * @return 
             */
            ConstElementSPtr createImplication( const Formula<Pol>& _premise, const Formula<Pol>& _conclusion )
            {
                #ifdef SIMPLIFY_FORMULA
                if( _premise.mpContent == mpFalse )
                    return mpTrue;
                if( _premise.mpContent == mpTrue )
                    return _conclusion.mpContent;
                if( _conclusion.mpContent == mpTrue )
                    return mpTrue;
                if( _conclusion.mpContent == mpFalse )
                    return createNegation( _premise.mpContent );
                #endif
                return add( std::move( ElementSPtr( new Element( _premise, _conclusion ) ) ) );
            }
    
            /**
             * Create a formula representing an if-then-else.
             * @param _condition Condition.
             * @param _then Then.
			 * @param _else Else.
             * @return
             */
            ConstElementSPtr createIte( const Formula<Pol>& _condition, const Formula<Pol>& _then, const Formula<Pol>& _else )
            {
                #ifdef SIMPLIFY_FORMULA
                if( _condition.mpContent == mpFalse || _then == _else )
                    return _else.mpContent;
                if( _condition.mpContent == mpTrue )
                    return _then.mpContent;
                #endif
                return add( std::move( ElementSPtr( new Element( _condition, _then, _else ) ) ) );
            }

			/**
			 *
			 * @param _type
			 * @param _vars
			 * @param _term
			 * @return
			 */
			ConstElementSPtr create(FormulaType _type, const std::vector<Variable>&& _vars, const Formula<Pol>& _term)
			{
				assert(_type == FormulaType::EXISTS || _type == FormulaType::FORALL);
				if (_vars.size() > 0) {
					return add( std::move( ElementSPtr( new Element(_type, std::move(_vars), _term ) ) ) );
				} else {
					return _term.mpContent;
				}
			}
            
            /**
             * @param _type The type of the n-ary operator (n>1) of the formula to create.
             * @param _subFormulaA The first sub-formula of the formula to create.
             * @param _subFormulaB The second sub-formula of the formula to create.
             * @return A formula with the given operator and sub-formulas.
             */
            ConstElementSPtr create( FormulaType _type, const Formula<Pol>& _subFormulaA, const Formula<Pol>& _subFormulaB )
            {
                std::set<Formula<Pol>> subFormulas;
                subFormulas.insert( _subFormulaA );
                subFormulas.insert( _subFormulaB );
                return create( _type, std::move( subFormulas ) );
            }
            
            /**
             * @param _subformulas The sub-formulas of the formula to create.
             * @return A formula with the given operator and sub-formulas.
             */
            ConstElementSPtr create( const std::multiset<Formula<Pol>>& _subformulas )
            {
                if( _subformulas.empty() ) return mpFalse;
                if( _subformulas.size() == 1 ) return _subformulas.begin()->mpContent;
                std::set<Formula<Pol>> subFormulas;
                auto lastSubFormula = _subformulas.begin();
                auto subFormula = lastSubFormula;
                ++subFormula;
                int counter = 1;
                while( subFormula != _subformulas.end() )
                {
                    if( **lastSubFormula == **subFormula )
                    {
                        ++counter;
                    }
                    else
                    {
                        if( counter % 2 == 1 )
                        {
                            subFormulas.insert( subFormulas.end(), *lastSubFormula ); // set has same order as the multiset
                        }
                        lastSubFormula = subFormula;
                        counter = 1;
                    }
                    ++subFormula;
                }
                if( counter % 2 == 1 )
                {
                    subFormulas.insert( subFormulas.end(), *lastSubFormula );
                }
                return create( Formula<Pol>::XOR, std::move( subFormulas ) );
            }
            
            /**
             * @param _type The type of the n-ary operator (n>1) of the formula to create.
             * @param _subformulas The sub-formulas of the formula to create.
             * @return A formula with the given operator and sub-formulas.
             */
            ConstElementSPtr create( FormulaType _type, const std::set<Formula<Pol>>& _subformulas )
            {
                return create( _type, std::move( std::set<Formula<Pol>>( _subformulas ) ) );
            }

			ConstElementSPtr create( const UEquality::Arg& _lhs, const UEquality::Arg& _rhs, bool _negated )
			{
                #ifdef SIMPLIFY_FORMULA
                if( boost::apply_visitor(UEquality::IsUVariable(), _lhs) && boost::apply_visitor(UEquality::IsUVariable(), _rhs) )
                {
                    if( boost::get<UVariable>(_lhs) < boost::get<UVariable>(_rhs) )
                        return add( std::move( ElementSPtr( new Element( UEquality( boost::get<UVariable>(_lhs), boost::get<UVariable>(_rhs), _negated, true ) ) ) ) );
                    if( boost::get<UVariable>(_rhs) < boost::get<UVariable>(_lhs) )
                        return add( std::move( ElementSPtr( new Element( UEquality( boost::get<UVariable>(_rhs), boost::get<UVariable>(_lhs), _negated, true ) ) ) ) );
                    else if( _negated )
                        return mpFalse;
                    else
                        return mpTrue;
                }
				else if( boost::apply_visitor(UEquality::IsUVariable(), _lhs) && boost::apply_visitor(UEquality::IsUFInstance(), _rhs) )
                {
                    return add( std::move( ElementSPtr( new Element( UEquality( boost::get<UVariable>(_lhs), boost::get<UFInstance>(_rhs), _negated ) ) ) ) );
                }
                else if( boost::apply_visitor(UEquality::IsUFInstance(), _lhs) && boost::apply_visitor(UEquality::IsUVariable(), _rhs) )
                {
                    return add( std::move( ElementSPtr( new Element( UEquality( boost::get<UVariable>(_rhs), boost::get<UFInstance>(_lhs), _negated ) ) ) ) );
                }
                else
                {
                    assert( boost::apply_visitor(UEquality::IsUFInstance(), _lhs) && boost::apply_visitor(UEquality::IsUFInstance(), _rhs) );
                    if( boost::get<UFInstance>(_lhs) < boost::get<UFInstance>(_rhs) )
                        return add( std::move( ElementSPtr( new Element( UEquality( boost::get<UFInstance>(_lhs), boost::get<UFInstance>(_rhs), _negated, true ) ) ) ) );
                    if( boost::get<UFInstance>(_rhs) < boost::get<UFInstance>(_lhs) )
                        return add( std::move( ElementSPtr( new Element( UEquality( boost::get<UFInstance>(_rhs), boost::get<UFInstance>(_lhs), _negated, true ) ) ) ) );
                    else if( _negated )
                        return mpFalse;
                    else
                        return mpTrue;
                }
                #else
                return add( std::move( ElementSPtr( new Element( UEquality( _lhs, _rhs, _negated ) ) ) ) );
                #endif
			}

			ConstElementSPtr create( UEquality&& eq )
			{
				return add( std::move( ElementSPtr( new Element( std::move( eq ) ) ) ) );
			}
            
            template<typename ArgType>
            void forallDo( void (*_func)( ArgType*, const Formula<Pol>& ), ArgType* _arg ) const
            {
                FORMULA_POOL_LOCK_GUARD
                for( ConstElementSPtr formula : mPool )
                    (*_func)( _arg, Formula<Pol>( formula ) );
            }
            
            template<typename ReturnType, typename ArgType>
            std::map<const Formula<Pol>,ReturnType> forallDo( ReturnType (*_func)( ArgType*, const Formula<Pol>& ), ArgType* _arg ) const
            {
                FORMULA_POOL_LOCK_GUARD
                std::map<const Formula<Pol>,ReturnType> result;
                for( ConstElementSPtr elem : mPool )
                {
                    Formula<Pol> form(elem);
                    result[form] = (*_func)( _arg, form );
                }
                return result;
            }
            
            /**
             */
            bool formulasInverse( const Formula<Pol>& _subformulaA, const Formula<Pol>& _subformulaB );
        
            /**
             * @param _type The type of the n-ary operator (n>1) of the formula to create.
             * @param _subformulas The sub-formulas of the formula to create.
             * @return A formula with the given operator and sub-formulas.
             * Note, that if you use this method to create a formula with the operator XOR
             * and you have collected the sub-formulas in a set, multiple occurrences of a
             * sub-formula are condensed. You should only use it, if you can exlcude this 
             * possibility. Otherwise use the method newExclusiveDisjunction.
             */
            ConstElementSPtr create( FormulaType _type, std::set<Formula<Pol>>&& _subformulas );
            
    private:
        
            /**
             * Creates a formula of the given type but with only one sub-formula.
             * @param _type
             * @param _subformula
             * @return True, if the given type is IFF;
             *         False, if the given type is XOR;
             *         The given sub-formula if the type is AND or OR.
             */
            ConstElementSPtr newFormulaWithOneSubformula( FormulaType _type, const Formula<Pol>& _subformula )
            {
                assert( FormulaType::OR || FormulaType::AND || FormulaType::XOR || FormulaType::IFF );
                // We expect that this only happens, if the intended sub-formulas are all the same.
                switch( _type )
                {
                    case FormulaType::XOR: // f xor f is false
                        return mpFalse;
                    case FormulaType::IFF: // f iff f is true
                        return mpTrue;
                    default: // f or f = f; f and f = f
                        return _subformula.mpContent;
                }
            }
            
            /**
             * Inserts the given formula to the pool, if it does not yet occur in there.
             * @param _formula The formula to add to the pool.
             * @param _elementNotInPool true, if the element is not yet in the pool; false, if this is not known.
             * @return The position of the given formula in the pool and true, if it did not yet occur in the pool;
             *         The position of the equivalent formula in the pool and false, otherwise.
             */
            std::pair<typename FastSharedPointerSet<Element>::iterator,bool> insert( ElementSPtr&& _formula, bool _elementNotInPool );
            
            /**
             * Adds the given formula to the pool, if it does not yet occur in there.
             * Note, that this method uses the allocator which is locked before calling.
             * @param _formula The formula to add to the pool.
             * @return The given formula, if it did not yet occur in the pool;
             *         The equivalent formula already occurring in the pool, otherwise.
             */
            ConstElementSPtr add( ElementSPtr&& _formula );
    };
}    // namespace carl

#include "FormulaPool.tpp"
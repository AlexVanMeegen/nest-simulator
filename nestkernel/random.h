/*
 *  random.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef RANDOM_H
#define RANDOM_H

// C++ includes:
#include <initializer_list>
#include <memory>
#include <random>
#include <utility>
#include <type_traits>

// Includes from sli:
#include "dictdatum.h"
#include "name.h"

namespace nest
{

// Forward declarations
struct BaseRNG;

template < class DIST_TYPE_ >
struct distribution;

// Type definitions
using RngPtr = std::shared_ptr< BaseRNG >;

using uniform_int_distribution = distribution< std::uniform_int_distribution< unsigned long > >;
using uniform_real_distribution = distribution< std::uniform_real_distribution<> >;
using poisson_distribution = distribution< std::poisson_distribution< unsigned long > >;
using normal_distribution = distribution< std::normal_distribution<> >;
using lognormal_distribution = distribution< std::lognormal_distribution<> >;
using binomial_distribution = distribution< std::binomial_distribution< unsigned long > >;
using gamma_distribution = distribution< std::gamma_distribution<> >;
using exponential_distribution = distribution< std::exponential_distribution<> >;


/**
 * @brief Base class for RNG engine wrappers.
 *
 * The base class has no function implementations; all functions are pure virtual.
 */
struct BaseRNG
{
  /**
   * @brief Calls the provided distribution with the wrapped RNG engine.
   * One operator per distribution must be defined.
   *
   * @param d Distribution that will be called.
   */
  virtual unsigned long operator()( std::uniform_int_distribution< unsigned long >& d ) = 0;
  virtual double operator()( std::uniform_real_distribution<>& d ) = 0;
  virtual unsigned long operator()( std::poisson_distribution< unsigned long >& d ) = 0;
  virtual double operator()( std::normal_distribution<>& d ) = 0;
  virtual double operator()( std::lognormal_distribution<>& d ) = 0;
  virtual unsigned long operator()( std::binomial_distribution< unsigned long >& d ) = 0;
  virtual double operator()( std::gamma_distribution<>& d ) = 0;
  virtual double operator()( std::exponential_distribution<>& d ) = 0;

  /**
   * @brief Calls the provided distribution with the wrapped RNG engine, using provided distribution parameters.
   * One operator per distribution must be defined.
   *
   * @param d Distribution that will be called.
   * @param p Distribution parameters.
   */
  virtual unsigned long operator()( std::uniform_int_distribution< unsigned long >& d,
    std::uniform_int_distribution< unsigned long >::param_type& p ) = 0;
  virtual double operator()( std::uniform_real_distribution<>& d, std::uniform_real_distribution<>::param_type& p ) = 0;
  virtual unsigned long operator()( std::poisson_distribution< unsigned long >& d,
    std::poisson_distribution< unsigned long >::param_type& p ) = 0;
  virtual double operator()( std::normal_distribution<>& d, std::normal_distribution<>::param_type& p ) = 0;
  virtual double operator()( std::lognormal_distribution<>& d, std::lognormal_distribution<>::param_type& p ) = 0;
  virtual unsigned long operator()( std::binomial_distribution< unsigned long >& d,
    std::binomial_distribution< unsigned long >::param_type& p ) = 0;
  virtual double operator()( std::gamma_distribution<>& d, std::gamma_distribution<>::param_type& p ) = 0;
  virtual double operator()( std::exponential_distribution<>& d, std::exponential_distribution<>::param_type& p ) = 0;

  /**
   * @brief Uses the wrapped RNG engine to draw a double from a uniform distribution in the range [0, 1).
   */
  virtual double drand() = 0;

  /**
   * @brief Uses the wrapped RNG engine to draw an unsigned long from a uniform distribution in the range [0, N).
   * @param N Maximum value that can be drawn.
   */
  virtual unsigned long ulrand( unsigned long N ) = 0;
};

/**
 * @brief Wrapper for RNG engines.
 * @tparam RNG_TYPE_ Type of the wrapped engine.
 */
template < typename RNG_TYPE_ >
struct RNG final : public BaseRNG
{
  using result_type = typename RNG_TYPE_::result_type;

  RNG() = delete;
  RNG( const RNG_TYPE_& rng ) = delete;

  RNG( std::initializer_list< std::uint32_t > seed )
    : rng_()
    , uniform_double_dist_0_1_( 0.0, 1.0 )
  {
	std::seed_seq sseq( seed );
	rng_.seed( sseq );
  }

    // TODO: Can we use templates with override here?
  inline unsigned long operator()( std::uniform_int_distribution< unsigned long >& d ) override
  {
    return d( rng_ );
  }

  inline double operator()( std::uniform_real_distribution<>& d ) override
  {
    return d( rng_ );
  }

  inline unsigned long operator()( std::poisson_distribution< unsigned long >& d ) override
  {
    return d( rng_ );
  }

  inline double operator()( std::normal_distribution<>& d ) override
  {
    return d( rng_ );
  }

  inline double operator()( std::lognormal_distribution<>& d ) override
  {
    return d( rng_ );
  }

  inline unsigned long operator()( std::binomial_distribution< unsigned long >& d ) override
  {
    return d( rng_ );
  }

  inline double operator()( std::gamma_distribution<>& d ) override
  {
    return d( rng_ );
  }

  inline double operator()( std::exponential_distribution<>& d ) override
  {
    return d( rng_ );
  }

  inline unsigned long operator()( std::uniform_int_distribution< unsigned long >& d,
    std::uniform_int_distribution< unsigned long >::param_type& p ) override
  {
    return d( rng_, p );
  }

  inline double operator()( std::uniform_real_distribution<>& d,
    std::uniform_real_distribution<>::param_type& p ) override
  {
    return d( rng_, p );
  }

  inline unsigned long operator()( std::poisson_distribution< unsigned long >& d,
    std::poisson_distribution< unsigned long >::param_type& p ) override
  {
    return d( rng_, p );
  }

  inline double operator()( std::normal_distribution<>& d, std::normal_distribution<>::param_type& p ) override
  {
    return d( rng_, p );
  }

  inline double operator()( std::lognormal_distribution<>& d, std::lognormal_distribution<>::param_type& p ) override
  {
    return d( rng_, p );
  }

  inline unsigned long operator()( std::binomial_distribution< unsigned long >& d,
    std::binomial_distribution< unsigned long >::param_type& p ) override
  {
    return d( rng_, p );
  }

  inline double operator()( std::gamma_distribution<>& d, std::gamma_distribution<>::param_type& p ) override
  {
    return d( rng_, p );
  }

  inline double operator()( std::exponential_distribution<>& d,
    std::exponential_distribution<>::param_type& p ) override
  {
    return d( rng_, p );
  }

  inline double
  drand() override
  {
    return uniform_double_dist_0_1_( rng_ );
  }

  inline unsigned long
  ulrand( unsigned long N ) override
  {
    std::uniform_int_distribution< unsigned long >::param_type param( 0, N - 1 );
    return uniform_ulong_dist_( rng_, param );
  }

private:
  RNG_TYPE_ rng_; //!< Wrapped RNG engine.
  std::uniform_int_distribution< unsigned long > uniform_ulong_dist_;
  std::uniform_real_distribution<> uniform_double_dist_0_1_;
};

struct BaseRNGFactory
{
  virtual ~BaseRNGFactory() {};

  /**
   * @brief Clones the RNG wrapper and sets the state of the cloned RNG engine.
   * @param seed Initializer list for seed sequence for RNG.
   */
  virtual RngPtr clone( std::initializer_list< std::uint32_t > seed ) const = 0;
};

template < typename RNG_TYPE_ >
struct RNGFactory final : public BaseRNGFactory
{
  inline RngPtr
  clone( std::initializer_list< std::uint32_t > seed ) const override
  {
	return std::make_shared< RNG< RNG_TYPE_ > >( seed );
  }
};

/**
 * @brief Wrapper for distributions.
 * The result_type of the distribution must be unsigned long or double.
 *
 * @tparam DIST_TYPE_ Type of the wrapped distribution.
 */
template < typename DIST_TYPE_ >
struct distribution
{
  using result_type = typename DIST_TYPE_::result_type;
  using param_type = typename DIST_TYPE_::param_type;

  /**
   * @brief Constructs a distribution object with its default parameters.
   */
  distribution() = default;
  ~distribution() = default;

  /**
   * @brief Generating function.
   *
   * This function just flips the call, to call the RNG wrapper with the distribution. This is done so that the native
   * C++ distribution can be called with the native C++ generator.
   *
   * @param g Pointer to the RNG wrapper.
   */
  inline result_type operator()( std::shared_ptr< BaseRNG > g )
  {
    static_assert( std::is_same< result_type, unsigned long >::value or std::is_same< result_type, double >::value,
      "result_type of the distribution must be unsigned long or double" );
    return g->operator()( distribution_ );
  }

  /**
   * @brief Generating function.
   *
   * This function just flips the call, to call the RNG wrapper with the distribution. This is done so that the native
   * C++ distribution can be called with the native C++ generator.
   *
   * @param g Pointer to the RNG wrapper.
   * @param params Distribution parameter set to use.
   */
  inline result_type operator()( std::shared_ptr< BaseRNG > g, param_type& params )
  {
    static_assert( std::is_same< result_type, unsigned long >::value or std::is_same< result_type, double >::value,
      "result_type of the distribution must be unsigned long or double" );
    return g->operator()( distribution_, params );
  }

  /**
   * @brief Sets the distribution's associated parameter set to params.
   * @param params New contents of the distribution's associated parameter set.
   */
  inline void
  param( const param_type& params )
  {
    distribution_.param( params );
  }

  /**
   * @brief Returns the minimum value potentially generated by the distribution.
   */
  inline result_type
  min() const
  {
    return distribution_.min();
  }

  /**
   * @brief Returns the maximum value potentially generated by the distribution.
   */
  inline result_type
  max() const
  {
    return distribution_.max();
  }

private:
  DIST_TYPE_ distribution_; //!< Wrapped distribution
};

} // namespace nest

#endif /* #ifndef RANDOM_H */

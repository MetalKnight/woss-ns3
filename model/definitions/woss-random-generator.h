/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Federico Guerra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Federico Guerra <WOSS@guerra-tlc.com>
 */

#ifdef NS3_WOSS_SUPPORT

#ifndef WOSS_RANDOM_GENERATOR_H
#define WOSS_RANDOM_GENERATOR_H


#include <random-generator-definitions.h>
#include <ns3/random-variable-stream.h>

namespace ns3 {

/**
* \class WossRandomGenerator
*
* \brief Wrapper class for woss::RandomGenerator.
*
* woss::RandomGenerator wraps a ns3::UniformRandomVariable into the WOSS framework.
*
*/
class WossRandomGenerator : public woss::RandomGenerator
{


public:
  /**
    * Default constructor used within the WOSS framework.
    * The seed parameter allows the WOSS framework to abstract
    * from the wrapped random generator model,
    * however in the NS3 integration framework it is
    * not used. The RandomVar is created with default random
    * generator seed, while the stream can be assigned via AssignStreams
    *
    * \param seed assigned stream
    */
  WossRandomGenerator ( int seed = 0 );

  /**
   * Copy constructor.
   * The returned object reflects the internal status of the source underlying
   * random var object.
   * initialize () should be called if it wasn't previously called on the source object
   */
  WossRandomGenerator ( const WossRandomGenerator& copy );

  /**
   * Assignment operator.
   * The returned reference reflects the internal status of the source underlying
   * random var object.
   * initialize () should be called if it wasn't previously called on the source object
   */
  WossRandomGenerator& operator= ( const WossRandomGenerator& copy );

  virtual ~WossRandomGenerator () = default;

  /**
   * Mandatory virtual factory method that creates a new object.
   * It is inherited and it is used within the WOSS framework.
   * It creates an new object; the seed parameter allows the WOSS framework to abstract
   * from the wrapped random generator model,
   * however in the NS3 integration framework it is
   * not used. The RandomVar is created with default random
   * generator seed, while the stream can be assigned via AssignStreams
   * on the private m_RandVar
   * \param seed assigned stream
   */
  virtual std::unique_ptr<woss::RandomGenerator> create ( int seed ) const override;

  /**
   * Virtual factory method that creates a new object.
   * It is inherited and it is used within the WOSS framework.
   * It creates an new object; the seed parameter allows the WOSS framework to abstract
   * from the wrapped random generator model,
   * however in the NS3 integration framework it is
   * not used. The RandomVar is created with default random
   * generator seed, while the stream can be assigned via AssignStreams
   * on the private m_RandVar
   * \param seed assigned seed in double format
   */
  virtual std::unique_ptr<woss::RandomGenerator> create ( double seed ) const;

  /**
   * Mandatory virtual factory method that clones the current object.
   * It is inherited from the WOSS framework.
   * The returned object will draw from the same sub-stream of the source object.
   */
  virtual std::unique_ptr<woss::RandomGenerator> clone () const override;

  /**
   * returns a random double numer
   * \returns a random double number
   */
  virtual double getRand () const override;

  /**
   * returns a random integer numer in range [0, INT_MAX]
   * \returns a random integer number
   */
  virtual int getRandInt () const override;

  /**
   * Mandatory initialization method inherited from the WOSS framework.
   * Usually a woss::RandomGenerator is created and then initialized
   * with this function.
   * The initialization is performed only once.
   * \see AssignStreams
   */
  virtual void initialize () override;


  /**
   * In the NS3 wrapper it should be used to change the current stream used.
   * It calls SetStream () on the underlying m_RandVar.
   */
  virtual int64_t AssignStreams (int64_t stream);


protected:
  /**
   * The wrapped NS3 UniformRandomVariable random generator
   */
  Ptr<UniformRandomVariable> m_RandVar;

};

}

#endif /* WOSS_RANDOM_GENERATOR_H */

#endif /* NS3_WOSS_SUPPORT */


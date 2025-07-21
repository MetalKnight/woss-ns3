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
 * Author: Federico Guerra <federico@guerra-tlc.com>
 */

#ifdef NS3_WOSS_SUPPORT

#ifndef WOSS_PROP_MODEL_H
#define WOSS_PROP_MODEL_H

#include <memory>
#include "ns3/uan-prop-model-thorp.h"
#include <woss-manager.h>

namespace ns3 {

class Time;

/**
 * \ingroup WOSS
 * \class WossPropModel
 *
 * Propagation Model that retrieves a power delay profile from the WOSS framework
 * and converts it in a UanPdp object
 */
class WossPropModel : public UanPropModelThorp
{
public:
  typedef ::std::vector< UanPdp > UanPdpVector; //!< ::std::vector of ns3::UanPDP objects
  typedef ::std::vector< Ptr<MobilityModel> > MobModelVector; //!< ::std::vector of ns3::MobilityModel smart pointers

  WossPropModel (); //!< Default constructor
  virtual ~WossPropModel () = default; //!< Default destructor

  /**
   * Register this type.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  /**
   * Mandatory method that allows the interaction with the WOSS framework.
   * WossHelper class will automatically handle all connections.
   *
   * \param woss::wossManagerPtr the pointer must point to the same object for all prop object
   */
  void SetWossManager (std::shared_ptr<woss::WossManager> wossManagerPtr);

  /**
   * returns a pointer to the woss::WossManager object
   */
  std::shared_ptr<woss::WossManager> const GetWossManager (void);

  /**
   * This function is not supported by the UAN-WOSS framework
   */
  virtual double GetPathLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b, UanTxMode mode);

  /**
   * This function returns the Power Delay Profile between the two input nodes
   * \param a transmitter node
   * \param b receiver node
   * \param mode transmission mode
   * \returns the Power Delay Profile
   */
  virtual UanPdp GetPdp (Ptr<MobilityModel> a, Ptr<MobilityModel> b, UanTxMode mode);

  /**
   * New function that takes advantage of the WOSS multithread feature.
   * It calculates the power delay profiles from a single transmitter to a set of receiver with the given tx mode.
   * \param a transmitter mobility model
   * \param b vector of receiver mobility model
   * \param mode transmission mode used by the transmitter
   * \returns the vector of the calculated power delay profiles
   */
  virtual UanPdpVector GetPdpVector (Ptr<MobilityModel> a, MobModelVector& b, UanTxMode mode);

  /**
   * This function is not supported by the UAN-WOSS framework
   */
  virtual Time GetDelay (Ptr<MobilityModel> a, Ptr<MobilityModel> b, UanTxMode mode);

  /**
   * This function should be used as for time delay purposes
   * It calculates the delay between two geographical coordinates
   * \param a geographical coordinates of transmitter node
   * \param b geographical coordinates of receiver node
   * \param mode transmitter mode
   * \param chAttThresDb The first channel tap that gives a SNR (dB)
   * greater than the threshold will be used as delay.
   * \returns the delay in seconds
   */
  virtual Time GetDelay (const woss::CoordZ &a, const woss::CoordZ &b, UanTxMode mode, double chAttThresDb);


protected:
  std::shared_ptr<woss::WossManager> m_wossManager; //!< woss::WossManager object used to trigger acoustic channel computations

  virtual void DoInitialize (void);

  /**
   * Converts a ns3::UanPdp from a woss::TimeArr object, and symbol time in seconds
   * \param timeArr pointer to a woss::TimeArr object
   * \param symbolTime the modulation symbol time in seconds
   * \returns a ns3::UanPdp object
   */
  UanPdp CreateUanPdp (std::unique_ptr<woss::TimeArr> timeArr, double symbolTime); // seconds

  /**
   * Returns a woss::CoordZ object from the current position of the given mobility model
   * \param mobModel smart pointer to a node's mobility model
   * \returns a woss::CoordZ object
   */
  woss::CoordZ CreateCoordZ (Ptr<MobilityModel> mobModel);

  /**
   * Converts a ns3::UanPdp from a woss::TimeArrVector object, and symbol time in seconds
   * \param timeArr reference to a woss::TimeArrVector object
   * \param symbolTime the modulation symbol time in seconds
   * \returns a ns3::UanPdpVector object
   */
  UanPdpVector CreateUanPdpVector (woss::TimeArrVector& timeArr, double symbolTime); // seconds

  /**
   * Creates a woss::CoordZPairVect from a woss::TimeArr object, and symbol time in seconds
   * \param tx transmitter mobility model
   * \param rx reference to a vector of mobility models (receivers)
   * \returns a ns3::CoordZPairVect object
   */
  woss::CoordZPairVect CreateCoordzPairVector (Ptr<MobilityModel> tx, MobModelVector& rxs);

private:

  bool m_memOptimization; //!< If true, WOSS objects are freed as soon as possible. 
};

}

#endif /* WOSS_PROP_MODEL_H */

#endif /* NS3_WOSS_SUPPORT */

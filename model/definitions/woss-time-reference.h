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

#ifndef WOSS_TIME_REFERENCE_H
#define WOSS_TIME_REFERENCE_H


#include <time-definitions.h>


namespace ns3 {

/**
* \class WossTimeReference
*
* \brief Wrapper class for woss::TimeReference
*
* the class woss::TimeReference wraps the NS3 scheduler clock into the WOSS framework
* and it is used as a simulation time reference.
* This class simply calls the static ns3::Simulator.Now ().
*/
class WossTimeReference : public woss::TimeReference
{


public:
  virtual ~WossTimeReference ();

  /**
   * returns the current NS3 time in seconds represented as a double.
   * \returns the current NS3 time in seconds in double representation
   */
  virtual double getTimeReference () const;

  /**
   * Mandatory virtual factory method that returns a new WossTimeReference created via
   * copy-constructor. The new object will return the same time-reference as the source
   * object since they both rely on ns3::Scheduler::Now ()
   */
  virtual WossTimeReference* clone ();


};

}

#endif // WOSS_TIME_REFERENCE_H

#endif /* NS3_WOSS_SUPPORT */


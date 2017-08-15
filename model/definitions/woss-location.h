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

#ifndef WOSS_LOCATION_H
#define WOSS_LOCATION_H


#include <ns3/mobility-model.h>
#include <location-definitions.h>


namespace ns3 {

/**
* \class WossLocation
*
* Wrapper class for woss::Location. woss::Location embeds the concept of
* node position and mobility into the WOSS framework.
*/
class WossLocation : public woss::Location
{

public:
  /**
   * \param model underlying ns3::MobilityModel
   */
  WossLocation (Ptr<MobilityModel> model);

  virtual ~WossLocation ();

  /**
   * This is the main function called by the WOSS framework in order to get
       * the node's current position.
   *
   * \returns converts the underlying MobilityModel::GetPosition()
   * into a woss::CoordZ object
   */
  virtual woss::CoordZ getLocation ();


  void SetMobilityModel (Ptr<MobilityModel> model);

  Ptr<MobilityModel> GetMobilityModel ();

protected:
  Ptr<MobilityModel> m_mobModel;

};


}

#endif // WOSS_LOCATION_H

#endif // NS3_WOSS_SUPPORT

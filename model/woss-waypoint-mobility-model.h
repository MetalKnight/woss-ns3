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

#ifndef WOSS_WAYPOINT_MOBILITY_MODEL_H
#define WOSS_WAYPOINT_MOBILITY_MODEL_H


#include "ns3/waypoint-mobility-model.h"


namespace ns3 {

/**
 * \ingroup WOSS
 * \brief Waypoint mobility model with geographic coordinates.
 *
 * The underlying model implements a waypoint mobility model with geographic coordinates.
 * The waypoint must be given in cartesian coordinates and will be automatically converted into
 * geographic coordinates by the class.
 * To convert the waypoints from geographic coordinates to cartesian coordinates please use the
 * helper function.
 * \see ns3::WossHelper
 */
class WossWaypointMobilityModel : public WaypointMobilityModel
{
public:
  /**
   * Register this type.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  WossWaypointMobilityModel () = default; //!< Default constructor

  virtual ~WossWaypointMobilityModel () = default; //!< Default destructor


private:
  friend class WaypointMobilityModelNotifyTest; //!< To allow Update() calls and access to m_current

  virtual void Update (void) const override;
  virtual void DoDispose (void) override;


};

} // namespace ns3

#endif /* WOSS_WAYPOINT_MOBILITY_MODEL_H */

#endif /* NS3_WOSS_SUPPORT */


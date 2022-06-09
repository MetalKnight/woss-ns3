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

#include "ns3/mobility-model.h"
#include "ns3/woss-location.h"


namespace ns3 {


WossLocation::WossLocation (Ptr<MobilityModel> m)
  : m_mobModel (m)
{

}


WossLocation::~WossLocation ()
{

}


woss::CoordZ
WossLocation::getLocation ()
{
  Vector vector = m_mobModel->GetPosition ();

  return (woss::CoordZ::getCoordZFromCartesianCoords (vector.x, 
                                                      vector.y, 
                                                      vector.z, 
                                                      woss::CoordZ::CoordZSpheroidType::COORDZ_WGS84));
}


void
WossLocation::SetMobilityModel (Ptr<MobilityModel> m)
{
  m_mobModel = m;
}


Ptr<MobilityModel>
WossLocation::GetMobilityModel ()
{
  return m_mobModel;
}

}

#endif /* NS3_WOSS_SUPPORT */

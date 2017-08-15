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

#include <coordinates-definitions.h>
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "woss-waypoint-mobility-model.h"



namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WossWaypointMobilityModel");

NS_OBJECT_ENSURE_REGISTERED (WossWaypointMobilityModel);


TypeId
WossWaypointMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WossWaypointMobilityModel")
    .SetParent<WaypointMobilityModel> ()
    .SetGroupName ("Woss")
    .AddConstructor<WossWaypointMobilityModel> ()
  ;
  return tid;
}


WossWaypointMobilityModel::WossWaypointMobilityModel ()
{
}

WossWaypointMobilityModel::~WossWaypointMobilityModel ()
{
}

void
WossWaypointMobilityModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  MobilityModel::DoDispose ();
}

void
WossWaypointMobilityModel::Update (void) const
{
  NS_LOG_FUNCTION (this);

  const Time now = Simulator::Now ();
  bool newWaypoint = false;

  if ( now < m_current.time )
    {
      return;
    }

  while ( now >= m_next.time )
    {
      if ( m_waypoints.empty () )
        {
          if ( m_current.time <= m_next.time )
            {
              /*
                Set m_next.time = -1 to make sure this doesn't happen more than once.
                The comparison here still needs to be '<=' in the case of mobility with one waypoint.
              */
              m_next.time = Seconds (-1.0);
              m_current.position = m_next.position;
              m_current.time = now;
              m_velocity = Vector (0,0,0);
              NotifyCourseChange ();
            }
          else
            {
              m_current.time = now;
            }

          return;
        }

      m_current = m_next;
      m_next = m_waypoints.front ();
      m_waypoints.pop_front ();
      newWaypoint = true;

      const double tSpan = (m_next.time - m_current.time).GetSeconds ();
      NS_ASSERT (tSpan > 0);

      m_velocity.x = (m_next.position.x - m_current.position.x) / tSpan;
      m_velocity.y = (m_next.position.y - m_current.position.y) / tSpan;
      m_velocity.z = (m_next.position.z - m_current.position.z) / tSpan;
    }

  if ( now > m_current.time ) // Won't ever be less, but may be equal
    {
      const double tSpan = (m_next.time - m_current.time).GetSeconds ();

      const double tDiff = (now - m_current.time).GetSeconds ();

      NS_LOG_DEBUG ("tSpan=" << tSpan << "; tDiff=" << tDiff);

      woss::CoordZ cCurrent = woss::CoordZ::getCoordZFromCartesianCoords (m_current.position.x,
                                                                          m_current.position.y,
                                                                          m_current.position.z);

      woss::CoordZ cNext = woss::CoordZ::getCoordZFromCartesianCoords (m_next.position.x,
                                                                       m_next.position.y,
                                                                       m_next.position.z);

      NS_LOG_DEBUG ("cCurrent=" << cCurrent << "; cNext=" << cNext);

      double circleDist = cCurrent.getGreatCircleDistance (cNext);
      double depthDiff = cNext.getDepth () - cCurrent.getDepth (); // signed!
      double ratio = tDiff / tSpan;

      NS_LOG_DEBUG ("circleDist=" << circleDist << "; depthDiff=" << depthDiff << "; ratio=" << ratio);

      NS_ASSERT (ratio <= 1.0);

      woss::Coord cNew = cCurrent;
      double newDepth = cCurrent.getDepth ();

      if (circleDist > 0.0)
        {
          cNew = woss::Coord::getCoordAlongGreatCircle (cCurrent, cNext, ratio * circleDist);
        }

      if (depthDiff != 0.0)
        {
          newDepth += ratio * (depthDiff);
        }

      NS_LOG_DEBUG ("cNew=" << cNew << "; newDepth=" << newDepth);

      woss::CoordZ czNew = woss::CoordZ (cNew, newDepth);

      NS_LOG_DEBUG ("czNew=" << czNew);

      m_current.position.x = czNew.getCartX ();
      m_current.position.y = czNew.getCartY ();
      m_current.position.z = czNew.getCartZ ();
      m_current.time = now;

      NS_LOG_DEBUG ("m_current=" << m_current);
    }

  if ( newWaypoint )
    {
      NotifyCourseChange ();
    }
}


} // namespace ns3

#endif /* NS3_WOSS_SUPPORT */

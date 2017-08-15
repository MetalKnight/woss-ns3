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

#include "woss-position-allocator.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/pointer.h"

#include <cmath>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WossPositionAllocator");

NS_OBJECT_ENSURE_REGISTERED (WossListPositionAllocator);

TypeId
WossListPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WossListPositionAllocator")
    .SetParent<PositionAllocator> ()
    .AddConstructor<WossListPositionAllocator> ()
  ;
  return tid;
}

WossListPositionAllocator::WossListPositionAllocator ()
  : m_listAllocator ()
{
}

void
WossListPositionAllocator::Add (woss::CoordZ coords)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("latitude=" << coords.getLatitude () << "; longitude=" << coords.getLongitude ()
                            << "; depth=" << coords.getDepth ());

  m_listAllocator.Add (Vector (coords.getCartX (), coords.getCartY (), coords.getCartZ ()) );
}

Vector
WossListPositionAllocator::GetNext (void) const
{
  return m_listAllocator.GetNext ();
}

int64_t
WossListPositionAllocator::AssignStreams (int64_t stream)
{
  return 0;
}


NS_OBJECT_ENSURE_REGISTERED (WossGridPositionAllocator);

TypeId
WossGridPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WossGridPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Woss")
    .AddConstructor<WossGridPositionAllocator> ()
    .AddAttribute ("GridWidth", "The number of objects layed out on a line.",
                   UintegerValue (10),
                   MakeUintegerAccessor (&WossGridPositionAllocator::m_n),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MinLatitude", "The Latitude coordinate where the grid starts [dec degrees].",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&WossGridPositionAllocator::m_LatMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinLongitude", "The Longitude coordinate where the grid starts [dec degrees].",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&WossGridPositionAllocator::m_LonMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Depth", "the depth of all generated coordinates [m]",
                   DoubleValue (100.0),
                   MakeDoubleAccessor (&WossGridPositionAllocator::m_Depth),
                   MakeDoubleChecker<double>  ())
    .AddAttribute ("DeltaLatitude", "The distance in meters between two latitude-consecutive objects.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&WossGridPositionAllocator::m_DeltaLat),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DeltaLongitude", "The distance in meters between two longitude-consecutive objects.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&WossGridPositionAllocator::m_DeltaLon),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("LayoutType", "The type of layout.",
                   EnumValue (ROW_FIRST),
                   MakeEnumAccessor (&WossGridPositionAllocator::m_layoutType),
                   MakeEnumChecker (ROW_FIRST, "RowFirst",
                                    COLUMN_FIRST, "ColumnFirst"))
  ;
  return tid;
}

WossGridPositionAllocator::WossGridPositionAllocator ()
  : m_current (0)
{
}

void
WossGridPositionAllocator::SetMinLatitude (double xLat)
{
  m_LatMin = xLat;
}

void
WossGridPositionAllocator::SetMinLongitude (double lonMin)
{
  m_LonMin = lonMin;
}

void
WossGridPositionAllocator::SetDepth (double depth)
{
  m_Depth = depth;
}

void
WossGridPositionAllocator::SetDeltaLatitude (double deltaLat)
{
  m_DeltaLat = deltaLat;
}

void
WossGridPositionAllocator::SetDeltaLongitude (double deltaLon)
{
  m_DeltaLon = deltaLon;
}

void
WossGridPositionAllocator::SetN (uint32_t n)
{
  m_n = n;
}

void
WossGridPositionAllocator::SetLayoutType (enum LayoutType layoutType)
{
  m_layoutType = layoutType;
}

double
WossGridPositionAllocator::GetMinLatitude (void) const
{
  return m_LatMin;
}

double
WossGridPositionAllocator::GetMinLongitude (void) const
{
  return m_LonMin;
}

double
WossGridPositionAllocator::GetDepth (void) const
{
  return m_Depth;
}

double
WossGridPositionAllocator::GetDeltaLatitude (void) const
{
  return m_DeltaLat;
}

double
WossGridPositionAllocator::GetDeltaLongitude (void) const
{
  return m_DeltaLon;
}

uint32_t
WossGridPositionAllocator::GetN (void) const
{
  return m_n;
}

WossGridPositionAllocator::LayoutType
WossGridPositionAllocator::GetLayoutType (void) const
{
  return m_layoutType;
}

Vector
WossGridPositionAllocator::GetNext (void) const
{
  NS_LOG_FUNCTION (this);

  double lat = m_LatMin, lon = m_LonMin;
  woss::Coord startCoord, originCoord (lat, lon);

  if (m_current > 0)
    {

      switch (m_layoutType)
        {

        case ROW_FIRST:
          lat = woss::Coord::getCoordFromBearing (originCoord, 0.0, (m_DeltaLat * (m_current / m_n)), m_Depth).getLatitude ();

          startCoord = woss::Coord (lat, lon);

          NS_LOG_DEBUG ("start coordinates=" << startCoord);

          lat = woss::Coord::getCoordFromBearing (startCoord, M_PI / 2.0, (m_DeltaLat * (m_current / m_n)), m_Depth).getLatitude ();
          lon = woss::Coord::getCoordFromBearing (startCoord, M_PI / 2.0, (m_DeltaLon * (m_current % m_n)), m_Depth).getLongitude ();

          break;

        case COLUMN_FIRST:
          lon = woss::Coord::getCoordFromBearing (originCoord, M_PI / 2.0, (m_DeltaLat * (m_current / m_n)), m_Depth).getLongitude ();

          startCoord = woss::Coord (lat, lon);

          NS_LOG_DEBUG ("start coordinates=" << startCoord);

          lat = woss::Coord::getCoordFromBearing (startCoord, 0.0, (m_DeltaLat * (m_current % m_n)), m_Depth).getLatitude ();
          lon = woss::Coord::getCoordFromBearing (startCoord, 0.0, (m_DeltaLon * (m_current / m_n)), m_Depth).getLongitude ();

          break;

        }
    }

  NS_LOG_DEBUG ("Grid position allocator: m_current=" << m_current << ", lat=" << lat
                                                      << ", lon=" << lon << ", distance=" << originCoord.getGreatCircleDistance (woss::Coord (lat, lon)) );

  m_current++;

  woss::CoordZ currCoord (lat, lon, m_Depth);

  return Vector (currCoord.getCartX (), currCoord.getCartY (), currCoord.getCartZ ());
}

int64_t
WossGridPositionAllocator::AssignStreams (int64_t stream)
{
  return 0;
}

NS_OBJECT_ENSURE_REGISTERED (WossRandomRectanglePositionAllocator);

TypeId
WossRandomRectanglePositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WossRandomRectanglePositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Woss")
    .AddConstructor<WossRandomRectanglePositionAllocator> ()
    .AddAttribute ("Latitude",
                   "A random variable which represents the Latitude coordinate of a position in a random rectangle on the spherical surfaces.",
                   StringValue ("ns3::UniformRandomVariable[Min=-90.0|Max=90.0]"),
                   MakePointerAccessor (&WossRandomRectanglePositionAllocator::m_Latitude),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Longitude",
                   "A random variable which represents the Longitude coordinate of a position in a random rectangle on the spherical surface.",
                   StringValue ("ns3::UniformRandomVariable[Min=-180.0|Max=180.0]"),
                   MakePointerAccessor (&WossRandomRectanglePositionAllocator::m_Longitude),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Depth", "the depth of all generated coordinates [m]",
                   DoubleValue (100.0),
                   MakeDoubleAccessor (&WossRandomRectanglePositionAllocator::m_Depth),
                   MakeDoubleChecker<double>  ())
  ;
  return tid;
}

WossRandomRectanglePositionAllocator::WossRandomRectanglePositionAllocator ()
{
}

WossRandomRectanglePositionAllocator::~WossRandomRectanglePositionAllocator ()
{
}

void
WossRandomRectanglePositionAllocator::SetLatitude (Ptr<RandomVariableStream> lat)
{
  m_Latitude = lat;
}

void
WossRandomRectanglePositionAllocator::SetLongitude (Ptr<RandomVariableStream> lon)
{
  m_Longitude = lon;
}

void
WossRandomRectanglePositionAllocator::SetDepth (double depth)
{
  m_Depth = depth;
}

double
WossRandomRectanglePositionAllocator::GetDepth (void) const
{
  return m_Depth;
}

Vector
WossRandomRectanglePositionAllocator::GetNext (void) const
{
  NS_LOG_FUNCTION (this);

  double lat = m_Latitude->GetValue ();
  double lon = m_Longitude->GetValue ();

  NS_LOG_DEBUG ("latitude=" << lat << " [dec degrees]; longitude=" << lon << " [dec degrees]"
                            << "; depth = " << m_Depth << " [m]");

  woss::CoordZ coordz (lat, lon, m_Depth);
  return Vector (coordz.getCartX (), coordz.getCartY (), coordz.getCartZ ());
}

int64_t
WossRandomRectanglePositionAllocator::AssignStreams (int64_t stream)
{
  m_Latitude->SetStream (stream);
  m_Longitude->SetStream (stream + 1);
  return 2;
}

NS_OBJECT_ENSURE_REGISTERED (WossRandomBoxPositionAllocator);

TypeId
WossRandomBoxPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WossRandomBoxPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Woss")
    .AddConstructor<WossRandomBoxPositionAllocator> ()
    .AddAttribute ("Latitude",
                   "A random variable which represents the Latitude coordinate of a position in a random box.",
                   StringValue ("ns3::UniformRandomVariable[Min=-90.0|Max=90.0]"),
                   MakePointerAccessor (&WossRandomBoxPositionAllocator::m_Latitude),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Longitude",
                   "A random variable which represents the Longitude coordinate of a position in a random box.",
                   StringValue ("ns3::UniformRandomVariable[Min=-180.0|Max=180.0]"),
                   MakePointerAccessor (&WossRandomBoxPositionAllocator::m_Longitude),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Depth",
                   "A random variable which represents the Depth coordinate of a position in a random box. Positive values only",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=5500.0]"),
                   MakePointerAccessor (&WossRandomBoxPositionAllocator::m_Depth),
                   MakePointerChecker<RandomVariableStream> ())
  ;
  return tid;
}

WossRandomBoxPositionAllocator::WossRandomBoxPositionAllocator ()
{
}
WossRandomBoxPositionAllocator::~WossRandomBoxPositionAllocator ()
{
}

void
WossRandomBoxPositionAllocator::SetLatitude (Ptr<RandomVariableStream> lat)
{
  m_Latitude = lat;
}

void
WossRandomBoxPositionAllocator::SetLongitude (Ptr<RandomVariableStream> lon)
{
  m_Longitude = lon;
}

void
WossRandomBoxPositionAllocator::SetDepth (Ptr<RandomVariableStream> depth)
{
  m_Depth = depth;
}

Vector
WossRandomBoxPositionAllocator::GetNext (void) const
{
  NS_LOG_FUNCTION (this);

  double lat = m_Latitude->GetValue ();
  double lon = m_Longitude->GetValue ();
  double depth = m_Depth->GetValue ();

  NS_LOG_DEBUG ("latitude=" << lat << " [dec degrees]; longitude=" << lon << " [dec degrees]"
                            << "; depth=" << depth << " [m]");

  woss::CoordZ coordz (lat, lon, ::std::abs (depth));

  return Vector (coordz.getCartX (), coordz.getCartY (), coordz.getCartZ ());
}

int64_t
WossRandomBoxPositionAllocator::AssignStreams (int64_t stream)
{
  m_Latitude->SetStream (stream);
  m_Longitude->SetStream (stream + 1);
  m_Depth->SetStream (stream + 2);
  return 3;
}

NS_OBJECT_ENSURE_REGISTERED (WossRandomDiscPositionAllocator);

TypeId
WossRandomDiscPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WossRandomDiscPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Woss")
    .AddConstructor<WossRandomDiscPositionAllocator> ()
    .AddAttribute ("Bearing",
                   "A random variable which represents the bearing [dec degrees] of a position in a random disc from the center coordinates.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=360.0]"), // radiants
                   MakePointerAccessor (&WossRandomDiscPositionAllocator::m_Bearing),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Range",
                   "A random variable which represents the range of a position in a random disc from the center coordinates.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=5000.0]"),
                   MakePointerAccessor (&WossRandomDiscPositionAllocator::m_Range),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Latitude",
                   "The Latitude coordinate of the center of the random position disc.",
                   DoubleValue (42.59),
                   MakeDoubleAccessor (&WossRandomDiscPositionAllocator::m_Latitude),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Longitude",
                   "The Longitude coordinate of the center of the random position disc.",
                   DoubleValue (10.125),
                   MakeDoubleAccessor (&WossRandomDiscPositionAllocator::m_Longitude),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Depth", "the depth of all generated coordinates [m]",
                   DoubleValue (100.0),
                   MakeDoubleAccessor (&WossRandomDiscPositionAllocator::m_Depth),
                   MakeDoubleChecker<double>  ())
  ;
  return tid;
}

WossRandomDiscPositionAllocator::WossRandomDiscPositionAllocator ()
{
}

WossRandomDiscPositionAllocator::~WossRandomDiscPositionAllocator ()
{
}

void
WossRandomDiscPositionAllocator::SetBearing (Ptr<RandomVariableStream> bearing)
{
  m_Bearing = bearing;
}

void
WossRandomDiscPositionAllocator::SetRange (Ptr<RandomVariableStream> range)
{
  m_Range = range;
}

void
WossRandomDiscPositionAllocator::SetLatitude (double latitude)
{
  m_Latitude = latitude;
}

void
WossRandomDiscPositionAllocator::SetLongitude (double longitude)
{
  m_Longitude = longitude;
}

void
WossRandomDiscPositionAllocator::SetDepth (double depth)
{
  m_Depth = depth;
}

double
WossRandomDiscPositionAllocator::GetDepth (void) const
{
  return m_Depth;
}

Vector
WossRandomDiscPositionAllocator::GetNext (void) const
{
  NS_LOG_FUNCTION (this);

  double bearing = m_Bearing->GetValue ();
  double range = m_Range->GetValue ();

  NS_LOG_DEBUG ("bearing=" << bearing << " [dec degrees]; range=" << range << " [m]");

  woss::Coord start_coord (m_Latitude, m_Longitude);
  woss::CoordZ coordz (woss::Coord::getCoordFromBearing (start_coord, bearing * M_PI / 180.0, range, m_Depth), m_Depth);

  NS_LOG_DEBUG ("Disc position latitude=" << coordz.getLatitude ()
                                          << ", longitude=" << coordz.getLongitude () << "; depth=" << coordz.getDepth () );

  return Vector ( coordz.getCartX (), coordz.getCartY (), coordz.getCartZ ());
}

int64_t
WossRandomDiscPositionAllocator::AssignStreams (int64_t stream)
{
  m_Bearing->SetStream (stream);
  m_Range->SetStream (stream + 1);
  return 2;
}

NS_OBJECT_ENSURE_REGISTERED (WossUniformDiscPositionAllocator);

TypeId
WossUniformDiscPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WossUniformDiscPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Woss")
    .AddConstructor<WossUniformDiscPositionAllocator> ()
    .AddAttribute ("Range",
                   "The max range of the disc",
                   DoubleValue (5000.0),
                   MakeDoubleAccessor (&WossUniformDiscPositionAllocator::m_Range),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Latitude",
                   "The Latitude coordinate of the center of the  disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&WossUniformDiscPositionAllocator::m_Latitude),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Longitude",
                   "The Longitude coordinate of the center of the  disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&WossUniformDiscPositionAllocator::m_Longitude),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Depth", "the depth of all generated coordinates [m]",
                   DoubleValue (100.0),
                   MakeDoubleAccessor (&WossUniformDiscPositionAllocator::m_Depth),
                   MakeDoubleChecker<double>  ())
  ;
  return tid;
}

WossUniformDiscPositionAllocator::WossUniformDiscPositionAllocator ()
{
  m_BearVar = CreateObject<UniformRandomVariable> ();
  m_RangeVar = CreateObject<UniformRandomVariable> ();
}

WossUniformDiscPositionAllocator::~WossUniformDiscPositionAllocator ()
{
}

void
WossUniformDiscPositionAllocator::SetRange (double range)
{
  m_Range = range;
}

void
WossUniformDiscPositionAllocator::SetLatitude (double latitude)
{
  m_Latitude = latitude;
}

void
WossUniformDiscPositionAllocator::SetLongitude (double longitude)
{
  m_Longitude = longitude;
}

void
WossUniformDiscPositionAllocator::SetDepth (double depth)
{
  m_Depth = depth;
}

double
WossUniformDiscPositionAllocator::GetDepth (void) const
{
  return m_Depth;
}

Vector
WossUniformDiscPositionAllocator::GetNext (void) const
{
  NS_LOG_FUNCTION (this);

  woss::Coord startCoord (m_Latitude, m_Longitude);

  double bearing = (m_BearVar->GetValue (0.0, 360.0)) * M_PI / 180.0;
  double range = m_RangeVar->GetValue ( 0.0, ::std::sqrt (m_Range * m_Range));

  NS_LOG_DEBUG ("generated bearing=" << bearing * 180.0 / M_PI << " [dec degrees]"
                                     << "; gen range=" << range << " [m]");

  woss::CoordZ coordz (woss::Coord::getCoordFromBearing (startCoord, bearing, range, m_Depth), m_Depth);

  NS_LOG_DEBUG ("Disc position latitude=" << coordz.getLatitude ()
                                          << ", longitude=" << coordz.getLongitude () << "; depth=" << coordz.getDepth () );

  return Vector (coordz.getCartX (), coordz.getCartY (), coordz.getCartZ ());
}


int64_t
WossUniformDiscPositionAllocator::AssignStreams (int64_t stream)
{
  m_BearVar->SetStream (stream);
  m_RangeVar->SetStream (stream + 1);
  return 2;
}

} // namespace ns3

#endif /* NS3_WOSS_SUPPORT */

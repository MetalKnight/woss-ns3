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

#ifndef WOSS_POSITION_ALLOCATOR_H
#define WOSS_POSITION_ALLOCATOR_H

#include <coordinates-definitions.h>
#include "ns3/position-allocator.h"
#include "ns3/random-variable-stream.h"


namespace ns3 {


/**
 * \ingroup WOSS
 * \brief Allocates positions from a list of geographic coordinates
 *
 * WossListPositionAllocator allocates its Vector from the given list of woss::CoordZ geographic coordinates
 */
class WossListPositionAllocator : public PositionAllocator
{
public:
  /**
   * Register this type.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  WossListPositionAllocator (); //!< Default constructor

  /**
   * Add new geographic coordinates to the list
   *
   * \param coords position in geographic coordinates (decimal degrees)
   */
  void Add (woss::CoordZ coords);

  /**
   * \returns the Vector position in cartesian coordinates
   */
  virtual Vector GetNext (void) const;

  virtual int64_t AssignStreams (int64_t stream);

protected:
  ListPositionAllocator m_listAllocator; //!< the list allocator
};

/**
 * \ingroup uan
 *
 * \brief WossGridPositionAllocator allocates positions on a rectangular 2d grid
 * with constant depth. The grid is measured in geographic coordinates
 */
class WossGridPositionAllocator : public PositionAllocator
{
public:
  /**
   * grid layout type
   */
  enum LayoutType
  {

    ROW_FIRST, //!< row allocation first

    COLUMN_FIRST //!< column allocation first
  };

  /**
   * Register this type.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  WossGridPositionAllocator (); //!< Default constructor

  /**
   * Sets the minimum latitude of the grid in decimal degrees
   *
   * \param lat the minimum latitude in decimal degrees
   */
  void SetMinLatitude (double lat);

  /**
   * Sets the minimum longitude of the grid in decimal degrees
   *
   * \param lon the minimum longitude in decimal degrees
   */
  void SetMinLongitude (double lon);

  /**
   * Sets the common depth of the grid in meters.
   *
   * \param depth the grid depth in meters (positive values)
   */
  void SetDepth (double depth);

  /**
   * \param deltaLat the distance between two latitude-consecutive positions [m]
   */
  void SetDeltaLatitude (double deltaLat);
  /**
   * \param deltaLon the distance between two longitude-consecutive positions [m]
   */
  void SetDeltaLongitude (double deltaLon);

  /**
   * Sets the number of nodes per column or row
   * \param n the number of nodes
   */
  void SetN (uint32_t n);

  /**
   * Sets the grid layout type
   * \param layoutType the layout type
   */
  void SetLayoutType (enum LayoutType layoutType);

  /**
   * \returns the minimum latitude in decimal degrees
   */
  double GetMinLatitude (void) const;

  /**
   * \returns the minimum longitude in decimal degrees
   */
  double GetMinLongitude (void) const;

  /**
   * \returns the distance between two latitude-consecutive positions [m]
   */
  double GetDeltaLatitude (void) const;

  /**
   * \returns the distance between two longitude-consecutive positions [m]
   */
  double GetDeltaLongitude (void) const;

  /**
   * \returns grid common depth in meters
   */
  double GetDepth (void) const;

  /**
   * \returns the current number of nodes per row or column
   */
  uint32_t GetN (void) const;
  /**
   * \returns the currently-selected layout type.
   */
  enum LayoutType GetLayoutType (void) const;

  /**
   * \returns the next position in cartesian coordinates
   */
  virtual Vector GetNext (void) const;

  virtual int64_t AssignStreams (int64_t stream);

protected:
  mutable uint32_t m_current; //!<
  enum LayoutType m_layoutType; //!< layout type
  double m_LatMin; //!< minimum latitude in decimal degrees
  double m_LonMin; //!< minimum longitude in decimal degrees
  double m_Depth; //!< common depth in meters
  uint32_t m_n; //!< number of nodes per row / column
  double m_DeltaLat; //!< distance between two consecutive latitudes in meters
  double m_DeltaLon; //!< distance between two consecutive longitudes in meters
};

/**
 * \ingroup uan
 *
 * \brief WossRandomRectanglePositionAllocator allocates random positions in a 2D rectangle with constant depth
 * defined in geographic coordinates
 */
class WossRandomRectanglePositionAllocator : public PositionAllocator
{
public:
  /**
   * Register this type.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  WossRandomRectanglePositionAllocator (); //!< Default constructor

  virtual ~WossRandomRectanglePositionAllocator (); //!< Default destructor

  /**
   * \param lat the random variable must return valid latitude values in decimal degrees
   */
  void SetLatitude (Ptr<RandomVariableStream> lat);

  /**
   * \param lon the random variable must return valid longitude values in decimal degrees
   */
  void SetLongitude (Ptr<RandomVariableStream> lon);

  /**
   * \param depth the grid depth in meters (positive values)
   */
  void SetDepth (double depth);

  /**
   * \returns the grid depth in meters
   */
  double GetDepth (void) const;

  /**
   * \returns the position in cartesian coordinates
   */
  virtual Vector GetNext (void) const;
  virtual int64_t AssignStreams (int64_t stream);

protected:
  double m_Depth; //!< grid depth in meters
  Ptr<RandomVariableStream> m_Latitude; //!< random variable that generates the latitude in decimal degrees
  Ptr<RandomVariableStream> m_Longitude; //!< random variable that generates the longitude in decimal degrees
};


/**
 * \ingroup uan
 *
 * \brief WossRandomBoxPositionAllocator allocates random positions in a 3D box defined in geographic coordinates
 */
class WossRandomBoxPositionAllocator : public PositionAllocator
{
public:
  /**
   * Register this type.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  WossRandomBoxPositionAllocator (); //!< Default constructor

  virtual ~WossRandomBoxPositionAllocator (); //!< Default destructor

  /**
   * \param latitude the random variable must return valid latitude values in decimal degrees
   */
  void SetLatitude (Ptr<RandomVariableStream> latitude);

  /**
   * \param longitude the random variable must return valid longitude values in decimal degrees
   */
  void SetLongitude (Ptr<RandomVariableStream> longitude);

  /**
   * \param depth the grid depth in meters (positive values)
   */
  void SetDepth (Ptr<RandomVariableStream> depth);

  /**
   * \returns the position in cartesian coordinates
   */
  virtual Vector GetNext (void) const;
  virtual int64_t AssignStreams (int64_t stream);


protected:
  Ptr<RandomVariableStream> m_Latitude; //!< random variable that generates the latitude in decimal degrees
  Ptr<RandomVariableStream> m_Longitude; //!< random variable that generates the latitude in decimal degrees
  Ptr<RandomVariableStream> m_Depth; //!< random variable that generates the latitude in meters
};

/**
 * \ingroup uan
 *
 * \brief WossRandomDiscPositionAllocator allocates random positions on a 2D disc (with constant depth) defined by geographic coordinates
 */
class WossRandomDiscPositionAllocator : public PositionAllocator
{
public:
  /**
   * Register this type.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  WossRandomDiscPositionAllocator (); //!< Default constructor

  virtual ~WossRandomDiscPositionAllocator (); //!< Default destructor

  /**
   * \param bearing the random variable must return valid bearing values in decimal degrees [0.0-360.0]
   */
  void SetBearing (Ptr<RandomVariableStream> bearing);

  /**
   * \param range the random variable must return a positive range value in meters
   */
  void SetRange (Ptr<RandomVariableStream> range);

  /**
   * \param lat the disc center's latitude in decimal degrees
   */
  void SetLatitude (double lat);

  /**
   * \param lon the disc center's longitude in decimal degrees
   */
  void SetLongitude (double lon);

  /**
   * \param depth the disc depth in meters (positive values)
   */
  void SetDepth (double depth);

  /**
   * \returns the disc center's latitude in decimal degrees
   */
  double GetLatitude (void) const;

  /**
   * \returns the disc center's longitude in decimal degrees
   */
  double GetLongitude (void) const;

  /**
   * \returns the disc center's depth in meters
   */
  double GetDepth (void) const;

  /**
   * \returns the position in cartesian coordinates
   */
  virtual Vector GetNext (void) const;

  virtual int64_t AssignStreams (int64_t stream);

protected:
  Ptr<RandomVariableStream> m_Bearing; //!< random variable that generates a random bearer in [0.0 - 360.0]
  Ptr<RandomVariableStream> m_Range; //!< random variable that generates a random range in meters
  double m_Latitude; //!< disc center's latitude in decimal degrees
  double m_Longitude; //!< disc center's longitude in decimal degrees
  double m_Depth; //!< disc center's depth in meters
};

/**
 * \ingroup uan
 *
 * \brief WossUniformDiscPositionAllocator allocates random positions on a 2D disc (with constant depth) defined by geographic coordinates
 */
class WossUniformDiscPositionAllocator : public PositionAllocator
{
public:
  /**
   * Register this type.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  WossUniformDiscPositionAllocator (); //!< Default constructor

  virtual ~WossUniformDiscPositionAllocator (); //!< Default destructor


  /**
   * \param range positive range values in meters
   */
  void SetRange (double range);

  /**
  * \param lat the disc center's latitude in decimal degrees
  */
  void SetLatitude (double lat);

  /**
   * \param lon the disc center's longitude in decimal degrees
   */
  void SetLongitude (double lon);

  /**
   * \param depth the constant depth in menters
   */
  void SetDepth (double depth);

  double GetDepth (void) const;

  /**
   * \returns the position in cartesian coordinates
   */
  virtual Vector GetNext (void) const;

  virtual int64_t AssignStreams (int64_t stream);

private:
  double m_Range; //!< range in meters
  double m_Latitude; //!< latitude in decimal degrees
  double m_Longitude; //!< longitude in decimal degrees
  double m_Depth; //!< depth in meters

  Ptr<UniformRandomVariable> m_BearVar; //!< random variable that generates bearing in [0.0 - 360.0]
  Ptr<UniformRandomVariable> m_RangeVar;  //!< random variable that generates a range in meters
};

}

#endif /* WOSS_POSITION_ALLOCATOR_H */

#endif /* NS3_WOSS_SUPPORT */

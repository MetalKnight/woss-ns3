/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015
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

#ifndef WOSS_ALOHA_EXAMPLE_H
#define WOSS_ALOHA_EXAMPLE_H

#include "ns3/network-module.h"
#include "ns3/stats-module.h"
#include "ns3/uan-module.h"
#include "ns3/woss-helper.h"

using namespace ns3;

/**
 * \ingroup WOSS
 *
 * Container for the parameters describing a single experiment.
 *
 * An experiment samples a range of parameter values.  The parameters
 * controls the number of nodes, the position allocator, etc...
 *
 * The default example creates a small node topology made of a receiving sink and m_numNodes=two transmitting nodes.
 *
 * The sink is placed at a latitude of 42.59 dec degrees, longitude of 10.125 dec degrees and a depth of 70m.
 * The m_numNodes are allocated as the model selected by the m_posAllocSelector variable.
 * The default model is the list allocator, which will place its node over a straight line
 * (same latitude, 90 degrees bearing) with a inter-distance of 500m.
 * All nodes rae configured with the Aloha MAC and the Q-PSK as PHY modulation.
 * The WOSS library will create:
 *  -# an output result database in binary formact, \see Experiment::InitWossHelper ResDbFilePath attribute
 *  -# a temporary directory with the original bellhop output files \see Experiment::InitWossHelper WossWorkDirPath attribute
 */
class Experiment
{
public:
  uint32_t m_posAllocSelector; //!< Allows the selection between all WOSS-related position allocators.
  uint32_t m_totalRate; //!< Total channel capacity.
  uint32_t m_maxRange; //!< Maximum range in meters.
  uint32_t m_numNodes; //!< Number of simulated nodes.
  uint32_t m_pktSize; //!< Packet size in bytes.
  Time m_simTime;   //!< Simulation time per trial

  std::string m_databasePath; //!< The path to the WOSS databases.
  bool m_useMultithread; //!< Enable/disable WOSS multithread feature.
  bool m_useThreadPool; //!< Enable/disable WOSS multithread thread pool feature.
  bool m_useTimeEvolution; //!< Enable/disable the WOSS time evolution feature.
  uint32_t m_bytesTotal; //!< Total number of bytes received in a simulation run.
  UanTxMode m_dataMode; //!< List of UanTxModes used for data channels.

  /**
  * Callback to receive a packet.
  *
  * \param socket The socket receiving packets.
  */
  void ReceivePacket (Ptr<Socket> socket);

  /**
   * Run a parametrized experiment.
   *
   * The parameter sets either the number of nodes (if m_doNode is true)
   * or the \"a\" parameter, which controls the UanMacRcGw MaxReservations
   * attribute.
   *
   * \param param The parameter value.
   * \return The total number of bytes delivered.
   */
  uint32_t Run (uint32_t param);

  /**
   * Init function for the WossHelper object
   */
  void InitWossHelper (Ptr<WossHelper> wossHelper, Ptr<WossPropModel> wossProp, woss::CoordZ &txCoordZ);

  /** Default constructor. */
  Experiment ();

};

#endif /* WOSS_ALOHA_EXAMPLE_H */

#endif /* NS3_WOSS_SUPPORT */

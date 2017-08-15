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
/**
 * \file woss-aloha-example.cc
 * \ingroup WOSS
 *
 *
 */

#ifndef NS3_WOSS_SUPPORT
int
main (int argc, char *argv[])
{
  return 0;
}
#else

#include "woss-aloha-example.h"
#include "ns3/woss-channel.h"
#include "ns3/woss-waypoint-mobility-model.h"
#include "ns3/woss-position-allocator.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/callback.h"
#include "ns3/stats-module.h"


#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WossAlohaExample");


Experiment::Experiment ()
  :   m_posAllocSelector (0),
    m_totalRate (4096),
    m_maxRange (3000),
    m_numNodes (2),
    m_pktSize (1000),
    m_simTime (Seconds (5000)),
    m_databasePath (""),
    m_useMultithread (true),
    m_useTimeEvolution (false),
    m_bytesTotal (0),
    m_dataMode ()
{
}

void
Experiment::InitWossHelper (Ptr<WossHelper> wossHelper, Ptr<WossPropModel> wossProp, woss::CoordZ &txCoordZ)
{
  wossHelper->SetAttribute ("ResDbUseBinary", BooleanValue (false));
  wossHelper->SetAttribute ("ResDbUseTimeArr", BooleanValue (true));
  wossHelper->SetAttribute ("ResDbFilePath", StringValue ("./woss-aloha-example-output/res-db/"));
  wossHelper->SetAttribute ("ResDbFileName", StringValue ("woss-aloha-example-results.dat"));
  if (m_databasePath != "")
    {
      wossHelper->SetAttribute ("SedimDbCoordFilePath", StringValue (m_databasePath + "/sea_floor/DECK41_coordinates.nc"));
      wossHelper->SetAttribute ("SedimDbMarsdenFilePath", StringValue (m_databasePath + "/sea_floor/DECK41_mardsen_square.nc"));
      wossHelper->SetAttribute ("SedimDbMarsdenOneFilePath", StringValue (m_databasePath + "/sea_floor/DECK41_mardsen_one_degree.nc"));
      wossHelper->SetAttribute ("SspDbCoordFilePath", StringValue (m_databasePath + "/ssp/standard_depth/2WOA2009_SSP_April.nc"));
      wossHelper->SetAttribute ("BathyDbCoordFilePath", StringValue (m_databasePath + "/bathymetry/GEBCO_2014_1D.nc"));
    }
  wossHelper->SetAttribute ("WossCleanWorkDir", BooleanValue (false));
  wossHelper->SetAttribute ("WossWorkDirPath", StringValue ("./woss-aloha-example-output/work-dir/"));
  wossHelper->SetAttribute ("WossSimTime", StringValue ("1|10|2012|0|1|1|1|10|2012|0|1|1"));
  wossHelper->SetAttribute ("WossManagerTimeEvoActive", BooleanValue (m_useTimeEvolution));
  wossHelper->SetAttribute ("WossManagerTotalThreads", IntegerValue (4));
  wossHelper->SetAttribute ("WossManagerUseMultithread", BooleanValue (m_useMultithread));

  wossHelper->Initialize (wossProp);

  if (m_databasePath == "")
    {
      wossHelper->SetCustomBathymetry ("5|0.0|100.0|100.0|200.0|300.0|150.0|400.0|100.0|700.0|300.0", txCoordZ);
      wossHelper->SetCustomSediment ("TestSediment|1560.0|200.0|1.5|0.9|0.8|300.0");
      wossHelper->SetCustomSsp ("12|0|1508.42|10|1508.02|20|1507.71|30|1507.53|50|1507.03|75|1507.56|100|1508.08|125|1508.49|150|1508.91|200|1509.75|250|1510.58|300|1511.42");
    }

}

void
Experiment::ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  while ((packet = socket->Recv ()))
    {
      NS_LOG_DEBUG ("Received a packet of size " << packet->GetSize () << " bytes");
      m_bytesTotal += packet->GetSize ();
    }
}



uint32_t
Experiment::Run (uint32_t param)
{
  UanHelper uan;


  uint32_t depth = 70;
  double sinkLatitude = 42.59;
  double sinkLongitude = 10.125;

  woss::CoordZ sinkCoord = woss::CoordZ (sinkLatitude, sinkLongitude, depth);

  m_bytesTotal = 0;

  uint32_t nNodes = param;

  Ptr<WossHelper> wossHelper = CreateObject<WossHelper> ();

//   Ptr<UanPhyCalcSinrFhFsk> sinr = CreateObject<UanPhyCalcSinrFhFsk> ();
  Ptr<UanPhyCalcSinrDefault> sinr = CreateObject<UanPhyCalcSinrDefault> ();

  UanTxMode mode;
//   mode = UanTxModeFactory::CreateMode (UanTxMode::FSK, 80, 80, 22000, 4000, 13, "FSK");
  mode = UanTxModeFactory::CreateMode (UanTxMode::PSK, 200, 200, 22000, 4000, 4, "QPSK");

  UanModesList myModes;
  myModes.AppendMode (mode);

  uan.SetPhy ("ns3::UanPhyGen",
              "SinrModel", PointerValue (sinr),
              "SupportedModes", UanModesListValue (myModes));

  uan.SetMac ("ns3::UanMacAloha");

  Ptr<WossPropModel> wossProp = CreateObjectWithAttributes<WossPropModel> ();
  InitWossHelper (wossHelper, wossProp, sinkCoord);

  Ptr<WossChannel> wossChannel = CreateObjectWithAttributes<WossChannel> ("PropagationModel", PointerValue (wossProp));
  wossChannel->SetAttribute ("ChannelEqSnrThresholdDb", DoubleValue (-100.0));

  NodeContainer sink;
  sink.Create (1);
  NetDeviceContainer sinkDev = uan.Install (sink, wossChannel);

  NodeContainer nodes;
  nodes.Create (nNodes);
  NetDeviceContainer devices = uan.Install (nodes, wossChannel);

  MobilityHelper mobilitySink, mobility;

  Ptr<PositionAllocator> pos;

  Ptr<WossListPositionAllocator> posSink = CreateObject<WossListPositionAllocator> ();
  posSink->Add (sinkCoord);

  if (m_posAllocSelector == 0)
    {
      pos = CreateObject<WossListPositionAllocator> ();
      Ptr<WossListPositionAllocator> posCast = DynamicCast<WossListPositionAllocator> (pos);

      for (uint32_t i = 0; i < nNodes; i++)
        {
          woss::CoordZ nodeCoord = woss::CoordZ (woss::Coord::getCoordFromBearing (sinkCoord, M_PI / 2.0, (i + 1) * 500.0), depth);
          posCast->Add (nodeCoord);
        }
    }
  else if (m_posAllocSelector == 1)
    {
      pos = CreateObject<WossGridPositionAllocator> ();

      pos->SetAttribute ("MinLatitude", DoubleValue (sinkLatitude));
      pos->SetAttribute ("MinLongitude", DoubleValue (sinkLongitude));
      pos->SetAttribute ("Depth", DoubleValue (depth));
      pos->SetAttribute ("DeltaLatitude", DoubleValue (500.0));
      pos->SetAttribute ("DeltaLongitude", DoubleValue (500.0));
      pos->SetAttribute ("LayoutType", EnumValue (WossGridPositionAllocator::COLUMN_FIRST));
      pos->SetAttribute ("GridWidth", UintegerValue (2));
    }
  else if (m_posAllocSelector == 2)
    {
      pos = CreateObject<WossRandomRectanglePositionAllocator> ();

      pos->SetAttribute ("Latitude", StringValue ("ns3::UniformRandomVariable[Min=42.59|Max=42.6]"));
      pos->SetAttribute ("Longitude", StringValue ("ns3::UniformRandomVariable[Min=10.125|Max=10.127]"));
      pos->SetAttribute ("Depth", DoubleValue (depth));
    }
  else if (m_posAllocSelector == 3)
    {
      pos = CreateObject<WossRandomDiscPositionAllocator> ();

      pos->SetAttribute ("Latitude", DoubleValue (sinkLatitude));
      pos->SetAttribute ("Longitude", DoubleValue (sinkLongitude));
      pos->SetAttribute ("Depth", DoubleValue (depth));
    }
  else if (m_posAllocSelector == 4)
    {
      Ptr<WossUniformDiscPositionAllocator> pos = CreateObject<WossUniformDiscPositionAllocator> ();

      pos->SetAttribute ("Latitude", DoubleValue (sinkLatitude));
      pos->SetAttribute ("Longitude", DoubleValue (sinkLongitude));
      pos->SetAttribute ("Depth", DoubleValue (depth));
    }
  else
    {
      NS_FATAL_ERROR ("m_posAllocSelector:" << m_posAllocSelector << " > 4 provided!");
    }

  mobilitySink.SetPositionAllocator (posSink);
  mobility.SetPositionAllocator (pos);

  mobilitySink.SetMobilityModel ("ns3::WossWaypointMobilityModel", "InitialPositionIsWaypoint", BooleanValue (true));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobilitySink.Install (sink);
  mobility.Install (nodes);

  Ptr<Node> nodeSink = sink.Get (0);
  Ptr<WossWaypointMobilityModel> sinkMob = nodeSink->GetObject<WossWaypointMobilityModel> ();
  NS_ASSERT (sinkMob != NULL);

  for (int cnt = 0; cnt < 5; ++cnt)
    {
      Waypoint wp ( Seconds ((cnt + 1.0) * 10.0), CreateVectorFromCoords (42.59, 10.125 + (cnt + 1.0) * 0.05, depth));
      sinkMob->AddWaypoint (wp);
    }

  PacketSocketHelper pktskth;
  pktskth.Install (nodes);
  pktskth.Install (sink);

  PacketSocketAddress socket;
  socket.SetSingleDevice (sinkDev.Get (0)->GetIfIndex ());
  socket.SetPhysicalAddress (sinkDev.Get (0)->GetAddress ());
  socket.SetProtocol (0);

  OnOffHelper app ("ns3::PacketSocketFactory", Address (socket));
  app.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  app.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  app.SetAttribute ("DataRate", DataRateValue (m_totalRate));
  app.SetAttribute ("PacketSize", UintegerValue (m_pktSize));

  ApplicationContainer apps = app.Install (nodes);

  apps.Start (Seconds (0.5));
  apps.Stop (m_simTime + Seconds (0.5));

  Ptr<Node> sinkNode = sink.Get (0);
  TypeId psfid = TypeId::LookupByName ("ns3::PacketSocketFactory");

  Ptr<Socket> sinkSocket = Socket::CreateSocket (sinkNode, psfid);
  sinkSocket->Bind (socket);
  sinkSocket->SetRecvCallback (MakeCallback (&Experiment::ReceivePacket, this));

  Simulator::Stop (m_simTime + Seconds (0.6));
  Simulator::Run ();
  Simulator::Destroy ();

  return m_bytesTotal;
}
int
main (int argc, char *argv[])
{

  LogComponentEnable ("WossAlohaExample", LOG_LEVEL_ALL);
//  LogComponentEnable ("WossHelper", LOG_LEVEL_ALL);
//  LogComponentEnable ("WossChannel", LOG_LEVEL_ALL);
//  LogComponentEnable ("WossPropModel", LOG_LEVEL_ALL);
//  LogComponentEnable ("UanChannel", LOG_LEVEL_ALL);
//  LogComponentEnable ("UanTransducerHd", LOG_LEVEL_ALL);
//  LogComponentEnable ("UanPhyGen", LOG_LEVEL_ALL);
//  LogComponentEnable ("UanPropModelThorp", LOG_LEVEL_ALL);
//  LogComponentEnable ("WossPositionAllocator", LOG_LEVEL_ALL);
//  LogComponentEnable ("WossWaypointMobilityModel", LOG_LEVEL_ALL);

  Experiment exp;

  CommandLine cmd;
  cmd.AddValue ("TotalRate", "Total channel capacity", exp.m_totalRate);
  cmd.AddValue ("PosAllocSelector", "Position Allocator Selector: 0 list, 1 grid, 2 random rectangle, 3 random disc, 4 uniform disc", exp.m_posAllocSelector);
  cmd.AddValue ("UseMultithread", "flag to set the WOSS multithread option", exp.m_useMultithread);
  cmd.AddValue ("UseTimeEvolution", "flag to set the WOSS time evolution option", exp.m_useTimeEvolution);
  cmd.AddValue ("NumberNodes", "Number of nodes", exp.m_numNodes);
  cmd.AddValue ("PktSize", "Packet size in bytes", exp.m_pktSize);
  cmd.AddValue ("SimTime", "Simulation time per trial", exp.m_simTime);
  cmd.Parse (argc, argv);

  uint32_t bytesRx = exp.Run (exp.m_numNodes);

  NS_LOG_DEBUG ("Received a total of " << bytesRx << " bytes at sink");

}

#endif // NS3_WOSS_SUPPORT


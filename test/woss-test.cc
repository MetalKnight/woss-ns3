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
 * Authors: Federico Guerra <federico@guerra-tlc.com>, Randall Plate <rplate@spawar.navy.mil>
 */

#ifdef NS3_WOSS_SUPPORT

#include "ns3/core-module.h"
#include "ns3/uan-module.h"
#include "ns3/woss-channel.h"
#include "ns3/uan-mac-aloha.h"
#include "ns3/uan-phy-gen.h"
#include "ns3/uan-transducer-hd.h"
#include "ns3/uan-prop-model-ideal.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"
#include "ns3/pointer.h"
#include "ns3/callback.h"
#include "ns3/woss-helper.h"
#include "ns3/woss-position-allocator.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/string.h"

using namespace ns3;

/**
 * \ingroup woss
 *
 * WOSS test
 *
 * The class test perform the following:
 * -# a Vector to woss::CoordZ conversion check. the conversion should be inside tolerance
 * -# it creates two nodes at 1000m distance and then it checks that the WOSS computed attenuation of both channels is within tolerance
 * -# it then performs PHY tests by:
 * -# testing a PSK no-collision scenario
 * -# testing a PSK collision scenario
 * -# Propagation Tests: verify WOSS attenuation matches Thorp for deep water column case
 */
class WossTest : public TestCase
{
public:
  WossTest ();

  virtual void DoRun (void);
private:
  Ptr<UanNetDevice> CreateNode (Vector pos, Ptr<UanChannel> chan);

  void InitWossHelper (Ptr<WossPropModel> wossProp);

  bool DoPhyTests (Ptr<WossPropModel> prop);
  uint32_t DoOnePhyTest (Time t1, Time t2, uint32_t r1, uint32_t r2, Ptr<WossPropModel> prop, uint32_t mode1 = 0, uint32_t mode2 = 0);
  bool RxPacket (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender);
  void SendOnePacket (Ptr<UanNetDevice> dev, uint32_t mode);
  void WossThorpRun(Ptr<UanChannel> chan);
  void DoWossThorpTests(Ptr<WossPropModel> prop, double ranges[], double windNoise, double shipNoise);
  void PktRxOkThorpCallback (Ptr< Packet > pkt, double sinr, UanTxMode mode);
  void PktRxOkWossCallback (Ptr< Packet > pkt, double sinr, UanTxMode mode);

  ObjectFactory m_phyFac;
  uint32_t m_bytesRx;
  std::vector <double> m_thorpSinr;
  std::vector <double> m_wossSinr;
  Ptr<WossHelper> m_wossHelper;
  ::std::string m_databasePath;
  bool m_useTimeEvolution;
  bool m_useMultithread;
  woss::CoordZ m_sinkCoord;

};


WossTest::WossTest ()
  : TestCase ("WOSS"),
    m_databasePath (""),
    m_useTimeEvolution (false),
    m_useMultithread (false),
    m_sinkCoord (42.59, 10.125, 70.0)
{
  //m_databasePath = "/home/fedwar/ns/ocean_database/dbs";
}

void
WossTest::InitWossHelper (Ptr<WossPropModel> wossProp)
{
  m_wossHelper->SetAttribute ("ResDbUseBinary", BooleanValue (false));
  m_wossHelper->SetAttribute ("ResDbUseTimeArr", BooleanValue (true));
  m_wossHelper->SetAttribute ("ResDbFilePath", StringValue ("./woss-test-output/res-db/"));
  m_wossHelper->SetAttribute ("WossBellhopBathyType", StringValue("L"));
  m_wossHelper->SetAttribute ("WossBathyWriteMethod", StringValue("D"));

  if (m_databasePath != "")
    {
#if defined (WOSS_NETCDF_SUPPORT)
      m_wossHelper->SetAttribute ("SedimDbCoordFilePath", StringValue (m_databasePath + "/sea_floor/DECK41_coordinates.nc"));
      m_wossHelper->SetAttribute ("SedimDbMarsdenFilePath", StringValue (m_databasePath + "/sea_floor/DECK41_mardsen_square.nc"));
      m_wossHelper->SetAttribute ("SedimDbMarsdenOneFilePath", StringValue (m_databasePath + "/sea_floor/DECK41_mardsen_one_degree.nc"));
      m_wossHelper->SetAttribute ("SspDbCoordFilePath", StringValue (m_databasePath + "/ssp/standard_depth/2WOA2009_SSP_April.nc"));
#if defined (WOSS_NETCDF4_SUPPORT)
      m_wossHelper->SetAttribute ("BathyDbGebcoFormat", IntegerValue (4)); // 15 seconds, 2D netcdf format
      m_wossHelper->SetAttribute ("BathyDbCoordFilePath", StringValue (m_databasePath + "/bathymetry/GEBCO_2019.nc"));
#else
      m_wossHelper->SetAttribute ("BathyDbGebcoFormat", IntegerValue (3)); // 30 seconds, 2D netcdf format
      m_wossHelper->SetAttribute ("BathyDbCoordFilePath", StringValue (m_databasePath + "/bathymetry/GEBCO_2014_2D.nc"));
#endif // defined (WOSS_NETCDF4_SUPPORT)
#endif // defined (WOSS_NETCDF_SUPPORT)
    }

  m_wossHelper->SetAttribute ("WossCleanWorkDir", BooleanValue (false));
  m_wossHelper->SetAttribute ("WossWorkDirPath", StringValue ("./woss-test-output/work-dir/"));
  m_wossHelper->SetAttribute ("WossSimTime", StringValue ("1|10|2012|0|1|1|1|10|2012|0|1|1"));
  m_wossHelper->SetAttribute ("WossManagerTimeEvoActive", BooleanValue (m_useTimeEvolution));
  m_wossHelper->SetAttribute ("WossManagerTotalThreads", IntegerValue (4));
  m_wossHelper->SetAttribute ("WossManagerUseMultithread", BooleanValue (m_useMultithread));

  m_wossHelper->Initialize (wossProp);

  if (m_databasePath == "")
    {
      m_wossHelper->SetCustomBathymetry ("5|0.0|100.0|100.0|202.0|300.0|150.0|400.0|100.0|700.0|303.0", m_sinkCoord);
      m_wossHelper->SetCustomSediment ("TestSediment|1560.0|200.0|1.5|0.9|0.8|300.0");
      m_wossHelper->SetCustomSsp ("12|0|1508.42|10|1508.02|20|1507.71|30|1507.53|50|1507.03|75|1507.56|100|1508.08|125|1508.49|150|1508.91|200|1509.75|250|1510.58|300|1511.42");
    }

}

bool
WossTest::RxPacket (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender)
{
  m_bytesRx += pkt->GetSize ();
  return true;
}
void
WossTest::SendOnePacket (Ptr<UanNetDevice> dev, uint32_t mode)
{
  Ptr<Packet> pkt = Create<Packet> (17);
  dev->Send (pkt, dev->GetBroadcast (), mode);

}

Ptr<UanNetDevice>
WossTest::CreateNode (Vector pos, Ptr<UanChannel> chan)
{


  Ptr<UanPhy> phy = m_phyFac.Create<UanPhy> ();
  phy->SetTxPowerDb(150);
  phy->SetRxThresholdDb(0);
  Ptr<Node> node = CreateObject<Node> ();
  Ptr<UanNetDevice> dev = CreateObject<UanNetDevice> ();
  Ptr<UanMacAloha> mac = CreateObject<UanMacAloha> ();
  Ptr<ConstantPositionMobilityModel> mobility = CreateObject<ConstantPositionMobilityModel> ();

  Ptr<UanTransducerHd> trans = CreateObject<UanTransducerHd> ();

  mobility->SetPosition (pos);
  node->AggregateObject (mobility);
  mac->SetAddress (Mac8Address::Allocate ());

  dev->SetPhy (phy);
  dev->SetMac (mac);
  dev->SetChannel (chan);
  dev->SetTransducer (trans);
  node->AddDevice (dev);

  return dev;
}


uint32_t
WossTest::DoOnePhyTest (Time txTime1,
                        Time txTime2,
                        uint32_t r1,
                        uint32_t r2,
                        Ptr<WossPropModel> prop,
                        uint32_t mode1,
                        uint32_t mode2)
{

  Ptr<WossChannel> channel = CreateObject<WossChannel> ();
  channel->SetAttribute ("PropagationModel", PointerValue (prop));
  channel->SetAttribute ("ChannelEqSnrThresholdDb", DoubleValue (-100.0));



  Ptr<UanNetDevice> dev0 = CreateNode (CreateVectorFromCoordZ (woss::CoordZ (woss::Coord::getCoordFromBearing (m_sinkCoord, M_PI / 2.0, r1), 70.0)), channel);
  Ptr<UanNetDevice> dev1 = CreateNode (CreateVectorFromCoordZ (m_sinkCoord), channel);
  Ptr<UanNetDevice> dev2 = CreateNode (CreateVectorFromCoordZ (woss::CoordZ (woss::Coord::getCoordFromBearing (m_sinkCoord, M_PI / 2.0, r1 + r2), 70.0)), channel);

  dev0->SetReceiveCallback (MakeCallback (&WossTest::RxPacket, this));

  Simulator::Schedule (txTime1, &WossTest::SendOnePacket, this, dev1, mode1);
  Simulator::Schedule (txTime2, &WossTest::SendOnePacket, this, dev2, mode2);

  m_bytesRx = 0;
  Simulator::Stop (Seconds (20.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return m_bytesRx;
}


bool
WossTest::DoPhyTests (Ptr<WossPropModel> prop)
{
  // Phy Gen / Default PER / Default SINR
  UanModesList mList;
  UanTxMode mode = UanTxModeFactory::CreateMode (UanTxMode::PSK, 200, 200, 22000, 4000, 4, "Test Mode");
  mList.AppendMode (UanTxMode (mode));
  Ptr<UanPhyPerGenDefault> perDef = CreateObject<UanPhyPerGenDefault> ();
  Ptr<UanPhyCalcSinrDefault> sinrDef = CreateObject<UanPhyCalcSinrDefault> ();
  m_phyFac.SetTypeId ("ns3::UanPhyGen");
  m_phyFac.Set ("PerModel", PointerValue (perDef));
  m_phyFac.Set ("SinrModel", PointerValue (sinrDef));
  m_phyFac.Set ("SupportedModes", UanModesListValue (mList));

//   Ptr<WossPropModel> prop = CreateObject<WossPropModel> ();

  // No collision (Get 2 packets)
  NS_TEST_ASSERT_MSG_EQ_RETURNS_BOOL (DoOnePhyTest (Seconds (1.0), Seconds (3.001), 50, 50, prop),
                                      34, "Should have received 34 bytes from 2 disjoint packets");

  // Collision (Lose both packets)
  NS_TEST_ASSERT_MSG_EQ_RETURNS_BOOL (DoOnePhyTest (Seconds (1.0), Seconds (1.5), 50, 50, prop),
                                      0, "Expected collision resulting in loss of both packets");

  return false;
}

void
WossTest::PktRxOkThorpCallback (Ptr< Packet > pkt, double sinr, UanTxMode mode)
{
  std::cout << "PktRxOkThorpCallback: SINR = " << sinr << std::endl;
  m_thorpSinr.push_back(sinr);
}

void
WossTest::PktRxOkWossCallback (Ptr< Packet > pkt, double sinr, UanTxMode mode)
{
  std::cout << "PktRxOkWossCallback: SINR = " << sinr << std::endl;
  m_wossSinr.push_back(sinr);
}

void
WossTest::DoWossThorpTests (Ptr<WossPropModel> prop, double ranges[], double windNoise, double shipNoise)
{
  // Phy Gen / Default PER / Default SINR
  UanModesList mList;

  uint32_t phy_bitrate = 160;
      //type, dataRateBps, phyRateSps, cfHz, bwHz, constellationSize, name
  UanTxMode mode = UanTxModeFactory::CreateMode (UanTxMode::PSK, //PSK, QAM, FSK, OTHER
          phy_bitrate, phy_bitrate/2, 11520, 160, 2, "Default mode");

  mList.AppendMode (UanTxMode (mode));
  Ptr<UanPhyPerGenDefault> perDef = CreateObject<UanPhyPerGenDefault> ();
  Ptr<UanPhyCalcSinrDefault> sinrDef = CreateObject<UanPhyCalcSinrDefault> ();
  m_phyFac.SetTypeId ("ns3::UanPhyGen");
  m_phyFac.Set ("PerModel", PointerValue (perDef));
  m_phyFac.Set ("SinrModel", PointerValue (sinrDef));
  m_phyFac.Set ("SupportedModes", UanModesListValue (mList));

  double depth = 2500; //depth of all nodes (m)
  m_sinkCoord = woss::CoordZ(38.1, -130.7, depth);

  //WOSS channel
  m_wossHelper->SetCustomBathymetry ("2|0.0|5000.0|10000.0|5000.0", m_sinkCoord);
  m_wossHelper->SetCustomSediment ("TestSediment|1560.0|210.0|1.5|0.9|0.8|5000.0");
  m_wossHelper->SetCustomSsp ("2|0|1510|5000|1510");

  Ptr<WossChannel> wossChannel = CreateObject<WossChannel> ();
  wossChannel->SetAttribute ("PropagationModel", PointerValue (prop));
  wossChannel->SetAttribute ("ChannelEqSnrThresholdDb", DoubleValue (-100.0));


  //Thorp channel
  double spread_coef = 2.0;
  Ptr<UanPropModelThorp> propThorp = CreateObjectWithAttributes<UanPropModelThorp> ("SpreadCoef", DoubleValue (spread_coef)); //create the propagation model for the channel
  Ptr<UanChannel> thorpChannel = CreateObjectWithAttributes<UanChannel> ("PropagationModel", PointerValue (propThorp)); //plug prop model (and default noise model) into channel

  Ptr<UanNoiseModel> noise_model = CreateObjectWithAttributes<UanNoiseModelDefault> ("Wind", DoubleValue (windNoise), "Shipping", DoubleValue(shipNoise));
  thorpChannel->SetNoiseModel(PointerValue(noise_model));
  wossChannel->SetNoiseModel(PointerValue(noise_model));

  //Create the Thorp nodes
  Ptr<UanNetDevice> devt0 = CreateNode (Vector(0, 0, depth), thorpChannel);
  Ptr<UanNetDevice> devt1 = CreateNode (Vector(ranges[0], 0, depth), thorpChannel);
  Ptr<UanNetDevice> devt2 = CreateNode (Vector(ranges[1], 0, depth), thorpChannel);
  Ptr<UanNetDevice> devt3 = CreateNode (Vector(ranges[2], 0, depth), thorpChannel);

  //Create the WOSS nodes
  Ptr<UanNetDevice> devw0 = CreateNode (CreateVectorFromCoordZ (m_sinkCoord), wossChannel);
  Ptr<UanNetDevice> devw1 = CreateNode (CreateVectorFromCoordZ (woss::CoordZ (woss::Coord::getCoordFromBearing (m_sinkCoord, M_PI / 2.0, ranges[0]), depth)), wossChannel);
  Ptr<UanNetDevice> devw2 = CreateNode (CreateVectorFromCoordZ (woss::CoordZ (woss::Coord::getCoordFromBearing (m_sinkCoord, M_PI / 2.0, ranges[1]), depth)), wossChannel);
  Ptr<UanNetDevice> devw3 = CreateNode (CreateVectorFromCoordZ (woss::CoordZ (woss::Coord::getCoordFromBearing (m_sinkCoord, M_PI / 2.0, ranges[2]), depth)), wossChannel);

  //Connect RxOk callbacks
  devt1->GetPhy()->SetReceiveOkCallback(MakeCallback(&WossTest::PktRxOkThorpCallback, this));
  devt2->GetPhy()->SetReceiveOkCallback(MakeCallback(&WossTest::PktRxOkThorpCallback, this));
  devt3->GetPhy()->SetReceiveOkCallback(MakeCallback(&WossTest::PktRxOkThorpCallback, this));

  devw1->GetPhy()->SetReceiveOkCallback(MakeCallback(&WossTest::PktRxOkWossCallback, this));
  devw2->GetPhy()->SetReceiveOkCallback(MakeCallback(&WossTest::PktRxOkWossCallback, this));
  devw3->GetPhy()->SetReceiveOkCallback(MakeCallback(&WossTest::PktRxOkWossCallback, this));

  Simulator::Schedule (Seconds (1.0), &WossTest::SendOnePacket, this, devt0, 0);
  Simulator::Schedule (Seconds (10.0), &WossTest::SendOnePacket, this, devw0, 0);

  Simulator::Stop (Seconds (20.0));
  Simulator::Run ();
  Simulator::Destroy ();

  //Verify that the transmitted packets are received at all the nodes for both channels
  NS_TEST_ASSERT_MSG_EQ(m_thorpSinr.size(), 3, "Not all packets received at Thorp nodes");
  NS_TEST_ASSERT_MSG_EQ(m_wossSinr.size(), 3, "Not all packets received at Woss nodes");

  //Verify that the SINR matches between Thorp and WOSS for all ranges
  std::ostringstream oss;
  for(unsigned int i=0;i<m_thorpSinr.size();i++)
  {
    std::cout << "Thorp SINR = " << m_thorpSinr[i] << "; WOSS SINR = " << m_wossSinr[i] << std::endl;
    oss << "WOSS SINR does not match Thorp within tolerance: case " << i;
    NS_TEST_ASSERT_MSG_EQ_TOL (m_thorpSinr[i], m_wossSinr[i], 1.0, oss.str());
  }

  m_thorpSinr.clear();
  m_wossSinr.clear();
}


void
WossTest::DoRun (void)
{
  UanTxMode mode = UanTxModeFactory::CreateMode (UanTxMode::PSK, 200, 200, 22000, 4000, 4, "Test Mode");

  Ptr<ConstantPositionMobilityModel> mobility = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> mobility2 = CreateObject<ConstantPositionMobilityModel> ();

  Ptr<WossPropModel> wossProp = CreateObjectWithAttributes<WossPropModel> ();
  m_wossHelper = CreateObjectWithAttributes<WossHelper> ();

  InitWossHelper (wossProp);

  // CoordZ to Vector conversion check
  woss::CoordZ txCoordz = woss::CoordZ (woss::Coord::getCoordFromBearing (m_sinkCoord, M_PI / 2.0, 1000.0), 70.0);

  Vector txVector = CreateVectorFromCoordZ (CreateCoordZFromVector (CreateVectorFromCoordZ (txCoordz)));

  mobility->SetPosition (CreateVectorFromCoordZ (txCoordz));
  mobility2->SetPosition (txVector);

  double dist = mobility->GetDistanceFrom (mobility2);

  NS_TEST_ASSERT_MSG_EQ_TOL (dist, 0.0, 0.001, "Vector conversion outside of tolerance");

  // propagation delay of forward == reverse channel check
  Time delay = wossProp->GetDelay (m_sinkCoord, txCoordz, mode, -100.0);
  Time delay2 = wossProp->GetDelay (txCoordz, m_sinkCoord, mode, -100.0);

  NS_TEST_ASSERT_MSG_EQ_TOL (delay.GetSeconds (), delay2.GetSeconds (), 0.001, "Got two delays outside of tolerance");

  // channel total attenuation db of forward == reverse channel check
  mobility->SetPosition (CreateVectorFromCoordZ (m_sinkCoord));
  mobility2->SetPosition (CreateVectorFromCoordZ (txCoordz));

  UanPdp pdp = wossProp->GetPdp (mobility, mobility2, mode);
  UanPdp pdp2 = wossProp->GetPdp (mobility2, mobility, mode);

  double channelAttDb = 0.0;
  double channelAttDb2 = 0.0;

  for (UanPdp::Iterator it = pdp.GetBegin (); it != pdp.GetEnd (); ++it)
    {
      channelAttDb += ::std::pow (::std::abs (it->GetAmp ()), 2.0);
    }

  for (UanPdp::Iterator it2 = pdp2.GetBegin (); it2 != pdp2.GetEnd (); ++it2)
    {
      channelAttDb2 += ::std::pow (::std::abs (it2->GetAmp ()), 2.0);
    }
  ::std::cout << "channelAttDb = " << channelAttDb << "; channelAttDb2 = " << channelAttDb2 << ::std::endl;

  channelAttDb = ::std::sqrt (channelAttDb);
  channelAttDb = -10.0 * ::std::log10 (channelAttDb);

  channelAttDb2 = ::std::sqrt (channelAttDb2);
  channelAttDb2 = -10.0 * ::std::log10 (channelAttDb2);

  ::std::cout << "channelAttDb = " << channelAttDb << "; channelAttDb2 = " << channelAttDb2 << ::std::endl;

  NS_TEST_ASSERT_MSG_EQ_TOL (channelAttDb, channelAttDb2, 0.2, "Got two attenuation dbs outside of tolerance");

  // phy tests
  DoPhyTests (wossProp);

#if 0 // disabled for now
  // Propagation Tests: verify WOSS attenuation matches Thorp for deep water column case
  double ranges[3] = {500, 1000, 2500}; //create nodes at these ranges to receive packets (m)
  DoWossThorpTests (wossProp, ranges, 0, 0);
  DoWossThorpTests (wossProp, ranges, 3, 0.5);
#endif 
}


class WossTestSuite : public TestSuite
{
public:
  WossTestSuite ();
};

WossTestSuite::WossTestSuite ()
  :  TestSuite ("devices-woss", Type::UNIT)
{
  AddTestCase (new WossTest, Duration::QUICK);
}

static WossTestSuite g_uanWossTestSuite;

#endif // NS3_WOSS_SUPPORT



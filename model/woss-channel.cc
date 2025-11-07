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
 * Author: Federico Guerra <WOSS@guerra-tlc.com>
 */

#ifdef NS3_WOSS_SUPPORT

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/double.h"

#include "woss-channel.h"
#include "woss-prop-model.h"
#include "ns3/uan-phy.h"
#include "ns3/uan-prop-model.h"
#include "ns3/uan-tx-mode.h"
#include "ns3/uan-net-device.h"
#include "ns3/uan-transducer.h"
#include "ns3/uan-noise-model-default.h"
#include "ns3/uan-prop-model-thorp.h"


#define WOSS_CHANNEL_SNR_EQ_THRES_DB (-100.0)

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WossChannel");

NS_OBJECT_ENSURE_REGISTERED (WossChannel);

TypeId
WossChannel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::WossChannel")
    .SetParent<UanChannel> ()
    .SetGroupName ("Woss")
    .AddConstructor<WossChannel> ()
    .AddAttribute ("ChannelEqSnrThresholdDb",
                   "The first channel tap that gives a SNR (dB) greater than the threshold will be used as delay.",
                   DoubleValue (WOSS_CHANNEL_SNR_EQ_THRES_DB),
                   MakeDoubleAccessor (&WossChannel::m_channelEqSnrThresDb),
                   MakeDoubleChecker<double> () )
  ;

  return tid;
}

WossChannel::WossChannel ()
  : UanChannel (),
    m_channelEqSnrThresDb (WOSS_CHANNEL_SNR_EQ_THRES_DB),
    // -infinite snr ==> first tap
    m_wossPropModel (nullptr)
{
}

void
WossChannel::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  m_wossPropModel = DynamicCast< WossPropModel > ( m_prop );

  NS_ASSERT ( m_wossPropModel != nullptr );

  UanChannel::DoInitialize ();
}


void
WossChannel::TxPacket (Ptr<UanTransducer> src, Ptr<Packet> packet,
                       double txPowerDb, UanTxMode txMode)
{
  NS_LOG_FUNCTION (this);

  Ptr<MobilityModel> senderMobility = 0;
  Ptr<MobilityModel> tempRxMobility = 0;
  WossPropModel::MobModelVector rxMobVector;
  WossPropModel::UanPdpVector uanPdpVector;

  double noisePwrDb = GetNoiseDbHz ( (double) txMode.GetCenterFreqHz () / 1000.0) + 10 * log10 (txMode.GetBandwidthHz ());
  double chAttThresDb = txPowerDb - m_channelEqSnrThresDb - noisePwrDb;

  NS_LOG_DEBUG ("noisePwrDb:" << noisePwrDb << "dB; chAttThresDb:"
                              << chAttThresDb << "dB; m_channelEqSnrThresDb:" << m_channelEqSnrThresDb << "dB");

  NS_LOG_DEBUG ("Channel scheduling");

  for (UanDeviceList::const_iterator i = m_devList.begin (); i != m_devList.end (); i++)
    {
      tempRxMobility = i->first->GetNode ()->GetObject<MobilityModel> ();

      NS_ASSERT (tempRxMobility != nullptr);

      NS_LOG_DEBUG ("src ptr:" << PeekPointer (src) << "; i->second:" << PeekPointer (i->second));

      if (src != i->second)
        {
          rxMobVector.push_back (tempRxMobility);
        }
      else
        {
          senderMobility = tempRxMobility;
        }
    }

  NS_LOG_DEBUG ("rxMobVector.size ():" << rxMobVector.size ());

  uanPdpVector = m_wossPropModel->GetPdpVector (senderMobility, rxMobVector, txMode);

  NS_LOG_DEBUG ("uanPdpVector.size ():" << uanPdpVector.size ()
                                        << "; m_devList.size ():" << m_devList.size ());

  uint32_t k = 0;
  WossPropModel::UanPdpVector::iterator j = uanPdpVector.begin ();
  for ( UanDeviceList::const_iterator i = m_devList.begin (); i != m_devList.end (); ++i, ++k)
    {
      if (src != i->second)
        {
          double rxPowerDb = -HUGE_VAL;
          double totalAttCh = 0.0;
          double totalAttChdB = HUGE_VAL;
          bool delayFound = false;

          NS_LOG_DEBUG ("src:" << src << "; dst:" << i->second
                               << "; UanPdp size:" << j->GetNTaps ());

          UanPdp::Iterator it = j->GetBegin ();

          Time delay = it->GetDelay ();

          if ( chAttThresDb < 0.0 )
            {
              chAttThresDb = 0.0;
            }

          for (int tapCnt = 0; it != j->GetEnd (); ++it, ++tapCnt)
            {
              double attChDb = -20.0 * ::std::log10 (::std::abs (it->GetAmp ()));

              if ( (attChDb < 0.0) || (attChDb <= chAttThresDb) )
                {
                  NS_LOG_DEBUG ("tap:" << tapCnt << "; attenuation below threshold, attChDb:" << attChDb << "dB" );

                  if ( delayFound == false )
                    {
                      delay = it->GetDelay ();
                      delayFound = true;

                      NS_LOG_DEBUG ("found delay:" << delay);
                    }
                }

              // we found first usable tap
              if (delayFound == true)
                {
                  totalAttCh += ::std::pow (::std::abs (it->GetAmp ()), 2.0);

                  NS_LOG_DEBUG ("summing tap:" << tapCnt << "; totalAttCh:" << totalAttCh);
                }
            }

          if (delayFound == true)
            {
              totalAttCh = ::std::sqrt (totalAttCh);
              totalAttChdB = -20.0 * ::std::log10 (totalAttCh);

              rxPowerDb = txPowerDb - totalAttChdB;

              NS_LOG_DEBUG ("totalAttCh:" << totalAttCh << "; totalAttChdB:" << totalAttChdB
                                          << "dB; rxPowerDb:" << rxPowerDb << "dB");

            }

          // else rxPowerDb is -infinite and rx is not possible

          uint32_t dstNodeId = i->first->GetNode ()->GetId ();
          Ptr<Packet> copy = packet->Copy ();

          Ptr<MobilityModel> rcvrMobility = i->first->GetNode ()->GetObject<MobilityModel> ();
          double pathLossDb = m_prop->GetPathLossDb (senderMobility, rcvrMobility, txMode);

          NS_LOG_DEBUG ("dstNodeId = " << dstNodeId << ", txPowerDb=" << txPowerDb << "dB, pathLossDb="
                                       << pathLossDb << "dB, rxPowerDb1="
                                       << rxPowerDb << "dB, rxPowerDb2=" << (txPowerDb - pathLossDb) << "dB, delay=" << delay);

          UanPdp normalizedPdp = j->NormalizeToSumNc ();

          Simulator::ScheduleWithContext (dstNodeId, delay,
                                          &WossChannel::SendUp,
                                          this,
                                          k,
                                          copy,
                                          rxPowerDb,
                                          txMode,
//                                       (*j));
                                          normalizedPdp);

          if ( j != uanPdpVector.end () )
            {
              ++j;
            }
        }
    }
}


} // namespace ns3

#endif /* NS3_WOSS_SUPPORT */



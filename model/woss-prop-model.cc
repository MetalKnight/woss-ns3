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

#include "ns3/woss-prop-model.h"
#include "ns3/uan-tx-mode.h"
#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/boolean.h"




namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WossPropModel");


NS_OBJECT_ENSURE_REGISTERED (WossPropModel);

WossPropModel::WossPropModel ()
  : m_wossManager (nullptr),
    m_memOptimization (false)
{
}


TypeId
WossPropModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WossPropModel")
    .SetParent<UanPropModelThorp> ()
    .SetGroupName ("Woss")
    .AddConstructor<WossPropModel> ()
    .AddAttribute ("MemoryOptimization",
               "Memory used by WOSS objects are freed as soon as possible. In scenarios with HIGH mobility, \
               this feature should be activated in order to avoid memory exhaustion",
               BooleanValue (false),
               MakeBooleanAccessor (&WossPropModel::m_memOptimization),
               MakeBooleanChecker () )
  ;
  return tid;
}

void
WossPropModel::SetWossManager (std::shared_ptr<woss::WossManager> wossManagerPtr)
{
  m_wossManager = wossManagerPtr;
}

std::shared_ptr<woss::WossManager> const
WossPropModel::GetWossManager (void)
{
  return m_wossManager;
}

void
WossPropModel::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_wossManager != NULL);

  UanPropModel::DoInitialize ();
}


double
WossPropModel::GetPathLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b, UanTxMode mode)
{
  NS_LOG_FUNCTION (this);

  return UanPropModelThorp::GetPathLossDb (a, b, mode);
}

UanPdp
WossPropModel::GetPdp (Ptr<MobilityModel> a, Ptr<MobilityModel> b, UanTxMode mode)
{
  NS_LOG_FUNCTION (this);

  woss::CoordZ txCoordz = CreateCoordZ (a);
  woss::CoordZ rxCoorz = CreateCoordZ (b);
  double startFreq = mode.GetCenterFreqHz ();
  double endFreq = startFreq;

  NS_LOG_DEBUG ("txCoordz: " << txCoordz << "; rxCoorz: " << rxCoorz << "; startFreq: " << startFreq << "; endFreq: " << endFreq);

  auto currTimeArr = m_wossManager->getWossTimeArr (txCoordz, rxCoorz, startFreq, endFreq);

  NS_ASSERT ( currTimeArr != NULL );

  NS_LOG_DEBUG ("timeArr: " << *currTimeArr);

  UanPdp pdp = CreateUanPdp (std::move (currTimeArr), (1.0 / mode.GetPhyRateSps ()));
  
  if (m_memOptimization)
  {
    m_wossManager->reset();
  }
  
  return pdp;
}

Time
WossPropModel::GetDelay (Ptr<MobilityModel> a, Ptr<MobilityModel> b, UanTxMode mode)
{
  return UanPropModelThorp::GetDelay (a, b, mode);
}

WossPropModel::UanPdpVector
WossPropModel::GetPdpVector (Ptr<MobilityModel> a, MobModelVector& b, UanTxMode mode)
{
  NS_LOG_FUNCTION (this);

  woss::CoordZPairVect coordzPairVector = CreateCoordzPairVector (a, b);
  double startFreq = mode.GetCenterFreqHz ();
  double endFreq = startFreq;

  NS_LOG_DEBUG ("coordzPairVector size: " << coordzPairVector.size () << "; startFreq: " << startFreq << "; endFreq: " << endFreq);

  woss::TimeArrVector timeArrVect = m_wossManager->getWossTimeArr (coordzPairVector, startFreq, endFreq);
  UanPdpVector pdpVector = CreateUanPdpVector (timeArrVect, (1.0 / mode.GetPhyRateSps ()));
  
  if (m_memOptimization)
  {
    m_wossManager->reset();
  }
  
  return pdpVector;   
}

UanPdp
WossPropModel::CreateUanPdp (std::unique_ptr<woss::TimeArr> timeArr, double symbolTime)
{
  NS_LOG_FUNCTION (this);

  std::vector< Tap > vectTap;

  NS_LOG_DEBUG ("timeArr: " << *timeArr << "; symbolTime: " << symbolTime);

  // we sum coherently at symbol time
  auto coherentSum = timeArr->coherentSumSample (symbolTime);

  NS_LOG_DEBUG ("coherentSum size: " << coherentSum->size ());

  // we need to create a UanPdp with fixed resolution at symbolTime
  if (coherentSum->size () > 0)
    {
      NS_LOG_DEBUG ("first time: " << coherentSum->begin ()->first << " [s]; last time: " << coherentSum->rbegin ()->first << " [s]");

      double start_time = coherentSum->begin ()->first;

      double end_time = coherentSum->rbegin ()->first;

      int n_taps = 1;

      // compute taps number only if there are more than one taps
      if(start_time < end_time) 
        {
          n_taps = std::ceil ((end_time - start_time) / symbolTime);
        }

      int cnt = 0;

      NS_LOG_DEBUG ("total n taps: " << n_taps);

      woss::TimeArrCIt it = coherentSum->begin ();

      for (cnt = 0; cnt < n_taps; ++cnt)
        {
          double tap_time = start_time + cnt * symbolTime;

          std::complex<double> tap_value (0.0, 0.0);

          if (it == coherentSum->end ())
            {
              break;
            }

          if (woss::PDouble (tap_time, symbolTime * 1.1 / 2.0) == it->first)
            { // if |tap_time - it->first| <= ((symbolTime+0.1*symbolTime)/2.0) ==> set tap, advance iterator
              tap_value = it->second;
              it++;
            }
          else if (woss::PDouble (tap_time, symbolTime * 1.1 / 2.0) > it->first)
            { // if tap_time > it->first ==> fatal error
              NS_FATAL_ERROR ("tap_time: " << tap_time << " > iterator time: " << it->first);
            }
          // else ==> tap_time < it->first ==> use tap_value already set to (0.0,0.0)

          NS_LOG_DEBUG ("Tap: " << tap_time << " [s]; " << woss::Pressure::getTxLossDb (tap_value) << " dB re uPa");

          vectTap.push_back (Tap (Seconds (tap_time), tap_value));
        }

        NS_ASSERT (cnt == n_taps);
    } 

  if (vectTap.size () == 0)
    {
      // return empty channel
      vectTap.push_back (Tap (Seconds (0.0), std::complex<double> (0.0, 0.0)));
    }

  return (UanPdp (vectTap, Seconds (symbolTime)));
}

WossPropModel::UanPdpVector
WossPropModel::CreateUanPdpVector (woss::TimeArrVector& timeArrVector, double symbolTime)
{
  NS_LOG_FUNCTION (this);

  UanPdpVector retVal;

  for ( woss::TimeArrVector::iterator it = timeArrVector.begin (); it != timeArrVector.end (); ++it )
    {
      retVal.push_back (CreateUanPdp (std::move(*it), symbolTime));
    }

  return retVal;
}

woss::CoordZ
WossPropModel::CreateCoordZ (Ptr<MobilityModel> mobModel)
{
  NS_LOG_FUNCTION (this);

  Vector vect = mobModel->GetPosition ();

  return (woss::CoordZ::getCoordZFromCartesianCoords (vect.x, 
                                                      vect.y, 
                                                      vect.z, 
                                                      woss::CoordZ::CoordZSpheroidType::COORDZ_WGS84));
}

woss::CoordZPairVect
WossPropModel::CreateCoordzPairVector ( Ptr<MobilityModel> tx, MobModelVector& rxs)
{
  NS_LOG_FUNCTION (this);

  woss::CoordZPairVect retVal;

  for ( MobModelVector::iterator it = rxs.begin (); it != rxs.end (); ++it )
    {
      retVal.push_back (std::make_pair (CreateCoordZ (tx), CreateCoordZ (*it)));
    }

  return retVal;
}

Time
WossPropModel::GetDelay (const woss::CoordZ &a, const woss::CoordZ &b, UanTxMode mode, double chAttThresDb)
{
  NS_LOG_FUNCTION (this);

  double startFreq = mode.GetCenterFreqHz ();
  double endFreq = startFreq;

  NS_LOG_DEBUG ("a: " << a << "; b: " << b << "; startFreq: " << startFreq << "; endFreq: " << endFreq);

  auto currTimeArr = m_wossManager->getWossTimeArr (a, b, startFreq, endFreq);

  NS_ASSERT ( currTimeArr != NULL );

  NS_LOG_DEBUG ("timeArr: " << *currTimeArr);

  UanPdp uanPdp = CreateUanPdp (std::move (currTimeArr), (1.0 / mode.GetPhyRateSps ()));

  if (m_memOptimization)
  {
    m_wossManager->reset();
  }
     
  NS_LOG_DEBUG ("UanPdp size: " << uanPdp.GetNTaps ());

  UanPdp::Iterator it = uanPdp.GetBegin ();

  Time delay = it->GetDelay ();

  for (int tapCnt = 0; it != uanPdp.GetEnd (); ++it, ++tapCnt)
    {
      double attChDb = -20.0 * std::log10 (std::abs (it->GetAmp ()));

      NS_LOG_DEBUG ("tap: " << tapCnt << "; attChDb: " << attChDb << "dB" );

      if ( (attChDb < 0.0) || (attChDb <= chAttThresDb) )
        {
          NS_LOG_DEBUG ("tap: " << tapCnt << "; attenuation below threshold, attChDb: " << attChDb << "dB" );

          delay = it->GetDelay ();

          NS_LOG_DEBUG ("found delay: " << delay.GetSeconds ());

          break;
        }
    }

  return delay;
}

} // namespace ns3

#endif /* NS3_WOSS_SUPPORT */

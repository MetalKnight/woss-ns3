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

#include "woss-helper.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/string.h"


#define WH_SPACE_SAMPLING_DEFAULT (0.0)
#define WH_DEBUG_DEFAULT false
#define WH_TIME_EVOLUTION_DEFAULT (-1.0)
#define WH_STRING_DEFAULT ""
#define WH_TOTAL_RUNS_DEFAULT (5)
#define WH_FREQUENCY_STEP_DEFAULT (0.0)
#define WH_TOTAL_RANGE_STEPS_DEFAULT (3000.0)
#define WH_TX_MIN_DEPTH_OFFSET_DEFAULT (0.0)
#define WH_TX_MAX_DEPTH_OFFSET_DEFAULT (0.0)
#define WH_TOTAL_TRANSMITTERS_DEFAULT (1)
#define WH_TOTAL_RX_DEPTHS_DEFAULT (2)
#define WH_RX_MIN_DEPTH_OFFSET_DEFAULT (-0.1)
#define WH_RX_MAX_DEPTH_OFFSET_DEFAULT (0.1)
#define WH_TOTAL_RX_RANGES_DEFAULT (2)
#define WH_RX_MIN_RANGE_OFFSET_DEFAULT (-0.1)
#define WH_RX_MAX_RANGE_OFFSET_DEFAULT (0.1)
#define WH_TOTAL_RAYS_DEFAULT (0)
#define WH_MIN_ANGLE_DEFAULT (-180.0)
#define WH_MAX_ANGLE_DEFAULT (180.0)
#define WH_SSP_DEPTH_PRECISION_DEFAULT (1.0E-8)
#define WH_NORMALIZED_SSP_DEPTH_STEPS_DEFAULT (100000)
#define WH_WORK_PATH_DEFAULT "./woss-output/work-dir"
#define WH_BELLHOP_MODE_DEFAULT "A"
#define WH_BEAM_OPTIONS_DEFAULT "B"
#define WH_BATHYMETRY_TYPE_DEFAULT "LL"
#define WH_BATHYMETRY_METHOD_DEFAULT "D"
#define WH_ALTIMETRY_TYPE_DEFAULT "L"
#define WH_SIMULATION_TIMES_DEFAULT ()
#define WH_CONCURRENT_THREADS_DEFAULT (0.0)
#define WH_BELLHOP_ARR_SYNTAX_DEFAULT (2)
#define WH_BELLHOP_ARR_SYNTAX_MIN (0)
#define WH_BELLHOP_ARR_SYNTAX_MAX (2)
#define WH_BELLHOP_SHD_SYNTAX_DEFAULT (1)
#define WH_BELLHOP_SHD_SYNTAX_MIN (0)
#define WH_BELLHOP_SHD_SYNTAX_MAX (1)
#define WH_BOX_DEPTH (-3000.0)
#define WH_BOX_RANGE (-3000.0)
#define WH_GEBCO_FORMAT_DEFAULT (3)
#define WH_GEBCO_FORMAT_MIN (0)
#define WH_GEBCO_FORMAT_MAX (4)
#if defined (WOSS_NETCDF4_SUPPORT)
#define WH_WOA_DB_TYPE_DEFAULT (1)
#define WH_WOA_DB_TYPE_MIN (0)
#define WH_WOA_DB_TYPE_MAX (1)
#define WH_SEDIMENT_DECK41_FORMAT_DEFAULT (1)
#define WH_SEDIMENT_DECK41_FORMAT_MIN (0)
#define WH_SEDIMENT_DECK41_FORMAT_MAX (1)
#endif // defined (WOSS_NETCDF4_SUPPORT)

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WossHelper");

ATTRIBUTE_HELPER_CPP (WossSimTime);

Vector
CreateVectorFromCoords (double latitude, double longitude, double depth)
{
  woss::CoordZ coordz (latitude, longitude, depth);
  woss::CoordZ::CartCoords currCartCoords = coordz.getCartCoords (woss::CoordZ::CoordZSpheroidType::COORDZ_WGS84);

  return Vector (currCartCoords.getX (), currCartCoords.getY (), currCartCoords.getZ ());
}

Vector
CreateVectorFromCoordZ (const woss::CoordZ &coordz)
{
  woss::CoordZ::CartCoords currCartCoords = coordz.getCartCoords (woss::CoordZ::CoordZSpheroidType::COORDZ_WGS84);

  return Vector (currCartCoords.getX (), currCartCoords.getY (), currCartCoords.getZ ());
}

woss::CoordZ
CreateCoordZFromVector (Vector vect)
{
  return (woss::CoordZ::getCoordZFromCartesianCoords (vect.x, 
                                                      vect.y, 
                                                      vect.z, 
                                                      woss::CoordZ::CoordZSpheroidType::COORDZ_WGS84));
}


woss::Coord
CreateCoordFromVector (Vector vect)
{
  woss::CoordZ coordZtemp = woss::CoordZ::getCoordZFromCartesianCoords (vect.x, 
                                                                        vect.y, 
                                                                        vect.z, woss::CoordZ::CoordZSpheroidType::COORDZ_WGS84);
  return (woss::Coord (coordZtemp.getLatitude (), coordZtemp.getLongitude ()));
}


std::ostream &
operator << (std::ostream &os, const WossSimTime &simTime)
{
  os << simTime.start_time.getDay () << "|" << simTime.start_time.getMonth () << "|" << simTime.start_time.getYear () << "|"
  << simTime.end_time.getDay () << "|" << simTime.end_time.getMonth () << "|" << simTime.end_time.getYear ();

  return os;
}

std::istream &
operator >> (std::istream &is, WossSimTime &simTime)
{
  char c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11;
  int d1, mn1, y1, h1, m1, s1, d2, mn2, y2, h2, m2, s2;

  is >> d1 >> c1 >> mn1 >> c2 >> y1 >> c3 >> h1 >> c4 >> m1 >> c5 >> s1 >> c6
  >> d2 >> c7 >> mn2 >> c8 >> y2 >> c9 >> h2 >> c10 >> m2 >> c11 >> s2;

  if (c1 != '|'
      || c2 != '|'
      || c3 != '|'
      || c4 != '|'
      || c5 != '|'
      || c6 != '|'
      || c7 != '|'
      || c8 != '|'
      || c9 != '|'
      || c10 != '|'
      || c11 != '|'
      )
    {
      is.setstate (std::ios_base::failbit);
    }

  simTime.start_time.setDay (d1);
  simTime.start_time.setMonth (mn1);
  simTime.start_time.setYear (y1);
  simTime.start_time.setHours (h1);
  simTime.start_time.setMinutes (m1);
  simTime.start_time.setSeconds (s1);

  simTime.end_time.setDay (d2);
  simTime.end_time.setMonth (mn2);
  simTime.end_time.setYear (y2);
  simTime.end_time.setHours (h2);
  simTime.end_time.setMinutes (m2);
  simTime.end_time.setSeconds (s2);

  if ( !(simTime.start_time.isValid ()) || !(simTime.end_time.isValid ()) )
    {
      is.setstate (std::ios_base::failbit);
    }
  return is;
}


WossHelper::WossHelper ()
  : m_locMap (),
    m_sspProto (std::make_unique<woss::SSP> ()),
    m_sedimentProto (std::make_unique<woss::Sediment> ()),
    m_pressureProto (std::make_unique<woss::Pressure> ()),
    m_timeArrProto (std::make_unique<woss::TimeArr> ()),
    m_transducerProto (std::make_unique<woss::Transducer> ()),
    m_altimBretProto (std::make_unique<woss::AltimBretschneider> ()),
    m_locationProto (std::make_unique<WossLocation> ()),
    m_wossRandomGenStream (0),
    m_randomGenProto (std::make_unique<WossRandomGenerator> ()),
    m_timeRefProto (std::make_unique<WossTimeReference> ()),
    m_defHandler (woss::SDefHandler::instance ()),
    m_resDbCreatorDebug (WH_DEBUG_DEFAULT),
    m_resDbDebug (WH_DEBUG_DEFAULT),
    m_resDbUseBinary (true),
    m_resDbUseTimeArr (true),
    m_resDbSpaceSampling (WH_SPACE_SAMPLING_DEFAULT),
    m_resDbFilePath (WH_STRING_DEFAULT),
    m_resDbFileName (WH_STRING_DEFAULT),
    m_resDbCreatorPressBin (std::make_shared<woss::ResPressureBinDbCreator> ()),
    m_resDbCreatorPressTxt (std::make_shared<woss::ResPressureTxtDbCreator> ()),
    m_resDbCreatorTimeArrBin (std::make_shared<woss::ResTimeArrBinDbCreator> ()),
    m_resDbCreatorTimeArrTxt (std::make_shared<woss::ResTimeArrTxtDbCreator> ()),
#if defined (WOSS_NETCDF_SUPPORT)
    m_sedimDbCreatorDebug (WH_DEBUG_DEFAULT),
    m_sedimDbDebug (WH_DEBUG_DEFAULT),
#if defined (WOSS_NETCDF4_SUPPORT)
    m_sedimDbDeck41DbType(WH_SEDIMENT_DECK41_FORMAT_DEFAULT),
#endif // defined (WOSS_NETCDF4_SUPPORT)
    m_sedimDbCoordFilePath (WH_STRING_DEFAULT),
    m_sedimDbMarsdenFilePath (WH_STRING_DEFAULT),
    m_sedimDbMarsdenOneFilePath (WH_STRING_DEFAULT),
    m_sedimDbCreator (std::make_shared<woss::SedimDeck41DbCreator> ()),
    m_sspDbCreatorDebug (WH_DEBUG_DEFAULT),
    m_sspDbDebug (WH_DEBUG_DEFAULT),
    m_sspDbFilePath (WH_STRING_DEFAULT),
#if defined (WOSS_NETCDF4_SUPPORT)
    m_sspWoaDbType (WH_WOA_DB_TYPE_DEFAULT),
    m_sspDbCreator (std::make_shared<woss::SspWoa2005DbCreator> ((woss::WOADbType)m_sspWoaDbType)),
#else
    m_sspDbCreator (std::make_shared<woss::SspWoa2005DbCreator> ()),
#endif // defined (WOSS_NETCDF_SUPPORT)
    m_bathyDbCreatorDebug (WH_DEBUG_DEFAULT),
    m_bathyDbDebug (WH_DEBUG_DEFAULT),
    m_bathyDbGebcoFormat(WH_GEBCO_FORMAT_DEFAULT),
    m_bathyDbFilePath (WH_STRING_DEFAULT),
    m_bathyDbCreator (std::make_shared<woss::BathyGebcoDbCreator> ()),
#endif // defined (WOSS_NETCDF_SUPPORT)
    m_wossDbManagerDebug (WH_DEBUG_DEFAULT),
    m_wossDbManager (std::make_shared<woss::WossDbManager> ()),
    m_wossCreatorDebug (WH_DEBUG_DEFAULT),
    m_wossDebug (WH_DEBUG_DEFAULT),
    m_wossClearWorkDir (true),
    m_evolutionTimeQuantum (WH_TIME_EVOLUTION_DEFAULT),
    m_totalRuns (WH_TOTAL_RUNS_DEFAULT),
    m_frequencyStep (WH_FREQUENCY_STEP_DEFAULT),
    m_totalRangeSteps (WH_TOTAL_RANGE_STEPS_DEFAULT),
    m_txMinDepthOffset (WH_TX_MIN_DEPTH_OFFSET_DEFAULT),
    m_txMaxDepthOffset (WH_TX_MAX_DEPTH_OFFSET_DEFAULT),
    m_totalTransmitters (WH_TOTAL_TRANSMITTERS_DEFAULT),
    m_totalRxDepths (WH_TOTAL_RX_DEPTHS_DEFAULT),
    m_rxMinDepthOffset (WH_RX_MIN_DEPTH_OFFSET_DEFAULT),
    m_rxMaxDepthOffset (WH_RX_MAX_DEPTH_OFFSET_DEFAULT),
    m_totalRxRanges (WH_TOTAL_RX_RANGES_DEFAULT),
    m_rxMinRangeOffset (WH_RX_MIN_RANGE_OFFSET_DEFAULT),
    m_rxMaxRangeOffset (WH_RX_MAX_RANGE_OFFSET_DEFAULT),
    m_totalRays (WH_TOTAL_RAYS_DEFAULT),
    m_minAngle (WH_MIN_ANGLE_DEFAULT),
    m_maxAngle (WH_MAX_ANGLE_DEFAULT),
    m_sspDepthPrecision (WH_SSP_DEPTH_PRECISION_DEFAULT),
    m_normalizedSspDepthSteps (WH_NORMALIZED_SSP_DEPTH_STEPS_DEFAULT),
    m_workDirPath (WH_WORK_PATH_DEFAULT),
    m_bellhopPath (WH_STRING_DEFAULT),
    m_bellhopMode (WH_BELLHOP_MODE_DEFAULT),
    m_bellhopBeamOptions (WH_BEAM_OPTIONS_DEFAULT),
    m_bellhopBathyType (WH_BATHYMETRY_TYPE_DEFAULT),
    m_bathyMethod(WH_BATHYMETRY_METHOD_DEFAULT),
    m_bellhopAltimType (WH_ALTIMETRY_TYPE_DEFAULT),
    m_bellhopArrSyntax (WH_BELLHOP_ARR_SYNTAX_DEFAULT),
    m_bellhopShdSyntax (WH_BELLHOP_SHD_SYNTAX_DEFAULT),
    m_simTime (),
    m_bellhopCreator (std::make_shared<woss::BellhopCreator> ()),
    m_boxDepth (WH_BOX_DEPTH),
    m_boxRange (WH_BOX_RANGE),
    m_wossManagerDebug (WH_DEBUG_DEFAULT),
    m_isTimeEvolutionActive (false),
    m_concurrentThreads (WH_CONCURRENT_THREADS_DEFAULT),
    m_wossManagerSpaceSampling (WH_SPACE_SAMPLING_DEFAULT),
    m_wossManagerUseMultiThread (false),
    m_wossManagerSimple (std::make_shared< woss::WossManagerSimple<woss::WossManagerResDb> > ()),
    m_wossManagerMulti (std::make_shared< woss::WossManagerSimple<woss::WossManagerResDbMT> > ()),
    m_wossTransducerHndlDebug (WH_DEBUG_DEFAULT),
    m_wossTransducerHndl (std::make_shared<woss::TransducerHandler> ()),
    m_wossControllerDebug (WH_DEBUG_DEFAULT),
    m_wossController (std::make_shared<woss::WossController> ()),
    m_initialized (false)
{
  m_defHandler.setSSP (std::move (m_sspProto));
  m_defHandler.setSediment (std::move (m_sedimentProto));
  m_defHandler.setTransducer (std::move (m_transducerProto));
  m_defHandler.setAltimetry (std::move (m_altimBretProto));
  m_defHandler.setPressure (std::move (m_pressureProto));
  m_defHandler.setTimeArr (std::move (m_timeArrProto));
  m_defHandler.setLocation (std::move (m_locationProto));
  m_defHandler.setTimeReference (std::move (m_timeRefProto));
}

void
WossHelper::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_initialized = false;
}

void
WossHelper::CheckInitialized (void) const
{
  if (m_initialized == false)
    {
      NS_FATAL_ERROR ("Initialize (Ptr<WossPropModel> wossPropModel) has not been called!");
    }
}

void
WossHelper::DoInitialize (void)
{
  CheckInitialized ();
}

void
WossHelper::Initialize (Ptr<WossPropModel> wossPropModel)
{
  NS_LOG_FUNCTION (this << wossPropModel);

  if (m_initialized == true)
    {
      return;
    }

  NS_LOG_DEBUG ("Setting DefHandler");

  m_randomGenProto->AssignStreams (m_wossRandomGenStream);
  m_defHandler.setRandGenerator (std::move (m_randomGenProto));

#if defined (WOSS_NETCDF_SUPPORT)
  NS_LOG_DEBUG ("Setting BathymetryDbCreator");

  if ( m_bathyDbFilePath != WH_STRING_DEFAULT )
    {
      m_bathyDbCreator->setDbPathName (m_bathyDbFilePath);
      m_bathyDbCreator->setDebug (m_bathyDbCreatorDebug);
      m_bathyDbCreator->setWossDebug (m_bathyDbDebug);
      m_bathyDbCreator->setGebcoBathyType ((woss::GEBCO_BATHY_TYPE)m_bathyDbGebcoFormat);

      m_wossController->setBathymetryDbCreator (m_bathyDbCreator);
    }

  NS_LOG_DEBUG ("Setting SedimDbCreator");

  if ( m_sedimDbCoordFilePath != WH_STRING_DEFAULT && m_sedimDbMarsdenFilePath != WH_STRING_DEFAULT
       && m_sedimDbMarsdenOneFilePath != WH_STRING_DEFAULT )
    {
      m_sedimDbCreator->setDeck41CoordPathName (m_sedimDbCoordFilePath);
      m_sedimDbCreator->setDeck41MarsdenPathName (m_sedimDbMarsdenFilePath);
      m_sedimDbCreator->setDeck41MarsdenOnePathName (m_sedimDbMarsdenOneFilePath);
      m_sedimDbCreator->setDebug (m_sedimDbCreatorDebug);
      m_sedimDbCreator->setWossDebug (m_sedimDbDebug);
#if defined (WOSS_NETCDF4_SUPPORT)
      m_sedimDbCreator->setDeck41DbType ((woss::DECK41DbType)m_sedimDbDeck41DbType);
#endif // defined (WOSS_NETCDF4_SUPPORT)

      m_wossController->setSedimentDbCreator (m_sedimDbCreator);
    }

  NS_LOG_DEBUG ("Setting SspDbCreator");

  if ( m_sspDbFilePath != WH_STRING_DEFAULT )
    {
      m_sspDbCreator->setDbPathName (m_sspDbFilePath);
      m_sspDbCreator->setDebug (m_sspDbCreatorDebug);
      m_sspDbCreator->setWossDebug (m_sspDbDebug);

      m_wossController->setSSPDbCreator (m_sspDbCreator);
    }
#endif // defined (WOSS_NETCDF_SUPPORT)

  NS_LOG_DEBUG ("Setting ResDbCreator");

  if ( (m_resDbFilePath != WH_STRING_DEFAULT) && (m_resDbFileName != WH_STRING_DEFAULT) )
    {
      NS_ASSERT ( CreateDirectory (m_resDbFilePath) == true );

      if ( m_resDbUseBinary == true && m_resDbUseTimeArr == true )
        {
          NS_LOG_DEBUG ("Setting TimeArr binary");

          m_resDbCreatorTimeArrBin->setDbPathName (m_resDbFilePath + "/" + m_resDbFileName);
          m_resDbCreatorTimeArrBin->setDebug (m_resDbCreatorDebug);
          m_resDbCreatorTimeArrBin->setWossDebug (m_resDbDebug);

          m_wossController->setTimeArrDbCreator (m_resDbCreatorTimeArrBin);
        }
      else if ( m_resDbUseBinary == false && m_resDbUseTimeArr == true )
        {
          NS_LOG_DEBUG ("Setting TimeArr ASCII");

          m_resDbCreatorTimeArrTxt->setDbPathName (m_resDbFilePath + "/" +  m_resDbFileName);
          m_resDbCreatorTimeArrTxt->setDebug (m_resDbCreatorDebug);
          m_resDbCreatorTimeArrTxt->setWossDebug (m_resDbDebug);

          m_wossController->setTimeArrDbCreator (m_resDbCreatorTimeArrTxt);
        }
      else if ( m_resDbUseBinary == true && m_resDbUseTimeArr == false )
        {
          NS_LOG_DEBUG ("Setting Pressure binary");

          m_resDbCreatorPressBin->setDbPathName (m_resDbFilePath + "/" + m_resDbFileName);
          m_resDbCreatorPressBin->setDebug (m_resDbCreatorDebug);
          m_resDbCreatorPressBin->setWossDebug (m_resDbDebug);

          m_wossController->setPressureDbCreator (m_resDbCreatorPressBin);
        }
      else if ( m_resDbUseBinary == false && m_resDbUseTimeArr == false )
        {
          NS_LOG_DEBUG ("Setting Pressure ASCII");

          m_resDbCreatorPressTxt->setDbPathName (m_resDbFilePath + "/" + m_resDbFileName);
          m_resDbCreatorPressTxt->setDebug (m_resDbCreatorDebug);
          m_resDbCreatorPressTxt->setWossDebug (m_resDbDebug);

          m_wossController->setPressureDbCreator (m_resDbCreatorPressTxt);
        }
    }

  NS_LOG_DEBUG ("Setting Bellhop Creator");

  m_bellhopCreator->setDebug (m_wossCreatorDebug);
  m_bellhopCreator->setWossDebug (m_wossDebug);
  m_bellhopCreator->setWrkDirPath (m_workDirPath);
  m_bellhopCreator->setCleanWorkDir (m_wossClearWorkDir);
  m_bellhopCreator->setEvolutionTimeQuantum (m_evolutionTimeQuantum);
  m_bellhopCreator->setThorpeAttFlag (true);
  m_bellhopCreator->setTotalRuns (m_totalRuns);
  m_bellhopCreator->setFrequencyStep (m_frequencyStep);
  m_bellhopCreator->setTotalRangeSteps (m_totalRangeSteps);
  m_bellhopCreator->setTxMinDepthOffset (m_txMinDepthOffset);
  m_bellhopCreator->setTxMaxDepthOffset (m_txMaxDepthOffset);
  m_bellhopCreator->setTotalTransmitters (m_totalTransmitters);
  m_bellhopCreator->setRxTotalDepths (m_totalRxDepths);
  m_bellhopCreator->setRxMinDepthOffset (m_rxMinDepthOffset);
  m_bellhopCreator->setRxMaxDepthOffset (m_rxMaxDepthOffset);
  m_bellhopCreator->setRxTotalRanges (m_totalRxRanges);
  m_bellhopCreator->setRxMinRangeOffset (m_rxMinRangeOffset);
  m_bellhopCreator->setRxMaxRangeOffset (m_rxMaxRangeOffset);
  m_bellhopCreator->setRaysNumber (m_totalRays);
  m_bellhopCreator->setAngles (woss::CustomAngles (m_minAngle, m_maxAngle));
  m_bellhopCreator->setSspDepthPrecision (m_sspDepthPrecision);
  m_bellhopCreator->setSspDepthSteps (m_normalizedSspDepthSteps);
  m_bellhopCreator->setBellhopPath (m_bellhopPath);
  m_bellhopCreator->setBhMode (m_bellhopMode);
  m_bellhopCreator->setBeamOptions (m_bellhopBeamOptions);
  m_bellhopCreator->setBathymetryType (m_bellhopBathyType);
  m_bellhopCreator->setBathymetryMethod (m_bathyMethod);
  m_bellhopCreator->setAltimetryType (m_bellhopAltimType);
  NS_ASSERT ((m_simTime.start_time.isValid () == true) && (m_simTime.end_time.isValid ()) );
  m_bellhopCreator->setSimTime (m_simTime);
  m_bellhopCreator->setBellhopArrSyntax ((woss::BellhopArrSyntax)m_bellhopArrSyntax);
  m_bellhopCreator->setBellhopShdSyntax ((woss::BellhopShdSyntax)m_bellhopShdSyntax);
  m_bellhopCreator->setBoxDepth(m_boxDepth);
  m_bellhopCreator->setBoxRange(m_boxRange);
  m_wossController->setWossCreator (m_bellhopCreator);

  NS_LOG_DEBUG ("Setting WossDbManager");

  m_wossDbManager->setDebug (m_wossDbManagerDebug);

  m_wossController->setWossDbManager (m_wossDbManager);

  if (m_wossManagerUseMultiThread == false)
    {
      NS_LOG_DEBUG ("Setting WossManager Single Threaded");

      m_wossManagerSimple->setDebugFlag (m_wossManagerDebug);
      m_wossManagerSimple->setTimeEvolutionActiveFlag (m_isTimeEvolutionActive);
      m_wossManagerSimple->setSpaceSampling (m_wossManagerSpaceSampling);

      m_wossController->setWossManager (m_wossManagerSimple);
      wossPropModel->SetWossManager (m_wossManagerSimple);
    }
  else
    {
      NS_LOG_DEBUG ("Setting WossManager Multi Threaded");

      m_wossManagerMulti->setDebugFlag (m_wossManagerDebug);
      m_wossManagerMulti->setTimeEvolutionActiveFlag (m_isTimeEvolutionActive);
      m_wossManagerMulti->setSpaceSampling (m_wossManagerSpaceSampling);
      m_wossManagerMulti->setConcurrentThreads (m_concurrentThreads);

      m_wossController->setWossManager (m_wossManagerMulti);
      wossPropModel->SetWossManager (m_wossManagerMulti);
    }

  NS_LOG_DEBUG ("Setting TransducerHandler");

  m_wossTransducerHndl->setDebug (m_wossTransducerHndlDebug);
  m_wossController->setTransducerHandler (m_wossTransducerHndl);

  bool isOk = m_wossController->initialize ();

  NS_LOG_DEBUG ("WossController is initialized: " << (isOk ? "true" : "false") );

  if (isOk == false)
    {
      NS_FATAL_ERROR ("WossController is not initialized");
    }

  m_initialized = true;
}


std::shared_ptr<WossLocation>
WossHelper::GetWossLocation ( Ptr< MobilityModel > ptr )
{
  NS_LOG_FUNCTION (this << ptr);

  MLMCIter it = m_locMap.find (PeekPointer (ptr));

  if ( it == m_locMap.end () )
    {
      NS_LOG_DEBUG ("WossController MobilityModel not found, creating new WossLocation. map size " << m_locMap.size ());

      auto retValue = std::make_shared<WossLocation> ( ptr );
      m_locMap.insert ( std::make_pair ( PeekPointer (ptr), retValue ));
      return retValue;
    }
  else
    {
      return it->second;
    }
}


bool
WossHelper::SetAngles (const std::string &angleString, Ptr<MobilityModel> tx, Ptr<MobilityModel> rx)
{
  CheckInitialized ();

  std::string angleTmp = angleString;
  std::string::size_type tmp;
  double param[2];

  for (int cnt = 0; cnt < 2; ++cnt)
    {
      tmp = angleTmp.find ("|");
      if ((tmp == std::string::npos) && (cnt != 2 - 1))
        {
          NS_LOG_ERROR ("cnt:" << cnt << "; separator | not found, string parsed: " << angleTmp);
          return false;
        }

      std::string paramStr = angleTmp.substr (0, tmp);
      angleTmp = angleTmp.substr (tmp + 1, angleTmp.npos);
      param[cnt] = std::atof (paramStr.c_str ());

      NS_LOG_DEBUG ("cnt:" << cnt << "; param:" << param[cnt]);
    }

  woss::CustomAngles customAngles = woss::CustomAngles (param[0], param[1]);
  return SetAngles (customAngles, tx, rx);
}


bool
WossHelper::SetAngles ( const woss::CustomAngles& angles, Ptr<MobilityModel> tx, Ptr<MobilityModel> rx )
{
  CheckInitialized ();

  if ( tx == nullptr )
    {
      if ( rx == nullptr )
        {
          m_bellhopCreator->setAngles (angles);
        }
      else
        {
          auto rxLoc = GetWossLocation ( rx );
          m_bellhopCreator->setAngles (angles, woss::BellhopCreator::CCAngles::ALL_LOCATIONS, rxLoc );
        }
    }
  else
    {
      auto txLoc = GetWossLocation ( tx );

      if ( rx == nullptr )
        {
          m_bellhopCreator->eraseAngles ( txLoc );
        }
      else
        {
          auto rxLoc = GetWossLocation ( rx );
          m_bellhopCreator->eraseAngles ( txLoc, rxLoc );
        }
    }

  return true;
}


woss::CustomAngles
WossHelper::GetAngles ( Ptr<MobilityModel> tx, Ptr<MobilityModel> rx ) const
{
  CheckInitialized ();

  if ( tx == nullptr )
    {
      if ( rx == nullptr )
        {
          return m_bellhopCreator->getAngles ();
        }
      else
        {
          auto rxLoc = const_cast<WossHelper*> (this)->GetWossLocation ( rx );
          return m_bellhopCreator->getAngles (woss::BellhopCreator::CCAngles::ALL_LOCATIONS, rxLoc );
        }
    }
  else
    {
      auto txLoc = const_cast<WossHelper*> (this)->GetWossLocation ( tx );

      if ( rx == nullptr )
        {
          return m_bellhopCreator->getAngles ( txLoc );
        }
      else
        {
          auto rxLoc = const_cast<WossHelper*> (this)->GetWossLocation ( rx );
          return m_bellhopCreator->getAngles ( txLoc, rxLoc );
        }
    }
}


void
WossHelper::EraseAngles ( Ptr<MobilityModel> tx, Ptr<MobilityModel> rx )
{
  CheckInitialized ();

  if ( tx == nullptr )
    {
      if ( rx == nullptr )
        {
          m_bellhopCreator->eraseAngles ();
        }
      else
        {
          auto rxLoc = const_cast<WossHelper*> (this)->GetWossLocation ( rx );
          m_bellhopCreator->eraseAngles (woss::BellhopCreator::CCAngles::ALL_LOCATIONS, rxLoc );
        }
    }
  else
    {
      auto txLoc = const_cast<WossHelper*> (this)->GetWossLocation ( tx );

      if ( rx == nullptr )
        {
          m_bellhopCreator->eraseAngles ( txLoc );
        }
      else
        {
          auto rxLoc = const_cast<WossHelper*> (this)->GetWossLocation ( rx );
          m_bellhopCreator->eraseAngles ( txLoc, rxLoc );
        }
    }
}

bool
WossHelper::ImportTransducerAscii (const std::string &typeName, const std::string &fileName)
{
  CheckInitialized ();

  return m_wossTransducerHndl->importValueAscii (typeName, fileName);
}

bool WossHelper::ImportTransducerBinary (const std::string &typeName, const std::string &fileName)
{
  CheckInitialized ();

  return m_wossTransducerHndl->importValueBinary (typeName, fileName);
}

bool
WossHelper::SetCustomTransducer ( const std::string &transducerString, Ptr<MobilityModel> tx, Ptr<MobilityModel> rx)
{
  CheckInitialized ();

  std::string transducerType;
  std::string transTmp = transducerString;
  std::string::size_type tmp;
  double param[5];

  tmp = transTmp.find ("|");
  if (tmp == std::string::npos)
    {
      NS_LOG_ERROR ("separator | not found, string parsed:" << transTmp);
      return false;
    }

  transducerType = transTmp.substr (0, tmp);
  transTmp = transTmp.substr (tmp + 1, transTmp.npos);

  NS_LOG_ERROR ("transducerType: " << transducerType);

  for (int cnt = 0; cnt < 5; ++cnt)
    {
//    double range;
//    double depth;

      tmp = transTmp.find ("|");
      if ((tmp == std::string::npos) && (cnt != 5 - 1))
        {
          NS_LOG_ERROR ("cnt:" << cnt << "; separator | not found, string parsed: " << transTmp);
          return false;
        }

      std::string paramStr = transTmp.substr (0, tmp);
      transTmp = transTmp.substr (tmp + 1, transTmp.npos);
      param[cnt] = std::atof (paramStr.c_str ());

      NS_LOG_DEBUG ("cnt:" << cnt << "; param:" << param[cnt]);
    }

  woss::CustomTransducer customTrasducer = woss::CustomTransducer (transducerType, param[0], param[1], param[2], param[3], param[4]);

  return SetCustomTransducer (customTrasducer, tx, rx);
}


bool
WossHelper::SetCustomTransducer ( const woss::CustomTransducer& type, Ptr<MobilityModel> tx, Ptr<MobilityModel> rx )
{
  CheckInitialized ();

  if ( tx == nullptr )
    {
      if ( rx == nullptr )
        {
          m_bellhopCreator->setCustomTransducer (type);
        }
      else
        {
          auto rxLoc = const_cast<WossHelper*> (this)->GetWossLocation ( rx );
          m_bellhopCreator->setCustomTransducer (type, woss::BellhopCreator::CCTransducer::ALL_LOCATIONS, rxLoc );
        }
    }
  else
    {
      auto txLoc = const_cast<WossHelper*> (this)->GetWossLocation ( tx );

      if ( rx == nullptr )
        {
          m_bellhopCreator->setCustomTransducer ( type, txLoc );
        }
      else
        {
          auto rxLoc = const_cast<WossHelper*> (this)->GetWossLocation ( rx );
          m_bellhopCreator->setCustomTransducer ( type, txLoc, rxLoc );
        }
    }

  return true;
}


woss::CustomTransducer
WossHelper::GetCustomTransducer ( Ptr<MobilityModel> tx, Ptr<MobilityModel> rx ) const
{
  CheckInitialized ();

  if ( tx == nullptr )
    {
      if ( rx == nullptr )
        {
          return m_bellhopCreator->getCustomTransducer ();
        }
      else
        {
          auto rxLoc = const_cast<WossHelper*> (this)->GetWossLocation ( rx );
          return m_bellhopCreator->getCustomTransducer (woss::BellhopCreator::CCTransducer::ALL_LOCATIONS, rxLoc );
        }
    }
  else
    {
      auto txLoc = const_cast<WossHelper*> (this)->GetWossLocation ( tx );

      if ( rx == nullptr )
        {
          return m_bellhopCreator->getCustomTransducer ( txLoc );
        }
      else
        {
          auto rxLoc = const_cast<WossHelper*> (this)->GetWossLocation ( rx );
          return m_bellhopCreator->getCustomTransducer ( txLoc, rxLoc );
        }
    }
}


void
WossHelper::EraseCustomTransducer ( Ptr<MobilityModel> tx, Ptr<MobilityModel> rx )
{
  CheckInitialized ();

  if ( tx == nullptr )
    {
      if ( rx == nullptr )
        {
          m_bellhopCreator->eraseCustomTransducer ();
        }
      else
        {
          auto rxLoc = const_cast<WossHelper*> (this)->GetWossLocation ( rx );
          m_bellhopCreator->eraseCustomTransducer (woss::BellhopCreator::CCTransducer::ALL_LOCATIONS, rxLoc );
        }
    }
  else
    {
      auto txLoc = const_cast<WossHelper*> (this)->GetWossLocation ( tx );

      if ( rx == nullptr )
        {
          m_bellhopCreator->eraseCustomTransducer ( txLoc );
        }
      else
        {
          auto rxLoc = const_cast<WossHelper*> (this)->GetWossLocation ( rx );
          m_bellhopCreator->eraseCustomTransducer ( txLoc, rxLoc );
        }
    }
}


bool
WossHelper::SetCustomAltimetry (const woss::Altimetry& altimetry, const woss::Coord& txCoord, double bearing,
                                double range)
{
  CheckInitialized ();

  return m_wossDbManager->setCustomAltimetry (altimetry, txCoord, bearing, range);
}


std::unique_ptr<woss::Altimetry>
WossHelper::GetCustomAltimetry (const woss::Coord& txCoord, double bearing, double range)
{
  CheckInitialized ();

  return m_wossDbManager->getCustomAltimetry (txCoord, bearing, range);
}


void
WossHelper::EraseCustomAltimetry (const woss::Coord& txCoord, double bearing, double range)
{
  CheckInitialized ();

  m_wossDbManager->eraseCustomAltimetry (txCoord, bearing, range);
}


bool
WossHelper::SetCustomSediment (const woss::Sediment& sediment, const woss::Coord& txCoord, double bearing,
                               double range)
{
  CheckInitialized ();

  return m_wossDbManager->setCustomSediment (sediment, txCoord, bearing, range);
}


bool
WossHelper::SetCustomSediment (const std::string &sedimentString, const woss::Coord& txCoord, double bearing, double range)
{
  NS_LOG_FUNCTION (this);

  CheckInitialized ();

  std::string sedimentType;
  std::string sedimTmp = sedimentString;
  std::string::size_type tmp;
  double param[5];

  tmp = sedimTmp.find ("|");
  if (tmp == std::string::npos)
    {
      NS_LOG_ERROR ("separator | not found, string parsed:" << sedimTmp);
      return false;
    }

  sedimentType = sedimTmp.substr (0, tmp);
  sedimTmp = sedimTmp.substr (tmp + 1, sedimTmp.npos);

  NS_LOG_DEBUG ("sedimentType: " << sedimentType);

  for (int cnt = 0; cnt < 5; ++cnt)
    {
      tmp = sedimTmp.find ("|");
      if ((tmp == std::string::npos) && (cnt != 5 - 1))
        {
          NS_LOG_ERROR ("cnt:" << cnt << "; separator | not found, string parsed: " << sedimTmp);
          return false;
        }

      std::string paramStr = sedimTmp.substr (0, tmp);
      sedimTmp = sedimTmp.substr (tmp + 1, sedimTmp.npos);
      param[cnt] = std::atof (paramStr.c_str ());

      NS_LOG_DEBUG ("cnt:" << cnt << "; param:" << param[cnt]);
    }

  woss::Sediment sediment = woss::Sediment ( sedimentType, param[0], param[1], param[2], param[3], param[4]);

  return m_wossDbManager->setCustomSediment (sediment, txCoord, bearing, range);
}


std::unique_ptr<woss::Sediment>
WossHelper::GetCustomSediment (const woss::Coord& txCoord, double bearing, double range)
{
  CheckInitialized ();

  return m_wossDbManager->getCustomSediment (txCoord, bearing, range);
}


void
WossHelper::EraseCustomSediment (const woss::Coord& txCoord, double bearing, double range)
{
  CheckInitialized ();

  m_wossDbManager->eraseCustomSediment (txCoord, bearing, range);
}


bool
WossHelper::SetCustomSsp (const woss::SSP& ssp, const woss::Coord& txCoord, double bearing,
                          double range, const woss::Time& timeValue)
{
  CheckInitialized ();

  return m_wossDbManager->setCustomSSP ( ssp, txCoord, bearing, range, timeValue);
}


bool
WossHelper::SetCustomSsp (const std::string &sspString, const woss::Coord& txCoord, double bearing,
                          double range, const woss::Time& timeValue)
{
  NS_LOG_FUNCTION (this);

  CheckInitialized ();

  woss::SSP ssp;
  std::string sspTmp = sspString;
  std::string::size_type tmp;

  int totalDepths = 0;

  tmp = sspTmp.find ("|");
  if (tmp == std::string::npos)
    {
      NS_LOG_ERROR ("WossHelper::SetCustomSsp() separator | not found, string parsed:" << sspTmp);
      return false;
    }

  std::string totDepths = sspTmp.substr (0, tmp);
  sspTmp = sspTmp.substr (tmp + 1, sspTmp.npos);
  totalDepths = std::atoi (totDepths.c_str ());

  NS_LOG_DEBUG ("totalDepths: " << totalDepths);

  if (totalDepths <= 0)
    {
      NS_LOG_ERROR ("totalDepths given < 0:" << totalDepths);
      return false;
    }

  for (int cnt = 0; cnt < totalDepths; ++cnt)
    {
      double depth;
      double sspValue;

      tmp = sspTmp.find ("|");
      if (tmp == std::string::npos)
        {
          NS_LOG_ERROR ("WossHelper::SetCustomSsp() cnt:" << cnt << "; separator | not found, string parsed:" << sspTmp);
          return false;
        }

      std::string depthStr = sspTmp.substr (0, tmp);
      sspTmp = sspTmp.substr (tmp + 1, sspTmp.npos);
      depth = std::atof (depthStr.c_str ());

      tmp = sspTmp.find ("|");
      if ((tmp == std::string::npos) && (cnt != totalDepths - 1))
        {
          NS_LOG_ERROR ("cnt:" << cnt << "; separator | not found, string parsed: " << sspTmp);
          return false;
        }

      std::string sspValueStr = sspTmp.substr (0, tmp);
      sspTmp = sspTmp.substr (tmp + 1, sspTmp.npos);
      sspValue = std::atof (sspValueStr.c_str ());

      NS_LOG_DEBUG ("cnt:" << cnt << "; depth:" << depth << "; sspValue:" << sspValue);

      if (sspValue < 0.0)
        {
          NS_LOG_ERROR ("cnt:" << cnt << " sspValue:" << depth << " < 0");
          return false;
        }
      else
        {
          ssp.insertValue (depth, sspValue);
        }
    }

  return m_wossDbManager->setCustomSSP (ssp, txCoord, bearing, range, timeValue);
}


bool
WossHelper::ImportCustomSsp (const std::string &sspFileName, const woss::Coord& txCoord, double bearing, const woss::Time& timeValue)
{
  CheckInitialized ();

  return m_wossDbManager->importCustomSSP (sspFileName, timeValue, txCoord, bearing);
}


std::unique_ptr<woss::SSP>
WossHelper::GetCustomSsp (const woss::Coord& txCoord, double bearing,
                          double range, const woss::Time& timeValue)
{
  CheckInitialized ();

  return m_wossDbManager->getCustomSSP (txCoord, bearing, range, timeValue);
}


void
WossHelper::EraseCustomSsp (const woss::Coord& txCoord, double bearing,
                            double range, const woss::Time& timeValue)
{
  CheckInitialized ();

  m_wossDbManager->eraseCustomSSP (txCoord, bearing, range, timeValue);
}


bool
WossHelper::SetCustomBathymetry (const woss::Bathymetry& bathymetry, const woss::Coord& txCoord,
                                 double bearing,  double range)
{
  CheckInitialized ();

  return m_wossDbManager->setCustomBathymetry (bathymetry, txCoord, bearing, range);
}


bool
WossHelper::SetCustomBathymetry (const std::string &bathyLine, const woss::Coord& txCoord, double bearing)
{
  NS_LOG_FUNCTION (this);

  CheckInitialized ();

  std::string bathyTmp = bathyLine;
  std::string::size_type tmp;

  int totalRanges = 0;

  NS_LOG_DEBUG ("txCoord: " << txCoord << "; bearing: " << bearing);

  tmp = bathyTmp.find ("|");
  if (tmp == std::string::npos)
    {
      NS_LOG_ERROR ("separator | not found, string parsed:" << bathyTmp);
      return false;
    }

  std::string totRanges = bathyTmp.substr (0, tmp);
  bathyTmp = bathyTmp.substr (tmp + 1, bathyTmp.npos);
  totalRanges = std::atoi (totRanges.c_str ());

  NS_LOG_DEBUG ("totalRanges: " << totalRanges);

  if (totalRanges <= 0)
    {
      NS_LOG_ERROR ("totalRanges given < 0:" << totalRanges);
      return false;
    }

  for (int cnt = 0; cnt < totalRanges; ++cnt)
    {
      double range;
      double depth;

      tmp = bathyTmp.find ("|");
      if (tmp == std::string::npos)
        {
          NS_LOG_ERROR ("cnt:" << cnt << "; separator | not found, string parsed:" << bathyTmp);
          return false;
        }

      std::string rangeStr = bathyTmp.substr (0, tmp);
      bathyTmp = bathyTmp.substr (tmp + 1, bathyTmp.npos);
      range = std::atof (rangeStr.c_str ());

      tmp = bathyTmp.find ("|");
      if ((tmp == std::string::npos) && (cnt != totalRanges - 1))
        {
          NS_LOG_ERROR ("cnt:" << cnt << "; separator | not found, string parsed: " << bathyTmp);
          return false;
        }

      std::string depthStr = bathyTmp.substr (0, tmp);
      bathyTmp = bathyTmp.substr (tmp + 1, bathyTmp.npos);
      depth = std::atof (depthStr.c_str ());

      NS_LOG_DEBUG ("cnt:" << cnt << "; range:" << range << "; depth:" << depth);

      if (depth < 0.0)
        {
          NS_LOG_ERROR ("cnt:" << cnt << " depth:" << depth << " < 0");
          return false;
        }
      else
        {
          m_wossDbManager->setCustomBathymetry (depth, txCoord, bearing, range);
        }
    }

  return true;
}


bool
WossHelper::ImportCustomBathymetry (const std::string &bathyFile, const woss::Coord& txCoord, double bearing)
{
  CheckInitialized ();

  return m_wossDbManager->importCustomBathymetry (bathyFile, txCoord, bearing);
}


woss::Bathymetry
WossHelper::GetCustomBathymetry (const woss::Coord& txCoord, double bearing, double range)
{
  CheckInitialized ();

  return m_wossDbManager->getCustomBathymetry (txCoord, bearing, range);
}


void
WossHelper::EraseCustomBathymetry (const woss::Coord& txCoord, double bearing, double range)
{
  CheckInitialized ();

  m_wossDbManager->eraseCustomBathymetry (txCoord, bearing, range);
}


bool
WossHelper::CreateDirectory (const std::string& path)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (path.size () > 0);

  int retValue = -1;
  static std::stringstream strOut;

  strOut << "mkdir -p " << path;

  std::string command = strOut.str ();
  strOut.str ("");

  NS_LOG_DEBUG ("WossHelper::CreateDirectory () command = " << command);

  if (system (nullptr))
    {
      retValue = system (command.c_str ());
    }

  return ( retValue == 0 );
}


NS_OBJECT_ENSURE_REGISTERED (WossHelper);

TypeId
WossHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::WossHelper")
    .SetParent<Object> ()
    .SetGroupName ("Woss")
    .AddConstructor<WossHelper> ()
    .AddAttribute ("WossRandomGenStream",
                   "int stream value for WOSS Uniform Random Generator",
                   IntegerValue (-1),
                   MakeIntegerAccessor (&WossHelper::m_wossRandomGenStream),
                   MakeIntegerChecker<int64_t> () )
    .AddAttribute ("ResDbCreatorDebug",
                   "A boolean that enables or disables the debug screen output of chosen ResDbCreator",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_resDbCreatorDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("ResDbDebug",
                   "A boolean that enables or disables the debug screen output of chosen ResDb",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_resDbDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("ResDbUseBinary",
                   "If true the ResDb will use its binary version, the ASCII one otherwise",
                   BooleanValue (true),
                   MakeBooleanAccessor (&WossHelper::m_resDbUseBinary),
                   MakeBooleanChecker () )
    .AddAttribute ("ResDbUseTimeArr",
                   "if true the ResDb will store the computed channels in their TimeArr form, in Pressure otherwise",
                   BooleanValue (true),
                   MakeBooleanAccessor (&WossHelper::m_resDbUseTimeArr),
                   MakeBooleanChecker () )
    .AddAttribute ("ResDbSpaceSampling",
                   "ResDb will use the given double as a space sampling radius",
                   DoubleValue (WH_SPACE_SAMPLING_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_resDbSpaceSampling),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("ResDbFilePath",
                   "ResDb will write/read from this path (full path required)",
                   StringValue (WH_STRING_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_resDbFilePath),
                   MakeStringChecker () )
    .AddAttribute ("ResDbFileName",
                   "ResDb will write/read from this file. \
                   In scenarios with HIGH mobility, the ResDb shouldn't be used in order to avoid memory exhaustion.",
                   StringValue (WH_STRING_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_resDbFileName),
                   MakeStringChecker () )
#if defined (WOSS_NETCDF_SUPPORT)
    .AddAttribute ("SedimentDbCreatorDebug",
                   "A boolean that enables or disables the debug screen output of Sediment Db Creator",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_sedimDbCreatorDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("SedimentDbDebug",
                   "A boolean that enables or disables the debug screen output of Sediment Db",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_sedimDbDebug),
                   MakeBooleanChecker () )
#if defined (WOSS_NETCDF4_SUPPORT)
    .AddAttribute ("SedimentDbDeck41DbType",
                   "SSP WOA Db Type: 0 = V1 Format Db, 1 = V2 Format Db",
                   IntegerValue (WH_SEDIMENT_DECK41_FORMAT_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_sedimDbDeck41DbType),
                   MakeIntegerChecker<int> (WH_SEDIMENT_DECK41_FORMAT_MIN, WH_SEDIMENT_DECK41_FORMAT_MAX ) )
#endif // defined (WOSS_NETCDF4_SUPPORT)
    .AddAttribute ("SedimDbCoordFilePath",
                   "Sediment Db will read the coordinates database from this file (full path required)",
                   StringValue (WH_STRING_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_sedimDbCoordFilePath),
                   MakeStringChecker () )
    .AddAttribute ("SedimDbMarsdenFilePath",
                   "Sediment Db will read the marsden square database from this file (full path required)",
                   StringValue (WH_STRING_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_sedimDbMarsdenFilePath),
                   MakeStringChecker () )
    .AddAttribute ("SedimDbMarsdenOneFilePath",
                   "Sediment Db will read the marsden one square from this file (full path required)",
                   StringValue (WH_STRING_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_sedimDbMarsdenOneFilePath),
                   MakeStringChecker () )
    .AddAttribute ("SspDbCreatorDebug",
                   "A boolean that enables or disables the debug screen output of SSP Db Creator",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_sspDbCreatorDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("SspDbDebug",
                   "A boolean that enables or disables the debug screen output of SSP Db",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_sspDbDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("SspDbCoordFilePath",
                   "SSP Db will read the WOA database from this file (full path required)",
                   StringValue (WH_STRING_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_sspDbFilePath),
                   MakeStringChecker () )
#if defined (WOSS_NETCDF4_SUPPORT)
    .AddAttribute ("SspDbWoaDbType",
                   "SSP WOA Db Type: 0 = 2005 Format Db, 1 = 2013 Format Db",
                   IntegerValue (WH_WOA_DB_TYPE_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_sspWoaDbType),
                   MakeIntegerChecker<int> (WH_WOA_DB_TYPE_MIN, WH_WOA_DB_TYPE_MAX) )
#endif // defined (WOSS_NETCDF4_SUPPORT)
    .AddAttribute ("BathyDbCreatorDebug",
                   "A boolean that enables or disables the debug screen output of Bathymetry Db Creator",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_bathyDbCreatorDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("BathyDbDebug",
                   "A boolean that enables or disables the debug screen output of Bathymetry Db",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_bathyDbDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("BathyDbGebcoFormat",
                   "Sets up the the GEBCO database format: 0 = 1D one minute, 1 = 1D 30 seconds, 2 = 2D one minute, 3 = 2D 30 seconds, 4 = 2D 15 seconds",
                   IntegerValue (WH_GEBCO_FORMAT_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_bathyDbGebcoFormat),
                   MakeIntegerChecker<int> (WH_GEBCO_FORMAT_MIN, WH_GEBCO_FORMAT_MAX) )
    .AddAttribute ("BathyDbCoordFilePath",
                   "Bathymetry Db will read the GEBCO database from this file (full path required)",
                   StringValue (WH_STRING_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_bathyDbFilePath),
                   MakeStringChecker () )
#endif // defined (WOSS_NETCDF_SUPPORT)
    .AddAttribute ("WossDbManagerDebug",
                   "A boolean that enables or disables the debug screen output of WossDbManager",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_wossDbManagerDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("WossCreatorDebug",
                   "A boolean that enables or disables the debug screen output of WossCreator",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_wossCreatorDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("WossDebug",
                   "A boolean that enables or disables the debug screen output of created Woss objects",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_wossDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("WossCleanWorkDir",
                   "A boolean that enables or disables the removal of the working dir at the end of the simulation",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_wossClearWorkDir),
                   MakeBooleanChecker () )
    .AddAttribute ("WossEvolutionTimeQuantum",
                   "the created WOSS objects will use the given double as a time evolution quantum [s]. Use -1.0 to disable the feature",
                   DoubleValue (WH_TIME_EVOLUTION_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_evolutionTimeQuantum),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossTotalRuns",
                   "the created WOSS objects will run the simulation for the given uint value",
                   IntegerValue (WH_TOTAL_RUNS_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_totalRuns),
                   MakeIntegerChecker<int> () )
    .AddAttribute ("WossFrequencyStep",
                   "the created WOSS objects will run the simulation at each frequency step [Hz]. Use 0.0 to disable the feature",
                   DoubleValue (WH_FREQUENCY_STEP_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_frequencyStep),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossTotalRangeSteps",
                   "the created WOSS objects will divide each range by the given value",
                   IntegerValue (WH_TOTAL_RANGE_STEPS_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_totalRangeSteps),
                   MakeIntegerChecker<int> () )
    .AddAttribute ("WossTxMinDepthOffset",
                   "the created WOSS objects will create a number of WossTotalTransmitters equally spaced in range |WossTxMaxDepthOffset-WossTxMinDepthOffset|",
                   DoubleValue (WH_TX_MIN_DEPTH_OFFSET_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_txMinDepthOffset),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossTxMaxDepthOffset",
                   "the created WOSS objects will create a number of WossTotalTransmitters equally spaced in range |WossTxMaxDepthOffset-WossTxMinDepthOffset|",
                   DoubleValue (WH_TX_MAX_DEPTH_OFFSET_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_txMaxDepthOffset),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossTotalTransmitters",
                   "the created WOSS objects will create the given number of transmitter",
                   IntegerValue (WH_TOTAL_TRANSMITTERS_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_totalTransmitters),
                   MakeIntegerChecker<int> () )
    .AddAttribute ("WossTotalRxDepths",
                   "the created WOSS objects will place a receiver equally spaced in range |WossRxMaxDepthOffset - WossRxMinDepthOffset|",
                   IntegerValue (WH_TOTAL_RX_DEPTHS_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_totalRxDepths),
                   MakeIntegerChecker<uint32_t> () )
    .AddAttribute ("WossRxMinDepthOffset",
                   "the created WOSS objects will create a number of WossTotalRxDepths rx equally spaced in range |WossRxMaxDepthOffset-WossRxMinDepthOffset|",
                   DoubleValue (WH_RX_MIN_DEPTH_OFFSET_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_rxMinDepthOffset),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossRxMaxDepthOffset",
                   "the created WOSS objects will create a number of WossTotalRxDepths rx equally spaced in range |WossRxMaxDepthOffset-WossRxMinDepthOffset|",
                   DoubleValue (WH_RX_MAX_DEPTH_OFFSET_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_rxMaxDepthOffset),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossTotalRxRanges",
                   "the created WOSS objects will place a receiver equally spaced in range |WossRxMaxRangeOffset - WossRxMinRangeOffset|",
                   IntegerValue (WH_TOTAL_RX_RANGES_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_totalRxRanges),
                   MakeIntegerChecker<int> () )
    .AddAttribute ("WossRxMinRangeOffset",
                   "the created WOSS objects will create a number of WossTotalRxRanges rx equally spaced in range |WossRxMaxRangeOffset-WossRxMinRangeOffset|",
                   DoubleValue (WH_RX_MIN_RANGE_OFFSET_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_rxMinRangeOffset),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossRxMaxRangeOffset",
                   "the created WOSS objects will create a number of WossTotalRxRanges rx equally spaced in range |WossRxMaxRangeOffset-WossRxMinRangeOffset|",
                   DoubleValue (WH_RX_MAX_RANGE_OFFSET_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_rxMaxRangeOffset),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossTotalRays",
                   "the created WOSS objects will run bellhop with the given number of rays. 0 for auto",
                   IntegerValue (WH_TOTAL_RAYS_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_totalRays),
                   MakeIntegerChecker<int> () )
    .AddAttribute ("WossMinAngle",
                   "the created WOSS objects will run bellhop with given starting launch angle",
                   DoubleValue (WH_MIN_ANGLE_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_minAngle),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossMaxAngle",
                   "the created WOSS objects will run bellhop with given ending launch angle",
                   DoubleValue (WH_MAX_ANGLE_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_maxAngle),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossSspDepthPrecision",
                   "the created WOSS objects will have the given SSP depth precision [m]",
                   DoubleValue (WH_SSP_DEPTH_PRECISION_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_sspDepthPrecision),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossSspNormalizedDepthSteps",
                   "the created WOSS objects will have all normalized SSP with this number of depths",
                   DoubleValue (WH_NORMALIZED_SSP_DEPTH_STEPS_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_normalizedSspDepthSteps),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossWorkDirPath",
                   "the created WOSS objects will create its temporary files in this directory (full path)",
                   StringValue (WH_WORK_PATH_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_workDirPath),
                   MakeStringChecker () )
    .AddAttribute ("WossBellhopPath",
                   "the full path of bellhop executable. Leave blank if already in environment search path",
                   StringValue (WH_STRING_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_bellhopPath),
                   MakeStringChecker () )
    .AddAttribute ("WossBellhopMode",
                   "the bellhop running mode. see WOSS doxygen for supported modes",
                   StringValue (WH_BELLHOP_MODE_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_bellhopMode),
                   MakeStringChecker () )
    .AddAttribute ("WossBellhopBeamOptions",
                   "the bellhop beam options. see WOSS doxygen for supported modes",
                   StringValue (WH_BEAM_OPTIONS_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_bellhopBeamOptions),
                   MakeStringChecker () )
    .AddAttribute ("WossBellhopBathyType",
                   "the bellhop bathymetry type. see WOSS doxygen for supported modes",
                   StringValue (WH_BATHYMETRY_TYPE_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_bellhopBathyType),
                   MakeStringChecker () )
    .AddAttribute ("WossBathyWriteMethod",
                   "the woss bathymetry write method. see WOSS doxygen for supported modes",
                   StringValue (WH_BATHYMETRY_METHOD_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_bathyMethod),
                   MakeStringChecker () )
    .AddAttribute ("WossBellhopAltimType",
                   "the bellhop altimetry type. see WOSS doxygen for supported modes",
                   StringValue (WH_ALTIMETRY_TYPE_DEFAULT),
                   MakeStringAccessor (&WossHelper::m_bellhopAltimType),
                   MakeStringChecker () )
    .AddAttribute ("WossSimTime",
                   "the created Woss objects will use the given SimTime for time evolution purposes",
                   WossSimTimeValue (WossSimTime ()),
                   MakeWossSimTimeAccessor (&WossHelper::m_simTime),
                   MakeWossSimTimeChecker () )
    .AddAttribute ("WossBellhopArrSyntax", 
                   "Syntax to be used during bellhop arr file parsing, range [0-2]",
                   IntegerValue (WH_BELLHOP_ARR_SYNTAX_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_bellhopArrSyntax),
                   MakeIntegerChecker<int> (WH_BELLHOP_ARR_SYNTAX_MIN, WH_BELLHOP_ARR_SYNTAX_MAX) )
    .AddAttribute ("WossBellhopShdSyntax", 
                   "Syntax to be used during bellhop arr file parsing, range [0-1]",
                   IntegerValue (WH_BELLHOP_SHD_SYNTAX_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_bellhopShdSyntax),
                   MakeIntegerChecker<int> (WH_BELLHOP_SHD_SYNTAX_MIN, WH_BELLHOP_SHD_SYNTAX_MAX) )
    .AddAttribute ("WossManagerDebug",
                   "A boolean that enables or disables the debug screen output of WossManager object",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_wossManagerDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("WossManagerTimeEvoActive",
                   "A boolean that enables or disables the time evolution feature",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WossHelper::m_isTimeEvolutionActive),
                   MakeBooleanChecker () )
    .AddAttribute ("WossManagerTotalThreads",
                   "WossManager will try to run the given maximum of parallel Woss object. 0 for auto",
                   IntegerValue (WH_CONCURRENT_THREADS_DEFAULT),
                   MakeIntegerAccessor (&WossHelper::m_concurrentThreads),
                   MakeIntegerChecker<int> () )
    .AddAttribute ("WossManagerSpaceSampling",
                   "WossManager will use the given radius [m] to sample the surface. 0 to disable the feature",
                   DoubleValue (WH_SPACE_SAMPLING_DEFAULT),
                   MakeDoubleAccessor (&WossHelper::m_wossManagerSpaceSampling),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossManagerUseMultithread",
                   "A boolean that enables or disables the multithread feature",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WossHelper::m_wossManagerUseMultiThread),
                   MakeBooleanChecker () )
    .AddAttribute ("WossTransHandlerDebug",
                   "A boolean that enables or disables the TransducerHandler's debug screen output",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_wossTransducerHndlDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("WossControllerDebug",
                   "A boolean that enables or disables the WossController's debug screen output",
                   BooleanValue (WH_DEBUG_DEFAULT),
                   MakeBooleanAccessor (&WossHelper::m_wossControllerDebug),
                   MakeBooleanChecker () )
    .AddAttribute ("WossBoxDepth",
                   "The maximum depth to which Bellhop rays will be traced",
                   DoubleValue (WH_BOX_DEPTH),
                   MakeDoubleAccessor (&WossHelper::m_boxDepth),
                   MakeDoubleChecker<double> () )
    .AddAttribute ("WossBoxRange",
                   "The maximum range to which Bellhop rays will be traced",
                   DoubleValue (WH_BOX_RANGE),
                   MakeDoubleAccessor (&WossHelper::m_boxRange),
                   MakeDoubleChecker<double> () )
  ;

  return tid;
}


} // namespace ns3

#endif /* NS3_WOSS_SUPPORT */

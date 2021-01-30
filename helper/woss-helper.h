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

#ifndef WOSS_HELPER_H
#define WOSS_HELPER_H


#include <ssp-definitions.h>
#include <sediment-definitions.h>
#include <altimetry-definitions.h>
#include <pressure-definitions.h>
#include <time-arrival-definitions.h>
#include <transducer-definitions.h>
#include <transducer-handler.h>
#include <res-pressure-bin-db-creator.h>
#include <res-pressure-txt-db-creator.h>
#include <res-time-arr-bin-db-creator.h>
#include <res-time-arr-txt-db-creator.h>
#if defined (WOSS_NETCDF_SUPPORT)
#include <bathymetry-gebco-db-creator.h>
#include <sediment-deck41-db-creator.h>
#include <ssp-woa2005-db-creator.h>
#endif // defined (WOSS_NETCDF_SUPPORT)
#include <bellhop-creator.h>
#include <woss-manager-simple.h>
#include <woss-controller.h>

#include <ns3/woss-random-generator.h>
#include <ns3/woss-location.h>
#include <ns3/woss-time-reference.h>
#include <ns3/woss-prop-model.h>


#define WOSS_HELPER_ALL_COORDS(class ) woss::WossDbManager::CC ## class::DB_CDATA_ALL_OUTER_KEYS //!< WOSS custom container special db key valid for all geographic coordinates.
#define WOSS_HELPER_ALL_RANGES(class ) woss::WossDbManager::CC ## class::DB_CDATA_ALL_MEDIUM_KEYS //!< WOSS custom container special db key valid for all ranges.
#define WOSS_HELPER_ALL_BEARINGS(class ) woss::WossDbManager::CC ## class::DB_CDATA_ALL_INNER_KEYS //!< WOSS custom container special db key valid for all bearing.
#define WOSS_HELPER_ALL_TIMES(class ) woss::WossDbManager::CC ## class::DB_CDATA_ALL_TIME_KEYS //!< WOSS custom container special db key valid for all simulation times.


namespace ns3 {

/**
 * \ingroup WOSS
 *
 * wrapper class for the attribute system
 */
class WossSimTime : public woss::SimTime
{
};

/**
 * \ingroup WOSS
 *
 * \brief Helper class that configures the WOSS framework
 *
 * The WOSS custom databases (altimetry, bathymetry, sediment and SSP) allow the user to define the validity of the input value over complex polygons.
 * The geometry is based on:
 * 1) the coordinates [dec degrees] ==> they must be valid and they define a starting point on the earth surface
 * 2) the range [m] ==> the range define a positive distance from the generating coordinates
 * 3) the bearing [radians] ==> it defines the bearing of the resulting point/line
 * 4) the time value ==> defines its time validity for time evolution purposes.
 * Each and every key has a special value that extend the validity for all possible key values.
 *
 * Examples:
 * a) single point: 1) valid coordinates, 2) valid range, 3) valid bearing
 * b) whole surface: 1) special value, 2) special value, 3) special value
 * c) disc surface: 1) valid coordinates, 2) valid range, 3) special value
 * d) line: 1) valid coordinates, 2) special range, 3) valid value
 *
 * The user should try to quantize the geometry as much as possible. The WOSS framework will always approximate
 * to the closest coordinates available.
 *
 * Transducer related features (i.e. full transducer type and transducer max/min vertical angles) can be defined for:
 * 1) transmitter mobility model, NULL means all transmitters
 * 2) receiver mobility model, NULL means all receivers
 *
 * Examples:
 * a) single tx-rx pair: 1) valid transmitter mobility model, 2) valid transmitter mobility model
 * b) single tx-all rx: 1) valid transmitter mobility model, 2) NULL receiver
 * c) all tx-rx pairs: 1) NULL, 2) NULL
 *
 */
class WossHelper : public Object
{

public:

  WossHelper (); //!< Default constructor.

  virtual ~WossHelper (); //!< Default destructor.

  /**
   * see .c file for info on attributes
   */
  static TypeId GetTypeId (void);

  /**
   * \param wossPropModel the WossPropModel instance. the WossHelper class will
   * automatically initialize all the needed connections.
   */
  void Initialize (Ptr<WossPropModel> wossPropModel);


  /**
   * Bounds a woss::CustomAngles object to a node pair (transmitter - receiver). A woss::CustomAngles defines the minimum
   * and maximum vertical angles to be used by the acoustic channel simulator
   * \param angles the woss::CustomAngles reference
   * \param tx the transmitter
   * \param rx the receiver
   * \returns true if successful, false otherwise
   */
  bool SetAngles ( const woss::CustomAngles &angles, Ptr<MobilityModel> tx = NULL, Ptr<MobilityModel> rx = NULL );

  /**
   * Converts a ::std::string into a woss::CustomAngles object and bounds it to a node pair
   * \param angleString decimal degrees syntax = min|max
   * \param angles the woss::CustomAngles reference
   * \param tx the transmitter
   * \param rx the receiver
   * \returns true if successful, false otherwise
   */
  bool SetAngles (const ::std::string &angleString, Ptr<MobilityModel> tx = NULL, Ptr<MobilityModel> rx = NULL );

  /**
   * Returns the woss::CustomAngles associated to the given transmitter and receiver pair.
   * \param tx the transmitter
   * \param rx the receiver
   * \returns a valid woss::CustomAngles if the tx-rx pair is found, not valid object otherwise.
   */
  woss::CustomAngles GetAngles ( Ptr<MobilityModel> tx = NULL, Ptr<MobilityModel> rx = NULL ) const;

  /**
   * Erases the woss::CustomAngles associated to the given transmitter and receiver pair.
   * \param tx the transmitter
   * \param rx the receiver
   */
  void EraseAngles ( Ptr<MobilityModel> tx = NULL, Ptr<MobilityModel> rx = NULL );

  /**
   * Imports a custom woss::Transducer (described by a file in in ASCII format) and bounds it to its type name.
   * A woss::Transducer allows the simulation of
   * a) transmitter and receiver power
   * b) Sound Pressure Level
   * c) vertical beam pattern
   * The type name is used by the WOSS framework as a lookup name
   * \param typeName transducer type name
   * \param fileName path to file
   * \returns true if successful, false otherwise
   */
  bool ImportTransducerAscii (const ::std::string &typeName, const std::string &fileName);

  /**
   * Imports a custom woss::Transducer (described by a file in binary format) and bounds it to its type name.
   * \param typeName transducer type name
   * \param fileName path to file
   * \returns true if successful, false otherwise
   */
  bool ImportTransducerBinary (const ::std::string &typeName, const std::string &fileName);

  /**
   * Bounds a woss::CustomTransducer to a node pair (transmitter - receiver)
   * \param type woss::CustomTransducer object
   * \param tx the transmitter
   * \param rx the receiver
   * \returns true if successful, false otherwise
   */
  bool SetCustomTransducer ( const woss::CustomTransducer &type, Ptr<MobilityModel> tx = NULL, Ptr<MobilityModel> rx = NULL );

  /**
   * Converts a ::std::string into a woss::CustomTransducer and then bounds it to a node pair.
   * A CustomTransducer allows the user to define:
   * a) the woss::Transducer's type-name to be used
   * b) the beam pattern bearing orientation offset
   * c) the beam pattern vertical angle offset
   * d) the beam pattern horizontal angle offset
   * e) beam pattern value modifiers
   * \param transducerString type-name|bearing|vertical_rotation|horizontal_rotation|multiplier|additive
   * \param tx the transmitter
   * \param rx the receiver
   * \returns true if successful, false otherwise
   */
  bool SetCustomTransducer ( const ::std::string &transducerString, Ptr<MobilityModel> tx = NULL, Ptr<MobilityModel> rx = NULL );

  /**
   * Returns the woss::CustomTransducer associated to the given transmitter and receiver pair.
   * \param tx the transmitter
   * \param rx the receiver
   * \returns a valid woss::CustomTransducer if the tx-rx pair is found, not valid object otherwise.
   */
  woss::CustomTransducer GetCustomTransducer ( Ptr<MobilityModel> tx = NULL, Ptr<MobilityModel> rx = NULL ) const;

  /**
   * Erases the woss::CustomTransducer associated to the given transmitter and receiver pair.
   * \param tx the transmitter
   * \param rx the receiver
   */
  void EraseCustomTransducer ( Ptr<MobilityModel> tx = NULL, Ptr<MobilityModel> rx = NULL );

  /**
   * Imports a custom woss::Altimetry and binds it to the designed space geometry.
   * A woss::Altimetry allows the user to simulate a custom sea surface wave function.
   * @see WossHelper for help on how to define a valid geometry
   * \param altimetry a pointer the input woss::Altimetry
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   * \returns true if successful, false otherwise
   */
  bool SetCustomAltimetry ( woss::Altimetry* const altimetry, const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Altimetry),
                            double bearing = WOSS_HELPER_ALL_BEARINGS (Altimetry),
                            double range = WOSS_HELPER_ALL_RANGES (Altimetry) );

  /**
   * Returns the woss::Altimetry associated to the given input geometry.
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   * \returns a valid woss::Altimetry pointer if found, NULL otherwise
   */
  woss::Altimetry* GetCustomAltimetry ( const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Altimetry),
                                        double bearing = WOSS_HELPER_ALL_BEARINGS (Altimetry),
                                        double range = WOSS_HELPER_ALL_RANGES (Altimetry) );

  /**
   * Erases the woss::Altimetry associated to the given input geometry.
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   */
  void EraseCustomAltimetry ( const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Altimetry),
                              double bearing = WOSS_HELPER_ALL_BEARINGS (Altimetry),
                              double range = WOSS_HELPER_ALL_RANGES (Altimetry) );

  /**
   * Imports custom woss::Sediment and binds it to the designed space geometry.
   * A woss::Sediment allows the user to simulate a sea bottom with custom acoustic parameters
   * @see WossHelper for help on how to define a valid geometry
   */
  bool SetCustomSediment ( woss::Sediment* const sediment, const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Sediment),
                           double bearing = WOSS_HELPER_ALL_BEARINGS (Sediment),
                           double range = WOSS_HELPER_ALL_RANGES (Sediment) );

  /**
   * Converts the input :std::string into a woss::Sediment object and binds it to the designed geometry.
   * \param sediment_string SSP in string format. syntax:
   *  - Sed_typename|SSP[m/s]|vel_c[m/s]|vel_s[m/s]|dens[g/cm^3]|att_c[db/wavelength]|att_s[db/wavelength]|bottom_depth[m]
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   * \returns true if successful, false otherwise
   */
  bool SetCustomSediment ( const ::std::string &sediment_string, const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Sediment),
                           double bearing = WOSS_HELPER_ALL_BEARINGS (Sediment),
                           double range = WOSS_HELPER_ALL_RANGES (Sediment) );

  /**
   * Returns a pointer to the woss::Sediment associated to the given input geometry.
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   * \returns a valid woss::Sediment pointer if found, NULL otherwise
   */
  woss::Sediment* GetCustomSediment ( const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Sediment),
                                      double bearing = WOSS_HELPER_ALL_BEARINGS (Sediment),
                                      double range = WOSS_HELPER_ALL_RANGES (Sediment) );

  /**
   * Erases the woss::Sediment associated to the given input geometry.
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   */
  void EraseCustomSediment ( const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Sediment),
                             double bearing = WOSS_HELPER_ALL_BEARINGS (Sediment),
                             double range = WOSS_HELPER_ALL_RANGES (Sediment) );

  /**
   * Imports custom woss::SSP and binds it to the designed geometry and simulation time validity
   * A woss::SSP allows the user to simulate different sound speed functions which in turn impacts on the
   * acoustic propagation.
   * The @param time_value allows the user to simulate a woss::SSP with time evolution.
   * A SSP usually evolves during the day (day/night) and during the months (seasonal evolution)
   * @see WossHelper for help on how to define a valid geometry
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   * \param time_value the geometry time evolution step
   * \returns true if successful, false otherwise
   */
  bool SetCustomSsp ( woss::SSP* const ssp, const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (SSP),
                      double bearing = WOSS_HELPER_ALL_BEARINGS (SSP),
                      double range = WOSS_HELPER_ALL_RANGES (SSP),
                      const woss::Time& time_value = WOSS_HELPER_ALL_TIMES (SSP) );

  /**
   * Converts the input ::std::string into a custom woss::SSP object and assign it to the designed geometry
   * and simulation time validity.
   * \param sspString SSP in string format. syntax:
   *  - tot_values|depth_1[m]|SSP[m/s]|....|depth_final[m]|SSP_final[m/s]
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   * \param time_value the geometry time evolution step
   * \returns true if successful, false otherwise
   */
  bool SetCustomSsp ( const ::std::string &sspString, const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (SSP),
                      double bearing = WOSS_HELPER_ALL_BEARINGS (SSP),
                      double range = WOSS_HELPER_ALL_RANGES (SSP),
                      const woss::Time& time_value = WOSS_HELPER_ALL_TIMES (SSP) );

  /**
   * Reads the input file path and converts it to a woss::SSP and assign it to the input time-space geometry
   * \param sspFileName see WOSS manual for file syntax
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   * \param time_value the geometry time evolution step
   */
  bool ImportCustomSsp ( const ::std::string &sspFileName, const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (SSP),
                         double bearing = WOSS_HELPER_ALL_BEARINGS (SSP),
                         const woss::Time& time_value = WOSS_HELPER_ALL_TIMES (SSP) );

  /**
   * Returns a pointer to the woss::SSP associated to the given input geometry.
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   * \param time_value the geometry time evolution step
   * \returns a valid woss::SSP pointer if found, NULL otherwise
   */
  woss::SSP* GetCustomSsp ( const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (SSP),
                            double bearing = WOSS_HELPER_ALL_BEARINGS (SSP),
                            double range = WOSS_HELPER_ALL_RANGES (SSP),
                            const woss::Time& time_value = WOSS_HELPER_ALL_TIMES (SSP) );

  /**
   * Erases the woss::SSP associated to the given input geometry.
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   * \param time_value the geometry time evolution step
   */
  void EraseCustomSsp ( const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (SSP),
                        double bearing = WOSS_HELPER_ALL_BEARINGS (SSP),
                        double range = WOSS_HELPER_ALL_RANGES (SSP),
                        const woss::Time& time_value = WOSS_HELPER_ALL_TIMES (SSP) );

  /**
   * Imports custom woss::Bathymetry and binds it to the designed geometry
   * a woss::Bathymetry allows the user to design the geometry and the acoustic characteristics
   * of the sea bottom.
   * @see WossHelper for help on how to define a valid geometry
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   * \returns true if successful, false otherwise
   */
  bool SetCustomBathymetry ( woss::Bathymetry* const bathymetry, const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Bathymetry),
                             double bearing = WOSS_HELPER_ALL_BEARINGS (Bathymetry),
                             double range = WOSS_HELPER_ALL_RANGES (Bathymetry) );

  /**
   * reads the input ::std::string and converts it into a custom woss::Bathymetry object and binds it to
   * the input geometry
   * \param bathyLine bathymetry in string format. syntax:
   *  - tot_ranges|range_1[m]|dept_1[n]|....|range_final[m]|depth_final[m]
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \returns true if successful, false otherwise
   */
  bool SetCustomBathymetry ( const ::std::string &bathyLine, const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Bathymetry),
                             double bearing = WOSS_HELPER_ALL_BEARINGS (Bathymetry) );

  /**
   * Reads the given file and converts it to a custom woss::Bathymetry object, binded to the given
   * geometry
   * \param bathyFile see WOSS manual for file syntax
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \returns true if successful, false otherwise
   */
  bool ImportCustomBathymetry ( const ::std::string &bathyFile, const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Bathymetry),
                                double bearing = WOSS_HELPER_ALL_BEARINGS (Bathymetry) );

  /**
   * Returns a pointer to the woss::Bathymetry associated to the given input geometry.
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   * \returns a valid woss::Bathymetry pointer if found, NULL otherwise
   */
  woss::Bathymetry* GetCustomBathymetry ( const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Bathymetry),
                                          double bearing = WOSS_HELPER_ALL_BEARINGS (Bathymetry),
                                          double range = WOSS_HELPER_ALL_RANGES (Bathymetry) );

  /**
   * Erases the woss::Bathymetry associated to the given input geometry.
   * \param txCoord the geometry originating geographical coordinates
   * \param bearing the geometry bearing [radians]
   * \param range the geometry range
   */
  void EraseCustomBathymetry ( const woss::Coord& txCoord = WOSS_HELPER_ALL_COORDS (Bathymetry),
                               double bearing = WOSS_HELPER_ALL_BEARINGS (Bathymetry),
                               double range = WOSS_HELPER_ALL_RANGES (Bathymetry) );


protected:
  virtual void DoDispose (void); //!< action to be performed during de-initialization

  virtual void DoInitialize (void); //!< action to be performed during initialization

private:
  void CheckInitialized (void) const; //!< Checks if Helper is correctly initialized

  typedef ::std::map< MobilityModel*, WossLocation* > MobLocMap;  //!< ::std::map that associates a ns3::MobilityModel pointer to a ns3::WossLocation pointer.
  typedef MobLocMap::iterator MLMIter; //!< iterator typedef
  typedef MobLocMap::const_iterator MLMCIter; //!< constant iterator typedef
  typedef MobLocMap::reverse_iterator MLMRIter; //!< reverse iterator typedef
  typedef MobLocMap::const_reverse_iterator MLMCRIter; //!< constant reverse iterator typedef

  /**
   * Converts a input ns3::Ptr<MobilityModel> to a naked ns3::WossLocation pointer, in order to manage the current node position within the WOSS framework.
   * \param ptr a ns3::Ptr to a ns3::MobilityModel
   * \returns a valid pointer to a ns3::WossLocation
   */
  WossLocation* GetWossLocation ( Ptr<MobilityModel> ptr );

  /**
   * Clears the MobLocMap object
   */
  void DeleteWossLocationMap (void);

  /**
   * Creates a directory with the input path. The user must have proper authorization
   *
   * \param path the directory path
   * \returns true if successful, false otherwise
   */
  bool CreateDirectory (const ::std::string& path);

  MobLocMap m_locMap; //!< map of all simulated nodes

  woss::SSP m_sspProto; //!< woss::SSP prototype which will be plugged into the WOSS framework.
  woss::Sediment m_sedimentProto; //!< woss::Sediment prototype which will be plugged into the WOSS framework.
  woss::Pressure m_pressureProto; //!< woss::Pressure prototype which will be plugged into the WOSS framework.
  woss::TimeArr m_timeArrProto; //!< woss::TimeArr prototype which will be plugged into the WOSS framework.
  woss::Transducer m_transducerProto; //!< woss::Transducer prototype which will be plugged into the WOSS framework.
  woss::AltimBretschneider m_altimBretProto; //!< woss::AltimBretschneider prototype which will be plugged into the WOSS framework.
  int64_t m_wossRandomGenStream; //!< random generator used by the WOSS framework.
  WossRandomGenerator m_randomGenProto; //!< woss::RandomGenerator prototype which will be plugged into the WOSS framework.
  WossTimeReference m_timeRefProto; //!< woss::TimeReference prototype which will be plugged into the WOSS framework.

  woss::DefHandler *m_defHandler; //!< Definition Handler object allocated by the helper.

  bool m_resDbCreatorDebug; //!< enable/disable the debug prints of the woss results database creator.
  bool m_resDbDebug; //!< enable/disable the debug prints of the woss result databases.
  bool m_resDbUseBinary; //!< set up the woss result database format: binary or textual.
  bool m_resDbUseTimeArr; //!< set up the woss result databases data format: power delay profile or single pressure tap.
  double m_resDbSpaceSampling; //!< set up the woss result database 2D spatial sampling
  ::std::string m_resDbFilePath; //!< set up the woss result database file path
  ::std::string m_resDbFileName; //!< set up the woss result database file name (without path)
  woss::ResPressureBinDbCreator *m_resDbCreatorPressBin; //!< the helper will automatically allocate the desired result database creator based on current configuration.
  woss::ResPressureTxtDbCreator *m_resDbCreatorPressTxt; //!< the helper will automatically allocate the desired result database creator based on current configuration.
  woss::ResTimeArrBinDbCreator *m_resDbCreatorTimeArrBin; //!< the helper will automatically allocate the desired result database creator based on current configuration.
  woss::ResTimeArrTxtDbCreator *m_resDbCreatorTimeArrTxt; //!< the helper will automatically allocate the desired result database creator based on current configuration.

#if defined (WOSS_NETCDF_SUPPORT)
  bool m_sedimDbCreatorDebug; //!< enable/disable the debug prints of the woss sediment database creator.
  bool m_sedimDbDebug; //!< enable/disable the debug prints of the woss sediment database.
#if defined (WOSS_NETCDF4_SUPPORT)
  int m_sedimDbDeck41DbType; //!< set the DECK41 database data format type. 0 = V1 data format, 1 = V2 data format
#endif // defined (WOSS_NETCDF4_SUPPORT)
  ::std::string m_sedimDbCoordFilePath; //!< setup the path of the sediment database indexed by geographical coordinates with decimal degrees resolution
  ::std::string m_sedimDbMarsdenFilePath; //!< setup the path of the sediment database indexed by geographical coordinates with marsden square resolution
  ::std::string m_sedimDbMarsdenOneFilePath; //!< setup the path of the sediment database indexed by geographical coordinates with marsden one square resolution
  woss::SedimDeck41DbCreator *m_sedimDbCreator; //!< the helper will automatically allocate the woss sediment database creator

  bool m_sspDbCreatorDebug; //!< enable/disable the debug prints of the woss SSP database creator.
  bool m_sspDbDebug; //!< enable/disable the debug prints of the woss SSP database.
  ::std::string m_sspDbFilePath; //!< setup the path of the woss monthly SSP database indexed by geographical coordinates
#if defined (WOSS_NETCDF4_SUPPORT)
  int m_sspWoaDbType; //!< WOA SSP Db Type: 0 = 2005 format Db, 1 2013 Format Db
#endif // defined (WOSS_NETCDF_SUPPORT)
  woss::SspWoa2005DbCreator *m_sspDbCreator; //!< the helper will automatically allocate the woss sediment database creator

  bool m_bathyDbCreatorDebug; //!< enable/disable the debug prints of the woss Bathymetry database creator.
  bool m_bathyDbDebug; //!< enable/disable the debug prints of the woss Bathymetry database (GEBCO).
  int m_bathyDbGebcoFormat; //!< setup the the GEBCO database format: 0=1D one minute, 1=1D 30 seconds, 2=2D one minute, 3=2D 30 seconds, 4=2D 15 seconds
  ::std::string m_bathyDbFilePath; //!< setup the path of the woss GEBCO database
  woss::BathyGebcoDbCreator *m_bathyDbCreator; //!< the helper will automatically allocate the woss GEBCO bathymetry db creator
#endif // defined (WOSS_NETCDF_SUPPORT)

  bool m_wossDbManagerDebug; //!< enable/disable the debug prints of the woss DB manager object.

  woss::WossDbManager *m_wossDbManager; //!< the helper will automatically allocate the woss DB manager

  bool m_wossCreatorDebug; //!< enable/disable the debug prints of the woss creator.
  bool m_wossDebug; //!< enable/disable the debug prints of all woss objects
  bool m_wossClearWorkDir; //!< woss object configuration: clear the temporary files created by the underlying channel simulator
  double m_evolutionTimeQuantum;  //!< woss object configuration: evolution time quantum
  int m_totalRuns;  //!< woss object configuration: total number of channel simulator runs
  double m_frequencyStep;  //!< woss object configuration: number of frequency steps
  double m_totalRangeSteps;  //!< woss object configuration: tx - rx range sampling in meters
  double m_txMinDepthOffset;  //!< woss object configuration: transmitter min offset in meters
  double m_txMaxDepthOffset;  //!< woss object configuration: transmitter max offset in meters
  int m_totalTransmitters;  //!< woss object configuration: total number of transmitting sources
  int m_totalRxDepths;  //!< woss object configuration: receiver depth sampling
  double m_rxMinDepthOffset;  //!< woss object configuration: receiver min depth offset in meters
  double m_rxMaxDepthOffset;  //!< woss object configuration: receiver max depth offset in meters
  int m_totalRxRanges;  //!< woss object configuration: receiver range sampling
  double m_rxMinRangeOffset;  //!< woss object configuration: receiver min range offset in meters
  double m_rxMaxRangeOffset;  //!< woss object configuration: receiver max range offset in meters
  double m_totalRays;  //!< woss object configuration: total number of rays (0 = automatic)
  double m_minAngle;  //!< woss object configuration: minimum vertical angle in decimal degrees
  double m_maxAngle;  //!< woss object configuration: maximum vertical angle in decimal degrees
  double m_sspDepthPrecision;  //!< woss object configuration: SSP depth precision in meters
  double m_normalizedSspDepthSteps;  //!< woss object configuration: SSP depth quantization steps
  ::std::string m_workDirPath;  //!< woss object configuration: path of the temporary directory
  ::std::string m_bellhopPath;  //!< woss object configuration: path of the Bellhop executable
  ::std::string m_bellhopMode;  //!< woss object configuration: Bellhop mode string
  ::std::string m_bellhopBeamOptions;  //!< woss object configuration: Bellhop beam option string
  ::std::string m_bellhopBathyType;  //!< woss object configuration: Bellhop bathymetry type string
  ::std::string m_bathyMethod; //!< woss object configuration: Bathymetry write method string
  ::std::string m_bellhopAltimType;  //!< woss object configuration: Bellhop Altimetry type string
  int m_bellhopArrSyntax; //!< woss object configration: Bellhop Arr file syntax to be used during .arr file parsing, range [0,2]
  int m_bellhopShdSyntax; //!< woss object configration: Bellhop Shd file syntax to be used during .shd file parsing, range [0,1]
  WossSimTime m_simTime;  //!< woss object configuration: woss simulation times (start and end sim times)
  woss::BellhopCreator *m_bellhopCreator; //!< the helper will automatically allocate the woss creator
  double m_boxDepth; //!< woss object configuration: maximum depth to trace rays to; deeper rays will be ignored
  double m_boxRange; //!< woss object configuration: maximum range to trace rats to; longer rays will be ignored

  bool m_wossManagerDebug; //!< enable/disable the debug prints of the woss manager.
  bool m_isTimeEvolutionActive; //!< enable/disable the time evolution feature.
  int m_concurrentThreads; //!< number of concurrent threads (0 = auto).
  double m_wossManagerSpaceSampling; //!< woss manager space sampling in meters
  bool m_wossManagerUseMultiThread; //!< enable/disable the multithread feature
  woss::WossManagerSimple<woss::WossManagerResDb> *m_wossManagerSimple; //!<  the helper will automatically allocate the desired woss manager based on current configuration.
  woss::WossManagerSimple<woss::WossManagerResDbMT> *m_wossManagerMulti; //!<  the helper will automatically allocate the desired woss manaeger based on current configuration.

  bool m_wossTransducerHndlDebug; //!< enable/disable the debug prints of the woss transducer handler.
  woss::TransducerHandler *m_wossTransducerHndl; //!< the helper will automatically allocate the woss Transducer handler.

  bool m_wossControllerDebug; //!< enable/disable the debug prints of the woss conteroller
  woss::WossController *m_wossController; //!< the helper will automatically allocate the woss controller.

  bool m_initialized; //!< initialization complete flag.
};

/**
 * syntax: start_day|start_month|start_year|start_hour|start_minutes|start_seconds|end_day|end_month|end_year|end_hours|end_minutes|end_seconds
 * \param os ostream
 * \param simTime WossSimTime
 * \returns ostream
 */
::std::ostream &operator << (::std::ostream &os, const WossSimTime &simTime);
::std::istream &operator >> (::std::istream &is, WossSimTime &simeTime);

/**
 * Helper function that creates a Vector converted from geographic coordinates
 * \param latitude latitude in decimal degrees
 * \param longitude longitude in decimal degrees
 * \maram depth depth in meters
 * \returns a ns3::Vector
 */
Vector CreateVectorFromCoords (double latitude, double longitude, double depth);

/**
 * Helper function that creates a Vector converted from geographic coordinates woss::CoordZ
 * \param coordz woss::CoordZ object
 * \returns a ns3::Vector
 */
Vector CreateVectorFromCoordZ (woss::CoordZ coordz);

/**
 * Helper function that creates a CoordZ converted from cartesian coordinates
 * \param vect a valid ns3::Vector
 * \returns a woss::CoordZ
 */
woss::CoordZ CreateCoordZFromVector (Vector vect);

/**
 * Helper function that creates a Coord object converted from cartesian coordinates
 * \param vect a valid ns3::Vector
 * \returns a woss::Coord
 */
woss::Coord CreateCoordFromVector (Vector vect);


ATTRIBUTE_HELPER_HEADER (WossSimTime);

}

#endif /* WOSS_HELPER_H */

#endif /* NS3_WOSS_SUPPORT */

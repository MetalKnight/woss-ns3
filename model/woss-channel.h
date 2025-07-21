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

#ifndef WOSS_CHANNEL_H
#define WOSS_CHANNEL_H


#include "ns3/uan-channel.h"
#include "woss-prop-model.h"

namespace ns3 {

/**
 * \ingroup WOSS
 * \class WossChannel
 * \brief Channel class that computes the received power and the transmission delay
 *
 * the WossChannel class computes the received power and the transmission delay by manipulating
 * the UanPdp object returned by the associated WossPropModel object.
 */
class WossChannel : public UanChannel
{
public:
  WossChannel (); //!< Default Constructor
  virtual ~WossChannel (); //!< Default destructor

  /**
   * Register this type.
   * \return The object TypeId.
   */
  static TypeId GetTypeId ();

  /**
   * \param src Transducer transmitting packet
   * \param packet Packet to be transmitted
   * \param txPowerDb Transmission power in dB
   * \param txmode UanTxMode defining modulation of transmitted packet
   * Sends a packet out on the channel
   */
  virtual void TxPacket  (Ptr<UanTransducer> src, Ptr<Packet> packet, double txPowerDb,
                          UanTxMode txmode) override;

protected:
  /**
   * The first channel tap that gives a SNR (dB) greater
   * than the threshold will be used as transmission delay
   */
  double m_channelEqSnrThresDb;

  Ptr<WossPropModel> m_wossPropModel; //!< Smart ptr to a WossPropModel object

  virtual void DoInitialize (void) override;
};

}

#endif /* WOSS_CHANNEL_H */

#endif /* NS3_WOSS_SUPPORT */

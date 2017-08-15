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

#include <climits>
#include "woss-random-generator.h"


#define WOSS_INTEGER_MIN (0)
#define WOSS_INTEGER_MAX (INT_MAX)

namespace ns3 {

WossRandomGenerator::WossRandomGenerator (int seed)
  : RandomGenerator (seed),
    m_RandVar (NULL)
{
  m_RandVar = CreateObject<UniformRandomVariable> ();
}

WossRandomGenerator::~WossRandomGenerator ()
{

}

WossRandomGenerator::WossRandomGenerator (const WossRandomGenerator& copy)
  : RandomGenerator ( copy.seed ),
    m_RandVar ( copy.m_RandVar )
{
  initialized = copy.initialized;
}

WossRandomGenerator&
WossRandomGenerator::operator= (const WossRandomGenerator& copy)
{
  if (this == &copy)
    {
      return *this;
    }
  seed = copy.seed;
  m_RandVar = copy.m_RandVar;
  initialized = copy.initialized;
  return *this;
}

void
WossRandomGenerator::initialize ()
{
  initialized = true;
}

double
WossRandomGenerator::getRand () const
{
  return m_RandVar->GetValue ();
}

int
WossRandomGenerator::getRandInt () const
{
  return m_RandVar->GetInteger (WOSS_INTEGER_MIN, WOSS_INTEGER_MAX);
}

WossRandomGenerator*
WossRandomGenerator::create ( int s )
{
  return new WossRandomGenerator (s);
}

WossRandomGenerator*
WossRandomGenerator::clone () const
{
  return new WossRandomGenerator (*this);
}

int64_t
WossRandomGenerator::AssignStreams (int64_t stream)
{
  m_RandVar->SetStream (stream);
  return 1;
}

}

#endif /* NS3_WOSS_SUPPORT */

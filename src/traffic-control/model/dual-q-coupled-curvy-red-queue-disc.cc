/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 NITK Surathkal
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
 * Authors: 
 *          
 *          
 *
*/
#include "math.h"
#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/object-factory.h"
#include "ns3/string.h"
#include "dual-q-coupled-curvy-red-queue-disc.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/net-device-queue-interface.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DualQCoupledCurvyREDQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (DualQCoupledCurvyREDQueueDisc);

TypeId CurvyREDQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DualQCoupledCurvyREDQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<DualQCoupledCurvyREDQueueDisc> ()
    
  return tid;
}
class DualQCoupledCurvyREDTimestampTag : public Tag
{
public:
  DualQCoupledCurvyREDTimestampTag ();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  /**
   * \brief Gets the Tag creation time
   *
   * \return the time object stored in the tag
   */
  Time GetTxTime (void) const;

private:
  uint64_t m_creationTime; //!< Tag creation time
};


DualQCoupledCurvyREDTimestampTag::DualQCoupledCurvyREDTimestampTag ()
  : m_creationTime (Simulator::Now ().GetTimeStep ())
{
}

TypeId
DualQCoupledCurvyREDTimestampTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
DualQCoupledCurvyREDTimestampTag::GetSerializedSize (void) const
{
  return 8; 
}

void
DualQCoupledCurvyREDTimestampTag::Serialize (TagBuffer i) const
{
  i.WriteU64 (m_creationTime);
}

void
DualQCoupledCurvyREDTimestampTag::Deserialize (TagBuffer i)
{
  m_creationTime = i.ReadU64 ();
}

void
DualQCoupledCurvyREDTimestampTag::Print (std::ostream &os) const
{
  os << "CreationTime=" << m_creationTime;
}

Time
DualQCoupledCurvyREDTimestampTag::GetTxTime (void) const
{
  return TimeStep (m_creationTime);
}
TypeId
DualQCoupledCurvyREDTimestampTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DualQCoupledCurvyREDTimestampTag")
    .SetParent<Tag> ()
    .AddConstructor<DualQCoupledCurvyREDTimestampTag> ()
    .AddAttribute ("CreationTime",
                   "The time at which the timestamp was created",
                   StringValue ("0.0s"),
                   MakeTimeAccessor (&DualQCoupledCurvyREDTimestampTag::GetTxTime),
                   MakeTimeChecker ())
  ;
  return tid;
}

Time
DualQCoupledCurvyREDTimestampTag::GetTxTime (void) const
{
  return TimeStep (m_creationTime);
}


DualQCoupledCurvyREDQueueDisc::DualQCoupledCurvyREDQueueDisc ()
  : QueueDisc ()
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
  m_rtrsEvent = Simulator::Schedule (m_sUpdate, &DualQCoupledCurvyREDQueueDisc::CalculateP, this);
}
DualQCoupledCurvyREDQueueDisc::~DualQCoupledCurvyREDQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
DualQCoupledCurvyREDQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_uv = 0;
  Simulator::Remove (m_rtrsEvent);
  QueueDisc::DoDispose ();
}

void
DualQCoupledCurvyREDQueueDisc::SetMode (QueueDiscMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

DualQCoupledCurvyREDQueueDisc::QueueDiscMode
DualQCoupledCurvyREDQueueDisc::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void
DualQCoupledCurvyREDQueueDisc::SetQueueLimit (uint32_t lim)
{
  NS_LOG_FUNCTION (this << lim);
  m_queueLimit = lim;
}
DualQCoupledCurvyREDQueueDisc::Stats
DualQCoupledCurvyREDQueueDisc::GetStats ()
{
  NS_LOG_FUNCTION (this);
  return m_stats;
}

Time
DualQCoupledCurvyREDQueueDisc::GetQueueDelay (void)
{
  NS_LOG_FUNCTION (this);
  return m_qDelay;
}

double
DualQCoupledCurvyREDQueueDisc::GetDropProb (void)
{
  NS_LOG_FUNCTION (this);
  return m_dropProb;
}

int64_t
DualQCoupledCurvyREDQueueDisc::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}

DualQCoupledCurvyREDQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);
  uint8_t queueNumber;

  // attach arrival time to packet
  Ptr<Packet> p = item->GetPacket ();
  DualQCoupledCurvyREDTimestampTag tag;
  p->AddPacketTag (tag);


if ((GetMode () == QUEUE_DISC_MODE_PACKETS && nQueued >= m_queueLimit)
      || (GetMode () == QUEUE_DISC_MODE_BYTES && nQueued + item->GetSize () > m_queueLimit))
    {
      // Drops due to queue limit
      Drop (item);
      m_stats.forcedDrop++;
      return false;
    }
  else
    {
      if (item->IsL4S ())
        {
          queueNumber = 1;
        }
      else
        {
          queueNumber = 0;
        }
    }



Ptr<const QueueDiscItem>
DualQCoupledCurvyREDQueueDisc::DoPeek () const
{
  NS_LOG_FUNCTION (this);
  Ptr<const QueueDiscItem> item;

  for (uint32_t i = 0; i < GetNInternalQueues (); i++)
    {
      if ((item = GetInternalQueue (i)->Peek ()) != 0)
        {
          NS_LOG_LOGIC ("Peeked from queue number " << i << ": " << item);
          NS_LOG_LOGIC ("Number packets queue number " << i << ": " << GetInternalQueue (i)->GetNPackets ());
          NS_LOG_LOGIC ("Number bytes queue number " << i << ": " << GetInternalQueue (i)->GetNBytes ());
          return item;
        }
    }

  NS_LOG_LOGIC ("Queue empty");
  return item;
}

bool
DualQCoupledCurvyREDQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("DualQCoupledCurvyREDQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("DualQCoupledCurvyREDQueueDisc cannot have packet filters");
      return false;
    }


  if (GetNInternalQueues () == 0)
    {
      // Create 2 DropTail queues
      Ptr<InternalQueue> queue1 = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> > ("Mode", EnumValue (m_mode));
      Ptr<InternalQueue> queue2 = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> > ("Mode", EnumValue (m_mode));
      if (m_mode == QUEUE_DISC_MODE_PACKETS)
        {
          queue1->SetMaxPackets (m_queueLimit);
          queue2->SetMaxPackets (m_queueLimit);
        }
      else
        {
          queue1->SetMaxBytes (m_queueLimit);
          queue2->SetMaxBytes (m_queueLimit);
        }
      AddInternalQueue (queue1);
      AddInternalQueue (queue2);
    }

  if (GetNInternalQueues () != 2)
    {
      NS_LOG_ERROR ("DualQCoupledCurvyREDQueueDisc needs 2 internal queues");
      return false;
    }
        || (GetInternalQueue (0)->GetMode () == QueueBase::QUEUE_MODE_BYTES && m_mode == QUEUE_DISC_MODE_PACKETS))
    {
      NS_LOG_ERROR ("The mode provided for Classic traffic queue does not match the mode set on the DualQCoupledCurvyQueueDisc");
      return false;
    }

  if ((GetInternalQueue (1)->GetMode () == QueueBase::QUEUE_MODE_PACKETS && m_mode == QUEUE_DISC_MODE_BYTES)
      || (GetInternalQueue (1)->GetMode () == QueueBase::QUEUE_MODE_BYTES && m_mode == QUEUE_DISC_MODE_PACKETS))
    {
      NS_LOG_ERROR ("The mode provided for L4S traffic queue does not match the mode set on the DualQCoupledCurvyREDQueueDisc");
      return false;
    }

  if ((m_mode ==  QUEUE_DISC_MODE_PACKETS && GetInternalQueue (0)->GetMaxPackets () < m_queueLimit)
      || (m_mode ==  QUEUE_DISC_MODE_BYTES && GetInternalQueue (0)->GetMaxBytes () < m_queueLimit))
    {
      NS_LOG_ERROR ("The size of the internal Classic traffic queue is less than the queue disc limit");
      return false;
    }

  if ((m_mode ==  QUEUE_DISC_MODE_PACKETS && GetInternalQueue (1)->GetMaxPackets () < m_queueLimit)
      || (m_mode ==  QUEUE_DISC_MODE_BYTES && GetInternalQueue (1)->GetMaxBytes () < m_queueLimit))
    {
      NS_LOG_ERROR ("The size of the internal L4S traffic queue is less than the queue disc limit");
      return false;
    }

  return true;
}
} //namespace ns3
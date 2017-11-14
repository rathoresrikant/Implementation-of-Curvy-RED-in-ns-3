/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 NITK Surathkal
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
 * Authors: Srikant Singh <rathoresrikant@gmail.com>
 *          Vipin Singh <vipin.singh289@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
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
#define max(a,b)  (a > b) ? a : b

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DualQCoupledCurvyRedQueueDisc");

class DualQCoupledCurvyRedTimestampTag : public Tag
{
public:
  DualQCoupledCurvyRedTimestampTag ();
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


DualQCoupledCurvyRedTimestampTag::DualQCoupledCurvyRedTimestampTag ()
  : m_creationTime (Simulator::Now ().GetTimeStep ())
{
}

TypeId
DualQCoupledCurvyRedTimestampTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DualQCoupledCurvyRedTimestampTag")
    .SetParent<Tag> ()
    .AddConstructor<DualQCoupledCurvyRedTimestampTag> ()
    .AddAttribute ("CreationTime",
                   "The time at which the timestamp was created",
                   StringValue ("0.0s"),
                   MakeTimeAccessor (&DualQCoupledCurvyRedTimestampTag::GetTxTime),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId
DualQCoupledCurvyRedTimestampTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
DualQCoupledCurvyRedTimestampTag::GetSerializedSize (void) const
{
  return 8;
}

void
DualQCoupledCurvyRedTimestampTag::Serialize (TagBuffer i) const
{
  i.WriteU64 (m_creationTime);
}

void
DualQCoupledCurvyRedTimestampTag::Deserialize (TagBuffer i)
{
  m_creationTime = i.ReadU64 ();
}

void
DualQCoupledCurvyRedTimestampTag::Print (std::ostream &os) const
{
  os << "CreationTime=" << m_creationTime;
}

Time
DualQCoupledCurvyRedTimestampTag::GetTxTime (void) const
{
  return TimeStep (m_creationTime);
}

NS_OBJECT_ENSURE_REGISTERED (DualQCoupledCurvyRedQueueDisc);

TypeId DualQCoupledCurvyRedQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DualQCoupledCurvyRedQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<DualQCoupledCurvyRedQueueDisc> ()
    .AddAttribute ("Mode",
                   "Determines unit for QueueLimit",
                   EnumValue (QUEUE_DISC_MODE_PACKETS),
                   MakeEnumAccessor (&DualQCoupledCurvyRedQueueDisc::SetMode),
                   MakeEnumChecker (QUEUE_DISC_MODE_BYTES, "QUEUE_DISC_MODE_BYTES",
                                    QUEUE_DISC_MODE_PACKETS, "QUEUE_DISC_MODE_PACKETS"))    
    .AddAttribute ("K0",
                   "Constant used to calculate L4SQueueScalingFactor ",
                   UintegerValue (1),
                   MakeUintegerAccessor (&DualQCoupledCurvyRedQueueDisc::m_k0),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ClassicQueueScalingFactor",
                   "Scaling factor for drop probabilty in classic queue",
                   DoubleValue (-1),
                   MakeDoubleAccessor (&DualQCoupledCurvyRedQueueDisc::m_classicQScalingFact),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Curviness",
                   "Curviness Parameter",
                   UintegerValue (1),
                   MakeUintegerAccessor (&DualQCoupledCurvyRedQueueDisc::m_curviness),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("L4SQueueSizeThreshold",
                   "Queue size in bytes at which the marking starts in the L4S queue.",
                   UintegerValue (5*1500),
                   MakeUintegerAccessor (&DualQCoupledCurvyRedQueueDisc::m_l4SQSizeThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Fc",
                   "Constant to calculate alpha",
                   UintegerValue (5),
                   MakeUintegerAccessor (&DualQCoupledCurvyRedQueueDisc::m_calcAlpha),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("QueueLimit",
                   "Queue limit in bytes/packets",
                   UintegerValue (25),
                   MakeUintegerAccessor (&DualQCoupledCurvyRedQueueDisc::SetQueueLimit),
                   MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}

DualQCoupledCurvyRedQueueDisc::DualQCoupledCurvyRedQueueDisc ()
  : QueueDisc ()
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
}

DualQCoupledCurvyRedQueueDisc::~DualQCoupledCurvyRedQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
DualQCoupledCurvyRedQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_uv = 0;
  QueueDisc::DoDispose ();
}

void
DualQCoupledCurvyRedQueueDisc::SetMode (QueueDiscMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

DualQCoupledCurvyRedQueueDisc::QueueDiscMode
DualQCoupledCurvyRedQueueDisc::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void
DualQCoupledCurvyRedQueueDisc::SetQueueLimit (uint32_t lim)
{
  NS_LOG_FUNCTION (this << lim);
  m_queueLimit = lim;
}

uint32_t
DualQCoupledCurvyRedQueueDisc::GetQueueSize (void)
{
  NS_LOG_FUNCTION (this);
  if (GetMode () == QUEUE_DISC_MODE_BYTES)
    {
      return (GetInternalQueue (0)->GetNBytes () + GetInternalQueue (1)->GetNBytes ());
    }
  else if (GetMode () == QUEUE_DISC_MODE_PACKETS)
    {
      return (GetInternalQueue (0)->GetNPackets () + GetInternalQueue (1)->GetNPackets ());
    }
  else
    {
      NS_ABORT_MSG ("Unknown Dual Queue Curvy Red mode.");
    }
}

uint32_t
DualQCoupledCurvyRedQueueDisc::Getl4sQueueSize (void)        //to calculate the current size of l4s queue in bytes
{
  NS_LOG_FUNCTION (this);
  return (GetInternalQueue (1)->GetNBytes ());
}

DualQCoupledCurvyRedQueueDisc::Stats
DualQCoupledCurvyRedQueueDisc::GetStats ()
{
  NS_LOG_FUNCTION (this);
  return m_stats;
}

int64_t
DualQCoupledCurvyRedQueueDisc::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}

void DualQCoupledCurvyRedQueueDisc::InitializeParams (void)
{
  m_l4sQScalingFact = m_classicQScalingFact + m_k0;
  m_stats.forcedDrop = 0;
  m_stats.unforcedClassicDrop = 0;
  m_stats.unforcedL4SMark = 0;
  m_avgQueuingTime = Time (Seconds (0));
}

bool
DualQCoupledCurvyRedQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);
  uint8_t queueNumber;

  // attach arrival time to packet
  Ptr<Packet> p = item->GetPacket ();
  DualQCoupledCurvyRedTimestampTag tag;
  p->AddPacketTag (tag);

  uint32_t nQueued = GetQueueSize ();
  if ((GetMode () == QUEUE_DISC_MODE_PACKETS && nQueued >= m_queueLimit)
      || (GetMode () == QUEUE_DISC_MODE_BYTES && nQueued + item->GetSize () > m_queueLimit))
    {
      // Drops due to queue limit
      Drop (item);
      m_stats.forcedDrop++;
      return false;
    }

  if (item->IsL4S ())
    {
      queueNumber = 1;
    }
  else
    {
      queueNumber = 0;
    }

  bool retval = GetInternalQueue (queueNumber)->Enqueue (item);
  NS_LOG_LOGIC ("Number packets in queue-number " << queueNumber << ": " << GetInternalQueue (queueNumber)->GetNPackets ());
  return retval;
}

double
DualQCoupledCurvyRedQueueDisc::MaxRand (int u)
{
  NS_LOG_FUNCTION (this);
  double maxr = 0.0;
  while (u-- > 0)
    {
      maxr = max (maxr, m_uv->GetValue ());
    }
  return maxr;
}

Ptr<QueueDiscItem>
DualQCoupledCurvyRedQueueDisc::DoDequeue ()
{
  NS_LOG_FUNCTION (this);

  if (GetInternalQueue (1)->Peek () != 0)
    {
      Ptr<const QueueDiscItem> item1 = GetInternalQueue (0)->Peek ();
      DualQCoupledCurvyRedTimestampTag tag;
      Time classicQueueTime;
      double l4sDropProb;
      if (item1 != 0)
        {         
          item1->GetPacket ()->PeekPacketTag (tag);
          classicQueueTime = tag.GetTxTime ();                             //arrival time of the packet at the head of classic queue
        }
      else
        {
          classicQueueTime = Time (Seconds (0));
        }
  
      Ptr<QueueDiscItem> item = GetInternalQueue (1)->Dequeue ();
      l4sDropProb = (Simulator::Now ().GetSeconds () - classicQueueTime.GetSeconds ()) / pow (2, m_l4sQScalingFact);
      if (Getl4sQueueSize () > m_l4SQSizeThreshold || l4sDropProb > MaxRand (m_curviness))
        {
          item->Mark ();
          m_stats.unforcedL4SMark++;
        }
      item->GetPacket ()->RemovePacketTag (tag); 
      return item;
    }

  while (GetInternalQueue (0)->Peek ())                 //if there is a packet in classic queue to drop
    {
      Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();
      DualQCoupledCurvyRedTimestampTag tag;
      double sqrtClassicDropProb;
      item->GetPacket ()->PeekPacketTag (tag);
      
      Time classicQueueDelay = Simulator::Now () - tag.GetTxTime ();             //instantaneous queuing time of the current classic packet
      m_avgQueuingTime += (classicQueueDelay - m_avgQueuingTime) / pow(2,m_calcAlpha);           //classic Queue EWMA
      sqrtClassicDropProb = (double) m_avgQueuingTime.GetSeconds () / pow (2, m_classicQScalingFact);

      if (sqrtClassicDropProb > MaxRand (2 * m_curviness))
        {
          Drop (item);
          m_stats.unforcedClassicDrop++;
        }
      else
        { 
          item->GetPacket ()->RemovePacketTag (tag);
          return item; 
        }  
    }
  return 0;
}

Ptr<const QueueDiscItem>
DualQCoupledCurvyRedQueueDisc::DoPeek () const
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
DualQCoupledCurvyRedQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("DualQCoupledCurvyRedQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("DualQCoupledCurvyRedQueueDisc cannot have packet filters");
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
      NS_LOG_ERROR ("DualQCoupledCurvyRedQueueDisc needs 2 internal queue");
      return false;
    }

  if ((GetInternalQueue (0)->GetMode () == QueueBase::QUEUE_MODE_PACKETS && m_mode == QUEUE_DISC_MODE_BYTES)
      || (GetInternalQueue (0)->GetMode () == QueueBase::QUEUE_MODE_BYTES && m_mode == QUEUE_DISC_MODE_PACKETS))
    {
      NS_LOG_ERROR ("The mode provided for Classic traffic queue does not match the mode set on the DualQCoupledCurvyRedQueueDisc");
      return false;
    }

  if ((GetInternalQueue (1)->GetMode () == QueueBase::QUEUE_MODE_PACKETS && m_mode == QUEUE_DISC_MODE_BYTES)
      || (GetInternalQueue (1)->GetMode () == QueueBase::QUEUE_MODE_BYTES && m_mode == QUEUE_DISC_MODE_PACKETS))
    {
      NS_LOG_ERROR ("The mode provided for L4S traffic queue does not match the mode set on the DualQCoupledCurvyRedQueueDisc");
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

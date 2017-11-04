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
 */

/*
 * PORT NOTE: This code was ported from ns-2.36rc1 (queue/pie.cc).
 * Most of the comments are also ported from the same.
 */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "pie-queue-disc.h"
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
      NS_LOG_ERROR ("The mode provided for Classic traffic queue does not match the mode set on the DualQCoupledPiSquareQueueDisc");
      return false;
    }

  if ((GetInternalQueue (1)->GetMode () == QueueBase::QUEUE_MODE_PACKETS && m_mode == QUEUE_DISC_MODE_BYTES)
      || (GetInternalQueue (1)->GetMode () == QueueBase::QUEUE_MODE_BYTES && m_mode == QUEUE_DISC_MODE_PACKETS))
    {
      NS_LOG_ERROR ("The mode provided for L4S traffic queue does not match the mode set on the DualQCoupledPiSquareQueueDisc");
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

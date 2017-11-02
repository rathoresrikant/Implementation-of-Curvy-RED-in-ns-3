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
 
 *
 */

#ifndef DUAL_Q_COUPLED_CURVY_RED_QUEUE_DISC_H
#define DUAL_Q_COUPLED_CURVY_RED_QUEUE_DISC_H

#include <queue>
#include "ns3/packet.h"
#include "ns3/queue-disc.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/timer.h"
#include "ns3/string.h"
#include "ns3/event-id.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/trace-source-accessor.h"

namespace ns3 {

class TraceContainer;
class UniformRandomVariable;

/**
 * \ingroup traffic-control
 *
 * \brief Implements Curvy RED queue discipline with
 *        DualQ Structure and Coupled AQM functionality
 */
class DualQCoupledCurvyREDQueueDisc : public QueueDisc
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief DualQCoupledCurvyREDQueueDisc Constructor
   */
  DualQCoupledCurvyREDQueueDisc ();

  /**
   * \brief DualQCoupledCurvyREDQueueDisc Destructor
   */
  virtual ~DualQCoupledCurvyREDQueueDisc ();

  /**
   * \brief Stats
   */
  typedef struct
  {
    uint32_t unforcedClassicDrop;      //!< Probability drops of Classic traffic: proactive
    uint32_t unforcedClassicMark;      //!< Probability marks of Classic traffic: proactive
    uint32_t unforcedL4SMark;          //!< Probability marks of L4S traffic: proactive
    uint32_t forcedDrop;               //!< Drops due to queue limit: reactive
  } Stats;

  /**
   * \brief Enumeration of the modes supported in the class.
   */
  enum QueueDiscMode
  {
    QUEUE_DISC_MODE_PACKETS,     /**< Use number of packets for maximum queue disc size */
    QUEUE_DISC_MODE_BYTES,       /**< Use number of bytes for maximum queue disc size */
  };

  /**
   * \brief Set the operating mode of this queue.
   *
   * \param mode The operating mode of this queue.
   */
  void SetMode (QueueDiscMode mode);

  /**
   * \brief Get the encapsulation mode of this queue.
   *
   * \returns The encapsulation mode of this queue.
   */
  QueueDiscMode GetMode (void);

  /**
   * \brief Get the current value of the queue in bytes or packets.
   *
   * \returns The queue size in bytes or packets.
   */
  uint32_t GetQueueSize (void);

  /**
   * \brief Set the limit of the queue in bytes or packets.
   *
   * \param lim The limit in bytes or packets.
   */
  void SetQueueLimit (uint32_t lim);

  /**
   * \brief Get queue delay
   */
  Time GetQueueDelay (void);

  /**
   * \brief Get the drop probability
   */
  double GetDropProb (void);

  /**
   * \brief Get Dual Queue PI Square statistics after running.
   *
   * \returns The drop statistics.
   */
  Stats GetStats ();

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);

private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void) const;
  virtual bool CheckConfig (void);

  /**
   * \brief Initialize the queue parameters.
   */
  virtual void InitializeParams (void);

  /**
   * \brief Periodically calculate the drop probability
   */
  void CalculateP ();

  Stats m_stats;                                //!< DualQ Coupled Curvy RED statistics

  // ** Variables supplied by user
  QueueDiscMode m_mode;                         //!< Mode (bytes or packets)
  Time m_classicQueueDelayRef;                  //!< Queue delay target for Classic traffic
  uint32_t m_meanPktSize;                       //!< Average packet size in bytes
  double m_alpha;                               //!< Parameter to PI Square controller
  Time m_l4sThreshold;                          //!< L4S marking threshold (in time)
  uint32_t m_k;                                 //!< Coupling factor
  uint32_t m_queueLimit;                        //!< Queue limit in bytes / packets

  // ** Variables maintained by DualQ Coupled Curvy RED
  Time m_classicQueueTime;                      //!< Arrival time of a packet of Classic Traffic
  Time m_l4sQueueTime;                          //!< Arrival time of a packet of L4S Traffic
  Time m_tShift;                                //!< Scheduler time bias
  uint32_t m_minL4SLength;                      //!< Mininum threshold (in bytes) for marking L4S traffic
  double m_dropProb;                            //!< Variable used in calculation of drop probability
  double m_classicDropProb;                     //!< Variable used in calculation of drop probability of Classic traffic
  double m_l4sDropProb;                         //!< Variable used in calculation of drop probability of L4S traffic
 
  double s_c;                                   //!< scaling factor for Classic queuing time
  double f_c;                                   //!< parameter used to calculate alpha
  double s_l;                                   //!<scaling factor for L4S queuing time
  double qsize_thres;                           //!<queue size in bytes at which threshold marking starts in the queue
  Time m_qDelayOld;                             //!< Old value of queue delay
  Time m_qDelay;                                //!< Current value of queue delay
  EventId m_rtrsEvent;                          //!< Event used to decide the decision of interval of drop probability calculation
  Ptr<UniformRandomVariable> m_uv;              //!< Rng stream
};

}    // namespace ns3

#endif

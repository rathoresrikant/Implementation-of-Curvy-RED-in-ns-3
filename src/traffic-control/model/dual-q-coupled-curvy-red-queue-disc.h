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
 * \brief Implements Curvy Red queue discipline with
 *        DualQ Structure and Coupled AQM functionality
 */
class DualQCoupledCurvyRedQueueDisc : public QueueDisc
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief DualQCoupledCurvyRedQueueDisc Constructor
   */
  DualQCoupledCurvyRedQueueDisc ();

  /**
   * \brief DualQCoupledCurvyRedQueueDisc Destructor
   */
  virtual ~DualQCoupledCurvyRedQueueDisc ();

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
   * \brief Get the current value of the l4s queue in bytes or packets.
   *
   * \returns The queue size in bytes or packets.
   */
  uint32_t Getl4sQueueSize ( void );

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
   * \brief find maximum of U random numbers
   */
  double MaxRand( int U );

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


  Stats m_stats;                                //!< DualQ Coupled Curvy Red statistics

  // ** Variables supplied by user
  QueueDiscMode m_mode;                         //!< Mode (bytes or packets)
  Time m_classicQueueDelayRef;                  //!< Queue delay target for Classic traffic
  uint32_t m_meanPktSize;                       //!< Average packet size in bytes
  Time m_l4sThreshold;                          //!< L4S marking threshold (in time)
  //uint32_t m_k;                                 //!< Coupling factor
  uint32_t m_k0;                                //!< constant to adjust the value of m_k
  uint32_t m_queueLimit;                        //!< Queue limit in bytes / packets
  uint32_t m_classicQScalingFact;               //!< scaling factor for Classic queuing time
  uint32_t m_calcAlpha;                         //!< parameter used to calculate alpha
  int32_t  m_l4SQScalingFact;                   //! <scaling factor for L4S queuing time
  uint32_t m_cUrviness;                         //!< cUrviness parameter for Curvy Red
  // ** Variables maintained by DualQ Coupled Curvy Red
  Time m_classicQueueTime;                      //!< Arrival time of a packet of Classic Traffic
  Time m_l4sQueueTime;                          //!< Arrival time of a packet of L4S Traffic
  uint32_t m_l4sMarkingThreshold;               //!< Mininum threshold (in bytes) for marking L4S traffic
  double m_sqrtClassicDropProb                  //!< Variable used in calculation of drop probability of Classic traffic
  double m_l4sDropProb;                         //!< Variable used in calculation of drop probability of L4S traffic
  Time m_avgQueuingTime;                        //!< Averaged Queuing time
  //Time m_qDelay;                              //!< value of queue delay
  Time m_classicQueueDelay;                     //!< Current value of queue delay of classic packet
  Ptr<UniformRandomVariable> m_uv;              //!< Rng stream
};

}    // namespace ns3

#endif

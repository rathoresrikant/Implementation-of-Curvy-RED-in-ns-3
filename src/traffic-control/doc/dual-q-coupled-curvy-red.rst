.. include:: replace.txt
.. highlight:: cpp

Dual Queue Coupled Curvy RED queue disc
-------------------------------

DualQ Coupled Curvy RED [Schepper17]_ is a DualQ Coupled AQM algorithm based
on the Curvy RED algorithm. Curvy RED is a generalization as well as simplification
of the RED queue disc. It requires less operations per packet than RED and can be 
used if the range of RTTs is limited. 

This chapter describes the DualQ Coupled Curvy RED queue disc implementation
in |ns3|. 

Model Description
*****************

The source code is located in ``src/traffic-control/model`` and consists of 2 files:
`dual-q-coupled-curvy-red-queue-disc.h` and `dual-q-coupled-curvy-red-queue-disc.cc`
defining a DualQCoupledCurvyRedQueueDisc class. The code was ported to |ns3| based 
on the IETF draft [Schepper17]_.  

* class :cpp:class:`DualQCoupledCurvyRedQueueDisc`: This class implements the main
algorithm:

* ``DualQCoupledCurvyRedQueueDisc::DoEnqueue ()``: This routine checks whether
the queue is full, and if so, drops the packets and records the number of drops 
due to queue overflow. If queue is not full, this routine calls the
``QueueDisc::IsL4S()`` method to check if a packet is of type Classic or L4S
and enqueues it in appropriate queue. 

* ``DualQCoupledCurvyRedQueueDisc::DoDequeue ()``: This routine schedules one packet
for dequeuing (or zero if the queue is empty). It has two blocks (if-block  and while
-block) to take the decision of marking or dropping of packets in the queues. The 
if-block tests whether there is a L4S packet to dequeue. It then calculates the drop 
probabilty using the queuing time of the packet at the head of classic queue. The packet
is marked if the probabilty is greater than the maximum  of U randomly generated numbers
or if the current size of L4S queue is greater than a threshold. The while-block is used
in the classic queue to drop each packet until a packet is forwarded. It calculates average
queuing delay using EWMA equation which has old average queuing delay and current queuing 
delay. The drop probability in this case is calculated by dividing the average queuing delay 
by a scaling factor. If the drop probabilty is greater than the maximum of 2*U randomly 
generated numbers the packet is dropped, otherwise the packet is returned. 
   
* ``DualQCoupledCurvyRedQueueDisc::MaxRand ()``: This routine simply generates U random 
numbers and returns the maximum of these numbers. U is the curviness parameter which is a 
small positive integer. We have used U = 1 in our implementation and the result might get 
better with U = 2 or more.  

References
==========

.. [Schepper17] De Schepper, K., Briscoe, B. Bondarenko, O., & Tsang, I,Internet-Draft: DualQ Coupled AQM for Low Latency, Low Loss and Scalable Throughput, July 2017. Available online at `<https://tools.ietf.org/html/draft-ietf-tsvwg-aqm-dualq-coupled-01>`_.

.. [Schepper16] De Schepper, K., Bondarenko, O., Tsang, I., & Briscoe, B. (2016, November). PI2: A Linearized AQM for both Classic and Scalable TCP. In Proceedings of the 12th International on Conference on emerging Networking Experiments and Technologies (pp. 105-119). ACM.`_.

Attributes
==========

The key attributes that the DualQCoupledCurvyRedQueueDisc class holds include
the following: 

* ``Mode:`` DualPI2 operating mode (BYTES or PACKETS). The default mode is PACKETS. 
* ``QueueLimit:`` The maximum number of bytes or packets the queue can hold.
The default value is 25 bytes / packets.
* ``ClassicQueueScalingFactor:`` Scaling factor for Classic Queuing time. The default value is -1.  
* ``Curviness:`` Curviness parameter. The default value is 1.
* ``K0:`` Constant used in the calculation of L4SQueueScalingFactor. The default value is 1.
* ``Fc:`` Used in the EWMA equation to calculate alpha. Its default value is 5.
* ``L4SQueueSizeThreshold:`` Queue size in bytes at which the marking starts in the L4S queue. 
The default value is 5 MTU.

Examples
========

The example for DualQCoupled Curvy RED is `dual-q-coupled-curvy-red-example.cc` located in
``examples/traffic-control``. To run the file (the first invocation below
shows the available command-line options):

:: 

   $ ./waf --run "dual-q-coupled-curvy-red-example --PrintHelp"
   $ ./waf --run "dual-q-coupled-curvy-red-example"

Validation
**********

DualQCoupled Curvy RED model is tested using :cpp:class:`DualQCoupledCurvyRedQueueDiscTestSuite`
class defined in `src/traffic-control/test/dual-q-coupled-curvy-red-queue-disc-test-suite.cc`.
The suite includes 4 test cases:

* Test 1: simple enqueue/dequeue with defaults, no drops
* Test 2: more packets of both L4S and Classic with L4S having higher marks than Classic
* Test 3: Send L4S traffic only
* Test 4: Send Classic traffic only

The test suite can be run using the following commands: 

::

  $ ./waf configure --enable-examples --enable-tests
  $ ./waf
  $ ./test.py -s dual-q-coupled-curvy-red-queue-disc

or  

::

$ NS_LOG="DualQCoupledCurvyRedQueueDisc" ./waf --run "test-runner --suite=dual-q-coupled-curvy-red-queue-disc"

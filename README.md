# Implementation-of-DualQ-Coupled-Curvy-RED-in-ns-3

## Course Code: CS822

## Assignment: #FP2

## Overview

DualQ Coupled Curvy RED [1] is an extension of Random Early Detection (RED) [2] tailored towards DualQ framework [3]. Curvy RED is a generalization as well as simplification of the RED queue disc. However, till date the implementation of DualQ Coupled Curvy RED is not publicly available in any network simulator. This repository provides an implementation of DualQ Coupled Curvy RED algorithm in ns-3 [4].

### DualQ Coupled Curvy RED example

An example program for DualQ Coupled Curvy RED has been provided in

`examples/traffic-control/dual-q-coupled-curvy-red-example.cc`

and should be executed as

`./waf --run dual-q-coupled-curvy-red-example`


References

[1] Insights from Curvy RED (Random Early Detection) https://www.bobbriscoe.net/projects/latency/credi_tr.pdf

[2] Floyd, S., & Jacobson, V. (1993). Random early detection gateways for congestion avoidance. IEEE/ACM Transactions on networking.

[3] DualQ Coupled AQM for Low Latency, Low Loss and Scalable Throughput (Link:https://tools.ietf.org/html/draft-ietf-tsvwg-aqm-dualq-coupled-01)

[4] http://www.nsnam.org/

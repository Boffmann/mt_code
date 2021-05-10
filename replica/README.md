# Replica

The implementation that is intended to run on each component in the cluster. It implements leader election and log replication concepts of `Raft` and `OpenSplice DDS`.

## Structure
The replica implementation is subdivided into the following components:

* `consensus:` Contains logic for leader election, log replication, and consensus- and voting-based decision making. Further, it ensures that there is always a leader in the system and that not more than one leader is active at a time.

* `datamodel:` Provided the data mdel in the form of DDS topic structures and data types. It is provided in `IDL` format that is automatically translated during the compilation process.

* `evaluation:` Provides an API for logging certain events into evaluation files. These files are required by the automatic testing process and were used for the evaluation chapter in the Thesis.

* `DIO:` Contains API to address the digital IO expansion module for `Revolution Pis`.

* `DDSCreator:` Collection of functions to create and manage DDS entities.

* `src:` Main entry point for the replica implementation. Implements the input consumption and voting based on the consensus module.

* `state:` Provides an interface to the distributed system's global state that is managed by DDS.

## Compile and Deployment
The compilation and deployment process for the replica program is described in the root README.

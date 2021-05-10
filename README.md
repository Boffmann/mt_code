# An ETCS On-Board Unit
This repository is part of my master's thesis. It provides an implementation of a redundant and fault-tolerant ETCS on-board system using real-time DCPS machine-to-machine communication and consensus-based voting. It implements leader election and log replication concepts of [Raft](https://raft.github.io/raft.pdf) with [Vortex OpenSplice DDS](https://github.com/ADLINK-IST/opensplice) as communication middleware. The application is built for and tested on a cluster of [Revolution Pis](https://revolution.kunbus.de/). Although the system is intended for use on a real train, testing during production takes place in a simulation.

The implementation can be executed both on separate machines and on one machine using multiple processes. Either way, a running instance is called `replica`.

## Structure
The project is structured as follows:

* `ansible:` Contains ansible configuration files for deploying and setting up the project on the cluster.

* `replica:` This is the actual implementation that is intended to run on each component in the cluster.

* `simulator:` Implementation of a simulator for simulating a train's journey. Used for integration testing.

* `tests:` Provides a Python Script that is used for automatically testing the implementation.

For detailed information for each subproject, see the `README` files in the corresponding directory and the wiki.

## Prerequisites
The solution has the following dependencies

```
- Vortex OpenSplice
- CMake
- A suitable C compiler such as gcc
```

## Configure Revolution Pis

### Vortex OpenSplice

Install the prerequisites on each `Revolution Pi`. To install `Vortex OpenSplice`, follow the instructions from [here](https://github.com/ADLINK-IST/opensplice) and execute `source $OSPL_HOME/release.com`.

### Network
When run on separate machines that are interconnected via Ethernet, `Vortex OpenSplice` needs to be configured to use the network interface. Therefore, set the `NetworkInterfaceAddress` in `$OSPL_HOME/etc/config/ospl.xml` to the current IP address.

## Compile
The project is managed by `CMake`. After installing all dependencies, follow these steps to compile:

```
$ cd $ETCSOnBoard/replica
$ mkdir build & cd build
$ cmake ..
$ make
```

## Manual Deployment

While the on-board system is intended to be executed on separate `Revolution Pis`, it can also be run on one machine in multiple processes. Either way, after compiling the project, a `replica` can be started using the following command:

```
$ build/replica <ID>
```

The `ID` parameter is used to distinguish between multiple replicas and has influence on the leader election process. Make sure not to have multiple replicas with the same `ID`. As is, the cluster expects three replicas in the system. The number of expected replicas is configurable in `replica/consensus/replica.h`. `IDs` that are higher or equal to the number of expected replicas are used as spare replicas.

## Automatic Deployment
For automatic deployment, the `connect_in_tmux.sh` and `run_ansible.sh` scripts can be used. 

`connect_in_tmux.sh` connects to all `Revolution Pis` in the cluster and provides a grid view.

`run_ansible.sh` utilizes `ansible` and `tmux` in background and provides four functionalities:

* `./run_ansible.sh -d`: Copy and compile a new version to each `Revolution Pi`.
* `./run_ansible.sh -r`: Start a new replica on each `Revolution Pi` with increasing `IDs`
* `./run_ansible.sh -c`: Abort all running replica instances.
* `./run_ansible.sh -s`: Shutdown all `Revolution Pis` in the cluster.

Running `./run_ansible.sh -h` provides a help page.

You may need to change the IP addresses in `connect_in_tmux.sh`.
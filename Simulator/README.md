# Simulator

A simulator that simulates train journeys based on an ETCS subset in the form of scenarios. Such a scenario consists of a Movement Authority (MA), a set of linked balises, and track-side balises.

In each scenario, the train goes through a journey that begins as soon as the train starts moving and stops when it brakes. At the beginning of each scenario, the simulator provides a MA and a set of linked balises to the on-board system. Further, the simulator sends balise telegrams when the simulated train reaches a virtual track-side balise.

Scenarios are specified in `json` format.

## Structure
The simulator implementation is structured into the following directories:

* `Scenarios:` Contains `json` files for different scenarios.

* `src:` Provides the actual simulator implementation that parses scenarios from `json` files, sends MA, links balises, and sends simulated balise telegrams.

## Thesis Scenarios

In the thesis, three scenarios were used for evaluation.

* `Reach End of MA:` A valid MA is provided and all balises are linked. Further, all balises are provided at exact positions so that the train should brake right before reaching the MA's end.

* `Balise not linked:` A valid MA is provided, but the last balise is not linked. Thus, the train should stop as soon as the unlinked balise is encountered.

* `Balise not where expected:` A valid MA is provided and all balises are linked. However, the last balise's telegram is transmitted at another location than it is linked. Thus, the train should stop as soon as the incorrectly positioned balise is encountered.

## Compilation and Run

**IMPORTANT:** The simulator solution depends on the replica solution's `datamodel` and `state` library. Therefore, the replica solution must to be compiled before the simulator.

The simulator has no further external dependencies to those described in the root README.

Execute the following commands to compile and run the simulator:

```
$ mkdir build && cd build
$ cmake ..
$ make
$ ./RevPiTrainSimulator <scenario>
```

The simulator requires a `<scenario>` that it should execute. A `<scenario>` is provided as a path to a scenario `json` file. For an exemplary file, look at the `Scenario` directory.
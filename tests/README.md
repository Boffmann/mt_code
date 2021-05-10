# Tests

Because the scenarios are simulated, a reproducible test environment is created. This enables automated integration testing and facilitates the measurement of system characteristics under comparable circumstances. Further, data reproducibility allows to make changes to the system and to evaluate its performance compared to other versions.

For automatic integration testing, a python script is used that utilized the simulator and, after each simulated scenario, assesses the evaluation files to check whether the system behaved as intended.

## Execution
When the replicas are running, run the following command in a separated terminal on a machine that is part of the cluster:

```
$ python3 test.py
```
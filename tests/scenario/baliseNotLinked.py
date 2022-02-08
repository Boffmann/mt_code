from . import Scenario, Entry
import subprocess

class BaliseNotLinked(Scenario):

    def __init__(self, isRemoteExecution):
        super().__init__(isRemoteExecution)

    def execute_simulator(self):
        subprocess.call(["../Simulator/build/RevPiTrainSimulator", "../Simulator/Scenarios/baliseNotLinked.json"])

    def printError(self, message):
        print('\033[1;31;40m ERROR - ', end="")
        print("BaliseNotLinked: " + message)
        print('\033[0;0m')

    def printSuccess(self):
        print('\033[1;32;40m SUCCESS - ', end="")
        print("BaliseNotLinked")
        print('\033[0;0m')

    def evaluate(self) -> bool:

        result = True

        print('\033[1;33;40m ------------------')
        print('\033[0;0m')
        self.printEvaluationFile()
        print('\033[1;33;40m ------------------')
        print('\033[0;0m')

        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 0.5, 1.6, "Reached Balise", 0, "")
        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 4.25, 5.75, "Reached Balise", 1, "")
        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 6.25, 7.75, "Stopped", 2, "Balise Not Linked")

        if result:
            self.printSuccess()

        print('\033[1;33;40m ------------------')
        print('\033[0;0m')

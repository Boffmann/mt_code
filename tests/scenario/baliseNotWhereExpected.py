from . import Scenario, Entry
import subprocess

class BaliseNotWhereExpected(Scenario):

    def __init__(self):
        super().__init__()

    def execute_simulator(self):
        subprocess.call(["../Simulator/build/RevPiTrainSimulator", "../Simulator/Scenarios/baliseNotWhereExpected.json"])

    def printError(self, message):
        print("BaliseNotWhereExpected: ERROR Occurred: " + message)

    def printSuccess(self):
        print("BaliseNotWhereExpected: SUCCESS")


    def evaluate(self) -> bool:

        result = True

        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 1.0, 1.5, "Reached Balise", 0, "")
        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 4.25, 5.75, "Reached Balise", 1, "")
        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 7.25, 8.75, "Stopped", 2, "Balise Not Where Expected")

        if result:
            self.printSuccess()



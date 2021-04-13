from . import Scenario, Entry
import subprocess

class ReachEndOfMA(Scenario):

    def __init__(self):
        super().__init__()

    def execute_simulator(self):
        subprocess.call(["../Simulator/build/RevPiTrainSimulator", "../Simulator/Scenarios/reachEndOfMA.json"])

    def printError(self, message):
        print("ReachEndOfMA: ERROR Occurred: " + message)

    def printSuccess(self):
        print("ReachEndOfMA: SUCCESS")


    def evaluate(self) -> bool:

        result = True

        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 0.5, 1.5, "Reached Balise", 0, "")
        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 4.25, 5.75, "Reached Balise", 1, "")
        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 6.25, 7.75, "Reached Balise", 2, "")
        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 7.5, 8.75, "Stopped", -1, "Reached End of MA")

        if result:
            self.printSuccess()


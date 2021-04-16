from . import Scenario, Entry
import subprocess

class BaliseNotWhereExpected(Scenario):

    def __init__(self):
        super().__init__()

    def execute_simulator(self):
        subprocess.call(["../Simulator/build/RevPiTrainSimulator", "../Simulator/Scenarios/baliseNotWhereExpected.json"])

    def printError(self, message):
        print('\033[1;31;40m ERROR - ', end="")
        print("BaliseNotWhereExpected: " + message)
        print('\033[0;0m')

    def printSuccess(self):
        print('\033[1;32;40m SUCCESS - ', end="")
        print("BaliseNotWhereExpected")
        print('\033[0;0m')


    def evaluate(self) -> bool:

        result = True

        print('\033[1;33;40m ------------------')
        print('\033[0;0m')
        self.printEvaluationFile()
        print('\033[1;33;40m ------------------')
        print('\033[0;0m')

        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 1.0, 1.5, "Reached Balise", 0, "")
        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 4.25, 5.75, "Reached Balise", 1, "")
        entry = self.getNextTestLine()
        result &= self.evaluateEntry(entry, 7.25, 8.75, "Stopped", 2, "Balise Not Where Expected")

        if result:
            self.printSuccess()

        print('\033[1;33;40m ------------------')
        print('\033[0;0m')

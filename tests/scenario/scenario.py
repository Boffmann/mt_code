import time
import subprocess

def getTimestamp(line):
    return line[0]

class Entry(object):

    def __init__(self, line:str):
        splitLine = line.split(";")
        self.position = float(splitLine[1])
        self.action = splitLine[2]
        self.baliseID = int(splitLine[3])
        self.reason = splitLine[4].strip()

class Scenario(object):

    def __init__(self, isRemoteExecution):
        self.nextLineID = 1
        self.lines = []
        self.isRemoteExecution = isRemoteExecution

    def run(self):
        self.execute_simulator()
        time.sleep(1)
        if self.isRemoteExecution:

            subprocess.run(["scp", "pi@192.168.178.121:~/revpi/build/scenario_evaluation.yml", "./scenario_evaluation121.yml"])
            subprocess.run(["scp", "pi@192.168.178.122:~/revpi/build/scenario_evaluation.yml", "./scenario_evaluation122.yml"])
            # subprocess.run(["scp", "pi@192.168.178.123:~/revpi/build/scenario_evaluation.yml", "./scenario_evaluation123.yml"])
            subprocess.run(["scp", "pi@192.168.178.124:~/revpi/build/scenario_evaluation.yml", "./scenario_evaluation124.yml"])

            eval121 = open('./scenario_evaluation121.yml', 'r').readlines()
            eval122 = open('./scenario_evaluation122.yml', 'r').readlines()
            # eval123 = open('./scenario_evaluation123.yml', 'r').readlines()
            eval124 = open('./scenario_evaluation124.yml', 'r').readlines()

            totalLines = []

            for line in eval121:
                totalLines.append(line)
            for line in eval122:
                totalLines.append(line)
            # for line in eval123:
            #     totalLines.append(line)
            for line in eval124:
                totalLines.append(line)

            totalLines.sort(key=getTimestamp)

            with open('../replica/build/scenario_evaluation.yml', 'w') as the_file:
                the_file.write("Time;Position;Action;BaliseNumber;Reason")
                for line in totalLines:
                    the_file.write(line)

        file = open('../replica/build/scenario_evaluation.yml')
        self.lines = file.readlines()

    def getNextTestLine(self) -> Entry:
        if len(self.lines) < self.nextLineID + 1:
            return None
        line = self.lines[self.nextLineID]
        self.nextLineID += 1
        return Entry(line)

    def printEvaluationFile(self):
        print("Evaluation File Content:")
        print("")
        for line in self.lines:
            print(line)

    def evaluateEntry(self, entry, min_position, max_position, action, baliseID, reason):
        if entry == None:
            self.printError(action + ": The entry was None")
            return False

        if entry.action != action:
            self.printError(action + ": Action was: " + entry.action + " but should have been: " + action)
            return False

        if entry.reason != reason:
            self.printError(action + ": Wrong reason: Reason should be " + reason  + " but was " + entry.reason)
            return False

        if entry.baliseID != baliseID:
            self.printError(action + ": Got Balise ID " + str(entry.baliseID) + " but should have been: " + str(baliseID))
            return False

        if entry.position < min_position:
            self.printError(action + ": Position was: " + str(entry.position) + " but should have been after" + str(min_position))
            return False

        if entry.position > max_position:
            self.printError(action + ": Position was: " + str(entry.position) + " but should have been before " + str(max_position))
            return False

        return True

    def printError(self, message):
        raise NotImplementedError("Subclass must override printError()")

    def printSuccess(self):
        raise NotImplementedError("Subclass must override printSuccess()")

    def execute_simulator(self):
        raise NotImplementedError("Subclass must override execute_simulator()")

    def evaluate(self) -> bool:
        raise NotImplementedError("Subclass must override evaluate()")

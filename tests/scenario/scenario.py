import time

class Entry(object):

    def __init__(self, line:str):
        splitLine = line.split(";")
        self.position = float(splitLine[0])
        self.action = splitLine[1]
        self.baliseID = int(splitLine[2])
        self.reason = splitLine[3].strip()

class Scenario(object):

    def __init__(self):
        self.nextLineID = 1
        self.lines = []

    def run(self):
        self.execute_simulator()
        time.sleep(1)
        # file = open('../replica/build/scenario_evaluation.yml')
        # self.lines = file.readlines()

    def getNextTestLine(self) -> Entry:
        if len(self.lines) < self.nextLineID + 1:
            return None
        line = self.lines[self.nextLineID]
        self.nextLineID += 1
        return Entry(line)

    def printEvaluationFile(self):
        print("Evaluation File Content:")
        print("")
        # for line in self.lines:
        #     print(line)

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

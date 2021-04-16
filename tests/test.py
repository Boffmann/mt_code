from scenario import ReachEndOfMA, BaliseNotLinked, BaliseNotWhereExpected

def main():
    reachEndOfMA = ReachEndOfMA()
    baliseNotLinked = BaliseNotLinked()
    baliseNotWhereExpected = BaliseNotWhereExpected()
    reachEndOfMA.run()
    reachEndOfMA.evaluate()
    baliseNotLinked.run()
    baliseNotLinked.evaluate()
    baliseNotWhereExpected.run()
    baliseNotWhereExpected.evaluate()

if __name__ == '__main__':
    main()

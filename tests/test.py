from scenario import ReachEndOfMA, BaliseNotLinked, BaliseNotWhereExpected

remoteExeuction = True

def main():
    reachEndOfMA = ReachEndOfMA(remoteExeuction)
    baliseNotLinked = BaliseNotLinked(remoteExeuction)
    baliseNotWhereExpected = BaliseNotWhereExpected(remoteExeuction)
    reachEndOfMA.run()
    reachEndOfMA.evaluate()
    baliseNotLinked.run()
    baliseNotLinked.evaluate()
    baliseNotWhereExpected.run()
    baliseNotWhereExpected.evaluate()

if __name__ == '__main__':
    main()

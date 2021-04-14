from scenario import ReachEndOfMA, BaliseNotLinked

def main():
    reachEndOfMA = ReachEndOfMA()
    baliseNotLinked = BaliseNotLinked()
    reachEndOfMA.run()
    reachEndOfMA.evaluate()
    baliseNotLinked.run()
    baliseNotLinked.evaluate()

if __name__ == '__main__':
    main()

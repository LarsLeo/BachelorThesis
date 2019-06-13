import click
import re
import numpy as np

@click.command()
@click.option('--path', default="../simulations/results/General-#0.out", help='Path to OMNeT++ Log file.')

def main(path):
    # These two dictionaries are identified by seq as key, Entries has botmaster publish time as value,
    # Receptions has receive times from superpeers as list of values. Receptions will be sorted, since
    # OMNeT++ is sequential by default.
    urlPackEntries = {}
    urlPackReceptions = {}

    logFile = open(path, "r")
    numPeers = extractEntities(logFile, urlPackEntries, urlPackReceptions)
    logFile.close()

    printStats(urlPackEntries, urlPackReceptions, numPeers)

def printStats(urlPackEntries, urlPackReceptions, numPeers):
    for seqNum, entryTime in urlPackEntries.items():
        receiptList = urlPackReceptions[seqNum]

        for i in np.arange(0.2, 1, 0.3):
            printPropagationTime(receiptList, i, numPeers, seqNum, entryTime)


def printPropagationTime(receiptList, percentage, numPeers, seqNum, entryTime):
    index = int(numPeers * percentage)
    if len(receiptList) > index:
        print("URL Pack number: ", seqNum, " ", percentage * 100 ,"% propagation after: " , receiptList[index] -  entryTime, "seconds")

def extractEntities(logFile, urlPackEntries, urlPackReceptions):
    numPeers = 0

    for line in logFile:
        line = line.strip()
        m = re.search(r"\[INFO\]	peer: id=(\d+) seq=(\d+) t=(\d+\.{0,1}\d+)", line)
        if m:
            #id = m.group(1) # id is currently not needed for statistics, could be used later on
            seq = m.group(2)
            t = m.group(3)

            if not seq in urlPackReceptions:
                urlPackReceptions[seq] = list()
            urlPackReceptions[seq].append(float(t))

        else:
            m = re.search(r"\[INFO\]	botmaster: seq=(\d+) t=(\d+\.{0,1}\d+)", line)
            if m:
                seq = m.group(1)
                t = m.group(2)
                urlPackEntries[seq] = float(t)
            else:
                m = re.search(r"number peers: (\d+)", line) 
                if m:
                    numPeers = int(m.group(1))
    
    return numPeers


if __name__ == '__main__':
    main()
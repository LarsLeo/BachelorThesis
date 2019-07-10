import click
import re
import numpy as np

from os import listdir
from os.path import isfile, join

@click.command()
@click.option('--path', default='../simulations/results', help='Path to OMNeT++ results directory, that contains the log files.')

def main(path):
    filesDict = {}

    extractFiles(filesDict, path)    
    extractAllStatistics(filesDict)
    
'''
Extracts all run files names and saves these names for the same simulation under the same key in filesDict.
'''
def extractFiles(filesDict, path):
    for f in listdir(path):
        filePath = join(path, f)
        
        if not isfile(filePath):
            continue
        
        m = re.search(r"General-(\d+)s-\#(\d+).out", f)
        if m:
            # runNumber = m.group(2) currently not used
            simTimeLimit = m.group(1)
            
            if not simTimeLimit in filesDict:
                filesDict[simTimeLimit] = list()
            filesDict[simTimeLimit].append(filePath)

'''
Extracts mean package loss/propagation for all simulations over the different seeds. 
'''
def extractAllStatistics(filesDict):
    for simTimeLimit, runFiles in filesDict.items():
        percentageDict = {}
        meanLoss = 0
        minDict = {}
        maxDict = {}
        
        for runFile in runFiles:
            runResults = extractFileData(runFile)
            for i in np.arange(0.2, 1, 0.2):
                if not i in percentageDict:
                    percentageDict[i] = list()
                if not i in minDict:
                    minDict[i] = list()
                if not i in maxDict:
                    maxDict[i] = list()

                percentageDict[i].append(runResults[0][i])
                minDict[i].append(runResults[2][i])
                maxDict[i].append(runResults[3][i])

            meanLoss = meanLoss + runResults[1]

        meanLoss = meanLoss / len(runFiles)

        print("Simulation - ", simTimeLimit, ":")
        for i in np.arange(0.2, 1, 0.2):
            percentageDict[i] = np.mean(percentageDict[i])
            minDelay = min(minDict[i])
            maxDelay = max(maxDict[i])
    
            print("  ", i, "% propagation: \n\t"
                , "mean distr. time: ", np.mean(percentageDict[i]), "\n\t",
                "min delay: ", minDelay, "\n\t",
                    "max delay: ", maxDelay)
        print("  ", "mean loss: ", meanLoss, "\n") 

'''
extracts relevant stats for a single run of a simulation.
Returns: (percentageDict, meanLoss, minDict, maxDict), where percentageDict is the dictionary, stating the propagation
time for x%, meanLoss is the mean URL Pack loss for this run, minDict and maxDict are dictionaries with min/max
propagation times for the given percentages.
'''
def extractFileData(filePath):
    # These two dictionaries are identified by seq as key, Entries has botmaster publish time as value,
    # Receptions has receive times from superpeers as list of values.
    urlPackEntries = {}
    urlPackReceptions = {}

    logFile = open(filePath, "r")
    numPeers = extractEntities(logFile, urlPackEntries, urlPackReceptions)
    logFile.close()

    return extractStats(urlPackEntries, urlPackReceptions, numPeers)

'''
Extracts the urlPackEntries, as well as urlPackReceptions from the logFile.
Returns total number of peers.
'''
def extractEntities(logFile, urlPackEntries, urlPackReceptions):
    numPeers = 0

    for line in logFile:
        line = line.strip()
        m = re.search(r"\[INFO\]	peer: id=(\d+) seq=(\d+) t=(\d+\.{0,1}\d+)", line)
        if m:
            #id = m.group(1) # id of the peer is currently not needed for statistics, could be used later on
            seq = m.group(2)
            t = m.group(3)

            if not seq in urlPackReceptions:
                urlPackReceptions[seq] = list()
            # receptions are sorted, since log is sequential by time and read each line
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

'''
Extracts meanLoss and mean propagation delay for a singular run.
Returns: (percentageDict, meanLoss, minDict, maxDict), where percentageDict is the dictionary, stating the propagation
time for x%, meanLoss is the mean URL Pack loss for this run, minDict and maxDict are dictionaries with min/max
propagation times for the given percentages.
'''
def extractStats(urlPackEntries, urlPackReceptions, numPeers):
    percentageDict = {}
    lossList = []
    minDict = {}
    maxDict = {}

    for seqNum, entryTime in urlPackEntries.items():
        receiptList = urlPackReceptions[seqNum]
        lossList.append(numPeers - len(receiptList))

        for i in np.arange(0.2, 1, 0.2):
            if not i in percentageDict:
                percentageDict[i] = list()

            extractPropagationTime(receiptList, i, numPeers, seqNum, entryTime, percentageDict)

    # for i in np.arange(0.2, 1, 0.2):
    #     print("Mean distribution time for ", i, "%: ", np.mean(percentageDict[i]))
    # print("Mean number of superpeers that do not receive a URL Pack: ", meanLoss)

    meanLoss = np.mean(lossList)

    for i in np.arange(0.2, 1, 0.2):
        minDict[i] = min(percentageDict[i])
        maxDict[i] = max(percentageDict[i])

    for key, val in percentageDict.items():
        percentageDict[key] = np.mean(val)

    return (percentageDict, meanLoss, minDict, maxDict)

'''
Extracts the propagation time of a URL Pack for a given percentage.
'''
def extractPropagationTime(receiptList, percentage, numPeers, seqNum, entryTime, percentageDict):
    index = int((numPeers * percentage) - 1)
    if (len(receiptList) > index) and (index >= 0) :
        percentageDict[percentage].append(receiptList[index] -  entryTime)
        #print("URL Pack number: ", seqNum, " ", percentage * 100 ,"% propagation after: " , receiptList[index] -  entryTime, "seconds")

if __name__ == '__main__':
    main()
import click
import re
import numpy as np
import csv

from os import listdir
from os.path import isfile, join

@click.command()
@click.option('--path', default='../simulations/results', help='Path to OMNeT++ results directory, that contains the log files.')

def main(path):
    # in the form {'botmasterVersion + ':' + percentageKnownPeers' : {'simTimeLimit' : list()}}
    filesDict = {}

    extractFiles(filesDict, path)  
    extractStatistics(filesDict)
    
'''
Extracts all simulation names and saves all files of the individual runs of the simulation
under the simulation name key in combination with the simTimeLimit in filesDict.
'''
def extractFiles(filesDict, path):
    for f in listdir(path):
        filePath = join(path, f)
        
        if not isfile(filePath):
            continue
        
        m = re.search(r"BV(\d+)-(\d+),(\d+)s-\#(\d+).out", f)
        if m:
            botmasterVersion = m.group(1)
            percentageKnownPeers = m.group(2)
            simTimeLimit = m.group(3)
            # runNumber = m.group(4) currently not used

            runName = botmasterVersion + ':' + percentageKnownPeers

            if not runName in filesDict:
                filesDict[runName] = {}
            
            if not simTimeLimit in filesDict[runName]:
                filesDict[runName][simTimeLimit] = list()

            filesDict[runName][simTimeLimit].append(filePath)

'''
Extracts mean package loss/propagation for all simulations over the different seeds. 
'''
def extractStatistics(filesDict):
    for runName, simTimeLimits in filesDict.items():
        for simTimeLimit, runFiles in simTimeLimits.items():
            percentageDict = {}
            meanLoss = list()
            minDict = {}
            maxDict = {}

            # save statistics for 0-100% distribution of URL packs
            for i in range(0, 101, 5):
                percentageDict[i] = list()
                minDict[i] = list()
                maxDict[i] = list()
            
            for runFile in runFiles:
                extractFileData(runFile, percentageDict, minDict, maxDict, meanLoss)

            meanLoss = np.mean(meanLoss)

            with open('results/' + runName + ':' + simTimeLimit + '.csv', 'w') as csvfile:
                filewriter = csv.writer(csvfile, delimiter=',',
                                        quotechar='|', quoting=csv.QUOTE_MINIMAL)
                filewriter.writerow(['Percentage', 'Propagation Time', 'Min Delay', 'Max Delay', 'Mean Loss'])

                #print("Simulation - ", runName, ", simTimeLimit - ", simTimeLimit, ":")
                for i in range(0, 101, 5):
                    minDict[i] = min(percentageDict[i])
                    maxDict[i] = max(percentageDict[i])
                    percentageDict[i] = np.mean(percentageDict[i])

                    filewriter.writerow([i, "{:20.2f}".format(percentageDict[i]), "{:20.2f}".format(minDict[i]), 
                                        "{:20.2f}".format(maxDict[i]), meanLoss])

                    # print("  ", i, "% propagation: \n\t",
                    #     "mean distribution time: ", percentageDict[i], "\n\t",
                    #     "min delay: ", minDict[i], "\n\t",
                    #     "max delay: ", maxDict[i], "  ", "mean loss: ", meanLoss, "\n")

'''
Extracts relevant stats (meanLoss, min/max distribution times of URL packs) for a single run of a simulation.
'''
def extractFileData(filePath, percentageDict, minDict, maxDict, meanLoss):
    # These two dictionaries are identified by sequence number as key, Entries has botmaster publish times as values,
    # Receptions has receive times from superpeers as list of values.
    urlPackEntries = {}
    urlPackReceptions = {}
    lossList = []

    logFile = open(filePath, "r")
    numPeers = extractEntities(logFile, urlPackEntries, urlPackReceptions)
    logFile.close()

    for seqNum, entryTime in urlPackEntries.items():
        receiptList = urlPackReceptions[seqNum]
        lossList.append(numPeers - len(receiptList))

        for i in range(0, 101, 5):
            if not i in percentageDict:
                percentageDict[i] = list()

            extractPropagationTime(receiptList, i, numPeers, seqNum, entryTime, percentageDict)

    meanLoss.append(np.mean(lossList))

'''
Extracts the urlPackEntries, as well as urlPackReceptions from the logFile.
Returns total number of peers in the botnet.
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
                else:
                    numPeers = 677
    
    return numPeers

'''
Extracts the propagation time of a URL Pack for a given percentage. 
If it has not reached the percentage a -1 denotes that.
'''
def extractPropagationTime(receiptList, percentage, numPeers, seqNum, entryTime, percentageDict):
    if percentage == 0:
        percentageDict[percentage].append(0)
    else:
        index = int(numPeers * percentage / 100 - 1)
        if (len(receiptList) > index) and (index >= 0) :
            percentageDict[percentage].append(receiptList[index] -  entryTime)
        else:
            percentageDict[percentage].append(-1)

if __name__ == '__main__':
    main()
import click
import re
import numpy as np
import csv

from os import listdir
from os.path import isfile, join

@click.command()
@click.option('--path', default='../simulations/results', 
    help='Path to OMNeT++ results directory, that contains the log files.')

def main(path):
    filesDict = {}

    extractFiles(filesDict, path)
    resultsDict = extractStatistics(filesDict)
    createCSVs(resultsDict)

def extractFiles(filesDict, path):
    for f in listdir(path):
        filePath = join(path, f)
        
        if not isfile(filePath):
            continue

        m = re.search(r"Crawler-PS(\d+)-(\d+)-\#(\d+).out", f)
        if m:
            peerSelectVersion = m.group(1)
            distributionPercentage = m.group(2)

            runName = 'Crawler-PS' + peerSelectVersion + '-' + distributionPercentage

            if not runName in filesDict:
                filesDict[runName] = list()

            filesDict[runName].append(filePath)

def extractStatistics(filesDict):
    resultsDict = {}
    for runName, runFiles in filesDict.items():
        resultsDict[runName] = list()
        for runFile in runFiles:
            resultsDict[runName].append(extractFileStatistics(runFile))
    
    return resultsDict
        
def extractFileStatistics(runFile):
    botmasterPeerList = list()
    statisticsDict = {}

    with open(runFile) as f:
        for line in f:
            m = re.search(r"crawler peers at t=(\d+):((\d+,)+)", line)
            if m:
                simTime = m.group(1)
                peerList = list(m.group(2).split(','))
                statisticsDict[simTime] = {}
                foundPeerCount = len(list(filter(
                    (lambda x: x in botmasterPeerList),
                    peerList)))
                statisticsDict[simTime]['precision'] = float(foundPeerCount) / len(peerList)
                statisticsDict[simTime]['recall'] = float(foundPeerCount) / len(botmasterPeerList)
            else:
                m = re.search(r"botmaster peers:((\d+,)+)", line)
                if m:
                    botmasterPeerList = list(m.group(1).split(','))

    return statisticsDict

def createCSVs(resultsDict):
    for runName, results in resultsDict.items():
         with open('results/simulation/' + runName + '.csv', 'w') as csvfile:
            filewriter = csv.writer(csvfile, delimiter=',',
                                    quotechar='|', quoting=csv.QUOTE_MINIMAL)
            filewriter.writerow(['Time', 'Precision', 'Recall'])

            for entry in results:
                for simTime, statisticsDict in entry.items():
                    filewriter.writerow([simTime, statisticsDict['precision'], statisticsDict['recall']])

if __name__ == '__main__':
    main()
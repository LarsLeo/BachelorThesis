import click
import re
import numpy as np
import csv
import collections

from os import listdir
from os.path import isfile, join

@click.command()
@click.option('--path', default='../simulations/results', 
    help='Path to OMNeT++ results directory, that contains the log files.')
@click.option('--evalversion', default=1, 
    help='''The version of the good count evaluation method.
    1: Geometric series averaging
    2: Maximum good count drop
    3: Maximum good count drop averaging''')

def main(path, evalversion):
    filesDict = {}

    extractFiles(filesDict, path)
    resultsDict = extractStatistics(filesDict, evalversion)
    resultsDict = averageResults(resultsDict)
    createCSVs(resultsDict, evalversion)

def averageResults(resultsDict):
    averagedDict = {}

    for runName, resultsList in resultsDict.items():
        averagedDict[runName] = {}

        for packNum, statisticsDict in resultsList[0].items():
            if not packNum in resultsList[1] or not packNum in resultsList[2]:
                continue

            averagePrecision = statisticsDict['precision']
            averageRecall = statisticsDict['recall']
            for i in range(1, 3):
                averagePrecision += resultsList[i][packNum]['precision']
                averageRecall += resultsList[i][packNum]['recall']
            averagePrecision /= 3
            averageRecall /= 3

            averagedDict[runName][packNum] = {'precision': averagePrecision, 'recall': averageRecall}
    
    return averagedDict

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
                filesDict[runName] = []

            filesDict[runName].append(filePath)

def extractStatistics(filesDict, evalVersion):
    resultsDict = {}
    for runName, runFiles in filesDict.items():
        resultsDict[runName] = []
        for runFile in runFiles:
            resultsDict[runName].append(extractFileStatistics(runFile, evalVersion))
    
    return resultsDict
        
def extractFileStatistics(runFile, evalVersion):
    botmasterPeerList = []
    statisticsDict = {}

    with open(runFile) as f: 
        for line in f: 
            m = re.search(r"GoodCounts after pack: (\d+):	(((\d+:\d+),)+)", line) 
            if m: 
                packNum = m.group(1)
                peerList = list(m.group(2).split(','))
                goodCountDict = extractGoodCounts(peerList)
                foundPeers = extractFoundPeers(goodCountDict, evalVersion)

                extractCrawlerStatistics(foundPeers, botmasterPeerList, statisticsDict, packNum)
            else:
                m = re.search(r"botmaster peers:((\d+,)+)", line)
                if m:
                    botmasterPeerList = list(m.group(1).split(','))
                    botmasterPeerList.remove('')

    return statisticsDict

def extractGoodCounts(peerList):
    goodCountDict = {}

    for entry in peerList:
        regM = re.search(r"(\d+):(\d+)", entry)
        if regM:
            peerId = regM.group(1)
            goodCount = int(regM.group(2))
            goodCountDict[peerId] = goodCount

    return goodCountDict

def extractCrawlerStatistics(foundPeers, botmasterPeerList, statisticsDict, packNum):
    statisticsDict[packNum] = {}
    numberRightPeers = len(list(
        filter(
            (lambda x: x in botmasterPeerList),
            foundPeers
        )
    ))

    statisticsDict[packNum]['precision'] = float(numberRightPeers) / len(foundPeers)
    statisticsDict[packNum]['recall'] = float(numberRightPeers) / len(botmasterPeerList)
 
def extractFoundPeers(goodCountDict, evalVersion):
    foundPeers = []

    sortedGoodCounts = collections.OrderedDict(
        sorted(goodCountDict.items(), key=lambda kv: kv[1], reverse=True)
    )

    targetGoodCount = 0
    
    if evalVersion == 1:
        targetGoodCount = goodCountGSA(sortedGoodCounts)
    elif evalVersion == 2:
        targetGoodCount = goodCountDrop(sortedGoodCounts)
    elif evalVersion == 3:
        targetGoodCount = goodCountDropAveraging(sortedGoodCounts)

    for peerId, goodCount in sortedGoodCounts.items():
        if goodCount >= targetGoodCount:
            foundPeers.append(peerId)

    return foundPeers

def goodCountDropAveraging(sortedGoodCounts):
    prevGoodCount = 0
    maxDrop = 0
    lastPeerId = 0

    # danach averagen mit den übrigen superpeers???
    for peerId, goodCount in sortedGoodCounts.items():
        if prevGoodCount == 0:
            prevGoodCount = goodCount
            continue

        drop = prevGoodCount - goodCount
        if drop >= maxDrop:
            maxDrop = drop
            lastPeerId = peerId

        prevGoodCount = goodCount

    targetGoodCount = 0    
    count = 0
    for peerId, goodCount in sortedGoodCounts.items():
        if peerId == lastPeerId:
            break
        
        targetGoodCount += goodCount
        count += 1

    return int(targetGoodCount / count)

def goodCountDrop(sortedGoodCounts):
    prevGoodCount = 0
    maxDrop = 0
    lastGoodCount = 0

    # danach averagen mit den übrigen superpeers???
    for goodCount in sortedGoodCounts.values():
        if prevGoodCount == 0:
            prevGoodCount = goodCount
            continue

        drop = prevGoodCount - goodCount
        if drop >= maxDrop:
            maxDrop = drop
            lastGoodCount = prevGoodCount

        prevGoodCount = goodCount
        
    return lastGoodCount

def goodCountGSA(sortedGoodCounts):
    targetGoodCount = 0 

    for goodCount in sortedGoodCounts.values():
        targetGoodCount += (float(goodCount) / 2 ** delta)
        delta += 1

    return int(targetGoodCount)

def createCSVs(resultsDict, evalVersion):
    for runName, results in resultsDict.items():
        evaluationString = ''
        if evalVersion == 1:
            evaluationString ='GSA'
        elif evalVersion == 2:
            evaluationString = 'DROP'
        elif evalVersion == 3:
            evaluationString = 'MGCDA'

        with open('results/simulation/' + runName + '-' + evaluationString + '.csv', 'w') as csvfile:
            filewriter = csv.writer(csvfile, delimiter=',',
                                    quotechar='|', quoting=csv.QUOTE_MINIMAL)
            filewriter.writerow(['PackNum', 'Precision', 'Recall'])
            
            for packNum, statisticsDict in results.items():
                filewriter.writerow([packNum, statisticsDict['precision'], statisticsDict['recall']])

if __name__ == '__main__':
    main()
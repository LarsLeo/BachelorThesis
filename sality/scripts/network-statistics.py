import click
import re
import collections
from functools import reduce

@click.command()
@click.option('--path', help='Path to GraphML file.', required=True)

def main(path):
    outgoingEdges = {}
    ingoingEdges = {}
    extractEntities(outgoingEdges, ingoingEdges, path)
    outgoingEdges = collections.OrderedDict(sorted(outgoingEdges.items(), 
        key = (lambda kv: len(kv[1])), reverse=True))
    extractStats(outgoingEdges, ingoingEdges)

def extractEntities(outgoingEdges, ingoingEdges, path):
    with open(path) as graphFile:
        for line in graphFile:
            line = line.strip()
            m = re.search(r"<edge source=\"(\w+)\" target=\"(\w+)\" />", line)
            if m:
                edgeSource = m.group(1)
                edgeTarget = m.group(2) 

                if not edgeSource in outgoingEdges:
                    outgoingEdges[edgeSource] = set()
                if not edgeTarget in outgoingEdges:
                    outgoingEdges[edgeTarget] = set()
                if not edgeTarget in ingoingEdges:
                    ingoingEdges[edgeTarget] = set()

                outgoingEdges[edgeSource].add(edgeTarget)
                ingoingEdges[edgeTarget].add(edgeSource)

def extractStats(outgoingEdges, ingoingEdges):
    # General calculations
    numberNodes = len(outgoingEdges)
    connections = list(map((lambda kv: kv[1]), outgoingEdges.items()))
    numberConnections = reduce((lambda x, y: x + y), 
        list(map((lambda x: len(x)), connections)))
    numberPossibleConnections = numberNodes * (numberNodes - 1)
    meanNumberConnections = numberConnections / numberNodes
    medianNumberConnections = len(connections[int(numberNodes / 2)])

    # Calculations for outgoing edge connectiveness
    closelyConnectedPercentage = 50
    badlyConnectedPercentage = 10
    numberCloselyConnectedPeers = len(list(filter(
        (lambda x: len(x) >= int(closelyConnectedPercentage / 100 * (numberNodes - 1))),
        connections)))
    outlierPeers = list(
        map(
        (lambda kv: kv[0]),
        filter(
        (lambda kv: len(kv[1]) <= int(badlyConnectedPercentage / 100 * (numberNodes - 1))),
        outgoingEdges.items())))
    numberOutlierPeers = len(outlierPeers)
    numberRegularPeers = numberNodes - numberCloselyConnectedPeers - numberOutlierPeers
    wellConnectedPeers = [x for x in outgoingEdges.keys() if x not in outlierPeers]

    # Calculations for ingoing edge connectiveness
    outlierPercentage = 1 # Outliers are superpeers that are in less than 1% of neighbourlists
    outlierDict = dict(filter(
        (lambda kv: len(kv[1]) <= int(outlierPercentage / 100 * (numberNodes - 1))),
        ingoingEdges.items()))
    outlierIds = outlierDict.keys()
    percentageOutliersInNetwork = len(outlierIds) / numberNodes * 100
    #longestOutlierChain = extractLongestOutlierPeerChain(outlierDict, outlierIds)
    numberOutliersOnlyKnownToOutliers = 0
    for neighbours in outlierDict.values():
        if len(list(filter((lambda x: x in outlierIds), neighbours))) == 0:
            numberOutliersOnlyKnownToOutliers += 1
    percentageTrueOutliers = numberOutliersOnlyKnownToOutliers / len(outlierIds) * 100

    # Extract percentage of wellConnectedPeers overlap with  coreNetwork
    overlap = len(list(filter(
        (lambda x: x not in outlierIds),
        wellConnectedPeers
    )))
    percentageOverlap = overlap / len(wellConnectedPeers) * 100

    printFormattedStats(numberNodes, numberConnections, numberPossibleConnections, meanNumberConnections,
        medianNumberConnections, closelyConnectedPercentage, numberCloselyConnectedPeers, badlyConnectedPercentage,
        numberOutlierPeers, numberRegularPeers, outlierPercentage,
        percentageOutliersInNetwork, percentageTrueOutliers, percentageOverlap)


def extractLongestOutlierPeerChain(outlierDict, outlierIds):
    longestOutlierChain = 1

    for k, v in outlierDict.items():
        for outlier in v:
            if outlier in outlierIds:
                visited = set()
                visited.add(k)
                currentChain = longestOutlierRec(1, outlierIds, outlierDict, outlier, visited)
                longestOutlierChain = currentChain if currentChain > longestOutlierChain else longestOutlierChain

    return longestOutlierChain

def longestOutlierRec(depth, outlierIds, outlierDict, outlier, visited):
    depth += 1
    localDepth = depth
    currentMax = depth
    visited.add(outlier)

    for neighbour in outlierDict[outlier]:
        if neighbour in outlierIds and neighbour not in visited:
            depth += longestOutlierRec(depth, outlierIds, outlierDict, neighbour, visited)

        currentMax = currentMax if depth < currentMax else depth
        depth = localDepth

    visited.remove(outlier)
    return currentMax

def printFormattedStats(numberNodes, numberConnections, numberPossibleConnections, meanNumberConnections,
        medianNumberConnections, closelyConnectedPercentage, numberCloselyConnectedPeers, badlyConnectedPercentage,
        numberOutlierPeers, numberRegularPeers, outlierPercentage,
        percentageOutliersInNetwork, percentageTrueOutliers, percentageOverlap):
    print("General stats:")
    print("  Number of superpeers:", numberNodes)
    print("  Total number of connections:", numberConnections)
    print("  Number of possible connections (n * (n - 1)):", numberPossibleConnections)
    print("  Connectiveness: %.2f" % (numberConnections / numberPossibleConnections * 100), "%")
    print("  Mean number of connections: %.2f" % (meanNumberConnections))
    print("  Median number of connections: ", medianNumberConnections)

    print("Neighbourlist connectiveness:")
    print("  Percentage closely connected (neighbourlist >= %i %% of botnet) superpeers: %.2f" %
        (closelyConnectedPercentage, numberCloselyConnectedPeers / numberNodes * 100), "%")
    print("  Percentage badly connected (neighbourlist <= %i %% of botnet) superpeers:  %.2f" % 
        (badlyConnectedPercentage, numberOutlierPeers / numberNodes * 100), "%")
    print("  Percentage regular (%i %% > neighbourlist > %i %% of botnet) superpeers:  %.2f" % 
        (closelyConnectedPercentage, badlyConnectedPercentage, numberRegularPeers / numberNodes * 100), "%")

    print("Superpeer reputation:")
    print("  Percentage outliers (known by <= %i %% of botnet): %.2f" %
        (outlierPercentage, percentageOutliersInNetwork), "%")
    print("  Percentage of isolated superpeers (outliers only known by outliers): %.2f" %
    (percentageTrueOutliers / 100 * percentageOutliersInNetwork), "%")
    
    print("  Percentage of well connected superpeers in core superpeers: %.2f %%" % (percentageOverlap))

if __name__ == '__main__':
    main()
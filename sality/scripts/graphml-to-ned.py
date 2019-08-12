import click
import re
import collections

@click.command()
@click.option('--path', help='Path to GraphML file.', required=True)
@click.option('--output', default='Sality.ned', help='Path to output NED file.')
@click.option('--pb', default=2, help="""Version of the botmaster used:
1: Passive, 2: Active.""")
@click.option('--ce', default=0, help="""Enable/Disable creation of a crawler:
0: No crawler, 1: Crawler created.""")

def main(path, output, ce, pb):
    edges = {}

    extractEntities(path, edges) # Extract all nodes and Edges from the Graphml file
    edges = collections.OrderedDict(sorted(edges.items(), key=lambda kv: len(kv[1]), reverse=True)) # Sort by number of connections
    peers = list(edges.keys()) # peers also are sorted by number of connections, index 0 == most connections.

    nedFile = open(output,"w+")
    writePreamble(nedFile, len(peers), ce)
    writeConnections(nedFile, peers , edges, ce)
    writePostamble(nedFile)

    nedFile.close()

def extractEntities(path, edges):
    graphFile = open(path, "r")
    for line in graphFile:
        line = line.strip()
        m = re.search(r"<edge source=\"(\w+)\" target=\"(\w+)\" />", line)
        if m:
            edgeSource = m.group(1)
            edgeTarget = m.group(2)   
            if not edgeSource in edges:
                edges[edgeSource] = list()
            if not edgeTarget in edges:
                edges[edgeTarget] = list()
            edges[edgeSource].append(edgeTarget)

    graphFile.close()

def writePreamble(nedFile, numPeers, ce):
    crawlerImport = "import sality.ned_files.Crawler;\n" if ce == 1 else ""
    crawlerCreation = "\t\tcrawler: Crawler;\n" if ce == 1 else ""

    preamble = "package sality.ned_files;\n\n" + \
        "import sality.ned_files.Superpeer;\n" + \
        crawlerImport + \
        "import sality.ned_files.Botmaster;\n\n" + \
        "network Sality\n{\n\ttypes:\n\t\tchannel Channel extends ned.DelayChannel\n" + \
        "\t\t{\n\t\t\tdelay = 50ms;\n\t\t}\n\tsubmodules:\n" + \
        "\t\tpeer["+str(numPeers)+"]: Superpeer;\n" + \
        crawlerCreation + \
        "\t\tbotmaster: Botmaster;\n" + \
        "\tconnections:\n"

    nedFile.write(preamble)

def writeConnections(nedFile, peers, edges, ce):
    for source, targets in edges.items():
        sourceString = generateConnectionString(source, peers)

        for target in targets:
            targetString = generateConnectionString(target, peers)
            nedFile.write("\t\t%s.outputGate++ <--> Channel <--> %s.inputGate++;\n" %(sourceString, targetString))
    
    # Create allmighty Crawler nodes
    if ce == 1:
        generateGodmodeConnection(nedFile, "crawler", len(peers))
    generateGodmodeConnection(nedFile, "botmaster", len(peers))

def generateConnectionString(node, peers):
    return ("peer[%d]" % (peers.index(node)))

def generateGodmodeConnection(nedFile, entity, numPeers):
    for i in range(numPeers):
        nedFile.write("\t\t%s.gate++ <--> Channel <--> peer[%d].inputGate++;\n" %(entity, i))

def writePostamble(nedFile):
    nedFile.write("}\n")
    
if __name__ == '__main__':
    main()
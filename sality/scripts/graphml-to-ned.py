import click
import re

@click.command()
@click.option('--path', help='Path to GraphML file.', required=True)
@click.option('--output', default='Sality.ned', help='Path to output NED file.')
@click.option('--n', default=1, help='Number of botmaster peers')
@click.option('--cv', default=1, help='Version of the crawler used')

def main(path, output, n, cv):
    peers = []
    botmasters = []
    edges = {}

    extractEntities(path, peers, edges) # Extract all nodes and Edges from the Graphml file
    createBotmasters(peers, botmasters, n) # Switch n peers to botmasters evenly distributed

    nedFile = open(output,"w+")
    writePreamble(nedFile, peers, botmasters, cv)
    writeConnections(nedFile, peers, botmasters, edges)
    writePostamble(nedFile)

    nedFile.close()

def extractEntities(path, peers, edges):
    graphFile = open(path, "r")
    for line in graphFile:
        line = line.strip()
        m = re.search(r"<node id=\"(\w+)\" />", line)
        if m:
            nodeID = m.group(1)
            peers.append(nodeID)
        else:
            m = re.search(r"<edge source=\"(\w+)\" target=\"(\w+)\" />", line)
            if m:
                edgeSource = m.group(1)
                edgeTarget = m.group(2)   
                if edgeSource in edges:
                    edges[edgeSource].append(edgeTarget)  
                else:
                    edges[edgeSource] = list(edgeTarget)
    graphFile.close()

def createBotmasters(peers, botmasters, n):
    numNodes = len(peers)
    modVal = int(numNodes / n)
    for i in range(1, n + 1):
        botmasters.append(peers.pop(numNodes - (i * modVal - 1)))

def writePreamble(nedFile, peers, botmasters, cv):
    preamble = "package sality.ned_files;\n\n" + \
        "import sality.ned_files.Superpeer;\n" + \
        "import sality.ned_files.crawlers.CrawlerV%d;\n" %(cv) + \
        "import sality.ned_files.Botmaster;\n\n"\
        "network Sality\n{\n\ttypes:\n\t\tchannel Channel extends ned.DelayChannel\n" + \
        "\t\t{\n\t\t\tdelay = 100ms;\n\t\t}\n\tsubmodules:\n" + \
        "\t\tpeer["+str(len(peers))+"]: Superpeer;\n" + \
        "\t\tbotmaster["+str(len(botmasters))+"]: Botmaster;\n" + \
        "\t\tcrawler: CrawlerV%d;\n" %(cv) + \
        "\tconnections:\n"

    nedFile.write(preamble)

def writeConnections(nedFile, peers, botmasters, edges):
    for source, targets in edges.items():
        sourceString = generateConnectionString(source, peers, botmasters)

        for target in targets:
            targetString = generateConnectionString(target, peers, botmasters)
            nedFile.write("\t\t%s.outputGate++ <--> Channel <--> %s.inputGate++;\n" %(sourceString, targetString))
    
    # Create allmighty Crawler node
    for i in range(len(peers)):
        nedFile.write(generateCrawlerConnectionString("peer", i))
    for i in range(len(botmasters)):
        nedFile.write(generateCrawlerConnectionString("botmaster", i))

def generateConnectionString(node, peers, botmasters):
    return ("peer[%d]" % (peers.index(node))) if (node in peers) else "botmaster[%d]" % (botmasters.index(node))

def generateCrawlerConnectionString(entity, i):
    return "\t\tcrawler.gate++ <--> Channel <--> %s[%d].inputGate++;\n" %(entity, i)

def writePostamble(nedFile):
    nedFile.write("}\n")
    
if __name__ == '__main__':
    main()
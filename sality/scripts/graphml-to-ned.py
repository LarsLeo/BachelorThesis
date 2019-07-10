import click
import re

@click.command()
@click.option('--path', help='Path to GraphML file.', required=True)
@click.option('--output', default='Sality.ned', help='Path to output NED file.')
@click.option('--pb', default=2, help="""Version of the botmaster used:
1: Passive, 2: Active.""")

# @click.option('--percentage', help="""percentage of botmaster peers to create,
# when using botmaster version 1, else percentage of superpeers the botmaster contacts""", default=10)

def main(path, output, pb):
    peers = []
    #botmasterPeers = []
    edges = {}

    extractEntities(path, peers, edges) # Extract all nodes and Edges from the Graphml file
    #createBotmasters(peers, botmasterPeers, percentage) # Switch n peers to botmasterPeers evenly distributed

    nedFile = open(output,"w+")
    writePreamble(nedFile, peers)
    writeConnections(nedFile, peers, edges)
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
                if not edgeSource in edges:
                    edges[edgeSource] = list()
                edges[edgeSource].append(edgeTarget)

    graphFile.close()

def writePreamble(nedFile, peers):
    preamble = "package sality.ned_files;\n\n" + \
        "import sality.ned_files.Superpeer;\n" + \
        "import sality.ned_files.Crawler;\n" + \
        "import sality.ned_files.Botmaster;\n\n" + \
        "network Sality\n{\n\ttypes:\n\t\tchannel Channel extends ned.DelayChannel\n" + \
        "\t\t{\n\t\t\tdelay = 50ms;\n\t\t}\n\tsubmodules:\n" + \
        "\t\tpeer["+str(len(peers))+"]: Superpeer;\n" + \
        "\t\tcrawler: Crawler;\n" + \
        "\t\tbotmaster: Botmaster;\n" + \
        "\tconnections:\n"

    nedFile.write(preamble)

def writeConnections(nedFile, peers, edges):
    for source, targets in edges.items():
        sourceString = generateConnectionString(source, peers)

        for target in targets:
            targetString = generateConnectionString(target, peers)
            nedFile.write("\t\t%s.outputGate++ <--> Channel <--> %s.inputGate++;\n" %(sourceString, targetString))
    
    # Create allmighty Crawler nodes
    generateGodmodeConnection(nedFile, "crawler", len(peers))
    generateGodmodeConnection(nedFile, "botmaster", len(peers))

def generateConnectionString(node, peers):
    return ("peer[%d]" % (peers.index(node)))

def generateGodmodeConnection(nedFile, entity, numPeers):
    for i in range(numPeers):
        nedFile.write("\t\t%s.gate++ <--> Channel <--> peer[%d].inputGate++;\n" %(entity, i))

def writePostamble(nedFile):
    nedFile.write("}\n")

# def createBotmasters(peers, botmasterPeers, percentage):
#     numNodes = len(peers)
#     percentageBotmasters = int(numNodes * percentage / 100)
#     numBotmasterPeers = percentageBotmasters if percentageBotmasters > 0 else 1
#     print("Number of botmasterPeers: ", numBotmasterPeers)
#     stepSize = int(numNodes / numBotmasterPeers)
#     for i in range(0, numBotmasterPeers):
#         print(numNodes - (i * stepSize) - 1)
#         botmasterPeers.append(peers.pop(numNodes - (i * stepSize) - 1))
    
if __name__ == '__main__':
    main()
import click
import pandas as pd
import numpy as np
import datetime
import csv
import re
from os import listdir
from os.path import isfile, join

@click.command()
@click.option('--directory', default='messages/', help='Directory of the message file')
#@click.option('--seqNum', required=True, help='The sequence number of the URL Pack to analyze')

def main(directory):
    # seqNum, time
    distributionTimes = {}
    # seqNum, dataframe
    dataframes = {}

    # initialize dicts
    for i in range(2, 101, 2):
        distributionTimes[i] = list()
    distributionTimes[0] = [0]

    extractDataframes(dataframes, directory)

    for seqNum, df in dataframes.items():
        extractPackPropagation(seqNum, df, distributionTimes)
    
    with open('results/sality/V3packDistribution.csv', 'w') as csvfile:
        filewriter = csv.writer(csvfile, delimiter=',',
                                quotechar='|', quoting=csv.QUOTE_MINIMAL)
        filewriter.writerow(['Percentage', 'Propagation Time', 'Min Delay', 'Max Delay'])
        
        for i in range(0, 101, 2):
            minDelay = min(distributionTimes[i])
            maxDelay = max(distributionTimes[i])
            meanDelay = np.mean(distributionTimes[i])  
            filewriter.writerow([i, meanDelay, minDelay, maxDelay])

'''
Extracts all message logs of URL pack distributions into dataframes.
'''
def extractDataframes(dataframes, directory):
    for f in listdir(directory):
        filePath = join(directory, f)

        if not isfile(filePath):
            continue
            
        m = re.search(r"messages_for_v3_pack_(\d+).csv", f)
        seqNum = m.group(1)
        df = pd.read_csv(filePath, sep='\t')
        dataframes[seqNum] =  cleanData(df, int(seqNum))

'''
Drops unnecessary columns from dataframe and sorts new pack arrival by timestamp for IPs.
Returns the new cleaned dataframe.
'''
def cleanData(df, packid):
    df = df.loc[df['PackID'] == packid] 
    df = df.loc[df['NodeType'] == 'server']
    df.drop(columns=['Port', 'PackID', 'CMD', 'NodeType'], inplace=True)
    df.sort_values(by=['Timestamp'], inplace=True)
    df.drop_duplicates(inplace=True, keep='first', subset='IP')

    return df
    
'''
Extracts propagation statistics for a singular URL pack.
'''
def extractPackPropagation(seqNum, df, distributionTimes):
    numPeers = df.shape[0]
    startTime = datetime.datetime.strptime(df.iloc[0, 0], '%Y-%m-%d %H:%M:%S')
    print('Sequence number: ', seqNum)
    print('\tTotal number of peers: ', numPeers)
    print('\tPack first seen: ', startTime)

    for i in range(2, 101, 2):
        index = int(i / 100 * numPeers) - 1

        currentTime = datetime.datetime.strptime(df.iloc[index, 0], '%Y-%m-%d %H:%M:%S')
        distributionTimes[i].append((currentTime - startTime).total_seconds())
    
if __name__ == '__main__':
    main()
import click
import pandas as pd
import numpy as np
import datetime

@click.command()
@click.option('--packid', required=True, help='Pack ID to be evaluated in the V3 Sality botnet.')
@click.option('--directory', default='messages/', help='Directory of the message file')

def main(packid, directory):
    filename = "messages_for_v3_pack_" + packid + ".csv"
    packid = int(packid)
    df = pd.read_csv(directory + filename, sep='\t')

    print("Analysis for PackID {}".format(packid))

    df = cleanData(df, packid)
    extractPackPropagation(df)

'''
Drops unnecessary columns from dataframe and sorts new pack arrival by timestamp for IPs.
Returns the new cleaned dataframe.
'''
def cleanData(df, packid):
    df = df.loc[df['PackID'] == packid] 
    df = df.loc[df['CMD'] == 'hello']
    df = df.loc[df['NodeType'] == 'client']
    df = df.drop(columns=['Port', 'PackID', 'CMD', 'NodeType'])
    df = df.sort_values(by=['Timestamp'])
    df.drop_duplicates(inplace=True, keep='first', subset='IP')

    return df
    
def extractPackPropagation(df):
    numPeers = df.shape[0]
    startTime = datetime.datetime.strptime(df.iloc[0, 0], '%Y-%m-%d %H:%M:%S')
    print('Total number of peers: ', numPeers)
    print('Pack first seen: ', startTime)

    for i in range(0, 101, 5):
        index = int(i / 100 * numPeers) - 1 if i > 0 else 0

        #propagationTime = df.iloc[index, 0] - startTime
        currentTime = datetime.datetime.strptime(df.iloc[index, 0], '%Y-%m-%d %H:%M:%S')
        print((currentTime - startTime).total_seconds())
    

if __name__ == '__main__':
    main()
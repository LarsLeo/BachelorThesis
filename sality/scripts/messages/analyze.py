import sys
from datetime import datetime, timedelta
from pathlib import Path
import pickle

def _getFirstTimestamp(filename, tau):
    cache_file = Path(filename + "." + str(tau.seconds) + "_ts_pack")
    if cache_file.exists(): 
        with open(str(cache_file), "rb") as f:
            return pickle.load(f)

    with open(filename, "r") as f:
        ts_pack = min(datetime.strptime(l.split("\t")[0], "%Y-%m-%d %H:%M:%S") for i, l in enumerate(f) if i!=0 and l.strip() and l.split("\t")[5].strip() == pack_id)

    with open(str(cache_file), "wb") as f:
        pickle.dump(ts_pack, f)
    return ts_pack

def _getIPsInTimespan(filename, tau, ts_pack):
    cache_file = Path(filename + "." + str(tau.seconds) + "_ips")
    if cache_file.exists(): 
        with open(str(cache_file), "rb") as f:
            return pickle.load(f)

    updated_servers = set()
    updated_clients = set()
    timespan_servers = set()
    timespan_clients = set()
    
    with open(filename, "r") as f:
        for i, l in enumerate(f):
            if i == 0: continue
            l = l.strip()
            if not l: continue
            r = l.split("\t")
            
            ts = datetime.strptime(r[0], "%Y-%m-%d %H:%M:%S")
            if ts < ts_pack: continue
            if ts > ts_pack+tau: continue
    
            if r[3] == "client": timespan_clients.add(r[1])
            elif r[3] == "server": timespan_servers.add(r[1])
    
            if r[5] != pack_id: continue
            if r[3] == "client": updated_clients.add(r[1])
            elif r[3] == "server": updated_servers.add(r[1])

    updated_hosts = updated_servers.union(updated_clients)
    timespan_hosts = timespan_servers.union(timespan_clients)

    result = (updated_hosts, updated_servers, updated_clients,
        timespan_hosts, timespan_servers, timespan_clients)

    with open(str(cache_file), "wb") as f:
        pickle.dump(result, f)
    return result

def _extractMessages(filename, tau, ts_pack, updated_hosts):
    cache_file = Path(filename + "." + str(tau.seconds) + "_messages")
    if cache_file.exists(): 
        with open(str(cache_file), "rb") as f:
            return pickle.load(f)

    lines = list()
    
    with open(filename, "r") as f:
        for i, l in enumerate(f):
            if i == 0: continue
            l = l.strip()
            if not l: continue
            r = l.split("\t")
    
            # - Allow IP
            if r[1] in updated_hosts:
                lines.append(l)
            
            ts = datetime.strptime(r[0], "%Y-%m-%d %H:%M:%S")
            if ts < ts_pack: continue
            if ts > ts_pack+tau: continue
            # - Allow timestamp
            lines.append(l)

    with open(str(cache_file), "wb") as f:
        pickle.dump(lines, f)
    return lines

def main(filename, pack_id, tau_int):
    tau = timedelta(seconds=int(tau_int))
    print("Analysis for PackID {} and Timespan {}".format(pack_id, tau))

    # Get first timestamp
    ts_pack = _getFirstTimestamp(filename, tau)
    print("Pack first seen on {}".format(ts_pack))

    # Get updated IPs within the timespan
    (updated_hosts, updated_servers, updated_clients,
        timespan_hosts, timespan_servers, timespan_clients) = _getIPsInTimespan(filename, tau, ts_pack)

    # Get messages from updated IPs and messages within timestamp
    lines = _extractMessages(filename, tau, ts_pack, updated_hosts)

    # Host Statistics
    print("Number of updated servers: {} (out of {})".format(len(updated_servers), len(timespan_servers)))
    print("\t{}".format(updated_servers))
    print("Number of updated clients: {} (out of {})".format(len(updated_clients), len(timespan_clients)))
    print("Number of updated hosts: {} (out of {})".format(len(updated_hosts), len(timespan_hosts)))

    # Missed Packets
    (extended_hosts, extended_servers, extended_clients,
        _, _, _) = _getIPsInTimespan(filename, timedelta(seconds=tau.seconds+12), ts_pack)
    print("Potentially missed {} (out of {}) updated servers".format(len(extended_servers - timespan_servers), len(extended_servers)))
    print("\t{}".format(extended_servers - timespan_servers))
    print("Potentially missed {} updated clients".format(len(extended_clients - timespan_clients)))
    print("Potentially missed {} updated hosts".format(len(extended_hosts - timespan_hosts)))
   

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('Require pack ID and timespan in seconds', file=sys.stderr)
        exit(1)

   
    pack_id = sys.argv[1]
    tau = sys.argv[2]
    filename = "messages_for_v3_pack_"+pack_id+".csv"

    main(filename, pack_id, tau)

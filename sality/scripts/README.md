# Base directory for all scripts

### graphml-to-ned.py:
- Arguments: --path to graphml file, --output name of NED file, --pb version of botmaster used, --ce crawler enabled.
- Parse graphml files, that contain node and edge definitions via the graphml-to-ned.py script. 
- The output is a NED file, that contains all the nodes, as well as a crawler if ce == 1.
- Use graphml-to-ned.py --help to gain more information on the usage.

### distribution-statistics.py
- Arguments: --path to the results directory.
- Print statistics about URL Pack distribution for different botmaster distribution methods.
- These statistics include mean, min, max distribution times, as well as package loss.
- The output are csv files that are saved under ./results/simulation.
- Use distribution-statistics.py --help to gain more information on the usage.

### sality-statistics.py
- Arguments: --directory path to the directory the Sality messages lie in. The default is in ./messages.
- Prints statistics about URL Pack distribution of the messages from the Strobo Crawler.
- These statistics are the same as for distribution-statistics.py and used for comparison.
- The output are csv files that are saved under ./results/sality.
- Use sality-statistics.py --help to gain more information on the usage.

### network-statistics.py
- Arguments: --path to the graphml file.
- Print statistics about a Sality snapshot.
- These statistics include number of superpeers, mean connections per superpeer, median connections per superpeer,
connectiveness, set of closely, loosely, regularly connected superpeers, outliers, isolated superpeers.
- Use network-statistics.py --help to gain more information on the usage.

### crawlerVisualizer.ipynb
- Python notebook that takes the output of the ./results directory and visualizes the results.
- Figures are saved in ./figures.

### botmasterStrategiesVisualizer.ipynb
- Python notebook that takes the output of the ./results directory and visualizes the results.
- Figures are saved in ./figures.

### resultsZipper.sh
- Creates tarball backups of the content in ./results and saves them in ./tarballResults

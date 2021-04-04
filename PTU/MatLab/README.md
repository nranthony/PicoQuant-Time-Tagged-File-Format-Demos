# Matlab MEX

`ptu_data.cpp` is compiled into `ptu_data.mexw64` for use in Matlab.  See below and `workflow.m` for details on usage.

## Installation

Add MatLab folder to path if cloning, or copy `ptu_data.mexw64` to a matlab path location if not.  

## Usage

use char array for file path
note '' creates char arrays, while "" creates Matlab strings

```matlab
% pull data from ptu file
% macrot: counts in units of pulse/sync
% microt: tcspc bin in units of time resolution
% decay: histogram of all counts
% meta: cell array of related metadata from ptu header
% fpath: full filepath as char, not Matlab string
% boolean flag: 1,0 for display header in command window
[macrot, microt, decay, meta] = ptu_data(fpath, 1);
```

Note, currently only works for 1000 TCSPC channels (actually using 1024 as channels upto approx 1006 may be populated depending on ps timming accuracy of PQ hardware).

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## Original Matlab
Read_PTU.m was the original code to read TTTR data from ptu, but is very slow and outputs to txt file
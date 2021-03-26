# Matlab MEX

`ptu_data.cpp` is compiled into `ptu_data.mexw64` for use in Matlab.  See below and `import_ptu_notes.m` for details on usage

## Installation

Add MatLab folder to path if cloning, or copy `ptu_data.mexw64` to a matlab path location if not.  Only `ptu_data.mexw64` is needed.

## Usage

use char array for file path
note '' creates char arrays, while "" creates Matlab strings

```matlab
[macrot, microt, decay] = ptu_data(fpath);
```

returns three arrays:

- `decay:`  histogram of photon counts into TCSPC time channels
- `macrot:`  count arrival times in units of laser pulses (also called sync clock)
- `microt:`  TCSPC collection channel number after laser pulse

note currently only works for 1000 TCSPC channels (actually using 1024 as channels upto approx 1006 may be populated depending on ps timming accuracy of PQ hardware


## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.


## License
TBD
Fork of PicoQuant/PicoQuant-Time-Tagged-File-Format-Demos repo.  Lower part of readme is their original readme.

This fork serves as a starting point for adapting their initial T2 and T3 files to text output (very inefficient for most cases; great to visualize and learn about on smaller files).  The end goal is to efficiently split (scanned FCS data), filter (using photon filters like LFCS), correlate (auto correlation function (ACF), spatio-temporal autocorrelation, both with semi-log multi-tau temporal binning).  See https://www.cell.com/biophysj/fulltext/S0006-3495(14)04751-1 for a detailed application of these techniques.



# PicoQuant Time Tagged File Format (ptu, phu)

Demo Code for PicoQuants Time Tagged File Formats

## Changelog
### 26.03.19
* updated LabVIEW demos, PTU demo was buggy

### 15.02.2019
* adapted C demos to compile under Linux also
* updated DOC to most recent tag dictionary

### 20.09.2018
* updated PTU demos to support MultiHarp 150
* updated DOC to most recent tag dictionary, removed links to pq-forum
* PHU demos remain unchanged

## Disclaimer

This demo software is provided free of charge, as is, without any guaranteed fitness for a specific purpose and without any liability for damage resulting from its use.

**Please check the doc folder for a documentation of the file format**

%% notes on usage of ptu_data.cpp as a Matlab mex function
% add folder containing this .m file and the .mexw64 files to the Malab
% path


%% compile if needed - mexw64 file should work as is on most Windows machines
% requires compiler; recommend VS2019
%clc
%mex ptu_data.cpp

%% use char array filepaths - see/change below example

% note '' creates char arrays, while "" creates Matlab strings
fpath = 'C:\ici-cloud-sections\WBRB Abberior STED\2021\Neil\2021-02-14\sFCStest_1_TimeHarp_2021-02-14_12-17-43.ptu';
% convert backslash to forward slash;  other option would be to convert to
% \\ to avoid \ being an escape character
fpath = strrep(fpath,'\','/');


%% pull data from ptu file
% returns three arrays
% decay:  histogram of photon counts into TCSPC time channels
% macrot: count arrival times in units of laser pulses (also called sync clock)
% microt: TCSPC collection channel number after laser pulse

% note currently only works for 1000 TCSPC channels (actually using 1024 as
% channels upto approx 1006 may be populated depending on ps timming
% accuracy of PQ hardware

[macrot, microt, decay] = ptu_data(fpath);
 
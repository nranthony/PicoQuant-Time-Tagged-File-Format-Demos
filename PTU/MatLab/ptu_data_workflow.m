%%  Set path

% note '' creates char arrays, while "" creates Matlab strings
% change to your path
fpath = 'C:\ici-cloud-sections\WBRB Abberior STED\2021\Neil\2021-02-26\TimeHarp_2021-02-26_16-59-34_40pcLP_180s_lightsOff.ptu';
% convert backslash to forward slash;  other option would be to convert to
% \\ to avoid \ being an escape character
fpath = strrep(fpath,'\','/');

%% compile if needed - mexw64 file should work as is on most Windows machines
% requires compiler; recommend VS2019
%clc
%mex ptu_data.cpp

% pull data from ptu file
% macrot: counts in units of pulse/sync
% microt: tcspc bin in units of time resolution
% decay: histogram of all counts
% meta: cell array of related metadata from ptu header
% fpath: full filepath as char, not Matlab string
% boolean flag: 1,0 for display header in command window
[macrot, microt, decay, meta] = ptu_data(fpath, 1);


%% Time Symmetric ACF
% see https://github.com/nranthony/FCS-Time-Symmetric-Analysis for correctedFCS(...)
% alternatively, use cross_corr(...), https://www.mathworks.com/matlabcentral/fileexchange/64605-photon-arrival-time-correlation


% generate log scaled tau
cx = lag_time(23,14);
% optional: view lag times
%semilogy(cx);

% note correctedFCS uses parfor, and needs parallel processing kit
% change to for if not installed
% first run will take longer, but subsequent runs will be significantly
% quicker depending on core count
tic;
[cor, pwave] = correctedFCS(decay, macrot, microt, cx);
cor = cor - 1;
toc;

%% plot results - change units and edit limits for your needs

% convert number of pulses to time scaled abcissa (here 25ns pulse to pulse
% window
cxus = double(cx) .* 0.025;  % converts to µs
semilogx(cxus(1:length(cor)),cor);
% y limits will likely need to be updated
ylim([0.0 0.1]);
xlim([0.1 1e6])








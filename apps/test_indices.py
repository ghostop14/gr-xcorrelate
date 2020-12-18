#!/usr/bin/python3

import math
d_num_inputs=16
d_num_baselines = int((d_num_inputs+1)*d_num_inputs/2)
d_num_channels=1024
d_integration_time=3
d_npol = 2

print('Testing Parameters:')
print('Number of stations: ' + str(d_num_inputs))
print('Number of baselines: ' + str(d_num_baselines))
print('Number of channels: ' + str(d_num_channels))
print('Number of polarizations: ' + str(d_npol))
print('Integration time: ' + str(d_integration_time))
print('')
print('Total i range: (channels*baselines): ' + str(d_num_channels*d_num_baselines))
print('')

for i in range(0,d_num_channels*d_num_baselines):
    f = i//d_num_baselines
    k = i - f * d_num_baselines
    station1=int(-0.5 + math.sqrt(0.25+2*k))
    station2=int(k - (station1+1)*station1/2)
    print('i:' + str(i) + ',  Station Pair: [' + str(station1) + ',' + str(station2) + '], Frequency: ' + str(f))
    
    for t in range(0, d_integration_time):
        input_index1 = ((t * d_num_channels + f) * d_num_inputs + station1)*d_npol
        input_index2 = ((t * d_num_channels + f) * d_num_inputs + station2)*d_npol
        # print('inputRowX and ColX Index (integ_time ' + str(t) + '): ' + str(input_index1) + ',  ' + str(input_index2))

#!/usr/bin/python3

import math
d_num_inputs=4
d_num_baselines = int((d_num_inputs+1)*d_num_inputs/2)
for i in range(0,10):
	f = i//10
	k = i - f * d_num_baselines
	station1=int(-0.5 + math.sqrt(0.25+2*k))
	station2=int(k - (station1+1)*station1/2)
	print('[' + str(station1) + ',' + str(station2) + ']')


#!/usr/bin/env python
# 

from gnuradio import gr
import numpy

class VectorMaxIndexToVar(gr.sync_block):
    """
    This block extracts the index of the max value and controls a variable.
    """
    def __init__(self, callback, lock, minCorrScore,  vec_len):
        gr.sync_block.__init__(self, name="VectorMaxIndexToVar", in_sig=[(numpy.float32, vec_len)], out_sig=None)

        self.callback = callback
        self.lock = lock
        self.minCorrScore = minCorrScore
        self.vec_len = vec_len

    def work(self, input_items, output_items):
        if self.lock:
            return len(input_items[0])
            
        max_val = input_items[0][0][0]
        max_index = 0
        
        for cur_vector in range(0, len(input_items[0])):
            in0 = input_items[0][cur_vector]
        
            cur_max = in0[0]
            cur_max_index = 0
            
            for i in range(1, self.vec_len):
                if in0[i] > cur_max:
                    cur_max = in0[i]
                    cur_max_index = i
            if cur_max > max_val:
                max_val = cur_max
                max_index = cur_max_index
                
        if (max_val > self.minCorrScore):
            new_delay = max_index - len(in0)/2
            
            if self.callback:
                self.callback(new_delay)
            
        return len(input_items[0])

    def set_lock(self, lock):
        self.lock = lock

    def stop(self):
        return True

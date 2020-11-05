#!/usr/bin/env python
# 

from gnuradio import gr
from gnuradio import blocks, fft

class xcorrelate_fft_vcf(gr.hier_block2):
    """
    This block performs cross-correlation in the frequency domain with FFT vector inputs and the correlation terms output.
    """
    def __init__(self, vlen):
        gr.hier_block2.__init__(self,"xcorrelate_fft_vcf",
            gr.io_signature(2, 2, gr.sizeof_gr_complex*vlen),  # Input signature
            gr.io_signature(1, 1, gr.sizeof_float*vlen)) # Output signature
        
        # Multiply conjugate
        mult_conj = blocks.multiply_conjugate_cc(vlen)
        
        # Reverse FFT, vec_len, Reverse, [1.0,]]*vec_len, no shift
        rev_fft = fft.fft_vcc(vlen, False, [1.0, ]*vlen,  False,  1)
        
        # Complex to mag - vlen
        cc_to_mag = blocks.complex_to_mag(vlen)

        # Vector to stream
        vec_to_stream = blocks.vector_to_stream(gr.sizeof_float,  vlen)
        
        # De-interleave
        deinterleave = blocks.deinterleave(gr.sizeof_float*1, int(vlen/2))
        
        # Cross de-interleave into interleave to swap FFT
        interleave = blocks.interleave(gr.sizeof_float*1, int(vlen/2))
        
        # Stream to Vector - vlen
        stream_to_vec = blocks.stream_to_vector(gr.sizeof_float*1,  vlen)

        # Make all the connections
        self.connect((self, 0), (mult_conj, 0))
        self.connect((self, 1), (mult_conj, 1))
        self.connect(mult_conj, rev_fft,  cc_to_mag, vec_to_stream,  deinterleave)
        self.connect((deinterleave, 0), (interleave, 1))
        self.connect((deinterleave, 1), (interleave, 0))
        self.connect(interleave, stream_to_vec, self)

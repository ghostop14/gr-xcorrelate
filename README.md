# gr-xcorrelate

XCorrelate is dedicated to perfroming cross-correlation of signals such that they can be constructively added.  The primary block is xcorrelate which takes 2 or more inputs (0..N) and performs a normalized cross correlation of in1..n against the reference signal at in0.  The resulting output is a best-correlation vector and an optimal lag scalar that can be used with the ExtractDelay block in this OOT to control a delay block.

There are several example flowgraphs including a test flowgraph that lets you control the input delay to watch the block adapt, and cross-correlation examples for 2, 3, and 4 input setups.  Note that for the xcorr_radio2, 3, and 4 input examples, a radio station is used as the target with RTLSDR's.  As a result, gr-correctiq blocks are used there to remove the DC spike, and gr-lfast's FFT Filter wrapper is used for better CPU performance on the required filters.  So you may want to install those OOT's as well with those examples.

With the tunable settings set in the flowgraph (including block CPU affinity pinning), the correlation flowgraphs run at about 50-60% CPU on an older i7 6th gen CPU/laptop.  The following combinations appear to produce decent correlations.  A radio station with audio out decoding was used to audibly detect when the flowgraph was having issues keeping up such as choppy output, audio underruns, or input overruns.

These settings worked okay on the test system:
 - RTL-SDR (2, 3, and 4 RTL-SDRs simultaneously): Sample rate: 2.4e6, frame size: 8192, max_search: 1000, 1-in-N frames: 4 to 6
 - UHD with UI controls: sample rate: *25e6*, frame size: 8192, max_search 500, 1-in-N frames: 100
 - UHD with no GUI: sample rate: *40e6*, frame size: 8192, max_search 400, 1-in-N frames: 200

Note: For the multiple RTL-SDR setup, a kerberossdr setup gave the best output quality.

Also keep an eye out on gr-clenabled as this will get ported to OpenCL for GPU processing soon.

## Building
Build is pretty standard:

mkdir build

cd build

cmake ..

make

[sudo if necessary] make install

sudo ldconfig



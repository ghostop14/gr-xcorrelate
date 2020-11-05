# gr-xcorrelate

XCorrelate is dedicated to perfroming cross-correlation of signals such that they can be constructively added.  There are both time-domain and frequency-domain correlation blocks available.  The time domain block is xcorrelate which takes 2 or more inputs (0..N) and performs a normalized cross correlation of in1..n against the reference signal at in0.  The resulting output is a PDU with the best-correlation vector and an optimal lag scalar that can be used with the ExtractDelay block in this OOT to control a delay block.  A variation of that called "xcorrelate stream" outputs the full correlation vector as well.  There is also a frequency-domain version "xcorrelate freq" that takes in two already-FFT'd vectors and performs the correlation in the frequency domain.  This latter approach is less CPU-intensive.

There are numerous example flowgraphs for both time and frequency domain versions, including a test flowgraph that lets you control the input delay to watch the block adapt, and cross-correlation examples for 2, 3, and 4 input setups.  Note that for the xcorr_radio2, 3, and 4 input examples, a radio station is used as the target with RTLSDR's.  As a result, gr-correctiq blocks are used there to remove the DC spike, and gr-lfast's FFT Filter wrapper is used for better CPU performance on the required filters.  So you may want to install those OOT's as well with those examples, or simply switch to another SDR and remove those blocks.

## Time Domain Notes
One design element that was incorporated to keep flowgraph performance optimal is two runtime modes: asynchronous or sequential.  Since we're acting as a sink block and don't need to output realtime streams of data, async mode can take a block of samples for processing, and pass them to another thread for processing in parallel.  The work function can then just return that it processed the samples until the other thread's processing is complete.  At which point the worker thread will signal the work function that it should produce the appropriate PDUs on the main thread and pick up the next block.  This allows the block to correlate as quickly as possible without holding up processing, and should allow for correlation at any flowgraph sample rate.  This also prevents the delay blocks from receiving buffer changes every frame.  In sequential mode, the work function will block until processing is completed, which in some scenarios may be the more appropriate approach.  Async is the default mode for the block.

Another design element that was included is some user-level tuning using 3 configurable parameters:

1. Analysis Window - This defines how many samples should be considered for a single correlation calculation.  The default is 8192.

2. Max Index Search Range - This defines how many samples in either shift direction (forward/backward) should be analyzed.  The default is 512.  More searches does require extra processing time, so this parameter also serves as one mechanism to regulate processing time.  Also, if correlation gets too small at the end of the analysis window, it is expected that incorrect offsets could be returned.  So setting this value to say 10-30% of the analysis window minimizes potentially incorrect results.

3. Keep 1 in N Frames - This mechanism can be used to minimize how frequently new delay updates are generated.  For stationary signals, the delays may not change that frequently, so no need to burden down the system and keep producing new varying delay values.  By default this value is 4 and applies to both asynchronous and sequential modes (in async mode, the 1-in-N is calculated from when processing completes and is less deterministic.  In sequential mode, it is truly a deterministic calcualtion).

These settings worked well on the test system:
 - RTL-SDR (2, 3, and 4 RTL-SDRs simultaneously): Sample rate: 2.4e6, frame size: 8192, max_search: 1000, 1-in-N frames: 4 to 6
 - UHD with UI controls: sample rate: *25e6*, frame size: 8192, max_search 500, 1-in-N frames: 100
 - UHD with no GUI: sample rate: *40e6*, frame size: 8192, max_search 400, 1-in-N frames: 200

Note: For the multiple RTL-SDR setup, a kerberossdr setup gave the best output quality.

For those with GPUs available, there is also an OpenCL implementation of this block in gr-clenabled (just make sure if you're on GNURadio 3.8 that you select the maint-3.8 branch when cloning).

## Building
Build is pretty standard:

mkdir build

cd build

cmake ..

make

[sudo if necessary] make install

sudo ldconfig



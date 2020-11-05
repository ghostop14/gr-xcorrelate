/* -*- c++ -*- */
/*
 * Copyright 2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cppunit/TextTestRunner.h>
#include <cppunit/XmlOutputter.h>

#include <gnuradio/unittests.h>
#include <gnuradio/block.h>
#include <iostream>
#include <fstream>
#include <boost/algorithm/string/replace.hpp>
#include <math.h>  // fabsf
#include <chrono>
#include <ctime>

#include "xcorrelate_impl.h"
#include "xcorrelate_fft_vcf2_impl.h"

bool verbose=false;
int largeBlockSize=8192;
int iterations = 100;
int maxsearch=512;
int num_inputs = 2;
int decimation = 1;
int input_type=XCORR_FLOAT;

class comma_numpunct : public std::numpunct<char>
{
  protected:
    virtual char do_thousands_sep() const
    {
        return ',';
    }

    virtual std::string do_grouping() const
    {
        return "\03";
    }
};

bool testXCorrelate() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing time-domain float xcorrelate with " << largeBlockSize << " data points." << std::endl;

	int input_size;

	gr::xcorrelate::xcorrelate_impl *test=NULL;
	try {
		if (input_type == XCORR_COMPLEX) {
			input_size = sizeof(gr_complex);
		}
		else {
			input_size=sizeof(float);
		}

		test = new gr::xcorrelate::xcorrelate_impl(num_inputs, largeBlockSize, input_type, input_size, maxsearch, decimation,0,false);
	}
	catch (...) {
		std::cout << "ERROR: error setting up environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	float frequency_signal = 10;
	float frequency_sampling = largeBlockSize*frequency_signal;
	float curPhase = 0.0;
	float signal_ang_rate = 2*M_PI*frequency_signal / frequency_sampling;

	std::vector<gr_complex> inputItems_complex;
	std::vector<gr_complex> inputItems_real;
	std::vector<gr_complex> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		inputItems_complex.push_back(gr_complex(sin(curPhase+(signal_ang_rate*i)),cos(curPhase+(signal_ang_rate*i))));
		inputItems_real.push_back(signal_ang_rate*i);
		outputItems.push_back(grZero);
	}

	for (int i=0;i<num_inputs;i++) {
		if (input_type == XCORR_COMPLEX) {
			inputPointers.push_back((const void *)&inputItems_complex[0]);
		}
		else {
			inputPointers.push_back((const void *)&inputItems_real[0]);
		}
	}

	for (int i=0;i<num_inputs-1;i++) {
		outputPointers.push_back((void *)&outputItems[0]);
	}

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	noutputitems = test->work_test(largeBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	float elapsed_time,throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	// ----------------------------------------------------------------------
	// Clean up
	if (test != NULL) {
		delete test;
	}

	inputPointers.clear();
	outputPointers.clear();
	inputItems_complex.clear();
	inputItems_real.clear();
	outputItems.clear();
	ninitems.clear();

	return true;
}

bool testFFTXCorrelate() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing frequency domain xcorrelate with " << largeBlockSize << " data points." << std::endl;

	int input_size;

	float p2 = log2(largeBlockSize);

	int new_largeBlockSize = (int)pow(2,ceil(p2));
	if (new_largeBlockSize != largeBlockSize) {
		std::cout << "Adjusting large block size to " << new_largeBlockSize << " for power-of-2 boundary" << std::endl;
		largeBlockSize = new_largeBlockSize;
	}

	gr::xcorrelate::xcorrelate_fft_vcf2_impl *test=NULL;
	try {
		test = new gr::xcorrelate::xcorrelate_fft_vcf2_impl(largeBlockSize);
	}
	catch (...) {
		std::cout << "ERROR: error setting up environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	float frequency_signal = 10;
	float frequency_sampling = largeBlockSize*frequency_signal;
	float curPhase = 0.0;
	float signal_ang_rate = 2*M_PI*frequency_signal / frequency_sampling;

	std::vector<gr_complex> inputItems_complex;
	std::vector<gr_complex> inputItems_real;
	std::vector<gr_complex> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		inputItems_complex.push_back(gr_complex(sin(curPhase+(signal_ang_rate*i)),cos(curPhase+(signal_ang_rate*i))));
		inputItems_real.push_back(signal_ang_rate*i);
		outputItems.push_back(grZero);
	}

	for (int i=0;i<num_inputs;i++) {
		inputPointers.push_back((const void *)&inputItems_complex[0]);
	}

	for (int i=0;i<num_inputs-1;i++) {
		outputPointers.push_back((void *)&outputItems[0]);
	}

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	// Note: working with vectors so work length = 1.
	noutputitems = test->work_test(1,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(1,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	float elapsed_time,throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	// ----------------------------------------------------------------------
	// Clean up
	if (test != NULL) {
		delete test;
	}

	inputPointers.clear();
	outputPointers.clear();
	inputItems_complex.clear();
	inputItems_real.clear();
	outputItems.clear();
	ninitems.clear();

	return true;
}

int
main (int argc, char **argv)
{
	// Add comma's to numbers
	std::locale comma_locale(std::locale(), new comma_numpunct());

	// tell cout to use our new locale.
	std::cout.imbue(comma_locale);

	if (argc > 1) {
		// 1 is the file name
		if (strcmp(argv[1],"--help")==0) {
			std::cout << std::endl;
			std::cout << "Usage: [--input_complex] [--num_inputs=#] [--maxsearch=<search_depth>] [number of samples (default is 8192)]" << std::endl;
			std::cout << "If not specified, maxsearch will default to 512." << std::endl;
			std::cout << "--input_complex will switch from float to complex inputs to the test routine." << std::endl;
			std::cout << std::endl;
			exit(0);
		}

		for (int i=1;i<argc;i++) {
			std::string param = argv[i];

			if (param.find("--maxsearch") != std::string::npos) {
				boost::replace_all(param,"--maxsearch=","");
				maxsearch=atoi(param.c_str());
			}
			else if (param.find("--num_inputs") != std::string::npos) {
				boost::replace_all(param,"--num_inputs=","");
				num_inputs=atoi(param.c_str());
			}
			else if (strcmp(argv[i],"--input_complex")==0) {
				input_type = XCORR_COMPLEX;
			}
			else if (atoi(argv[i]) > 0) {
				int newVal=atoi(argv[i]);

				largeBlockSize=newVal;
				std::cout << "Running with user-defined test buffer size of " << largeBlockSize << std::endl;
			}
			else {
				std::cout << "ERROR: Unknown parameter." << std::endl;
				exit(1);

			}
		}
	}
	bool was_successful;

	was_successful = testXCorrelate();

	if (num_inputs>2) {
		std::cout << "WARNING: Frquency domain correlation only supports 2 inputs.  Below score is still 2-signal comparison" << std::endl;
	}
	was_successful = testFFTXCorrelate();
	std::cout << std::endl;

	return 0;
}


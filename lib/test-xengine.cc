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

#include "xcorrelate_engine_impl.h"

bool verbose=false;
int num_channels=1024;
int num_inputs = 16;
// single_polarization=false = X/Y dual polarization
bool single_polarization = false;
int integration_time = 1024;
int iterations = 4;
int num_procs=0;

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

	std::cout << "Testing Full xengine cross-correlation with the following parameters: " << std::endl;
	std::cout << "Number of stations: " << num_inputs << std::endl;
	std::cout << "Number of baselines: " << (num_inputs+1)*num_inputs/2 << std::endl;
	std::cout << "Number of channels: " << num_channels << std::endl;
	std::cout << "Polarization: ";
	if (single_polarization) {
		std::cout << "single" << std::endl;
	}
	else {
		std::cout << "X/Y" << std::endl;
	}

	std::cout << "Integration time (NTIME): " << integration_time << std::endl;
	int polarization;

	gr::xcorrelate::xcorrelate_engine_impl *test=NULL;
	try {
		if (single_polarization) {
			polarization = 1;
		}
		else {
			polarization = 2;
		}

		// The one specifies output triangular order rather than full matrix.
		test = new gr::xcorrelate::xcorrelate_engine_impl(polarization, num_inputs, 1, num_channels, integration_time, num_procs);
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
	float frequency_sampling = num_channels*frequency_signal;
	float curPhase = 0.0;
	float signal_ang_rate = 2*M_PI*frequency_signal / frequency_sampling;

	std::vector<gr_complex> inputItems_complex;
	std::vector<gr_complex> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<num_channels*integration_time;i++) {
		inputItems_complex.push_back(gr_complex(sin(curPhase+(signal_ang_rate*i)),cos(curPhase+(signal_ang_rate*i))));
		outputItems.push_back(grZero);
	}

	for (int i=0;i<polarization;i++) {
		for (int i=0;i<num_inputs;i++) {
			inputPointers.push_back((const void *)&inputItems_complex[0]);
		}
	}

	/*
	for (int i=0;i<num_inputs-1;i++) {
		outputPointers.push_back((void *)&outputItems[0]);
	}
	*/

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	// Note: output items is 1 since it's expecting vectors in.
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
	throughput = num_channels*integration_time / elapsed_time;

	std::cout << "Elapsed time: " << elapsed_seconds.count() << std::endl;
	std::cout << "Timing Averaging Iterations: " << iterations << std::endl;
	std::cout << "Average Run Time:   " << std::fixed << std::setw(11) << std::setprecision(6) << elapsed_time << " s" << std::endl <<
				"Total throughput: " << std::setprecision(2) << throughput << " complex samples/sec" << std::endl <<
				"Synchronized stream (" << num_inputs << " inputs) throughput: " << throughput / num_inputs << " complex samples/sec" << std::endl << std::endl;

	// ----------------------------------------------------------------------
	// Clean up
	if (test != NULL) {
		delete test;
	}

	inputPointers.clear();
	outputPointers.clear();
	inputItems_complex.clear();
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
			std::cout << "Usage: [--single-polarization] [--num_inputs=#] [--integration-time=#] [--omp_threads=n] [number of channels (default is " << num_channels << ")]" << std::endl;
			std::cout <<"--num_inputs=n  Number of stations.  Default is " << num_inputs  << "." << std::endl;
			std::cout <<"--integration-time=n  Default is " << integration_time << "." << std::endl;
			std::cout <<"--omp_threads=n  Default is max available." << std::endl;
			std::cout <<"--single-polarization If not specified, correlation will assume X and Y polarizations per input." << std::endl;
			std::cout << std::endl;
			exit(0);
		}

		for (int i=1;i<argc;i++) {
			std::string param = argv[i];

			if (param.find("--num_inputs") != std::string::npos) {
				boost::replace_all(param,"--num_inputs=","");
				num_inputs=atoi(param.c_str());
			}
			else if (param.find("--integration-time") != std::string::npos) {
				boost::replace_all(param,"--integration-time=","");
				integration_time=atoi(param.c_str());
			}
			else if (param.find("--omp_threads") != std::string::npos) {
				boost::replace_all(param,"--omp_threads=","");
				num_procs=atoi(param.c_str());
			}
			else if (strcmp(argv[i],"--single-polarization")==0) {
				single_polarization=true;
			}
			else if (atoi(argv[i]) > 0) {
				int newVal=atoi(argv[i]);

				num_channels=newVal;
				std::cout << "Running with user-defined channel count of " << num_channels << std::endl;
			}
			else {
				std::cout << "ERROR: Unknown parameter." << std::endl;
				exit(1);

			}
		}
	}
	bool was_successful;

	was_successful = testXCorrelate();

	std::cout << std::endl;

	return 0;
}


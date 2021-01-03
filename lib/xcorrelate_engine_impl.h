/* -*- c++ -*- */
/*
 * Copyright 2020 ghostop14.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_XCORRELATE_XCORRELATE_ENGINE_IMPL_H
#define INCLUDED_XCORRELATE_XCORRELATE_ENGINE_IMPL_H

#include <xcorrelate/xcorrelate_engine.h>
#include <boost/thread/thread.hpp>
#include <xcorrelate/xcomplexstruct.h>

#define XCORR_TRIANGULAR_ORDER 1
#define XCORR_FULL_MATRIX 2

namespace gr {
namespace xcorrelate {

class xcorrelate_engine_impl : public xcorrelate_engine
{
protected:
	// Tracking pointers
	gr_complex *complex_input;
	gr_complex *output_matrix;
	gr_complex *thread_complex_input;
	gr_complex *thread_output_matrix;
	int current_write_buffer;

	// Actual Memory
	gr_complex *complex_input1;
	gr_complex *output_matrix1;
	gr_complex *complex_input2;
	gr_complex *output_matrix2;
	int d_npol;
	int d_num_inputs;
	int d_output_format;
	int d_num_channels;
	int d_num_baselines;
	int d_integration_time;
	int integration_tracker;
	int frame_size;
	int input_size;
	int num_chan_x2;
	size_t matrix_flat_length;
	int output_size;
	int num_procs;

	// For async mode, threading:
	boost::thread *proc_thread=NULL;
	bool threadRunning=false;
	bool stop_thread = false;
	bool thread_is_processing=false;
	bool thread_process_data=false;

	void cxmac(XComplex& accum, XComplex& z0, XComplex& z1) {
		accum.real += z0.real * z1.real + z0.imag * z1.imag;
		accum.imag += z0.imag * z1.real - z0.real * z1.imag;
	}

	virtual void runThread();

public:
	xcorrelate_engine_impl(int polarization, int num_inputs, int output_format, int num_channels, int integration, int omp_threads=0);
	virtual ~xcorrelate_engine_impl();

	bool stop();

	long get_input_buffer_size() { return d_num_inputs * d_num_channels * d_npol * d_integration_time; };
	long get_output_buffer_size() { return matrix_flat_length; };

	void xcorrelate(XComplex *input_matrix, XComplex *cross_correlation);

	// void forecast (int noutput_items, gr_vector_int &ninput_items_required);

	int work_test(
			int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items
	);
	int work(
			int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items
	);
};

} // namespace xcorrelate
} // namespace gr

#endif /* INCLUDED_XCORRELATE_XCORRELATE_ENGINE_IMPL_H */


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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "xcorrelate_impl.h"
#include <volk/volk.h>

namespace gr {
namespace xcorrelate {

xcorrelate::sptr
xcorrelate::make(int num_inputs, int signal_length,int data_type, int data_size, int max_search_index,int decim_frames)
{
	return gnuradio::get_initial_sptr
			(new xcorrelate_impl(num_inputs, signal_length, data_type, data_size, max_search_index, decim_frames));
}


/*
 * The private constructor
 */
xcorrelate_impl::xcorrelate_impl(int num_inputs, int signal_length, int data_type, int data_size, int max_search_index,
		int decim_frames)
: gr::sync_block("xcorrelate",
		gr::io_signature::make(2, num_inputs, data_size),
		gr::io_signature::make(0, 0, 0)),
		d_num_inputs(num_inputs), d_signal_length(signal_length), d_data_type(data_type),
		d_data_size(data_size), d_decim_frames(decim_frames), cur_frame_counter(1)
{
	if (data_size == 0) {
		// Had to wait to get insider here for access to d_logger
		GR_LOG_ERROR(d_logger, "Unknown data type.");
		exit(1);
	}

	// Set up local buffers
	size_t mem_alignment = volk_get_alignment();
	int mem_alloc_size = d_signal_length * sizeof(float);
	ref_mag_buffer = (float *)volk_malloc(mem_alloc_size, mem_alignment);
	mag_buffer = (float *)volk_malloc(mem_alloc_size, mem_alignment);
	xy_buffer = (float *)volk_malloc(mem_alloc_size, mem_alignment);
	xx_buffer = (float *)volk_malloc(mem_alloc_size, mem_alignment);
	yy_buffer = (float *)volk_malloc(mem_alloc_size, mem_alignment);
	sqrt_buffer = (float *)volk_malloc(mem_alloc_size, mem_alignment);

	correlation_factors = new float[num_inputs-1];
	corrective_lag = new int[num_inputs-1];

	message_port_register_out(pmt::mp("corr"));

	if (max_search_index > 0) {
		max_shift = max_search_index;
	}
	else {
		max_shift = (int)(0.7 * (float)d_signal_length);
	}

	// signal_length represents how long a signal in terms of chunk of samples
	// that we want to analyze for correlation.
	gr::block::set_output_multiple(d_signal_length);
}

bool xcorrelate_impl::stop() {
	if (correlation_factors) {
		delete[] correlation_factors;
		correlation_factors = NULL;
	}

	if (corrective_lag) {
		delete[] corrective_lag;
		corrective_lag = NULL;
	}

	if (ref_mag_buffer) {
		volk_free(ref_mag_buffer);
		ref_mag_buffer = NULL;
	}


	if (ref_mag_buffer) {
		volk_free(ref_mag_buffer);
		ref_mag_buffer = NULL;
	}

	if (mag_buffer) {
		volk_free(mag_buffer);
		mag_buffer = NULL;
	}

	if (xy_buffer) {
		volk_free(xy_buffer);
		xy_buffer = NULL;
	}

	if (xx_buffer) {
		volk_free(xx_buffer);
		xx_buffer = NULL;
	}

	if (yy_buffer) {
		volk_free(yy_buffer);
		yy_buffer = NULL;
	}

	if (sqrt_buffer) {
		volk_free(sqrt_buffer);
		sqrt_buffer = NULL;
	}

	return true;
}
/*
 * Our virtual destructor.
 */
xcorrelate_impl::~xcorrelate_impl()
{
	bool retval = stop();
}

void
xcorrelate_impl::xcorr(int num_items, float& corr, int& lag) {
	// Calc y squared just once
	volk_32f_x2_multiply_32f(yy_buffer,mag_buffer,mag_buffer,d_signal_length);

	float sum_xy;
	float sum_x2;
	float sum_y2;
	float cur_corr;
	int calc_len;
	float denom;

	corr = -99.0;  // calc values should be -1 to 1, so this is an initialization
	lag = 0;

	for (int ref_start=0; ref_start<max_shift;ref_start++) {
		calc_len = num_items - ref_start;

		// Calculate sum(x*y) / sqrt(sum(x^2) * sum(y^2))

		// Calc for this shift with in2 forward
		// x:     0   1   2   3   4   5
		// y:         0   1   2   3   4

		// x*y
		volk_32f_x2_multiply_32f(xy_buffer,&ref_mag_buffer[ref_start],mag_buffer,calc_len);
		volk_32f_accumulator_s32f(&sum_xy,xy_buffer,calc_len);

		// x^2 is already calculated once.  Just need to grab the indices we need
		volk_32f_accumulator_s32f(&sum_x2,&xx_buffer[ref_start],calc_len);

		// y^2 is already calculated once.  Just need to grab the indices we need
		volk_32f_accumulator_s32f(&sum_y2,yy_buffer,calc_len);

		denom = sum_x2 * sum_y2;

		if (denom != 0.0) {
			cur_corr = sum_xy / sqrt(sum_x2 * sum_y2);
		}
		else {
			cur_corr = -1.0;
			std::cout << "ERROR: one of the sum terms is zero." << std::endl;
			std::cout << "Current shift: " << ref_start << std::endl;
			std::cout << "calc_len: " << calc_len << std::endl;
			std::cout << "xx[ref_start]: " << xx_buffer[ref_start] << std::endl;
			std::cout << "sum_x2: " << sum_x2 << std::endl;
			std::cout << "sum_y2: " << sum_y2 << std::endl;
		}

		if (cur_corr > corr) {
			corr = cur_corr;
			lag = ref_start;
		}

		// Calc for this shift with ref signal forward
		// x:             0   1   2   3
		// y:     0   1   2   3   4   5
		// x*y
		volk_32f_x2_multiply_32f(xy_buffer,&mag_buffer[ref_start],ref_mag_buffer,calc_len);
		volk_32f_accumulator_s32f(&sum_xy,xy_buffer,calc_len);

		// x^2 is already calculated once.  Just need to grab the indices we need
		volk_32f_accumulator_s32f(&sum_x2,xx_buffer,calc_len);

		// y^2 is already calculated once.  Just need to grab the indices we need
		volk_32f_accumulator_s32f(&sum_y2,&yy_buffer[ref_start],calc_len);

		denom = sum_x2 * sum_y2;

		if (denom != 0.0) {
			cur_corr = sum_xy / sqrt(sum_x2 * sum_y2);
		}
		else {
			cur_corr = -1.0;
			std::cout << "ERROR: one of the sum terms is zero." << std::endl;
			std::cout << "Current shift: " << ref_start << std::endl;
			std::cout << "calc_len: " << calc_len << std::endl;
			std::cout << "yy[ref_start]: " << yy_buffer[ref_start] << std::endl;
			std::cout << "sum_x2: " << sum_x2 << std::endl;
			std::cout << "sum_y2: " << sum_y2 << std::endl;
		}

		if (cur_corr > corr) {
			corr = cur_corr;
			lag = -ref_start;
		}
	}

	// std::cout << "Best Correlation: " << corr << std::endl;
	// std::cout << "Best Lag: " << lag << std::endl << std::endl;
}

int
xcorrelate_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	if (noutput_items < d_signal_length) {
		return 0;
	}

	gr::thread::scoped_lock guard(d_setlock);

	if (d_decim_frames > 1) {
		// Let's see if we should drop some frames.
		if ((cur_frame_counter++ % d_decim_frames) == 0) {
			cur_frame_counter = 1;
		}
		else {
			return d_signal_length;
		}
	}
	// Processing note:
	// We set output multiple to d_signal_length in the constructor,
	// so noutput_items will be a multiple of, and >= d_signal_length.

	// Calculate the reference signal mag just once
	if (d_data_type == XCORR_COMPLEX) {
		const gr_complex *ref_signal = (const gr_complex *) input_items[0];
		volk_32fc_magnitude_32f(ref_mag_buffer,ref_signal,d_signal_length);
	}
	else {
		// already got complex->mag or float inputs, just need to move it
		// to the mag buffer
		const float *ref_signal = (const float *) input_items[0];

		memcpy(ref_mag_buffer, ref_signal,d_signal_length*d_data_size);
	}

	// Calculate the reference signal squared just once.
	volk_32f_x2_multiply_32f(xx_buffer,ref_mag_buffer,ref_mag_buffer,d_signal_length);

	// Cross correlation will shift signal noutput_items forward and backward
	// and calculate a normalized correlation factor for each shift.

	float corr;
	int lag;

	for (int signal_index=1;signal_index<d_num_inputs;signal_index++) {
		if (d_data_type == XCORR_COMPLEX) {
			const gr_complex *in2 = (const gr_complex *) input_items[signal_index];
			volk_32fc_magnitude_32f(mag_buffer,in2,d_signal_length);
		}
		else {
			const float *next_signal = (const float *) input_items[signal_index];
			memcpy(mag_buffer, next_signal,d_signal_length*d_data_size);
		}
		xcorr(d_signal_length, corr, lag);

		// Output vector is # of signals - 1 since xcorr of 1 with itself will always be 1.
		correlation_factors[signal_index-1] = corr;
		corrective_lag[signal_index-1] = lag;
	}

	pmt::pmt_t meta = pmt::make_dict();
	pmt::pmt_t corr_out(pmt::init_f32vector(d_num_inputs-1,correlation_factors));
	meta = pmt::dict_add(meta, pmt::mp("corrvect"), corr_out);
	pmt::pmt_t lag_out(pmt::init_s32vector(d_num_inputs-1,corrective_lag));
	meta = pmt::dict_add(meta, pmt::mp("corrective_lags"), lag_out);

	pmt::pmt_t pdu = pmt::cons(meta, pmt::PMT_NIL);
	message_port_pub(pmt::mp("corr"), pdu);

	// Tell runtime system how many output items we produced.
	return d_signal_length;
}

} /* namespace xcorrelate */
} /* namespace gr */


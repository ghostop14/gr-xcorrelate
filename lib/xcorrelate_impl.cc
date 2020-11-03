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
xcorrelate::make(int num_inputs, int signal_length,int data_type, int data_size, int max_search_index,int decim_frames, int num_outputs, bool async)
{
	return gnuradio::get_initial_sptr
			(new xcorrelate_impl(num_inputs, signal_length, data_type, data_size, max_search_index, decim_frames, num_outputs, async));
}


/*
 * The private constructor
 */
xcorrelate_impl::xcorrelate_impl(int num_inputs, int signal_length, int data_type, int data_size, int max_search_index,
		int decim_frames, int num_outputs, bool async)
: gr::sync_decimator("xcorrelate",
		gr::io_signature::make(2, num_inputs, data_size),
		gr::io_signature::make(0, num_outputs, sizeof(float)*2*signal_length),num_outputs==0?1:signal_length),
		d_num_inputs(num_inputs), d_signal_length(signal_length), d_data_type(data_type),
		d_data_size(data_size), d_decim_frames(decim_frames), max_shift(max_search_index), d_num_outputs(num_outputs), d_async(async), cur_frame_counter(1)
{
	if (data_size == 0) {
		// Had to wait to get insider here for access to d_logger
		GR_LOG_ERROR(d_logger, "Unknown data type.");
		exit(1);
	}

	if ( (d_signal_length % 2) > 0) {
		GR_LOG_ERROR(d_logger, "Signal length must be a multiple of 2.");
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

	if (max_search_index == 0) {
		max_shift = (int)(0.7 * (float)d_signal_length);
	}

	if ( (max_shift % 2) > 0) {
		GR_LOG_INFO(d_logger, "Adjusting max search to a multiple of 2.");
		max_shift += 1;
	}

	if (d_async) {
		if (d_data_type == XCORR_COMPLEX) {
			d_input_buffer_complex = new gr_complex[d_signal_length * d_num_inputs];
		}
		else {
			d_input_buffer_real = new float[d_signal_length * d_num_inputs];
		}

		proc_thread = new boost::thread(boost::bind(&xcorrelate_impl::runThread, this));
	}

	// signal_length represents how long a signal in terms of chunk of samples
	// that we want to analyze for correlation.
	if (d_num_outputs == 0) {
		gr::block::set_output_multiple(d_signal_length);
	}
}

bool xcorrelate_impl::stop() {
	if (proc_thread) {
		stop_thread = true;

		while (threadRunning)
			usleep(10);

		delete proc_thread;
		proc_thread = NULL;

		if (d_input_buffer_complex) {
			delete d_input_buffer_complex;
			d_input_buffer_complex = NULL;
		}

		if (d_input_buffer_real) {
			delete d_input_buffer_real;
			d_input_buffer_real = NULL;
		}
	}

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
xcorrelate_impl::xcorr(int num_items, float& corr, int& lag, float *corr_buffer) {
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

		if (corr_buffer) {
			corr_buffer[d_signal_length + ref_start] = cur_corr;
		}

		// Check if we're shifting.  No need to work twice if shift is zero.
		if (ref_start > 0) {
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

			if (corr_buffer) {
				corr_buffer[d_signal_length - ref_start] = cur_corr;
			}
		}
	}

	// std::cout << "Best Correlation: " << corr << std::endl;
	// std::cout << "Best Lag: " << lag << std::endl << std::endl;
}

int
xcorrelate_impl::work_test(int noutput_items,
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
		xcorr(d_signal_length, corr, lag, NULL);

		// Output vector is # of signals - 1 since xcorr of 1 with itself will always be 1.
		correlation_factors[signal_index-1] = corr;
		corrective_lag[signal_index-1] = lag;
	}

	// Tell runtime system how many output items we produced.
	return d_signal_length;
}

int
xcorrelate_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	gr::thread::scoped_lock guard(d_setlock);

	if (!d_async) {
		if (d_decim_frames > 1) {
			// Let's see if we should drop some frames.
			if ((cur_frame_counter++ % d_decim_frames) == 0) {
				cur_frame_counter = 1;
			}
			else {
				if (d_num_outputs == 0) {
					return d_signal_length;
				}
				else {
					return 1;
				}
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

			if (d_num_outputs > 0) {
				float *corr_out = (float *) output_items[signal_index-1];
				xcorr(d_signal_length, corr, lag, corr_out);
			}
			else {
				xcorr(d_signal_length, corr, lag, NULL);
			}

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

		if (d_num_outputs == 0) {
			return d_signal_length;
		}
		else {
			return 1;
		}
	}
	else {
		if (noutput_items < d_signal_length) {
			return 0;
		}

		// Async mode.  See if the thread is currently processing any data or not.
		if (!thread_process_data) {
			if (d_decim_frames > 1) {
				// For async mode, we'll need to do this here.
				// Let's see if we should drop some frames.
				if ((cur_frame_counter++ % d_decim_frames) == 0) {
					cur_frame_counter = 1;
				}
				else {
					return d_signal_length;
				}
			}

			// Copy the input signals to our local buffer for processing and trigger.
			for (int i=0;i<d_num_inputs;i++) {
				if (d_data_type == XCORR_COMPLEX) {
					memcpy(&d_input_buffer_complex[d_signal_length*i],input_items[i],d_signal_length*d_data_size);
				}
				else {
					memcpy(&d_input_buffer_real[d_signal_length*i],input_items[i],d_signal_length*d_data_size);
				}
			}

			// thread_is_processing will only be FALSE if thread_process_data == false on the first pass,
			// in which case we don't want to send any pmt's.  Otherwise, we're in async pickup mode.
			if (thread_is_processing) {
				pmt::pmt_t meta = pmt::make_dict();
				pmt::pmt_t corr_out(pmt::init_f32vector(d_num_inputs-1,correlation_factors));
				meta = pmt::dict_add(meta, pmt::mp("corrvect"), corr_out);
				pmt::pmt_t lag_out(pmt::init_s32vector(d_num_inputs-1,corrective_lag));
				meta = pmt::dict_add(meta, pmt::mp("corrective_lags"), lag_out);

				pmt::pmt_t pdu = pmt::cons(meta, pmt::PMT_NIL);
				message_port_pub(pmt::mp("corr"), pdu);
			}

			// clear the flag
			thread_process_data = true;
		}
		// The else with this is that the thread is processing, and we're just going to pass through.
	}

	// Tell runtime system how many output items we produced.
	return d_signal_length;
}

void xcorrelate_impl::runThread() {
	threadRunning = true;

	while (!stop_thread) {
		if (thread_process_data) {
			// This is really a one-time variable set to true on the first pass.
			thread_is_processing = true;

			// Trigger received to process data.
			// Calculate the reference signal mag just once
			if (d_data_type == XCORR_COMPLEX) {
				volk_32fc_magnitude_32f(ref_mag_buffer,d_input_buffer_complex,d_signal_length);
			}
			else {
				// already got complex->mag or float inputs, just need to move it
				// to the mag buffer
				memcpy(ref_mag_buffer, d_input_buffer_real,d_signal_length*d_data_size);
			}

			// Calculate the reference signal squared just once.
			volk_32f_x2_multiply_32f(xx_buffer,ref_mag_buffer,ref_mag_buffer,d_signal_length);

			// Cross correlation will shift signal noutput_items forward and backward
			// and calculate a normalized correlation factor for each shift.

			float corr;
			int lag;

			for (int signal_index=1;signal_index<d_num_inputs;signal_index++) {
				if (d_data_type == XCORR_COMPLEX) {
					volk_32fc_magnitude_32f(mag_buffer,&d_input_buffer_complex[d_signal_length * signal_index],d_signal_length);
				}
				else {
					memcpy(mag_buffer, &d_input_buffer_real[d_signal_length * signal_index],d_signal_length*d_data_size);
				}
				xcorr(d_signal_length, corr, lag, NULL);

				// Output vector is # of signals - 1 since xcorr of 1 with itself will always be 1.
				correlation_factors[signal_index-1] = corr;
				corrective_lag[signal_index-1] = lag;
			}

			// clear the trigger.  This will inform that data is ready.
			thread_process_data = false;
		}
		usleep(10);
	}

	threadRunning = false;
}

} /* namespace xcorrelate */
} /* namespace gr */


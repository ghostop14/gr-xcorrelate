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

#ifndef INCLUDED_XCORRELATE_XCORRELATE_IMPL_H
#define INCLUDED_XCORRELATE_XCORRELATE_IMPL_H

#include <xcorrelate/xcorrelate.h>
#include <boost/thread/thread.hpp>

// These are also set to align with DTYPEs in gr-clenabled
#define XCORR_COMPLEX 1
#define XCORR_FLOAT 2

namespace gr {
  namespace xcorrelate {

    class XCORRELATE_API xcorrelate_impl : public xcorrelate
    {
     private:
      int d_num_inputs;
      int d_signal_length;
      int d_data_type;
      int d_data_size;
      int d_decim_frames;
      int cur_frame_counter;
      int max_shift;
      int d_num_outputs;
      bool d_async;
      bool d_normalize;

      // Internal buffers
      float *ref_mag_buffer;
      float *mag_buffer;

      float *xy_buffer;
      float *xx_buffer;
      float *yy_buffer;
      float *sqrt_buffer;

      float *correlation_factors;
      int *corrective_lag;

		// For async mode, threading:
      boost::thread *proc_thread=NULL;
      bool threadRunning=false;
      bool stop_thread = false;
	  bool thread_is_processing=false;
	  bool thread_process_data=false;

	  // These are used to store our working buffer in async mode.
	  gr_complex *d_input_buffer_complex=NULL;
	  float *d_input_buffer_real=NULL;

      // xcorr expects ref_mag_buffer and mag_buffer to be pre-loaded.
      void xcorr(int num_items, float& corr, int& lag, float *corr_buffer=NULL);

	  virtual void runThread();

     public:
      xcorrelate_impl(int num_inputs, int signal_length, int data_type, int data_size, int max_search_index,
    		  int decim_frames, int num_outputs, bool basync=false, bool normalize=true);
      virtual ~xcorrelate_impl();

      bool stop();

      int work_test(
              int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items
      );

      // Where all the action really happens
      int work(
              int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items
      );
    };

  } // namespace xcorrelate
} // namespace gr

#endif /* INCLUDED_XCORRELATE_XCORRELATE_IMPL_H */


/* -*- c++ -*- */
/* 
 * Copyright 2018 <+YOU OR YOUR COMPANY+>.
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

//#define DEBUG

#include <gnuradio/io_signature.h>
#include "frame_detection_short_impl.h"

static __inline__ unsigned long GetCC(void)
{
  unsigned a, d; 
  asm volatile("rdtsc" : "=a" (a), "=d" (d)); 
  return ((unsigned long)a) | (((unsigned long)d) << 32); 
}

namespace gr {
  namespace ieee802_11 {

    frame_detection_short::sptr
    frame_detection_short::make()
    {
      return gnuradio::get_initial_sptr
        (new frame_detection_short_impl());
    }

    /*
     * The private constructor
     */
    frame_detection_short_impl::frame_detection_short_impl()
      : gr::block("wifi_input",
              gr::io_signature::make(1, 1, sizeof(int16_t)*2),
              gr::io_signature::make3(3, 3, sizeof(gr_complex), sizeof(gr_complex), sizeof(float))),
		d_avg64idx(0),
		d_avg64sum(0),
		d_avg48idx(0),
		d_avg48sum_r(0),
		d_avg48sum_i(0),
		d_del16idx(0)
    {
		memset(d_avg64buf, 0, sizeof(d_avg64buf));
		memset(d_avg48buf, 0, sizeof(d_avg48buf));
		memset(d_del16buf, 0, sizeof(d_del16buf));
	}

    /*
     * Our virtual destructor.
     */
    frame_detection_short_impl::~frame_detection_short_impl()
    {
    }

    void
    frame_detection_short_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
		unsigned ninputs = ninput_items_required.size ();

		for(unsigned i = 0; i < ninputs; i++)
		  ninput_items_required[i] = noutput_items*2;
    }

    int
    frame_detection_short_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const int16_t *in = (const int16_t *) input_items[0];
      gr_complex *out_in = (gr_complex*)output_items[0];
      gr_complex *out_abs = (gr_complex*)output_items[1];
      float *out_cor = (float*)output_items[2];
      int i;
	  int ninput = std::min(ninput_items[0], noutput_items);
	  int in_r, in_i, cmag2;
	  int del_r, del_i;
	  int ac, bd;
	  int mul_r, mul_i;
	  float cor;
	  double cmag1;
	  long long int test1, test2;
	  double cmag;
#ifdef DEBUG
	  unsigned long cstart, ccount;

     cstart = GetCC();
#endif

	  for (i=0; i<ninput; i++)
	  {
		in_r = in[i*2];
		in_i = in[i*2+1];
		cmag2 = in_r*in_r+in_i*in_i;

		/* roling avg 64 on cmag2 */
		d_avg64sum -= d_avg64buf[d_avg64idx];
		d_avg64sum += cmag2;
		d_avg64buf[d_avg64idx] = cmag2;
		d_avg64idx = (d_avg64idx+1) & 63;

		/* delayed sample */
        del_r = d_del16buf[d_del16idx*2];
		del_i = d_del16buf[d_del16idx*2+1];
        d_del16buf[d_del16idx*2] = in_r;
        d_del16buf[d_del16idx*2+1] = in_i;
		d_del16idx = (d_del16idx+1) & 15; 

		out_in[i].real(del_r);
		out_in[i].imag(del_i);

		/* complex conjugated */
		del_i *= -1;

		/* multiplied */
		ac = in_r*del_r;
		bd = in_i*del_i;
		mul_r = ac - bd;
		mul_i = (in_r+in_i)*(del_r+del_i) - ac - bd;

		/* average 48 */
		d_avg48sum_r += mul_r - d_avg48buf[d_avg48idx*2];
		d_avg48sum_i += mul_i - d_avg48buf[d_avg48idx*2+1];
		d_avg48buf[d_avg48idx*2] = mul_r;
		d_avg48buf[d_avg48idx*2+1] = mul_i;
		d_avg48idx ++;
		if (d_avg48idx == 48)
		{
			d_avg48idx = 0;
		}
		out_abs[i].real(d_avg48sum_r);
		out_abs[i].imag(d_avg48sum_i);

		test1 = (long long int)d_avg48sum_r*(long long int)d_avg48sum_r;
		test2 = (long long int)d_avg48sum_i*(long long int)d_avg48sum_i;
		cmag1 = test1+test2;

		/* here comes the sqrt */
		cmag = sqrt((double)cmag1);
		cor = (float)(cmag /(double)d_avg64sum);

		out_cor[i] = cor;
	  }

      consume_each (ninput);

#ifdef DEBUG
	  ccount = GetCC() - cstart;
      printf("cc: %d, %d/sample\n", ccount, ccount/i);
#endif

      return ninput;
    }

  } /* namespace ieee802_11 */
} /* namespace gr */


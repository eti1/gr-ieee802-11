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

#ifndef INCLUDED_IEEE802_11_FRAME_DETECTION_SHORT_IMPL_H
#define INCLUDED_IEEE802_11_FRAME_DETECTION_SHORT_IMPL_H

#include <ieee802-11/frame_detection_short.h>

namespace gr {
  namespace ieee802_11 {

    class frame_detection_short_impl : public frame_detection_short
    {
     private:
	 int d_avg64buf[64];
     unsigned d_avg64idx;
     int d_avg64sum;

	 int d_avg48buf[48*2];
     unsigned d_avg48idx;
     int d_avg48sum_r;
     int d_avg48sum_i;

     uint32_t d_del16buf[16*2];
     unsigned d_del16idx;

     public:
      frame_detection_short_impl();
      ~frame_detection_short_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace ieee802_11
} // namespace gr

#endif /* INCLUDED_IEEE802_11_FRAME_DETECTION_SHORT_IMPL_H */


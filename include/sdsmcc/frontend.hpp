///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2011-2014 Felipe Magno de Almeida.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// See http://www.boost.org/libs/foreach for documentation
//

#ifndef DFD_FRONTEND_HPP
#define DFD_FRONTEND_HPP

#include <cstdlib>
#include <cstring>

#include <linux/dvb/frontend.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <iostream>
#include <stdexcept>

namespace sdsmcc {

struct frontend
{
  frontend(int fd)
    : fd(fd)
  {}

  void tune(int frequency)
  {
    dvb_frontend_parameters p;
    std::memset(&p, 0, sizeof(p));
    p.frequency = frequency;
    p.inversion = INVERSION_AUTO;
    p.u.ofdm = dvb_ofdm_parameters ();
    p.u.ofdm.bandwidth = BANDWIDTH_6_MHZ;
    p.u.ofdm.code_rate_HP = p.u.ofdm.code_rate_LP = FEC_AUTO;
    p.u.ofdm.constellation = QAM_AUTO;
    p.u.ofdm.transmission_mode = TRANSMISSION_MODE_AUTO;
    p.u.ofdm.guard_interval = GUARD_INTERVAL_AUTO;
    p.u.ofdm.hierarchy_information = HIERARCHY_AUTO;

    int r;
    do
    {
      r = ioctl(fd, FE_SET_FRONTEND, &p);
    } while(r == -1 && errno == EINTR);
    if(r == -1)
    {
      std::cout << "Failed tuning frequency " << frequency << std::endl;
      throw std::runtime_error("Failed tuning frequency");
    }

    fe_status_t s;
    for(int i = 0; i != 10; ++i)
    {
      int r = 0;
      do
      {
        r = ioctl(fd, FE_READ_STATUS, &s);
      } while(r == -1 && errno == EINTR);
      if(r == -1)
      {
        std::cout << "Failed reading status" << std::endl;
        throw std::runtime_error("Failed reading status");
      }

      if(s & FE_HAS_LOCK)
        break;

      timespec ts = {1, 0};
      do
      {
        r = nanosleep(&ts, &ts);
      } while(r == -1 && errno == EINTR);
      if(r == -1)
      {
        std::cout << "Nanosleep error" << std::endl;
        std::abort();
      }
    }
    if(!(s & FE_HAS_LOCK))
    {
      std::cout << "Timeout for locking frontend" << std::endl;
      throw std::runtime_error("Timeout for locking frontend");
    }
  }

  int fd;
};

}

#endif

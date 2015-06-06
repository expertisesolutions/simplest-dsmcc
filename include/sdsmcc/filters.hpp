/* (c) Copyright 2011-2014 Felipe Magno de Almeida
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DFD_FILTERS_HPP
#define DFD_FILTERS_HPP

#include <boost/function.hpp>
#include <boost/utility.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <vector>
#include <list>

#include <cassert>

#include <linux/dvb/dmx.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

namespace sdsmcc {

struct filters
{
  struct table_filter
  {
    table_filter(__u16 pid)
      : pid(pid) {}

    void add_filter(__u8 table, __u8 mask = 0xff)
    {
      filter f = {table, mask};
      filters.push_back(f);
    }

    __u16 pid;

    struct filter
    {
      __u8 table;
      __u8 mask;
    };
    std::vector<filter> filters;
  };

  struct filter_data
  {
    boost::function<void(unsigned char const*, std::size_t, int)> callback;
    boost::posix_time::ptime idle_time;
    bool timeout;
  };

  filters(std::string demux_devname)
    : demux_devname(demux_devname) {}

  void add_table_filter(table_filter t, boost::function<void(unsigned char const*, std::size_t, int)> callback
                        , __u32 flags = 0)
  {
    std::cout << "add_table_filter for pid " << t.pid << std::endl;
    int fd = open(demux_devname.c_str(), O_RDWR|O_NONBLOCK);
    if(fd < 0)
    {
      std::cout << "Error opening demux device" << std::endl;
      throw std::runtime_error("Error opening demux device");
    }

    ioctl(fd, DMX_SET_BUFFER_SIZE, 4096*1000);

    dmx_sct_filter_params p;
    std::memset(&p, 0, sizeof(p));
    p.pid = t.pid;
    p.flags = flags;
    std::size_t i = 0;
    assert(DMX_FILTER_SIZE >= t.filters.size());
    for(std::vector<table_filter::filter>::const_iterator
          first = t.filters.begin(), last = t.filters.end()
          ;first != last; ++first, ++i)
    {
      p.filter.filter[i] = first->table;
      p.filter.mask[i] = first->mask;
    }

    int r = 0;
    do
    {
      r = ioctl(fd, DMX_SET_FILTER, &p);
    } while(r == -1 && errno == EINTR);
    if(r == -1)
    {
      std::cout << "Error adding filter" << std::endl;
      throw std::runtime_error("Error adding filter");
    }

    pollfd f;
    f.fd = fd;
    f.events = POLLIN;
    f.revents = 0;
    pollfds.push_back(f);
    filter_data d = {callback, boost::posix_time::second_clock::local_time(), false};
    datas.push_back(d);
  }
  void remove_filter(int fd)
  {
    std::list<pollfd>::iterator first = pollfds.begin()
      , last = pollfds.end();
    std::size_t i = 0;
    while(first != last && first->fd != fd)
    {
      std::cout << "Still have filter " << i++ << std::endl;
      ++first;
    }
    if(first != last)
    {
      std::size_t i = std::distance(pollfds.begin(), first);
      std::cout << "Removing filter " << i << std::endl;
      close(first->fd);
      pollfds.erase(first);
      datas.erase(boost::next(datas.begin(), i));
    }
  }

  void add_pes_filter()
  {
  }

  void run()
  {
    while(!this->pollfds.empty())
    {
      int r = 0;
      std::vector<pollfd> pollfds(this->pollfds.begin(), this->pollfds.end());
      std::vector<filter_data> datas(this->datas.begin(), this->datas.end());
      do
      {
        r = poll(&pollfds[0], pollfds.size(), 1000);
      } while (r == -1 && errno == EINTR);
      if(r == -1)
      {
        std::cout << "Failed reading poll with errno: " << errno << std::endl;
        throw std::runtime_error ("Failed poll'ing");
      }

      boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
      for(std::vector<pollfd>::iterator first = pollfds.begin()
            , last = pollfds.end(); first != last; ++first)
      {
        if(first->revents & POLLIN)
        {
          datas[std::distance(pollfds.begin(), first)].idle_time = now;
          std::cout << "Can read from " << std::distance(pollfds.begin(), first) << std::endl;
          unsigned char buffer[188*1000];
          
          int size = 0;
          do
          {
            size = read(first->fd, buffer, sizeof(buffer));
          } while(size == -1 && errno == EINTR);
          if(size == -1 && errno == EOVERFLOW)
          {
            std::cout << "Data overflow" << std::endl;
          }
          else if(size == -1)
          {
            std::cout << "Error reading data" << std::endl;
            throw std::runtime_error("Error reading data");
          }

          datas[std::distance(pollfds.begin(), first)].callback(buffer, size, first->fd);
        }
        else if(now - datas[std::distance(pollfds.begin(), first)].idle_time >= boost::posix_time::minutes(1))
        {
          std::cout << "Marking Timeout after " << (now - datas[std::distance(pollfds.begin(), first)].idle_time) << " time elapsed" << std::endl;
          datas[std::distance(pollfds.begin(), first)].timeout = true;
        }
      }

      std::vector<filter_data>::const_iterator data_first = datas.begin()
        , data_last = datas.end();
      std::vector<pollfd>::const_iterator poll_first = pollfds.begin();
      while(data_first != data_last)
      {
        std::size_t i = 0;
        for(std::list<pollfd>::iterator mutable_it = this->pollfds.begin()
              , mutable_end_it = this->pollfds.end(); mutable_it != mutable_end_it
              ;++mutable_it, ++i)
        {
          if(poll_first->fd == mutable_it->fd)
          {
            if(data_first->timeout)
            {
              this->datas.erase(boost::next(this->datas.begin(), i));
              this->pollfds.erase(mutable_it);
              break;
            }
            else
              boost::next(this->datas.begin(), i)->idle_time = data_first->idle_time;
          }
        }
        ++data_first;
        ++poll_first;
      }
      assert(this->pollfds.size() == this->datas.size());
    }
  }

  std::string demux_devname;
  std::list<pollfd> pollfds;
  std::list<filter_data> datas;
};

}

#endif

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

#ifndef DFD_PROGRAMS_HPP
#define DFD_PROGRAMS_HPP

#include <boost/date_time/posix_time/posix_time.hpp>

#include <map>

namespace sdsmcc {

struct block
{
  std::vector<unsigned char> raw;
};

struct module
{
  module()
    : completed(false), id(0), version(0)
    , size(0), block_size(0) {}

  module(int id, int version, std::size_t size
         , std::size_t block_size)
    : completed(false), id(id), version(version), size(size)
    , block_size(block_size)
  {
    std::size_t block_n = size / block_size;
    if(size % block_size)
      block_n++;
    blocks.resize(block_n);
  }

  bool completed;
  int id, version;
  std::size_t size, block_size;
  std::vector<block> blocks;
};

struct download_scenario
{
  download_scenario(int download_id)
    : download_id(download_id)
  {
    creation_time = boost::posix_time::second_clock::local_time();
  }

  void add_module(module m)
  {
    assert(modules.find(m.id) == modules.end());
    modules.insert(std::make_pair(m.id, m));
  }

  bool completed() const
  {
    for(module_const_iterator first = modules.begin(), last = modules.end()
          ;first != last; ++first)
    {
      if(!first->second.completed)
        return false;
    }
    return true;
  }

  boost::posix_time::ptime creation_time;
  int download_id;
  typedef std::map<int, module> modules_type;
  typedef modules_type::iterator module_iterator;
  typedef modules_type::const_iterator module_const_iterator;
  modules_type modules;
};

struct program
{
  program(__u16 pid)
    : pid(pid) {}

  program() : pid(0) {}
  
  __u16 pid;
  typedef std::map<int, download_scenario> download_scenarios_type;
  typedef download_scenarios_type::iterator download_scenario_iterator;
  download_scenarios_type download_scenarios;
};

struct programs
{
  void add_program(program p)
  {
    programs_.insert(std::make_pair(p.pid, p));
  }
  program& find_program(__u16 pid)
  {
    assert(programs_.find(pid) != programs_.end());
    return programs_[pid];
  }

  std::map<__u16, program> programs_;
};

}

#endif

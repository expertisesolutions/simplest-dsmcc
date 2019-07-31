///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2011-2014 Felipe Magno de Almeida.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// See http://www.boost.org/libs/foreach for documentation
//

#include <sdsmcc/filters.hpp>
#include <sdsmcc/programs.hpp>
#include <sdsmcc/parsers/parsers.hpp>

#include <gts/sections/program_association_section.hpp>

#include <boost/range.hpp>
#include <boost/bind.hpp>

#include <cstdlib>
#include <iostream>

namespace sdsmcc { namespace parsers {

void pat(unsigned char const* buf, std::size_t count, int fd, sdsmcc::filters& filters
         , sdsmcc::programs& programs, sdsmcc::carousels& carousels)
{
  std::cout << "PAT packet" << std::endl;
  filters.remove_filter(fd);
  typedef gts::sections::program_association_section<unsigned char const*> pat_section_type;
  assert(count >= 4);
  pat_section_type pat_section(buf, buf + count);

  typedef pat_section_type::table_id_iterator table_id_iterator;
  table_id_iterator table_id = pat_section.begin ();
  assert(table_id != pat_section.end ());

  typedef pat_section_type::section_syntax_indicator_iterator section_syntax_indicator_iterator;
  typedef pat_section_type::section_length_iterator section_length_iterator;
  typedef pat_section_type::transport_stream_id_iterator transport_stream_id_iterator;
  typedef pat_section_type::version_number_iterator version_number_iterator;
  typedef pat_section_type::current_next_indicator_iterator current_next_indicator_iterator;
  typedef pat_section_type::section_number_iterator section_number_iterator;
  typedef pat_section_type::last_section_number_iterator last_section_number_iterator;
  typedef pat_section_type::program_associations_iterator program_associations_iterator;
  typedef pat_section_type::crc32_iterator crc32_iterator;
  typedef pat_section_type::end_iterator end_iterator;

  std::cout << "PAT" << std::endl;
  section_syntax_indicator_iterator section_syntax_indicator = gts::iterators::next(table_id);
  assert(section_syntax_indicator != pat_section.end ());
  std::cout << "section_syntax_indicator: " << (int)*section_syntax_indicator << std::endl;
  section_length_iterator section_length = gts::iterators::next(section_syntax_indicator);
  assert(section_length != pat_section.end ()); 
  std::cout << "section_length: " << (int)*section_length << std::endl;
  transport_stream_id_iterator transport_stream_id = gts::iterators::next(section_length);
  assert(transport_stream_id != pat_section.end ()); 
  std::cout << "transport_stream_id: 0x" << std::hex << (int)*transport_stream_id << std::dec << std::endl;
  version_number_iterator version_number = gts::iterators::next(transport_stream_id);
  assert(version_number != pat_section.end ()); 
  std::cout << "version_number: " << (int)*version_number << std::endl;
  current_next_indicator_iterator current_next_indicator = gts::iterators::next(version_number);
  assert(current_next_indicator != pat_section.end ()); 
  std::cout << "current_next_indicator: " << (int)*current_next_indicator << std::endl;
  section_number_iterator section_number = gts::iterators::next(current_next_indicator);
  assert(section_number != pat_section.end ()); 
  std::cout << "section_number: " << (int)*section_number << std::endl;
  last_section_number_iterator last_section_number = gts::iterators::next(section_number);
  assert(last_section_number != pat_section.end ()); 
  std::cout << "last_section_number: " << (int)*last_section_number << std::endl;
  program_associations_iterator program_associations_it = gts::iterators::next(last_section_number);
  typedef program_associations_iterator::deref_type program_associations_range;
  assert(program_associations_it != pat_section.end ()); 

  program_associations_range program_associations = *program_associations_it;
  std::cout << "number of program associations: " << boost::distance(program_associations) << std::endl;
  for(boost::range_iterator<program_associations_range>::type
        first = boost::begin(program_associations)
        , last = boost::end(program_associations)
        ;first != last; ++first)
  {
    typedef boost::range_value<program_associations_range>::type program_association;
    program_association pa = *first;
    std::cout << " program number: 0x"
              << std::hex << pa.program_number << std::dec
              << " pid: 0x"
              << std::hex << pa.pid << std::dec
              << std::endl;

    if(pa.program_number == 0) // NIT
    {
      sdsmcc::filters::table_filter f(pa.pid);
      filters.add_table_filter(f, boost::bind(&sdsmcc::parsers::nit, _1, _2, _3, boost::ref(filters))
                               , DMX_IMMEDIATE_START|DMX_CHECK_CRC);
    }
    else if(pa.program_number != 0)
    {
      sdsmcc::filters::table_filter f(pa.pid);
      filters.add_table_filter(f, boost::bind(&sdsmcc::parsers::pmt, _1, _2, _3, boost::ref(filters)
                                              , boost::ref(programs), pa.pid, boost::ref(carousels))
                               , DMX_IMMEDIATE_START|DMX_CHECK_CRC);
    }
  }
}

} }

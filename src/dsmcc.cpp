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

#include <sdsmcc/filters.hpp>
#include <sdsmcc/programs.hpp>
#include <sdsmcc/parsers/parsers.hpp>

#include <gts/sections/program_association_section.hpp>
#include <gts/sections/extended_section.hpp>
#include <gts/dsmcc/sections/user_network_message_section.hpp>
#include <gts/dsmcc/download_info_indication.hpp>
#include <gts/dsmcc/download_server_initiate.hpp>
#include <gts/dsmcc/download_data_block.hpp>
#include <gts/dsmcc/download_message_id.hpp>

#include <cstdlib>

namespace sdsmcc { namespace parsers {

void dsmcc(unsigned char const* buf, std::size_t count, int fd, sdsmcc::filters& filters
           , sdsmcc::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid
           , sdsmcc::carousels& carousels)
{
  typedef gts::sections::extended_section<unsigned char const*> extended_section_type;
  typedef extended_section_type::table_id_iterator table_id_iterator;
  typedef extended_section_type::section_syntax_indicator_iterator section_syntax_indicator_iterator;
  typedef extended_section_type::private_indicator_iterator private_indicator_iterator;
  typedef extended_section_type::private_section_length_iterator private_section_length_iterator;
  typedef extended_section_type::end_iterator end_iterator;

  if(count >= private_section_length_iterator::bits_offset::value/CHAR_BIT)
  {
    extended_section_type extended_section(buf, buf + count);
    table_id_iterator table_id = extended_section.begin ();
    switch(*table_id)
    {
    case 0x3a:
      break;
    case 0x3b:
    {
      sdsmcc::parsers::dsmcc_un_messages(extended_section, fd, filters, programs, pmt_pid, dsmcc_pid);
    }
    case 0x3c:
    {
      sdsmcc::parsers::dsmcc_ddb(extended_section, fd, filters, programs, pmt_pid, dsmcc_pid
                              , carousels);
    }
    break;
    }
  }
}

} }

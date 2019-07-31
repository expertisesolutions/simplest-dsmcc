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
#include <gts/sections/extended_section.hpp>
#include <gts/dsmcc/sections/user_network_message_section.hpp>
#include <gts/dsmcc/download_info_indication.hpp>
#include <gts/dsmcc/download_server_initiate.hpp>
#include <gts/dsmcc/download_data_block.hpp>
#include <gts/dsmcc/download_message_id.hpp>

#include <cstdlib>

namespace sdsmcc { namespace parsers {

void dsmcc_un_messages(extended_section_type extended_section, int fd, sdsmcc::filters& filters
                       , sdsmcc::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid)
{
  std::cout << "DSMCC_section U-N Messages, except Data Download Messages" << std::endl; 
  typedef gts::dsmcc::sections::user_network_message_section
    <unsigned char const*> section_un_type;
  section_un_type dsmcc_section(extended_section);
      
  typedef section_un_type::table_id_iterator table_id_iterator;
  typedef section_un_type::section_syntax_indicator_iterator
    section_syntax_indicator_iterator;
  typedef section_un_type::private_indicator_iterator
    private_indicator_iterator;
  typedef section_un_type::dsmcc_section_length_iterator
    dsmcc_section_length_iterator;
  typedef section_un_type::table_id_extension_iterator
    table_id_extension_iterator;
  typedef section_un_type::version_number_iterator
    version_number_iterator;
  typedef section_un_type::current_next_indicator_iterator
    current_next_indicator_iterator;
  typedef section_un_type::section_number_iterator
    section_number_iterator;
  typedef section_un_type::last_section_number_iterator
    last_section_number_iterator;
  typedef section_un_type::user_network_message_iterator
    user_network_message_iterator;

  table_id_iterator table_id = dsmcc_section.begin ();
  section_syntax_indicator_iterator section_syntax_indicator
    = gts::iterators::next(table_id);
  std::cout << "section_syntax_indicator: " << *section_syntax_indicator
            << std::endl;
  std::cout << "section_syntax_indicator base: 0x" << (void*)section_syntax_indicator.base () << std::endl;
  
  private_indicator_iterator private_indicator = gts::iterators::next(section_syntax_indicator);
  std::cout << "private_indicator: " << *private_indicator << std::endl;
  std::cout << "private_indicator base: 0x" << (void*)private_indicator.base () << std::endl;
      
  dsmcc_section_length_iterator dsmcc_section_length = gts::iterators::next(private_indicator);
  std::cout << "dsmcc_section_length: " << *dsmcc_section_length << std::endl;

  table_id_extension_iterator table_id_extension = gts::iterators::next(dsmcc_section_length);
  std::cout << "table_id_extension: " << *table_id_extension << std::endl;
  std::cout << "table_id_extension base: 0x" << (void*)table_id_extension.base () << std::endl;
  version_number_iterator version_number = gts::iterators::next(table_id_extension);
  std::cout << "version_number: " << *version_number << std::endl;

  current_next_indicator_iterator current_next_indicator = gts::iterators::next(version_number);
  std::cout << "current_next_indicator: " << *current_next_indicator << std::endl;

  section_number_iterator section_number = gts::iterators::next(current_next_indicator);
  std::cout << "section_number: " << *section_number << std::endl;

  last_section_number_iterator last_section_number = gts::iterators::next(section_number);
  std::cout << "last_section_number: " << *last_section_number << std::endl;
      
  user_network_message_iterator user_network_message = gts::iterators::next(last_section_number);
  typedef user_network_message_iterator::deref_type user_network_type;
  user_network_type user_network = *user_network_message;
  typedef user_network_type::protocol_discriminator_iterator
    protocol_discriminator_iterator;
  typedef user_network_type::dsmcc_type_iterator dsmcc_type_iterator;
  typedef user_network_type::message_id_iterator message_id_iterator;
  
  protocol_discriminator_iterator protocol_discriminator = user_network.begin ();
  std::cout << "protocol_discriminator: " << *protocol_discriminator << std::endl;

  dsmcc_type_iterator dsmcc_type = gts::iterators::next(protocol_discriminator);
  std::cout << "dsmcc_type: " << *dsmcc_type << std::endl;              

  message_id_iterator message_id = gts::iterators::next(dsmcc_type);
  std::cout << "message_id: " << *message_id << std::endl;

  switch(*message_id)
  {
  case gts::dsmcc::download_message_id::download_info_indication:
  {
    typedef gts::dsmcc::download_info_indication<user_network_type::base_iterator>
      download_info_indication;
    download_info_indication dii(user_network);
    dsmcc_dii(dii, fd, filters, programs, pmt_pid, dsmcc_pid);

    break;
  }
  case gts::dsmcc::download_message_id::download_server_initiate:
  {
    typedef gts::dsmcc::download_server_initiate<user_network_type::base_iterator>
      download_server_initiate;
    download_server_initiate dsi(user_network);
    dsmcc_dsi(dsi, fd, filters, programs, pmt_pid, dsmcc_pid);
    break;
  }
  default:
    std::cout << "Unknown Download Control message" << std::endl;
    break;
  }
}

} }


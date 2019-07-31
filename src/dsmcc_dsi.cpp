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
#include <gts/dsmcc/descriptors/group_info_indication.hpp>

#include <cstdlib>

namespace sdsmcc { namespace parsers {

void dsmcc_dsi(download_server_initiate dsi, int fd, sdsmcc::filters& filters
               , sdsmcc::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid)
{
  std::cout << "A DSI message" << std::endl;

  sdsmcc::program& program = programs.find_program(pmt_pid);
  typedef download_server_initiate::protocol_discriminator_iterator protocol_discriminator_iterator;
  typedef download_server_initiate::dsmcc_type_iterator dsmcc_type_iterator;
  typedef download_server_initiate::message_id_iterator message_id_iterator;
  typedef download_server_initiate::transaction_id_iterator transaction_id_iterator;
  typedef download_server_initiate::adaptation_iterator adaptation_iterator;
  typedef download_server_initiate::server_id_iterator server_id_iterator;
  typedef download_server_initiate::compatibility_descriptor_iterator compatibility_descriptor_iterator;
  typedef download_server_initiate::private_data_iterator private_data_iterator;

  protocol_discriminator_iterator protocol_discriminator = dsi.begin ();
  std::cout << "DSI protocol_discriminator: " << *protocol_discriminator << std::endl;

  dsmcc_type_iterator dsmcc_type = gts::iterators::next(protocol_discriminator);
  std::cout << "DSI dsmcc_type: " << *dsmcc_type << std::endl;              

  message_id_iterator message_id = gts::iterators::next(dsmcc_type);
  std::cout << "DSI message_id: " << *message_id << std::endl;

  transaction_id_iterator transaction_id = gts::iterators::next<3>(dsi.begin ());
  std::cout << "DSI transaction_id: " << std::hex << *transaction_id << std::dec << std::endl;

  private_data_iterator private_data_it = gts::iterators::next<4>(transaction_id);
  private_data_iterator::deref_type private_data = *private_data_it;

  std::cout << "size of private data: " << boost::distance(private_data) << std::endl;

  typedef gts::dsmcc::descriptors::group_info_indication
    <unsigned char const*> group_info_indication;
  group_info_indication gii(boost::begin(private_data), boost::end(private_data));
  
  typedef group_info_indication::groups_iterator groups_iterator;

  groups_iterator groups_it = gii.begin();

  unsigned int group_i = 0;
  groups_iterator::deref_type groups = *groups_it;
  for(boost::range_iterator<groups_iterator::deref_type>::type
        first = boost::begin(groups), last = boost::end(groups)
        ;first != last; ++first)
  {
    std::cout << "group number " << group_i++ << std::endl;
  }
}

} }

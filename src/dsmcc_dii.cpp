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

void dsmcc_dii(download_info_indication dii, int fd, sdsmcc::filters& filters
               , sdsmcc::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid)
{
  std::cout << "A DII message" << std::endl;

  sdsmcc::program& program = programs.find_program(pmt_pid);

  typedef download_info_indication::protocol_discriminator_iterator protocol_discriminator_iterator;
  typedef download_info_indication::dsmcc_type_iterator dsmcc_type_iterator;
  typedef download_info_indication::message_id_iterator message_id_iterator;
  typedef download_info_indication::transaction_id_iterator transaction_id_iterator;
  typedef download_info_indication::adaptation_iterator adaptation_iterator;
  typedef download_info_indication::download_id_iterator download_id_iterator;
  typedef download_info_indication::block_size_iterator block_size_iterator;
  typedef download_info_indication::window_size_iterator window_size_iterator;
  typedef download_info_indication::ack_period_iterator ack_period_iterator;
  typedef download_info_indication::timeout_download_window_iterator
    timeout_download_window_iterator;
  typedef download_info_indication::timeout_download_scenario_iterator
    timeout_download_scenario_iterator;
  typedef download_info_indication::compatibility_descriptor_iterator 
    compatibility_descriptor_iterator;
  typedef download_info_indication::module_range_iterator module_range_iterator;

  protocol_discriminator_iterator protocol_discriminator = dii.begin ();
  std::cout << "DII protocol_discriminator: " << *protocol_discriminator << std::endl;

  dsmcc_type_iterator dsmcc_type = gts::iterators::next(protocol_discriminator);
  std::cout << "DII dsmcc_type: " << *dsmcc_type << std::endl;              

  message_id_iterator message_id = gts::iterators::next(dsmcc_type);
  std::cout << "DII message_id: " << *message_id << std::endl;

  transaction_id_iterator transaction_id = gts::iterators::next<3>(dii.begin ());
  std::cout << "DII transaction_id: " << std::hex << *transaction_id << std::dec << std::endl;
  if(*transaction_id <= 1)
    std::cout << "Is a one-layer carousel" << std::endl;
  else
    std::cout << "Is a two-layer carousel" << std::endl;

  adaptation_iterator adaptation_it = gts::iterators::next(transaction_id);
  adaptation_iterator::deref_type adaptation = *adaptation_it;
  if(adaptation.empty ())
  {
    std::cout << "No adaptation header" << std::endl;
  }
  else
  {
    std::cout << "DII adaptation_type: " << adaptation.type () << std::endl;
    std::cout << "DII adaptation_data size: " << boost::distance(adaptation.data()) << std::endl;
  }

  download_id_iterator download_id = gts::iterators::next(adaptation_it);
  std::cout << "DII download_id: " << std::hex << *download_id << std::dec << std::endl;

  // if(*transaction_id <= 1 || false)
  {
    sdsmcc::program::download_scenario_iterator download_scenario_iterator
      = program.download_scenarios.find(*download_id);
    if(download_scenario_iterator == program.download_scenarios.end())
    {
      sdsmcc::download_scenario ds (*download_id);

      block_size_iterator block_size = gts::iterators::next(download_id);
      std::cout << "DII block_size: " << *block_size << std::endl;

      window_size_iterator window_size = gts::iterators::next(block_size);
      std::cout << "DII window_size: " << *window_size << std::endl;
    
      ack_period_iterator ack_period = gts::iterators::next(window_size);
      std::cout << "DII ack_period: " << *ack_period << std::endl;
  
      timeout_download_window_iterator timeout_download_window = gts::iterators::next(ack_period);
      std::cout << "DII timeout_download_window: " << *timeout_download_window << std::endl;

      timeout_download_scenario_iterator timeout_download_scenario
        = gts::iterators::next(timeout_download_window);
      std::cout << "DII timeout_download_scenario: " << *timeout_download_scenario << std::endl;

      compatibility_descriptor_iterator compatibility_descriptor_it
        = gts::iterators::next(timeout_download_scenario);
      typedef compatibility_descriptor_iterator::deref_type compatibility_descriptor_type;
      compatibility_descriptor_type compatibility_descriptor = *compatibility_descriptor_it;
                  
      module_range_iterator modules_it = gts::iterators::next(compatibility_descriptor_it);
      typedef module_range_iterator::deref_type module_range_type;

      module_range_type modules = *modules_it;
      std::cout << "modules: " << boost::distance(modules) << std::endl;

      typedef boost::range_value<module_range_type>::type module_type;
      for(boost::range_iterator<module_range_type>::type
            first = boost::begin (modules), last = boost::end(modules)
            ;first != last; ++first)
      {
        typedef module_type::module_id_iterator module_id_iterator;
        typedef module_type::module_size_iterator module_size_iterator;
        typedef module_type::module_version_iterator module_version_iterator;
        typedef module_type::module_info_iterator module_info_iterator;

        module_id_iterator id = first->begin ();
        std::cout << "module id: " << *id << std::endl;
        module_size_iterator size = gts::iterators::next(id);
        std::cout << "module size: " << *size << std::endl;
        module_version_iterator version = gts::iterators::next(size);
        std::cout << "module version: " << *version << std::endl;
        module_info_iterator info = gts::iterators::next(version);
        std::cout << "info size: " << boost::distance(*info) << std::endl;

        sdsmcc::module module(*id, *version, *size, *block_size);
        ds.add_module(module);
      }

      program.download_scenarios.insert(std::make_pair(*download_id, ds));
    }
    else
      std::cout << "DII already handled" << std::endl;
  }
  // else
  //   std::cout << "Waiting for DSI in a two-layer carousel" << std::endl;
}

} }

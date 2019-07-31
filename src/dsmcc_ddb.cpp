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

void dsmcc_ddb(extended_section_type extended_section, int fd, sdsmcc::filters& filters
               , sdsmcc::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid
               , sdsmcc::carousels& carousels)
{
  std::cout << "DSMCC_section Download Data Messages" << std::endl;

  sdsmcc::program& program = programs.find_program(pmt_pid);
  
  typedef gts::dsmcc::sections::user_network_message_section
    <unsigned char const*> section_un_type;
  typedef section_un_type::user_network_message_iterator user_network_message_iterator;

  section_un_type dsmcc_section(extended_section);
  
  user_network_message_iterator user_network_message
    = gts::iterators::next<9>(dsmcc_section.begin ());
  typedef user_network_message_iterator::deref_type user_network_type;
  user_network_type user_network = *user_network_message;

  typedef gts::dsmcc::download_data_block<user_network_type::base_iterator>
    download_data_block;
  download_data_block ddb(user_network);

  typedef download_data_block::download_id_iterator download_id_iterator;
  typedef download_data_block::module_id_iterator module_id_iterator;
  typedef download_data_block::module_version_iterator module_version_iterator;
  typedef download_data_block::block_number_iterator block_number_iterator;
  typedef download_data_block::block_data_iterator block_data_iterator;
            
  download_id_iterator download_id = gts::iterators::next<3>(ddb.begin ());
  std::cout << "DDB download_id: " << *download_id << std::endl;

  sdsmcc::program::download_scenario_iterator download_scenario_iterator
    = program.download_scenarios.find(*download_id);
  if(download_scenario_iterator != program.download_scenarios.end())
  {
    sdsmcc::download_scenario& ds = download_scenario_iterator->second;
    module_id_iterator module_id = gts::iterators::next<2>(download_id);
    std::cout << "DDB module_id: " << *module_id << std::endl;

    module_version_iterator module_version = gts::iterators::next(module_id);
    std::cout << "DDB module_version: " << *module_version << std::endl;
                
    block_number_iterator block_number = gts::iterators::next(module_version);
    std::cout << "DDB block number: " << *block_number << std::endl;

    sdsmcc::download_scenario::module_iterator module_iterator = ds.modules.find(*module_id);
    if(module_iterator != ds.modules.end())
    {
      sdsmcc::module& module = module_iterator->second;
      std::cout << "Module found" << std::endl;
      if(!module.completed)
      {
        if(module.version == *module_version)
        {
          std::cout << "Same version" << std::endl;
          assert(module.blocks.size() >= *block_number);
          if(module.blocks[*block_number].raw.empty())
          {
            block_data_iterator block_data_it = gts::iterators::next(block_number);
            typedef block_data_iterator::deref_type block_data_type;
            block_data_type block_data = *block_data_it;
            std::cout << "Block size: " << module.block_size << std::endl;
            std::cout << "Actual block size: " << boost::distance(block_data) << std::endl;
            std::cout << "Number of blocks in module: " << module.blocks.size() << std::endl;
            //assert(module.block_size == boost::distance(block_data) || *block_number - 1 == module.blocks.size());

            module.blocks[*block_number].raw.insert(module.blocks[*block_number].raw.end()
                                                    , boost::begin(block_data), boost::end(block_data));

            if(*block_number - 1 != module.blocks.size()
               || boost::distance(block_data) > module.block_size)
              module.blocks[*block_number].raw.resize(module.block_size);

            bool completed = true;
            for(std::vector<sdsmcc::block>::const_iterator first = module.blocks.begin()
                  , last = module.blocks.end(); first != last; ++first)
            {
              if(first->raw.empty())
              {
                completed = false;
                break;
              }
            }
            if(completed)
            {
              std::cout << "Module completed" << std::endl;
              module.completed = true;
              dsmcc_module(*download_id, module, fd, filters, carousels);

              if(ds.completed())
              {
                std::cout << "Download scenario " << *download_id << " is completed" << std::endl;
                std::cout << "Time elapsed for download: " << (boost::posix_time::second_clock::local_time()
                                                               - ds.creation_time) << std::endl;
                std::cout << "Magic: ";
                std::copy(module.blocks[0].raw.begin(), boost::next(module.blocks[0].raw.begin(),4)
                          , std::ostream_iterator<unsigned char>(std::cout));
                std::cout << std::endl;
                filters.remove_filter(fd);
              }
            }

            std::cout << "Block copied" << std::endl;
          }
          else
            std::cout << "Block already has been copied" << std::endl;
        }
        else
          std::cout << "Wrong version" << std::endl;
      }
      else
        std::cout << "Module completed" << std::endl;
    }
    else
      std::cout << "Couldn't find module for some reason...." << std::endl;
  }
  else
    std::cout << "Waiting DII" << std::endl;
}

} }

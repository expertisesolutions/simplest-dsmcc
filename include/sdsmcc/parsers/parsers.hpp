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

#ifndef DFD_PARSERS_PAT_HPP
#define DFD_PARSERS_PAT_HPP

#include <sdsmcc/filters.hpp>
#include <sdsmcc/programs.hpp>
#include <sdsmcc/carousels.hpp>

#include <gts/sections/extended_section.hpp>
#include <gts/dsmcc/download_info_indication.hpp>
#include <gts/dsmcc/download_server_initiate.hpp>
#include <gts/dsmcc/sections/user_network_message_section.hpp>
#include <gts/dsmcc/biop/file_message.hpp>
#include <gts/dsmcc/biop/directory_message.hpp>

#include <cstdlib>
#include <vector>

namespace sdsmcc { namespace parsers {

typedef gts::sections::extended_section<unsigned char const*> extended_section_type;
typedef gts::dsmcc::sections::user_network_message_section<unsigned char const*> section_un_type;
typedef section_un_type::user_network_message_iterator user_network_message_iterator;
typedef user_network_message_iterator::deref_type user_network_type;
typedef gts::dsmcc::download_info_indication<user_network_type::base_iterator> download_info_indication;
typedef gts::dsmcc::download_server_initiate<user_network_type::base_iterator> download_server_initiate;
typedef gts::dsmcc::biop::file_message<std::vector<unsigned char>::const_iterator> file_message;
typedef gts::dsmcc::biop::directory_message<std::vector<unsigned char>::const_iterator> directory_message;

void pat(unsigned char const* buffer, std::size_t size, int fd, sdsmcc::filters& filters
         , sdsmcc::programs& program, sdsmcc::carousels& carousels);
void nit(unsigned char const* buffer, std::size_t size, int fd, sdsmcc::filters& filters);
void pmt(unsigned char const* buffer, std::size_t size, int fd, sdsmcc::filters& filters
         , sdsmcc::programs& program, __u16 pid, sdsmcc::carousels& carousels);
void ait(unsigned char const* buffer, std::size_t size, int fd, sdsmcc::filters& filters
         , sdsmcc::programs& program, __u16 pmt_pid, __u16 dsmcc_pid
         , sdsmcc::carousels& carousels);
void dsmcc(unsigned char const* buffer, std::size_t size, int fd, sdsmcc::filters& filters
           , sdsmcc::programs& program, __u16 pmt_pid, __u16 dsmcc_pid
           , sdsmcc::carousels& carousels);
void dsmcc_un_messages(extended_section_type extended_section, int fd, sdsmcc::filters& filters
                       , sdsmcc::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid);
void dsmcc_dii(download_info_indication dii, int fd, sdsmcc::filters& filters
               , sdsmcc::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid);
void dsmcc_dsi(download_server_initiate dii, int fd, sdsmcc::filters& filters
               , sdsmcc::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid);
void dsmcc_ddb(extended_section_type extended_section, int fd, sdsmcc::filters& filters
               , sdsmcc::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid
               , sdsmcc::carousels& carousels);
void dsmcc_module(unsigned int download_id, sdsmcc::module const& module, int fd, sdsmcc::filters& filters
                  , sdsmcc::carousels& carousels);
std::vector<unsigned char>::const_iterator
dsmcc_file(file_message const& file, int fd, sdsmcc::filters& filters
           , unsigned int download_id, unsigned int module_id
           , boost::iterator_range<std::vector<unsigned char>::const_iterator> object_key
           , sdsmcc::carousels& carousels);
std::vector<unsigned char>::const_iterator
dsmcc_directory(directory_message const& directory, int fd, sdsmcc::filters& filters
                , unsigned int download_id, unsigned int module_id
                , boost::iterator_range<std::vector<unsigned char>::const_iterator> object_key
                , sdsmcc::carousels& carousels);
std::vector<unsigned char>::const_iterator
dsmcc_gateway(directory_message const& directory, int fd, sdsmcc::filters& filters
              , unsigned int download_id, unsigned int module_id
              , boost::iterator_range<std::vector<unsigned char>::const_iterator> object_key
              , sdsmcc::carousels& carousels);

} }

#endif

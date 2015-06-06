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

#include <gts/dsmcc/biop/message_header.hpp>
#include <gts/dsmcc/biop/message_subheader.hpp>
#include <gts/dsmcc/biop/file_message.hpp>

namespace sdsmcc { namespace parsers {

std::vector<unsigned char>::const_iterator
dsmcc_file(file_message const& file, int fd, sdsmcc::filters& filters
                , unsigned int download_id, unsigned int module_id
                , boost::iterator_range<std::vector<unsigned char>::const_iterator> object_key
                , sdsmcc::carousels& carousels)
{
  typedef file_message::object_info_iterator object_info_iterator;
  typedef file_message::service_context_list_iterator service_context_list_iterator;
  typedef file_message::file_message_body_iterator file_message_body_iterator;
  typedef file_message::end_iterator file_end_iterator;

  object_info_iterator object_info_it = file.begin();
  object_info_iterator::deref_type object_info = *object_info_it;
  std::cout << "File Content Size: " << (int)object_info.content_size() << std::endl;
  service_context_list_iterator service_context_list_it = gts::iterators::next(object_info_it);
  service_context_list_iterator::deref_type context_list = *service_context_list_it;
  std::cout << "Service context list size: " << boost::distance(context_list)
            << std::endl;
  file_message_body_iterator file_message_body_it = gts::iterators::next(service_context_list_it);
  typedef file_message_body_iterator::deref_type file_message_body_type;
  typedef file_message_body_type::message_body_length_iterator message_body_length_iterator;
  typedef file_message_body_type::content_iterator content_iterator;
  file_message_body_type file_message_body = *file_message_body_it;
  message_body_length_iterator message_body_length_it = file_message_body.begin();
  std::cout << "Message Body Length " << *message_body_length_it << std::endl;
  content_iterator content_it = gts::iterators::next(message_body_length_it);
  content_iterator::deref_type content = *content_it;
  std::cout << "Content size: " << boost::distance(content) << std::endl;
  std::cout << "Should register file in carousel, carousel_id: "
            << download_id << " module_id: " << module_id << std::endl;
  sdsmcc::carousel& carousel = carousels.carousels_[download_id];
  carousel.add_file_object(object_key, content, module_id);
  return gts::iterators::next(content_it).base();
}

} }

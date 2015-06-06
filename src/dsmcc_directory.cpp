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
#include <gts/dsmcc/biop/directory_message.hpp>

namespace sdsmcc { namespace parsers {

std::vector<unsigned char>::const_iterator
dsmcc_directory(directory_message const& directory, int fd, sdsmcc::filters& filters
                , unsigned int download_id, unsigned int module_id
                , boost::iterator_range<std::vector<unsigned char>::const_iterator> object_key
                , sdsmcc::carousels& carousels)
{
  typedef directory_message::message_body_length_iterator message_body_length_iterator;
  typedef directory_message::binds_iterator binds_iterator;
  message_body_length_iterator message_body_length_it = directory.begin();
  std::cout << "Message body length: " << *message_body_length_it << std::endl;
  binds_iterator binds_it = gts::iterators::next(message_body_length_it);
  typedef binds_iterator::deref_type binds_type;
  binds_type binds = *binds_it;

  typedef boost::range_iterator<binds_type>::type bind_iterator;
  std::size_t i = 0;
  bind_iterator first = boost::begin(binds), last = boost::end(binds);
  for(;first != last; ++first, ++i)
  {
    std::cout << "Bind " << i << std::endl;
    typedef std::iterator_traits<bind_iterator>::value_type bind_type;
    bind_type bind = *first;
    std::cout << " binds: " << bind.names.size() << std::endl;
    for(std::vector<bind_type::bind_name>::const_iterator first = bind.names.begin()
          , last = bind.names.end();first != last; ++first)
    {
      std::cout << " id: " << first->id << " kind: " << first->kind << std::endl;
      std::cout << " IOR BIOP components: " << bind.ior.biop_profiles.size() << std::endl;
    }
  }

  sdsmcc::carousel& carousel = carousels.carousels_[download_id];
  carousel.add_directory_object(object_key, binds, module_id);
  return first.base();
}

} }


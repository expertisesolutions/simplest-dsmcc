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

#include <cstdlib>

namespace sdsmcc { namespace parsers {

void nit(unsigned char const* buf, std::size_t count, int fd, sdsmcc::filters& filters)
{
  std::cout << "NIT packet" << std::endl;
  filters.remove_filter(fd);
  
}

} }

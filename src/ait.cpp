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
#include <gts/partial_descriptor_sequence.hpp>
#include <gts/nit_private_sequence.hpp>
#include <gts/sections/private_section.hpp>
#include <gts/sections/program_map_section.hpp>
#include <gts/sections/application_information_section.hpp>

#include <boost/range.hpp>
#include <boost/bind.hpp>

#include <cstdlib>

namespace sdsmcc { namespace parsers {

void ait(unsigned char const* buf, std::size_t count, int fd, sdsmcc::filters& filters
         , sdsmcc::programs& programs, __u16 pmt_pid, __u16 es_pid
         , sdsmcc::carousels& carousels)
{
  std::cout << "private sections data" << std::endl;

  typedef gts::sections::extended_section<unsigned char const*> section_type;

  bool ait = false;
  {
    section_type section(buf, buf + count);
    if(/*section.begin() != section.end()
         &&*/ *section.begin() == 0x74 /*AIT*/)
      ait = true;
  }

  if(ait)
  {
    filters.remove_filter(fd);
    std::cout << "AIT" << std::endl;
    typedef gts::sections::application_information_section<unsigned char const*> ait_type;
    typedef ait_type::table_id_iterator table_id_iterator;
    typedef ait_type::section_syntax_identifier_iterator section_syntax_identifier_iterator;
    typedef ait_type::section_length_iterator section_length_iterator;
    typedef ait_type::application_type_iterator application_type_iterator;
    typedef ait_type::version_number_iterator version_number_iterator;
    typedef ait_type::current_next_indicator_iterator current_next_indicator_iterator;
    typedef ait_type::section_number_iterator section_number_iterator;
    typedef ait_type::last_section_number_iterator last_section_number_iterator;
    typedef ait_type::descriptors_iterator descriptors_iterator;
    typedef ait_type::applications_iterator applications_iterator;
    ait_type ait(buf, buf + count);

    table_id_iterator table_id_it = ait.begin();
    assert(*table_id_it == 0x74);

    section_syntax_identifier_iterator section_syntax_identifier_it = ++table_id_it;
    if(!*section_syntax_identifier_it)
    {
      std::cout << "AIT has wrong syntax" << std::endl;
      return;
    }

    application_type_iterator application_type_it = gts::iterators::next<2>(section_syntax_identifier_it);
    std::cout << "Application type: ";
    if(*application_type_it == 0x0001)
      std::cout << "Ginga-J" << std::endl;
    else if(*application_type_it == 0x0009)
      std::cout << "Ginga-NCL" << std::endl;
    else
      std::cout << "Unknown application" << std::endl;

    descriptors_iterator descriptors_it = gts::iterators::next<5>(application_type_it);
    typedef descriptors_iterator::deref_type descriptor_range;
    descriptor_range descriptors = *descriptors_it;

    std::size_t descriptors_ = 0;
    for(boost::range_iterator<descriptor_range>::type first = boost::begin(descriptors)
          , last = boost::end(descriptors); first != last; ++first)
    {
      std::cout << "descriptor " << ++descriptors_ << std::endl;
    }
    if(descriptors_ == 0)
      std::cout << "No descriptors" << std::endl;

    applications_iterator applications_it = ++descriptors_it;
    typedef applications_iterator::deref_type applications_range;
    applications_range applications = *applications_it;

    std::size_t applications_ = 0;
    for(boost::range_iterator<applications_range>::type first = boost::begin(applications)
          , last = boost::end(applications); first != last; ++first)
    {
      std::cout << "application " << ++applications_ << std::endl;

      typedef gts::sections::application_information<unsigned char const*> application_info_type;
      typedef application_info_type::organization_identifier_iterator organization_identifier_iterator;
      typedef application_info_type::application_identifier_iterator application_identifier_iterator;
      typedef application_info_type::application_control_code_iterator application_control_code_iterator;
      typedef application_info_type::application_descriptors_iterator application_descriptors_iterator;
      application_info_type application_info = *first;

      organization_identifier_iterator organization_identifier_it = application_info.begin();
      std::cout << "organization_identifier: " << *organization_identifier_it << std::endl;
      application_identifier_iterator application_identifier_it = ++organization_identifier_it;
      std::cout << "application_identifier: " << *application_identifier_it << std::endl;
      application_control_code_iterator application_control_code_it = ++application_identifier_it;
      std::cout << "application_control_code: ";
      switch(*application_control_code_it)
      {
      case 1:
        std::cout << "AUTOSTART" << std::endl;
        break;
      case 2:
        std::cout << "PRESENT" << std::endl;
        break;
      case 3:
        std::cout << "DESTROY" << std::endl;
        break;
      case 4:
        std::cout << "KILL" << std::endl;
        break;
      case 6:
        std::cout << "REMOTE" << std::endl;
        break;
      case 7:
        std::cout << "UNBOUND" << std::endl;
        break;
      default:
        std::cout << "unknown" << std::endl;
      }
      
      std::size_t app_descriptors_ = 0u;
      application_descriptors_iterator application_descriptors_it = ++application_control_code_it;
      typedef application_descriptors_iterator::deref_type application_descriptors_range;
      application_descriptors_range application_descriptors = *application_descriptors_it;
      for(boost::range_iterator<application_descriptors_range>::type first = boost::begin(application_descriptors)
            , last = boost::end(application_descriptors); first != last; ++first)
      {
        std::cout << "app descriptor " << ++app_descriptors_ << std::endl;
        typedef gts::descriptors::tagged_descriptor<unsigned char const*> descriptor_type;
        descriptor_type descriptor = *first;
        std::cout << "Descriptor tag: " << std::hex << *descriptor.begin() << std::dec << std::endl;
      }
    }
  }
}

} }


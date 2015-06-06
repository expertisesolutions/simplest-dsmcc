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

#include <sdsmcc/frontend.hpp>
#include <sdsmcc/filters.hpp>
#include <sdsmcc/parsers/parsers.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/bind.hpp>

#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace sdsmcc {

void save_directory(boost::filesystem::path path, sdsmcc::directory_object const& directory
                    , sdsmcc::carousels const& carousels)
{
  for(std::vector<sdsmcc::directory_bind>::const_iterator
        bind_first = directory.binds.begin()
        , bind_last = directory.binds.end()
        ;bind_first != bind_last; ++bind_first)
  {
    sdsmcc::file_object const* file = carousels.carousels_.find(bind_first->carousel_id)
      ->second.find_file(bind_first->object_key, bind_first->module_id);
    std::string filename (bind_first->names.id.begin(), bind_first->names.id.end());
    filename.erase(std::remove(filename.begin(), filename.end(), '\0'));
    
    if(file)
    {
      std::cout << "Found file " << filename << std::endl;
      boost::filesystem::ofstream fstream(path / filename);
      std::copy(file->content.begin(), file->content.end()
                , std::ostream_iterator<unsigned char>(fstream));
    }
    else
    {
      sdsmcc::directory_object const* object = carousels.carousels_.find(bind_first->carousel_id)
        ->second.find_directory(bind_first->object_key, bind_first->module_id);
      if(object)
      {
        boost::filesystem::path child_directory = path / filename;
        boost::filesystem::create_directory(child_directory);
        save_directory(child_directory, *object, carousels);
      }
      else
        std::cout << "Couldn't find file or directory named " << filename
                  << " with carousel: " << bind_first->carousel_id
                  << " and module_id " << bind_first->module_id << std::endl;
    }
  }
}

void save_dsmcc(unsigned int frequency, boost::filesystem::path directory
                , int adapter)
{
  std::cout << "Opening /dev/dvb/adapter" << adapter << "/frontend0" << std::endl;
  std::string adapter_devname;
  {
    std::stringstream s;
    s << "/dev/dvb/adapter" << adapter;
    adapter_devname = s.str();
  }
  int frontend_fd = 0;
  {
    const std::string frontend_devname = adapter_devname + "/frontend0";
    frontend_fd = open(frontend_devname.c_str(), O_RDWR);
    if(frontend_fd < 0)
    {
      std::cout << "Couldn't open frontend device " << frontend_devname << " for reading and writing" << std::endl;
      throw std::runtime_error("Coudl'nt open frontend device");
    }
  }

  try
  {
    sdsmcc::frontend frontend (frontend_fd);
    frontend.tune(frequency);

    sdsmcc::programs programs;
    sdsmcc::carousels carousels;

    sdsmcc::filters filters(adapter_devname + "/demux0");
    {
      sdsmcc::filters::table_filter pat(0);
      filters.add_table_filter(pat, boost::bind(&sdsmcc::parsers::pat, _1, _2, _3, boost::ref(filters)
                                                , boost::ref(programs), boost::ref(carousels))
                               , DMX_IMMEDIATE_START|DMX_ONESHOT|DMX_CHECK_CRC);
    }

    filters.run();

    std::cout << "Finished. Carousels: " << carousels.carousels_.size() << std::endl;
    unsigned int files = 0, directories = 0;
    for(std::map<unsigned int, sdsmcc::carousel>::const_iterator
          first = carousels.carousels_.begin()
          , last = carousels.carousels_.end()
          ; first != last;++first)
    {
      files += first->second.file_objects.size();
      directories += first->second.directory_objects.size();
    }
    std::cout << "Files: " << files << " directories: " << directories << std::endl;

    directories = 0;
    for(std::map<unsigned int, sdsmcc::carousel>::const_iterator
          first = carousels.carousels_.begin()
          , last = carousels.carousels_.end()
          ;first != last;++first)
    {
      std::stringstream s;
      s << "directory" << directories;
      boost::filesystem::path child_directory = directory / s.str();
      boost::filesystem::create_directory(child_directory);
      sdsmcc::directory_object const& gateway = first->second.gateway;
      save_directory(child_directory, gateway, carousels);
    }
  }
  catch(std::exception const& e)
  {
    std::cout << "Exception caught " << e.what() << std::endl;
  }
}

}

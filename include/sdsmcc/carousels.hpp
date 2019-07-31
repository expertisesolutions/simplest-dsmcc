///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2011-2014 Felipe Magno de Almeida.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// See http://www.boost.org/libs/foreach for documentation
//

#ifndef DFD_CAROUSELS_HPP
#define DFD_CAROUSELS_HPP

#include <boost/range.hpp>

#include <map>
#include <vector>

namespace sdsmcc {

struct file_object
{
  std::vector<unsigned char> object_key;
  std::vector<unsigned char> content;
};

struct bind_name
{
  std::vector<unsigned char> id, kind;
};

struct directory_bind
{
  bind_name names;
  unsigned int carousel_id, module_id;
  std::vector<unsigned char> object_key;
};

struct directory_object
{
  std::vector<unsigned char> object_key;
  std::vector<directory_bind> binds;
};

struct object_key
{
  std::vector<unsigned char> object_key;
  unsigned int module_id;
};

inline bool operator<(object_key const& lhs, object_key const& rhs)
{
  return lhs.object_key < rhs.object_key
    || (lhs.object_key == rhs.object_key && lhs.module_id < rhs.module_id);
}

struct carousel
{
  template <typename BaseIterator>
  void add_file_object(boost::iterator_range<BaseIterator> object_key
                       , boost::iterator_range<BaseIterator> content
                       , unsigned int module_id)
  {
    std::vector<unsigned char> object_key_ (boost::begin(object_key), boost::end(object_key));
    std::vector<unsigned char> content_ (boost::begin(content), boost::end(content));
    file_object f = {object_key_, content_};
    sdsmcc::object_key k = {object_key_, module_id};
    file_objects.insert(std::make_pair(k, f));
  }

  template <typename BaseIterator, typename BindRange>
  void add_directory_object(boost::iterator_range<BaseIterator> object_key
                            , BindRange binds
                            , unsigned int module_id)
  {
    std::vector<unsigned char> object_key_(boost::begin(object_key), boost::end(object_key));
    directory_object d = {object_key_};
    for(typename boost::range_iterator<BindRange>::type
          first = boost::begin(binds), last = boost::end(binds)
          ;first != last; ++first)
    {
      if(!first->ior.biop_profiles.empty() && !first->names.empty())
      {
        std::vector<unsigned char> id(boost::begin(first->names[0].id)
                                      , boost::end(first->names[0].id));
        std::vector<unsigned char> kind(boost::begin(first->names[0].kind)
                                        , boost::end(first->names[0].kind));
        bind_name name = {id, kind};
        std::vector<unsigned char> bind_object_key(boost::begin(first->ior.biop_profiles[0].object_location.object_key)
                                                   , boost::end(first->ior.biop_profiles[0].object_location.object_key));
        directory_bind b = {name, first->ior.biop_profiles[0].object_location.carousel_id
                            , first->ior.biop_profiles[0].object_location.module_id
                            , bind_object_key};
        d.binds.push_back(b);
      }
      else
        std::cout << "No biop profile" << std::endl;
    }
    sdsmcc::object_key k = {object_key_, module_id};
    directory_objects.insert(std::make_pair(k, d));
  }

  template <typename BaseIterator, typename BindRange>
  void add_gateway_object(boost::iterator_range<BaseIterator> object_key
                          , BindRange binds, unsigned int module_id)
  {
    std::vector<unsigned char> object_key_(boost::begin(object_key), boost::end(object_key));
    directory_object d = {object_key_};
    for(typename boost::range_iterator<BindRange>::type
          first = boost::begin(binds), last = boost::end(binds)
          ;first != last; ++first)
    {
      if(!first->ior.biop_profiles.empty() && !first->names.empty())
      {
        std::vector<unsigned char> id(boost::begin(first->names[0].id)
                                      , boost::end(first->names[0].id));
        std::vector<unsigned char> kind(boost::begin(first->names[0].kind)
                                        , boost::end(first->names[0].kind));
        bind_name name = {id, kind};
        std::vector<unsigned char> bind_object_key(boost::begin(first->ior.biop_profiles[0].object_location.object_key)
                                                   , boost::end(first->ior.biop_profiles[0].object_location.object_key));
        directory_bind b = {name, first->ior.biop_profiles[0].object_location.carousel_id
                            , first->ior.biop_profiles[0].object_location.module_id
                            , bind_object_key};
        d.binds.push_back(b);
      }
      else
        std::cout << "No biop profile" << std::endl;
    }
    gateway = d;
  }

  file_object const* find_file(std::vector<unsigned char> const& object_key, unsigned int module_id) const
  {
    return const_cast<carousel&>(*this).find_file(object_key, module_id);
  }

  file_object* find_file(std::vector<unsigned char> const& object_key, unsigned int module_id)
  {
    sdsmcc::object_key k = {object_key, module_id};
    std::map<sdsmcc::object_key, file_object>::iterator iterator
      = file_objects.find(k);
    if(iterator != file_objects.end())
      return &iterator->second;
    else
      return 0;
  }

  directory_object const* find_directory(std::vector<unsigned char> const& object_key, unsigned int module_id) const
  {
    return const_cast<carousel&>(*this).find_directory(object_key, module_id);
  }

  directory_object* find_directory(std::vector<unsigned char> const& object_key, unsigned int module_id)
  {
    sdsmcc::object_key k = {object_key, module_id};
    std::map<sdsmcc::object_key, directory_object>::iterator iterator
      = directory_objects.find(k);
    if(iterator != directory_objects.end())
      return &iterator->second;
    else
      return 0;
  }

  std::map<sdsmcc::object_key, file_object> file_objects;
  std::map<sdsmcc::object_key, directory_object> directory_objects;
  directory_object gateway;
};

struct carousels
{
  std::map<unsigned int, carousel> carousels_;
};

}

#endif

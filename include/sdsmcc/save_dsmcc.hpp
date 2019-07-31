///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2011-2014 Felipe Magno de Almeida.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// See http://www.boost.org/libs/foreach for documentation
//

#ifndef SDSMCC_SAVE_DSMCC_HPP
#define SDSMCC_SAVE_DSMCC_HPP

#include <boost/filesystem/path.hpp>

namespace sdsmcc {

void save_dsmcc(unsigned int frequency, boost::filesystem::path directory, int adapter);

}

#endif

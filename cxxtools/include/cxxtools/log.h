/* cxxtools/log.h
 *
 * cxxtools - general purpose C++-toolbox
 * Copyright (C) 2003, Tommi Maekitalo
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef CXXTOOLS_LOG_H
#define CXXTOOLS_LOG_H

#if defined(_WIN32) || defined(WIN32)
    #define CXXTOOLS_LOG_DISABLE
#endif

#if  !defined(CXXTOOLS_LOG_LOG4CXX) \
  && !defined(CXXTOOLS_LOG_LOG4CPLUS) \
  && !defined(CXXTOOLS_LOG_LOGBUILTIN) \
  && !defined(CXXTOOLS_LOG_DISABLE)
#  include <cxxtools/config.h>
#endif


#ifdef CXXTOOLS_LOG_LOG4CXX
# include <cxxtools/log/log4cxx.h>
#endif


#ifdef CXXTOOLS_LOG_LOG4CPLUS
# include <cxxtools/log/log4cplus.h>
#endif


#ifdef CXXTOOLS_LOG_LOGBUILTIN
# include <cxxtools/log/cxxtools.h>
#endif


#ifdef CXXTOOLS_LOG_DISABLE
# include <cxxtools/log/disable.h>
#endif


#endif // LOG_H

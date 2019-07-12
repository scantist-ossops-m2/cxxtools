/*
 * Copyright (C) 2019 Tommi Maekitalo
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * As a special exception, you may use this file as part of a free
 * software library without restriction. Specifically, if other files
 * instantiate templates or use macros or inline functions from this
 * file, or you compile this file and link it with other files to
 * produce an executable, this file does not by itself cause the
 * resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other
 * reasons why the executable file might be covered by the GNU Library
 * General Public License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef CXXTOOLS_INISERIALIZER_H
#define CXXTOOLS_INISERIALIZER_H

#include <cxxtools/serializationinfo.h>
#include <cxxtools/textstream.h>
#include <cxxtools/utf8codec.h>

namespace cxxtools
{
    class IniSerializer
    {
        public:
            explicit IniSerializer(std::ostream& os, TextCodec<Char, char>* codec = new Utf8Codec())
                : _pos(new TextOStream(os, codec)),
                  _os(*_pos)
            {
            }

            explicit IniSerializer(TextOStream& os)
                : _pos(0),
                  _os(os)
            {
            }

            ~IniSerializer()
            {  delete _pos; }

            IniSerializer& serialize(const SerializationInfo& si);

            template <typename T>
            IniSerializer& serialize(const T& v)
            {
                SerializationInfo si;
                si <<= v;
                serialize(si);
                return *this;
            }

        private:
            TextOStream* _pos;
            TextOStream& _os;
            bool _inObject;
    };

}

#endif // CXXTOOLS_INISERIALIZER_H


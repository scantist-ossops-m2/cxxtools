/*
 * Copyright (C) 2011 Tommi Maekitalo
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

#include <cxxtools/bin/valueparser.h>
#include <cxxtools/deserializer.h>
#include <cxxtools/bin/serializer.h>
#include <cxxtools/log.h>

log_define("cxxtools.bin.valueparser")

namespace cxxtools
{
namespace bin
{
namespace
{
    const char* typeName(char typeCode)
    {
        switch (typeCode)
        {
            case Serializer::TypeEmpty: return "";
            case Serializer::TypeBool: return "bool";
            case Serializer::TypeChar: return "char";
            case Serializer::TypeString: return "string";
            case Serializer::TypeInt:
            case Serializer::TypeInt8:
            case Serializer::TypeInt16:
            case Serializer::TypeInt32:
            case Serializer::TypeInt64:
            case Serializer::TypeUInt8:
            case Serializer::TypeUInt16:
            case Serializer::TypeUInt32:
            case Serializer::TypeUInt64: return "int";
            case Serializer::TypeDouble:
            case Serializer::TypeBcdDouble: return "double";
            case Serializer::TypeBinary2:
            case Serializer::TypeBinary4: return "binary";
            case Serializer::TypePair: return "pair";
            case Serializer::TypeArray: return "array";
            case Serializer::TypeList: return "list";
            case Serializer::TypeDeque: return "deque";
            case Serializer::TypeSet: return "set";
            case Serializer::TypeMultiset: return "multiset";
            case Serializer::TypeMap: return "map";
            case Serializer::TypeMultimap: return "multimap";
            default:
            {
                std::ostringstream msg;
                msg << "unknown serialization type code " << static_cast<unsigned>(static_cast<unsigned char>(typeCode));
                throw SerializationError(msg.str());
            }
        }
        return 0;  // never reached
    }

    static const char bcdDigits[16] = "0123456789+-. e";
}

void ValueParser::begin(DeserializerBase& handler)
{
    _state = state_0;
    _deserializer = &handler;
    _int = 0;
    _token.clear();
}


void ValueParser::beginSkip()
{
    _state = state_0;
    _deserializer = 0;
    _int = 0;
    _token.clear();
}


bool ValueParser::advance(char ch)
{
    switch (_state)
    {
        case state_0:
            {
                SerializationInfo::Category category = static_cast<SerializationInfo::Category>(ch);
                switch (category)
                {
                    case SerializationInfo::Value:     _state = state_value_name;     break;
                    case SerializationInfo::Object:    _state = state_object_name;    break;
                    case SerializationInfo::Array:     _state = state_array_name;     break;
                    default:
                    {
                        std::ostringstream msg;
                        msg << "invalid category code " << category;
                        throw SerializationError(msg.str());
                    }
                }

                if (_deserializer)
                    _deserializer->setCategory(category);
            }
            break;

        case state_value_name:
        case state_object_name:
        case state_array_name:
            if (ch == '\0')
            {
                if (_deserializer)
                    _deserializer->setName(_token);
                _token.clear();
                _state = _state == state_value_name  ? state_value_type
                       : _state == state_object_name ? state_object_type
                       : state_array_type;
            }
            else
                _token += ch;
            break;

        case state_value_type:
            if (ch == '\xff')
                _state = state_value_type_other;
            else
            {
                if (_deserializer)
                    _deserializer->setTypeName(typeName(ch));

                switch (ch)
                {
                    case Serializer::TypeEmpty:  if (_deserializer) _deserializer->setNull(); _state = state_end; break;
                    case Serializer::TypeInt8:   _count = 1; _state = state_value_intsign; break;
                    case Serializer::TypeInt16:  _count = 2; _state = state_value_intsign; break;
                    case Serializer::TypeInt32:  _count = 4; _state = state_value_intsign; break;
                    case Serializer::TypeInt64:  _count = 8; _state = state_value_intsign; break;
                    case Serializer::TypeUInt8:  _count = 1; _state = state_value_uint; break;
                    case Serializer::TypeUInt16: _count = 2; _state = state_value_uint; break;
                    case Serializer::TypeUInt32: _count = 4; _state = state_value_uint; break;
                    case Serializer::TypeUInt64: _count = 8; _state = state_value_uint; break;
                    case Serializer::TypeBool: _state = state_value_bool; break;
                    case Serializer::TypeBcdDouble: _state = state_value_bcd0; break;
                    case Serializer::TypeBinary2: _count = 2; _state = state_value_binary_length; break;
                    case Serializer::TypeBinary4: _count = 4; _state = state_value_binary_length; break;
                    default: _state = state_value_value;
                }

            }
            break;

        case state_value_type_other:
            if (ch == '\0')
            {
                if (_deserializer)
                    _deserializer->setTypeName(_token);

                _token.clear();
                _state = state_value_value;
            }
            break;

        case state_value_intsign:
            if (static_cast<signed char>(ch) < 0)
                _int = -1;
            else
                _int = 0;

            _state = state_value_int;
            // no break

        case state_value_int:
        case state_value_uint:
            _int = (_int << 8) | static_cast<unsigned char>(ch);
            if (--_count == 0)
            {
                if (_deserializer)
                {
                    if (_state == state_value_int)
                    {
                        DeserializerBase::int_type value = DeserializerBase::int_type(_int);
                        log_debug("int value=" << value);
                        _deserializer->setValue(value);
                    }
                    else
                    {
                        DeserializerBase::unsigned_type value = DeserializerBase::unsigned_type(_int);
                        log_debug("uint value=" << value);
                        _deserializer->setValue(value);
                    }
                }

                _int = 0;
                _state = state_end;
            }
            break;

        case state_value_bool:
            if (_deserializer)
                _deserializer->setValue(ch != '\0');

            _state = state_end;
            break;

        case state_value_bcd0:
            if (ch == '\xf0')
            {
                if (_deserializer)
                    _deserializer->setValue("nan");
                _state = state_end;
                break;
            }
            else if (ch == '\xf1')
            {
                if (_deserializer)
                    _deserializer->setValue("inf");
                _state = state_end;
                break;
            }
            else if (ch == '\xf2')
            {
                if (_deserializer)
                    _deserializer->setValue("-inf");
                _state = state_end;
                break;
            }

            _state = state_value_bcd;

            // no break

        case state_value_bcd:
            if (ch == '\xff')
            {
                if (_deserializer)
                    _deserializer->setValue(_token);
                _token.clear();
                return true;
            }
            else
            {
                _token += bcdDigits[static_cast<uint8_t>(ch) >> 4];
                if ((ch & '\xf') == '\xd')
                {
                    if (_deserializer)
                        _deserializer->setValue(_token);
                    _token.clear();
                    _state = state_end;
                }
                else
                {
                    _token += bcdDigits[static_cast<uint8_t>(ch) & '\xf'];
                }
            }

            break;

        case state_value_binary_length:
            _int = (_int << 8) | static_cast<unsigned char>(ch);
            if (--_count == 0)
            {
                _count = static_cast<unsigned>(_int);
                _int = 0;
                if (_count == 0)
                {
                    if (_deserializer)
                        _deserializer->setValue(std::string());
                    _state = state_end;
                }
                else
                {
                    _state = state_value_binary;
                }
            }
            break;

        case state_value_binary:
            if (_deserializer)
                _token += ch;

            if (--_count == 0)
            {
                if (_deserializer)
                    _deserializer->setValue(_token);
                _state = state_end;
            }
            break;

        case state_value_value:
            if (ch == '\0')
            {
                if (_deserializer)
                    _deserializer->setValue(_token);
                _token.clear();
                _state = state_end;
            }
            else
                _token += ch;
            break;

        case state_object_type:
            if (ch == '\xff')
                _state = state_object_type_other;
            else
            {
                if (_deserializer)
                    _deserializer->setTypeName(typeName(ch));
                _state = state_object_member;
            }
            break;

        case state_object_type_other:
            if (ch == '\0')
            {
                if (_deserializer)
                    _deserializer->setTypeName(_token);
                _token.clear();
                _state = state_object_member;
            }
            else
                _token += ch;
            break;

        case state_object_member:
            if (ch == '\xff')
            {
                return true;
            }
            if (ch != '\1')
                throw SerializationError("member expected");
            _state = state_object_member_name;
            break;

        case state_object_member_name:
            if (ch == '\0')
            {
                if (_next == 0)
                    _next = new ValueParser();

                if (_deserializer)
                {
                    _deserializer->beginMember(_token, "", SerializationInfo::Void);
                    _next->begin(*_deserializer);
                }
                else
                    _next->beginSkip();

                _token.clear();
                _state = state_object_member_value;
            }
            else
                _token += ch;
            break;

        case state_object_member_value:
            if (_next->advance(ch))
            {
                if (_deserializer)
                    _deserializer->leaveMember();
                _state = state_object_member;
            }
            break;

        case state_array_type:
            if (ch == '\xff')
                _state = state_array_type_other;
            else
            {
                if (_deserializer)
                    _deserializer->setTypeName(typeName(ch));
                _state = state_array_member;
            }
            break;

        case state_array_type_other:
            if (ch == '\0')
            {
                if (_deserializer)
                    _deserializer->setTypeName(_token);
                _token.clear();
                _state = state_array_member;
            }
            else
                _token += ch;
            break;

        case state_array_member:
            if (ch == '\xff')
                return true;

            if (_next == 0)
                _next = new ValueParser();

            if (_deserializer)
            {
                _deserializer->beginMember("", "", SerializationInfo::Void);
                _next->begin(*_deserializer);
            }
            else
            {
                _next->beginSkip();
            }

            _next->advance(ch);
            _state = state_array_member_value;
            break;

        case state_array_member_value:
            if (_next->advance(ch))
            {
                if (_deserializer)
                    _deserializer->leaveMember();
                _state = state_array_member_value_next;
            }
            break;

        case state_array_member_value_next:
            if (ch == '\xff')
            {
                return true;
            }
            else
            {
                if (_deserializer)
                {
                    _deserializer->beginMember("", "", SerializationInfo::Void);
                    _next->begin(*_deserializer);
                }
                else
                {
                    _next->beginSkip();
                }

                _next->advance(ch);
                _state = state_array_member_value;
            }
            break;

        case state_end:
            if (ch != '\xff')
                throw SerializationError("end of value marker expected");
            return true;
    }

    return false;
}

}
}

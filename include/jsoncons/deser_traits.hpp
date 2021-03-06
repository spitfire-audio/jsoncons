// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DESER_TRAITS_HPP
#define JSONCONS_DESER_TRAITS_HPP

#include <string>
#include <tuple>
#include <array>
#include <memory>
#include <type_traits> // std::enable_if, std::true_type, std::false_type
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/json_type_traits.hpp>
#include <jsoncons/staj_reader.hpp>
#include <jsoncons/convert_error.hpp>

namespace jsoncons {

    // deser_traits

    template <class T, class CharT, class Enable = void>
    struct deser_traits
    {
        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<CharT>& reader, 
                             json_decoder<Json,TempAllocator>& decoder, 
                             std::error_code& ec)
        {
            decoder.reset();
            reader.read(decoder, ec);
            return decoder.get_result().template as<T>();
        }
    };

    // specializations

    // primitive

    template <class T, class CharT>
    struct deser_traits<T,CharT,
        typename std::enable_if<jsoncons::detail::is_primitive<T>::value
    >::type>
    {
        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<CharT>& reader, 
                             json_decoder<Json,TempAllocator>&, 
                             std::error_code&)
        {
            T v = reader.current().template get<T>();
            return v;
        }
    };

    // string

    template <class T, class CharT>
    struct deser_traits<T,CharT,
        typename std::enable_if<jsoncons::detail::is_string<T>::value &&
                                std::is_same<typename T::value_type,CharT>::value
    >::type>
    {
        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<CharT>& reader, 
                             json_decoder<Json,TempAllocator>&, 
                             std::error_code&)
        {
            T v = reader.current().template get<T>();
            return v;
        }
    };

    template <class T, class CharT>
    struct deser_traits<T,CharT,
        typename std::enable_if<jsoncons::detail::is_string<T>::value &&
                                !std::is_same<typename T::value_type,CharT>::value
    >::type>
    {
        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<CharT>& reader, 
                             json_decoder<Json,TempAllocator>&, 
                             std::error_code&)
        {
            auto val = reader.current().template get<std::basic_string<CharT>>();
            T s;
            unicons::convert(val.begin(), val.end(), std::back_inserter(s));
            return s;
        }
    };

    // std::pair

    template <class T1, class T2, class CharT>
    struct deser_traits<std::pair<T1, T2>, CharT>
    {
        template <class Json, class TempAllocator>
        static std::pair<T1, T2> deserialize(basic_staj_reader<CharT>& reader,
            json_decoder<Json, TempAllocator>& decoder,
            std::error_code& ec)
        {
            using value_type = std::pair<T1, T2>;
            if (reader.current().event_type() != staj_event_type::begin_array)
            {
                ec = convert_errc::json_not_pair;
                return value_type();
            }
            reader.next(ec);
            if (ec)
            {
                ec = convert_errc::json_not_pair;
                return value_type();
            }

            T1 v1 = deser_traits<T1,CharT>::deserialize(reader, decoder, ec);
            if (ec)
            {
                ec = convert_errc::json_not_pair;
                return value_type();
            }
            reader.next(ec);
            if (ec)
            {
                ec = convert_errc::json_not_pair;
                return value_type();
            }
            T2 v2 = deser_traits<T2, CharT>::deserialize(reader, decoder, ec);
            if (ec)
            {
                ec = convert_errc::json_not_pair;
                return value_type();
            }
            reader.next(ec);
            if (ec || reader.current().event_type() != staj_event_type::end_array)
            {
                ec = convert_errc::json_not_pair;
                return value_type();
            }
            return std::make_pair(v1, v2);
        }
    };

    // vector like
    template <class T, class CharT>
    struct deser_traits<T,CharT,
        typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                 jsoncons::detail::is_list_like<T>::value &&
                 jsoncons::detail::has_push_back<T>::value &&
                 !jsoncons::detail::is_typed_array<T>::value 
    >::type>
    {
        using value_type = typename T::value_type;

        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<CharT>& reader, 
                             json_decoder<Json,TempAllocator>& decoder, 
                             std::error_code& ec)
        {
            T v;

            if (reader.current().event_type() != staj_event_type::begin_array)
            {
                ec = convert_errc::json_not_vector;
                return v;
            }
            reader.next(ec);
            while (reader.current().event_type() != staj_event_type::end_array && !ec)
            {
                v.push_back(deser_traits<value_type,CharT>::deserialize(reader, decoder, ec));
                reader.next(ec);
            }
            return v;
        }
    };

    template <class T>
    struct typed_array_visitor : public default_json_visitor
    {
        T& v_;
        int level_;
    public:
        using value_type = typename T::value_type;

        typed_array_visitor(T& v)
            : default_json_visitor(false,convert_errc::json_not_vector), v_(v), level_(0)
        {
        }
    private:
        bool visit_begin_array(semantic_tag, 
                               const ser_context&, 
                               std::error_code& ec) override
        {      
            if (++level_ != 1)
            {
                ec = convert_errc::json_not_vector;
                return false;
            }
            return true;
        }

        bool visit_begin_array(std::size_t size, 
                            semantic_tag, 
                            const ser_context&, 
                            std::error_code& ec) override
        {
            if (++level_ != 1)
            {
                ec = convert_errc::json_not_vector;
                return false;
            }
            v_.reserve(size);
            return true;
        }

        bool visit_end_array(const ser_context&, 
                          std::error_code& ec) override
        {
            if (level_ != 1)
            {
                ec = convert_errc::json_not_vector;
                return false;
            }
            return false;
        }

        bool visit_uint64(uint64_t value, 
                             semantic_tag, 
                             const ser_context&,
                             std::error_code&) override
        {
            v_.push_back(static_cast<value_type>(value));
            return true;
        }

        bool visit_int64(int64_t value, 
                            semantic_tag,
                            const ser_context&,
                            std::error_code&) override
        {
            v_.push_back(static_cast<value_type>(value));
            return true;
        }

        bool visit_half(uint16_t value, 
                           semantic_tag,
                           const ser_context&,
                           std::error_code&) override
        {
            return visit_half_(typename std::integral_constant<bool, std::is_integral<value_type>::value>::type(), value);
        }

        bool visit_half_(std::true_type, uint16_t value)
        {
            v_.push_back(static_cast<value_type>(value));
            return true;
        }

        bool visit_half_(std::false_type, uint16_t value)
        {
            v_.push_back(static_cast<value_type>(jsoncons::detail::decode_half(value)));
            return true;
        }

        bool visit_double(double value, 
                             semantic_tag,
                             const ser_context&,
                             std::error_code&) override
        {
            v_.push_back(static_cast<value_type>(value));
            return true;
        }

        bool visit_typed_array(const span<const value_type>& data,  
                            semantic_tag,
                            const ser_context&,
                            std::error_code&) override
        {
            v_ = std::vector<value_type>(data.begin(),data.end());
            return false;
        }
    };

    template <class T, class CharT>
    struct deser_traits<T,CharT,
        typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                 jsoncons::detail::is_list_like<T>::value &&
                 jsoncons::detail::is_typed_array<T>::value 
    >::type>
    {
        using value_type = typename T::value_type;

        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<CharT>& reader, 
                             json_decoder<Json,TempAllocator>&, 
                             std::error_code& ec)
        {
            T v;

            if (reader.current().event_type() != staj_event_type::begin_array)
            {
                ec = convert_errc::json_not_vector;
                return v;
            }

            typed_array_visitor<T> visitor(v);
            reader.read(visitor, ec);
            return v;
        }
    };

    // std::array

    template <class T, class CharT, std::size_t N>
    struct deser_traits<std::array<T,N>,CharT>
    {
        using value_type = typename std::array<T,N>::value_type;

        template <class Json,class TempAllocator>
        static std::array<T, N> deserialize(basic_staj_reader<CharT>& reader, 
                                            json_decoder<Json,TempAllocator>& decoder, 
                                            std::error_code& ec)
        {
            std::array<T,N> v;
            v.fill(T{});
            if (reader.current().event_type() != staj_event_type::begin_array)
            {
                ec = convert_errc::json_not_vector;
            }
            reader.next(ec);
            for (std::size_t i = 0; i < N && reader.current().event_type() != staj_event_type::end_array && !ec; ++i)
            {
                v[i] = deser_traits<value_type,CharT>::deserialize(reader, decoder, ec);
                reader.next(ec);
            }
            return v;
        }
    };

    // map like

    template <class T, class CharT>
    struct deser_traits<T,CharT,
        typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                                jsoncons::detail::is_map_like<T>::value &&
                                jsoncons::detail::is_constructible_from_const_pointer_and_size<typename T::key_type>::value
    >::type>
    {
        using mapped_type = typename T::mapped_type;
        using value_type = typename T::value_type;
        using key_type = typename T::key_type;

        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<CharT>& reader, 
                             json_decoder<Json,TempAllocator>& decoder, 
                             std::error_code& ec)
        {
            T val;
            if (reader.current().event_type() != staj_event_type::begin_object)
            {
                ec = convert_errc::json_not_map;
                return val;
            }
            reader.next(ec);

            while (reader.current().event_type() != staj_event_type::end_object && !ec)
            {
                if (reader.current().event_type() != staj_event_type::key)
                {
                    ec = json_errc::expected_name;
                    return val;
                }
                auto key = reader.current(). template get<key_type>();
                reader.next(ec);
                val.emplace(std::move(key),deser_traits<mapped_type,CharT>::deserialize(reader, decoder, ec));
                reader.next(ec);
            }
            return val;
        }
    };

    template <class T, class CharT>
    struct deser_traits<T,CharT,
        typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                                jsoncons::detail::is_map_like<T>::value &&
                                std::is_integral<typename T::key_type>::value
    >::type>
    {
        using mapped_type = typename T::mapped_type;
        using value_type = typename T::value_type;
        using key_type = typename T::key_type;

        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<CharT>& reader, 
                             json_decoder<Json,TempAllocator>& decoder, 
                             std::error_code& ec)
        {
            T val;
            if (reader.current().event_type() != staj_event_type::begin_object)
            {
                ec = convert_errc::json_not_map;
                return val;
            }
            reader.next(ec);

            while (reader.current().event_type() != staj_event_type::end_object && !ec)
            {
                if (reader.current().event_type() != staj_event_type::key)
                {
                    ec = json_errc::expected_name;
                    return val;
                }
                auto s = reader.current().template get<basic_string_view<typename Json::char_type>>();
                auto key = jsoncons::detail::to_integer<key_type>(s.data(), s.size()); 
                reader.next(ec);
                val.emplace(key.value(),deser_traits<mapped_type,CharT>::deserialize(reader, decoder, ec));
                reader.next(ec);
            }
            return val;
        }
    };

} // jsoncons

#endif


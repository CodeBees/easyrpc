#ifndef _PROTOCOLDEFINE_H
#define _PROTOCOLDEFINE_H

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/stack.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/serialization/hash_map.hpp>
#include <boost/serialization/hash_set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/boost_unordered_map.hpp>
#include <boost/serialization/boost_unordered_set.hpp>

struct PersonInfoReq
{
    int cardId;
    std::string name;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        ar & cardId;
        ar & name;
    }
};

struct PersonInfoRes
{
    int cardId;
    std::string name;
    int age;
    std::string national;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        ar & cardId;
        ar & name;
        ar & age;
        ar & national;
    }
};

#endif

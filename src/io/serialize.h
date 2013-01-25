// Copyright 2013 Eugen Sawin <esawin@me73.com>
#ifndef SRC_IO_SERIALIZE_H_
#define SRC_IO_SERIALIZE_H_

#include <istream>
#include <ostream>
#include <vector>
#include <unordered_map>
#include <glog/logging.h>

namespace pyt {
namespace io {

bool IsLittleEndian() {
  uint16_t i = 1;
  return *reinterpret_cast<uint8_t*>(&i);
}

template<typename T>
T ToNetworkFormat(const T& value) {
  static const bool is_le = IsLittleEndian();
  T net_value = value;
  if (!is_le) {
    LOG(FATAL) << "Big Endian systems are not supported yet.";
  }
  return net_value;
}

template<typename T>
T FromNetworkFormat(const T& value) {
  static const bool is_le = IsLittleEndian();
  T net_value = value;
  if (!is_le) {
    LOG(FATAL) << "Big Endian systems are not supported yet.";
  }
  return net_value;
}

template<typename T>
struct ContainerType {
  enum Type { kNone, kVector, kMap };
  static const Type type = kNone;
};

template<typename T>
struct ContainerType<std::vector<T> > {
  enum Type { kNone, kVector, kMap };
  static const Type type = kVector;
};

template<typename Key, typename Value>
struct ContainerType<std::unordered_map<Key, Value> > {
  enum Type { kNone, kVector, kMap };
  static const Type type = kMap;
};

template<typename T>
struct IsPair {
  static const bool value = false;
};

template<typename T1, typename T2>
struct IsPair<std::pair<T1, T2> >{
  static const bool value = true;
};

template<typename T>
struct Reader {
  static void Read(std::istream& stream, T* target) {
    stream.read(reinterpret_cast<char*>(target), sizeof(T));
    *target = FromNetworkFormat(*target);
  }
};

template<>
struct Reader<int> {
  static void Read(std::istream& stream, int* target) {
    int32_t value;
    stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    *target = FromNetworkFormat(value);
  }
};

template<>
struct Reader<std::string> {
  static void Read(std::istream& stream, std::string* target) {
    uint64_t size;
    Reader<uint64_t>::Read(stream, &size);
    uint64_t prev_size = target->size();
    target->resize(prev_size + size);
    stream.read(&(*target)[prev_size], size);
  }
};

template<typename T1, typename T2>
struct Reader<std::pair<T1, T2> > {
  static void Read(std::istream& stream, std::pair<T1, T2>* target) {
    Reader<T1>::Read(stream, &target->first);
    Reader<T2>::Read(stream, &target->second);
  }
};

template<typename T>
struct Reader<std::vector<T> > {
  static void Read(std::istream& stream, std::vector<T>* target) {
    uint64_t nelems;
    Reader<uint64_t>::Read(stream, &nelems);
    uint64_t prev_size = target->size();
    target->reserve(prev_size + nelems);
    for (uint64_t i = prev_size; i < prev_size + nelems; ++i) {
      target->push_back(T());
      Reader<T>::Read(stream, &target->back());
    }
  }
};

template<template<typename Key, typename Value> class Container,
    typename Key, typename Value>
struct Reader<Container<Key, Value> > {
  static void Read(std::istream& stream, Container<Key, Value>* target) {
    std::vector<std::pair<Key, Value> > vec;
    Reader<std::vector<std::pair<Key, Value> > >::Read(stream, &vec);
    target->insert(vec.begin(), vec.end());
  }
};

template<typename T>
struct Writer {
  static void Write(const T& target, std::ostream& stream) {
    T net_target = ToNetworkFormat(target);
    stream.write(reinterpret_cast<const char*>(&net_target), sizeof(T));
  }
};

template<>
struct Writer<int> {
  static void Write(const int& target, std::ostream& stream) {
    const int32_t value = ToNetworkFormat(target);
    stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
  }
};

template<>
struct Writer<std::string> {
  static void Write(const std::string& target, std::ostream& stream) {
    uint64_t size = target.size() + 1;
    Writer<uint64_t>::Write(size, stream);
    stream.write(&target[0], size);
  }
};

template<typename T1, typename T2>
struct Writer<std::pair<T1, T2> > {
  static void Write(const std::pair<T1, T2>& target, std::ostream& stream) {
    Writer<T1>::Write(target.first, stream);
    Writer<T2>::Write(target.second, stream);
  }
};

template<typename T>
struct Writer<std::vector<T> > {
  static void Write(const std::vector<T>& target, std::ostream& stream) {
    const uint64_t nelems = target.size();
    Writer<uint64_t>::Write(nelems, stream);
    for (const T& e: target) {
      Writer<T>::Write(e, stream);
    }
  }
};

template<template<typename Key, typename Value> class Container,
    typename Key, typename Value>
struct Writer<Container<Key, Value> > {
  static void Write(const Container<Key, Value>& target, std::ostream& stream) {
    std::vector<std::pair<Key, Value> > vec(target.begin, target.end());
    Writer<std::vector<std::pair<Key, Value> > >::Write(vec, stream);
  }
};

}  // namespace io
}  // namespace pyt
#endif  // SRC_IO_SERIALIZE_H_

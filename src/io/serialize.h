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
struct IsContainer {
  static const bool value = ContainerType<T>::type != ContainerType<T>::kNone;
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
void Read(std::istream& stream, T* target) {
  stream.read(reinterpret_cast<char*>(target), sizeof(T));
  *target = FromNetworkFormat(*target);
}

template<typename T>
void Read(std::istream& stream, const T* target) {
  T* t = const_cast<T*>(target);
  stream.read(reinterpret_cast<char*>(t), sizeof(T));
  *t = FromNetworkFormat(*target);
}

template<typename T1, typename T2>
void Read(std::istream& stream, std::pair<T1, T2>* target) {
  Read(stream, &target->first);
  Read(stream, &target->second);
}

template<template <typename...> class Container, typename... Args>
void Read(std::istream& stream, Container<Args...>* target) {
  typedef typename Container<Args...>::value_type T;
  LOG_IF(FATAL, target->size())
      << "Reading into non-empty containers is not supported yet.";
  uint64_t n;
  Read(stream, &n);
  std::vector<T> vec;
  vec.reserve(n);
  while (n--) {
    vec.push_back(T());
    Read(stream, &vec.back());
  }
  *target = Container<Args...>(vec.begin(), vec.end());
}

template<typename K, typename T, class H, class P, class A>
void Read(std::istream& stream, std::unordered_map<K, T, H, P, A>* target) {
  LOG_IF(FATAL, target->size())
      << "Reading into non-empty containers is not supported yet.";
  uint64_t n;
  Read(stream, &n);
  std::vector<std::pair<K, T> > vec;
  vec.reserve(n);
  while (n--) {
    vec.push_back(std::make_pair(K(), T()));
    Read(stream, &vec.back());
  }
  *target = std::unordered_map<K, T, H, P, A>(vec.begin(), vec.end());
}

template<>
void Read(std::istream& stream, std::string* target) {
  uint64_t size;
  Read(stream, &size);
  uint64_t prev_size = target->size();
  target->resize(prev_size + size);
  stream.read(&(*target)[prev_size], size);
}

template<>
void Read(std::istream& stream, const std::string* target) {
  uint64_t size;
  Read(stream, &size);
  uint64_t prev_size = target->size();
  std::string* t = const_cast<std::string*>(target);
  t->resize(prev_size + size);
  stream.read(&(*t)[prev_size], size);
}

template<typename T>
void Write(const T& target, std::ostream& stream) {
  const T net_target = ToNetworkFormat(target);
  stream.write(reinterpret_cast<const char*>(&net_target), sizeof(T));
}

template<typename T1, typename T2>
void Write(const std::pair<T1, T2>& target, std::ostream& stream) {
  Write(target.first, stream);
  Write(target.second, stream);
}

template<template <typename...> class Container, typename... Args>
void Write(const Container<Args...>& target, std::ostream& stream) {
  const uint64_t n = target.size();
  Write(n, stream);
  for (const auto& e: target) {
    Write(e, stream);
  }
}

template<>
void Write(const std::string& target, std::ostream& stream) {
  uint64_t size = target.size();
  Write(size, stream);
  stream.write(&target[0], size);
}

}  // namespace io
}  // namespace pyt
#endif  // SRC_IO_SERIALIZE_H_

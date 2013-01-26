// Copyright 2013 Eugen Sawin <esawin@me73.com>
#ifndef SRC_IO_SERIALIZE_H_
#define SRC_IO_SERIALIZE_H_

#include <glog/logging.h>
#include <istream>
#include <ostream>
#include <vector>

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
void Read(std::istream& stream, T* target) {  // NOLINT
  stream.read(reinterpret_cast<char*>(target), sizeof(T));
  *target = FromNetworkFormat(*target);
}

template<bool isMap, template <typename T, typename...> class Container,
         typename T, typename... Args>
struct Reader {
  static void _Read(std::istream& stream,  // NOLINT
                    Container<T, Args...>* target) {
    LOG(FATAL) << "Not implemented.";
  }
};

template<template<typename T, typename...> class Container,
         typename T, typename... Args>
struct Reader<false, Container, T, Args...> {
  static void _Read(std::istream& stream,  // NOLINT
                    Container<T, Args...>* target) {
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
    *target = Container<T, Args...>(vec.begin(), vec.end());
  }
};

template<template<typename T, typename...> class Container,
         typename T, typename... Args>
struct Reader<true, Container, T, Args...> {
  static void _Read(std::istream& stream,  // NOLINT
                    Container<T, Args...>* target) {
    typedef typename Container<T, Args...>::key_type K;
    typedef typename Container<T, Args...>::mapped_type M;
    LOG_IF(FATAL, target->size())
        << "Reading into non-empty containers is not supported yet.";
    uint64_t n;
    Read(stream, &n);
    std::vector<std::pair<K, M> > vec;
    vec.reserve(n);
    while (n--) {
      vec.push_back(std::make_pair(K(), M()));
      Read(stream, &vec.back());
    }
    *target = Container<T, Args...>(vec.begin(), vec.end());
  }
};

template<typename T1, typename T2>
struct Equal {
  static const bool value = false;
};

template<typename T>
struct Equal<T, T> {
  static const bool value = true;
};

template<template<typename T, typename...> class Container,
         typename T, typename... Args>
void Read(std::istream& stream, Container<T, Args...>* target) {  // NOLINT
  return Reader<!Equal<T, typename Container<T, Args...>::value_type>::value,
                Container, T, Args...>::_Read(stream, target);
}

template<typename T1, typename T2>
void Read(std::istream& stream, std::pair<T1, T2>* target) {  // NOLINT
  Read(stream, &target->first);
  Read(stream, &target->second);
}

template<>
void Read(std::istream& stream, std::string* target) {  // NOLINT
  uint64_t size;
  Read(stream, &size);
  uint64_t prev_size = target->size();
  target->resize(prev_size + size);
  stream.read(&(*target)[prev_size], size);
}

template<typename T>
void Write(const T& target, std::ostream& stream) {  // NOLINT
  const T net_target = ToNetworkFormat(target);
  stream.write(reinterpret_cast<const char*>(&net_target), sizeof(T));
}

template<typename T1, typename T2>
void Write(const std::pair<T1, T2>& target, std::ostream& stream) {  // NOLINT
  Write(target.first, stream);
  Write(target.second, stream);
}

template<template <typename...> class Container, typename... Args>
void Write(const Container<Args...>& target, std::ostream& stream) {  // NOLINT
  const uint64_t n = target.size();
  Write(n, stream);
  for (const auto& e: target) {
    Write(e, stream);
  }
}

template<>
void Write(const std::string& target, std::ostream& stream) {  // NOLINT
  uint64_t size = target.size();
  Write(size, stream);
  stream.write(&target[0], size);
}

}  // namespace io
}  // namespace pyt
#endif  // SRC_IO_SERIALIZE_H_

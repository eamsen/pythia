// Copyright 2013 Eugen Sawin <esawin@me73.com>
#ifndef SRC_IO_SERIALIZE_H_
#define SRC_IO_SERIALIZE_H_

#include <glog/logging.h>
#include <istream>
#include <ostream>
#include <vector>

namespace pyt {
namespace io {

inline bool IsLittleEndian() {
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
  // LOG(ERROR) << "read T";
  stream.read(reinterpret_cast<char*>(target), sizeof(T));
  *target = FromNetworkFormat(*target);
}

inline void Read(std::istream& stream, std::string* target) {  // NOLINT
  uint64_t size;
  Read(stream, &size);
  // LOG(ERROR) << "read string " << size;
  if (size == 0) {
    return;
  }
  target->clear();
  target->resize(size);
  stream.read(&((*target)[0]), size);
}

template<template<typename T, typename...> class Container,
         typename T, typename... Args>
void Read(std::istream& stream, Container<T, Args...>* target);  // NOLINT

template<typename T1, typename T2>
void Read(std::istream& stream, std::pair<T1, T2>* target) {  // NOLINT
  // LOG(ERROR) << "read pair";
  Read(stream, &target->first);
  Read(stream, &target->second);
}

template<bool isMap, template<typename T, typename...> class Container,
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
    uint64_t n;
    Read(stream, &n);
    // LOG(ERROR) << "read container " << n;
    if (n == 0) {
      return;
    }
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
    uint64_t n;
    Read(stream, &n);
    // LOG(ERROR) << "read map " << n;
    if (n == 0) {
      return;
    }
    std::vector<std::pair<K, M> > vec;
    vec.reserve(n);
    while (n--) {
      vec.push_back({});
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

template<typename T>
void Write(const T& target, std::ostream& stream) {  // NOLINT
  // LOG(ERROR) << "write T";
  const T net_target = ToNetworkFormat(target);
  stream.write(reinterpret_cast<const char*>(&net_target), sizeof(T));
}

inline void Write(const std::string& target, std::ostream& stream) {  // NOLINT
  uint64_t size = target.size();
  // LOG(ERROR) << "write string " << size;
  Write(size, stream);
  stream.write(target.c_str(), size);
}

template<typename T1, typename T2>
void Write(const std::pair<T1, T2>& target, std::ostream& stream) {  // NOLINT
  // LOG(ERROR) << "write pair";
  Write(target.first, stream);
  Write(target.second, stream);
}

template<template <typename...> class Container, typename... Args>
void Write(const Container<Args...>& target, std::ostream& stream) {  // NOLINT
  const uint64_t n = target.size();
  Write(n, stream);
  // LOG(ERROR) << "write container " << n;
  for (const auto& e: target) {
    Write(e, stream);
  }
}

}  // namespace io
}  // namespace pyt
#endif  // SRC_IO_SERIALIZE_H_

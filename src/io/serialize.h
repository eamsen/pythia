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
  static T Read(std::istream& stream) {
    T value;
    stream.read(reinterpret_cast<char*>(&value), sizeof(T));
    return FromNetworkFormat(value);
  }
};

template<>
struct Reader<int> {
  static int Read(std::istream& stream) {
    int32_t value;
    stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return FromNetworkFormat(value);
  }
};

template<>
struct Reader<std::string> {
  static std::string Read(std::istream& stream) {
    uint64_t size = Reader<uint64_t>::Read(stream);
    char str[size + 1];
    str[size] = 0;
    stream.read(str, size);
    return str;
  }
};

template<typename T>
struct Reader<std::vector<T> > {
  static std::vector<T> Read(std::istream& stream) {
    uint64_t nelems = Reader<uint64_t>::Read(stream);
    std::vector<T> vec;
    vec.reserve(nelems);
    for (uint64_t i = 0; i < nelems; ++i) {
      vec.push_back(Reader<T>::Read(stream));
    }
    return vec;
  }
};

template<typename Key, typename Value>
struct Reader<std::unordered_map<Key, Value> > {
  static std::unordered_map<Key, Value> Read(std::istream& stream) {
    auto elems = Reader<std::vector<std::pair<Key, Value> > >::Read(stream);
    return std::unordered_map<Key, Value>(elems.begin(), elems.end());
  }
};

}  // namespace io
}  // namespace pyt
#endif  // SRC_IO_SERIALIZE_H_

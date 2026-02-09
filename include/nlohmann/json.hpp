#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <stdint.h>
#include <cstring>
#include <deque>
#include <forward_list>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <regex>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

// nlohmann/json v3.11.2
// 简化版本，仅包含基本功能

namespace nlohmann {

class json {
private:
    enum class value_t {
        null,
        boolean,
        number_integer,
        number_unsigned,
        number_float,
        object,
        array,
        string,
        binary,
        discarded
    };
    
    union value_union {
        bool boolean;
        int64_t number_integer;
        uint64_t number_unsigned;
        double number_float;
        std::string* string;
        std::vector<uint8_t>* binary;
        std::vector<json>* array;
        std::map<std::string, json>* object;
        
        value_union() : boolean(false) {}
        ~value_union() {}
    };
    
    value_t type;
    value_union value;
    
public:
    json() : type(value_t::null) {}
    
    json(bool b) : type(value_t::boolean) {
        value.boolean = b;
    }
    
    json(int64_t i) : type(value_t::number_integer) {
        value.number_integer = i;
    }
    
    json(uint64_t u) : type(value_t::number_unsigned) {
        value.number_unsigned = u;
    }
    
    json(double d) : type(value_t::number_float) {
        value.number_float = d;
    }
    
    json(const std::string& s) : type(value_t::string) {
        value.string = new std::string(s);
    }
    
    json(const char* s) : json(std::string(s)) {}
    
    json(std::initializer_list<json> init) : type(value_t::array) {
        value.array = new std::vector<json>(init);
    }
    
    json(std::initializer_list<std::pair<std::string, json>> init) : type(value_t::object) {
        value.object = new std::map<std::string, json>(init);
    }
    
    ~json() {
        destroy();
    }
    
    json(const json& other) : type(other.type) {
        copy(other);
    }
    
    json& operator=(const json& other) {
        if (this != &other) {
            destroy();
            type = other.type;
            copy(other);
        }
        return *this;
    }
    
    json(json&& other) noexcept : type(other.type), value(other.value) {
        other.type = value_t::null;
    }
    
    json& operator=(json&& other) noexcept {
        if (this != &other) {
            destroy();
            type = other.type;
            value = other.value;
            other.type = value_t::null;
        }
        return *this;
    }
    
    // 基本访问方法
    bool is_null() const { return type == value_t::null; }
    bool is_boolean() const { return type == value_t::boolean; }
    bool is_number() const { return type == value_t::number_integer || type == value_t::number_unsigned || type == value_t::number_float; }
    bool is_object() const { return type == value_t::object; }
    bool is_array() const { return type == value_t::array; }
    bool is_string() const { return type == value_t::string; }
    
    // 类型转换
    bool get<bool>() const {
        if (type == value_t::boolean) {
            return value.boolean;
        }
        throw std::runtime_error("type error");
    }
    
    int64_t get<int64_t>() const {
        if (type == value_t::number_integer) {
            return value.number_integer;
        } else if (type == value_t::number_unsigned) {
            return static_cast<int64_t>(value.number_unsigned);
        } else if (type == value_t::number_float) {
            return static_cast<int64_t>(value.number_float);
        }
        throw std::runtime_error("type error");
    }
    
    uint64_t get<uint64_t>() const {
        if (type == value_t::number_unsigned) {
            return value.number_unsigned;
        } else if (type == value_t::number_integer) {
            return static_cast<uint64_t>(value.number_integer);
        } else if (type == value_t::number_float) {
            return static_cast<uint64_t>(value.number_float);
        }
        throw std::runtime_error("type error");
    }
    
    double get<double>() const {
        if (type == value_t::number_float) {
            return value.number_float;
        } else if (type == value_t::number_integer) {
            return static_cast<double>(value.number_integer);
        } else if (type == value_t::number_unsigned) {
            return static_cast<double>(value.number_unsigned);
        }
        throw std::runtime_error("type error");
    }
    
    std::string get<std::string>() const {
        if (type == value_t::string) {
            return *value.string;
        }
        throw std::runtime_error("type error");
    }
    
    // 对象访问
    json& operator[](const std::string& key) {
        if (type != value_t::object) {
            destroy();
            type = value_t::object;
            value.object = new std::map<std::string, json>();
        }
        return (*value.object)[key];
    }
    
    const json& operator[](const std::string& key) const {
        if (type != value_t::object) {
            throw std::runtime_error("type error");
        }
        return (*value.object)[key];
    }
    
    // 数组访问
    json& operator[](size_t index) {
        if (type != value_t::array) {
            destroy();
            type = value_t::array;
            value.array = new std::vector<json>();
        }
        if (index >= value.array->size()) {
            value.array->resize(index + 1);
        }
        return (*value.array)[index];
    }
    
    const json& operator[](size_t index) const {
        if (type != value_t::array || index >= value.array->size()) {
            throw std::runtime_error("type error");
        }
        return (*value.array)[index];
    }
    
    // 检查键是否存在
    bool contains(const std::string& key) const {
        if (type != value_t::object) {
            return false;
        }
        return value.object->find(key) != value.object->end();
    }
    
    // 输入输出
    friend std::istream& operator>>(std::istream& is, json& j) {
        // 简化实现，仅支持基本JSON格式
        std::string s;
        is >> s;
        
        // 这里应该实现完整的JSON解析
        // 为了简化，我们仅支持简单的字符串解析
        j = json(s);
        return is;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const json& j) {
        switch (j.type) {
            case value_t::null:
                os << "null";
                break;
            case value_t::boolean:
                os << (j.value.boolean ? "true" : "false");
                break;
            case value_t::number_integer:
                os << j.value.number_integer;
                break;
            case value_t::number_unsigned:
                os << j.value.number_unsigned;
                break;
            case value_t::number_float:
                os << j.value.number_float;
                break;
            case value_t::string:
                os << '"' << *j.value.string << '"';
                break;
            case value_t::array:
                os << '[';
                for (size_t i = 0; i < j.value.array->size(); i++) {
                    if (i > 0) os << ", ";
                    os << (*j.value.array)[i];
                }
                os << ']';
                break;
            case value_t::object:
                os << '{';
                bool first = true;
                for (const auto& pair : *j.value.object) {
                    if (!first) os << ", ";
                    os << '"' << pair.first << '"' << ": " << pair.second;
                    first = false;
                }
                os << '}';
                break;
            default:
                os << "null";
                break;
        }
        return os;
    }
    
private:
    void destroy() {
        switch (type) {
            case value_t::string:
                delete value.string;
                break;
            case value_t::binary:
                delete value.binary;
                break;
            case value_t::array:
                delete value.array;
                break;
            case value_t::object:
                delete value.object;
                break;
            default:
                break;
        }
    }
    
    void copy(const json& other) {
        switch (other.type) {
            case value_t::boolean:
                value.boolean = other.value.boolean;
                break;
            case value_t::number_integer:
                value.number_integer = other.value.number_integer;
                break;
            case value_t::number_unsigned:
                value.number_unsigned = other.value.number_unsigned;
                break;
            case value_t::number_float:
                value.number_float = other.value.number_float;
                break;
            case value_t::string:
                value.string = new std::string(*other.value.string);
                break;
            case value_t::binary:
                value.binary = new std::vector<uint8_t>(*other.value.binary);
                break;
            case value_t::array:
                value.array = new std::vector<json>(*other.value.array);
                break;
            case value_t::object:
                value.object = new std::map<std::string, json>(*other.value.object);
                break;
            default:
                break;
        }
    }
};

} // namespace nlohmann
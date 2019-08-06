#pragma once

#if !defined(NAMESPACE_BEGIN)
#define NAMESPACE_BEGIN(name) namespace name {
#endif

#if !defined(NAMESPACE_END)
#define NAMESPACE_END(name) }
#endif

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

NAMESPACE_BEGIN(pd)

enum class JsonType : uint8_t {
    kNull = 0,
    kFalse = 1,
    kTrue = 2,
    kNumber = 3,
    kString = 4,
    kArray = 5,
    kObject = 6

};

struct JsonNode
{
    union {
        std::string str_;
        double num_;
        bool bool_value_;
        std::unordered_map<std::string, std::shared_ptr<JsonNode>> obj_;
        std::vector<std::shared_ptr<JsonNode>> arr_;
    };
    //pure virtual function
    virtual JsonType get_type() = 0;
    virtual void write(std::ostream &out, int indent = 0) = 0;

    virtual std::string &get_string() { throw std::runtime_error("It's not a string"); }
    virtual double &get_number() { throw std::runtime_error("It's not a number"); }
    virtual bool &get_boolean() { throw std::runtime_error("It's not a bool"); }
    virtual std::unordered_map<std::string, std::shared_ptr<JsonNode>> &get_object()
    {
        throw std::runtime_error("It's not an object");
    }
    virtual std::vector<std::shared_ptr<JsonNode>> &get_array()
    {
        throw std::runtime_error("It's not an array");
    }

    virtual ~JsonNode() = default;

protected:
    inline void write_to_file(const std::string &filename)
    {
        std::ofstream out(filename);
        if (!out.good())
            throw(std::runtime_error("Can not open" + filename));
        this->write(out, 0);
    }

    static void process_string(std::ostream &out, const std::string &origin)
    {
        std::stringstream oss;
        oss << '"';
        for (auto i : origin) {
            switch (i) {
            case '"': {
                oss << '\\' << '"';
                break;
            }
            case '\n': {
                oss << '\\' << 'n';
                break;
            }
            case '\\': {
                oss << '\\' << '\\';
                break;
            }
            default:
                oss << i;
                break;
            }
        }
        oss << '"';
        out << oss.str();
    }
    static void indent(std::ostream &out, int depth)
    {
        for (int i = 0; i < depth; i++)
            out << '\t';
    }
};

NAMESPACE_END(pd)

#include "pdjson.cc"

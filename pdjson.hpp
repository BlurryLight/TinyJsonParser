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
    kBool = 1,
    kNumber = 2,
    kString = 3,
    kArray = 4,
    kObject = 5

};

struct JsonNode
{
    JsonNode() { this->type_ = JsonType::kNull; }
    virtual JsonType get_type() /*final*/ { return type_; }

    virtual void write(std::ostream &out, int indent = 0) { out << "null"; }

    virtual std::string &get_string() { throw std::runtime_error("It's not a string"); }
    virtual double &get_double() { throw std::runtime_error("It's not a number"); }
    virtual bool &get_bool() { throw std::runtime_error("It's not a bool"); }
    virtual std::unordered_map<std::string, std::shared_ptr<JsonNode>> &get_object()
    {
        throw std::runtime_error("It's not an object");
    }
    virtual std::vector<std::shared_ptr<JsonNode>> &get_array()
    {
        throw std::runtime_error("It's not an array");
    }

    virtual ~JsonNode() = default;

private:
    //TODO:
    //it's impossible to store a std::string in an union
    // should employ boost::any or boost::variant to solve it.
    //    union {
    //        std::string str_;
    //        double num_;
    //        bool bool_value_;
    //        std::unordered_map<std::string, std::shared_ptr<JsonNode>> obj_;
    //        std::vector<std::shared_ptr<JsonNode>> arr_;
    //    };

protected:
    JsonType type_;

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

struct JsonString : public JsonNode
{
private:
    std::string str_;

public:
    JsonString() { this->type_ = JsonType::kString; }
    JsonString(const std::string &str)
        : str_(str)
    {
        this->type_ = JsonType::kString;
    }

    virtual JsonType get_type() final { return type_; }
    virtual void write(std::ostream &out, int indent = 0) final { process_string(out, str_); }
    virtual std::string &get_string() final { return str_; }
};

struct JsonDouble : public JsonNode
{
private:
    double value_;

public:
    JsonDouble() { this->type_ = JsonType::kNumber; }
    JsonDouble(double value)
        : value_(value)
    {
        this->type_ = JsonType::kNumber;
    }

    virtual JsonType get_type() final { return type_; }
    virtual void write(std::ostream &out, int indent = 0) final { out << value_; }
    virtual double &get_double() final { return value_; }
};

struct JsonBool : public JsonNode
{
private:
    bool value_;

public:
    JsonBool() { this->type_ = JsonType::kBool; }
    JsonBool(bool value)
        : value_(value)
    {
        this->type_ = JsonType::kBool;
    }

    virtual JsonType get_type() final { return type_; }
    virtual void write(std::ostream &out, int indent = 0) final
    {
        out << (value_ ? "true" : "false");
    }
    virtual bool &get_bool() final { return value_; }
};

struct JsonArray : public JsonNode
{
private:
    std::vector<std::shared_ptr<JsonNode>> vec_;

public:
    JsonArray() { this->type_ = JsonType::kArray; }

    virtual void write(std::ostream &out, int idt = 0) final
    {
        out << '[';
        if (vec_.empty()) {
            out << (']');
            return;
        }

        bool first_element = true;
        for (auto i = vec_.cbegin(); i != vec_.cend(); ++i) {
            if (first_element) {
                first_element = false;
            } else {
                out << ',';
            }
            out << '\n';
            indent(out, idt);
            (*i)->write(out, idt + 1);
        }
        out << '\n';
        indent(out, idt);
        out << ']';
    }
    virtual JsonType get_type() final { return type_; }
    virtual std::vector<std::shared_ptr<JsonNode>> &get_array() { return vec_; }
};

struct JsonObject : public JsonNode
{
private:
    std::unordered_map<std::string, std::shared_ptr<JsonNode>> obj_;

public:
    JsonObject() { this->type_ = JsonType::kObject; }

    virtual void write(std::ostream &out, int idt = 0) final
    {
        out << '{';
        if (obj_.empty()) {
            out << ('}');
            return;
        }

        bool first_element = true;
        for (auto it = obj_.cbegin(); it != obj_.cend(); ++it) {
            if (first_element) {
                first_element = false;
            } else {
                out << ',';
            }
            out << '\n';
            indent(out, idt);
            process_string(out, it->first);
            out << " : ";
            it->second->write(out, idt + 1);
        }

        out << '\n';
        indent(out, idt);
        out << '}';
    }
    virtual JsonType get_type() final { return type_; }
    virtual std::unordered_map<std::string, std::shared_ptr<JsonNode>> &get_object()
    {
        return obj_;
    }
};

inline std::shared_ptr<JsonNode> parse_json(std::istream &in)
{
    auto trim_whitespace = [&in]() -> char {
        char letter;
        in.get(letter);
        while (letter == ' ' || letter == '\t' || letter == '\n' || letter == '\r') {
            in.get(letter);
        }
        return letter;
    };

    auto parse_string = [&in]() -> std::string {
        char letter;
        in.get(letter);
        std::stringstream oss;
        while (letter != '"') //the end of string
        {
            if (letter == '\\') {
                char p = static_cast<char>(in.get());
                switch (p) {
                case '"':
                    oss << '"';
                    break;
                case 'n':
                    oss << '\n';
                    break;
                case '/':
                    oss << '\/';
                    break;
                case '\\':
                    oss << '\\';
                    break;
                case 'b':
                    oss << '\b';
                    break;
                case 'f':
                    oss << '\f';
                    break;
                case 'r':
                    oss << '\r';
                    break;
                case 't':
                    oss << '\t';
                    break;
                default:
                    throw std::runtime_error("When parsing string INVALID char" + p);
                }
            } else {
                oss << letter;
            }
            in.get(letter);
        }
        return oss.str();
    };

    auto parse_double = [&in]() -> double {
        //WARNING:
        //No error checking here
        // std::stringstream will handle some usual errors, for example, "1.x"  will be converted to "1.0" but the
        // stream will stay at char 'x' so it's still an erroe parsing.
        // I don't care who the f**k inputs invaild values
        std::stringstream ins;
        double res;
        in.unget();

        char p = static_cast<char>(in.get());
        while (
            (!in.fail()) //a nice pitfall (when in.get() failed,p will be set as zero so without it this is a deadloop))
            && (p == '-' || p == 'e' || p == 'E' || p == '.' || p == ',' || p == '+' || p == '-'
                || (p >= '0' && p <= '9'))) {
            ins << p;
            in.get(p);
        }
        // [...1.0,{...] when the loop ends, char p has ',' value, and the stream 'in' now points at
        // char '{' which is incorrect. The designation is parse_* functions will always ends at the
        // first char after the element parsed.
        // So the following line is needed.
        in.unget();
        ins >> res;
        return res;
    };

    auto parse_bool = [&in]() -> bool {
        in.unget();
        char p;
        in.get(p);
        if (p == 't') {
            if (in.get() == 'r' && in.get() == 'u' && in.get() == 'e') {
                return true;
            } else {
                throw std::runtime_error("Invalid input when parsing bool 'true'");
            }
        } else {
            {
                if (in.get() == 'a' && in.get() == 'l' && in.get() == 's' && in.get() == 'e') {
                    return false;
                } else {
                    throw std::runtime_error("Invalid input when parsing bool 'false'");
                }
            }
        }
    };

    char letter = trim_whitespace(); //first_not_white_letter
                                     //    if (letter == 0 || letter == EOF) {
    if (!in.good()) {
        return std::make_shared<JsonNode>();
    }

    if (letter == '"') //string begins
    {
        return std::make_shared<JsonString>(parse_string());
    }
    //double
    else if (letter == '-' || (letter >= '0' && letter <= '9')) {
        return std::make_shared<JsonDouble>(parse_double());
    }
    //bool
    else if (letter == 't' || letter == 'f') {
        return std::make_shared<JsonBool>(parse_bool());
    }
    //null
    else if (letter == 'n') {
        if (in.get() == 'u' && in.get() == 'l' && in.get() == 'l') {
            return std::make_shared<JsonNode>();
        } else {
            throw std::runtime_error("Invalid input when parsing 'null'");
        }
    }
    // array
    else if (letter == '[') {
        auto res = std::make_shared<JsonArray>();

        do {
            letter = trim_whitespace();
            if (letter == ',') {
                letter = trim_whitespace(); //move to next char which is not whitespace
            }
            in.unget();
            res->get_array().push_back(parse_json(in));

            letter = in.peek(); //get the current stream char without extracting it from the stream
        } while (letter != ']');
        //now in is at ']'
        //pass it mannually
        in.get();
        return std::move(res);
    }

    else if (letter == '{') {
        auto res = std::make_shared<JsonObject>();

        // key-value pair
        do {
            letter = trim_whitespace();
            if (letter == ',') {
                letter = trim_whitespace(); //move to next char which is not whitespace
            }
            if (letter == '"') {
                std::string key = parse_string();
                letter = trim_whitespace();
                if (letter != ':') {
                    throw std::runtime_error("When Parsing object an ':' missed");
                }
                res->get_object()[key] = parse_json(in);

            } else {
                break;
            }
        } while (letter != '}');
        return std::move(res);
    }

    else {
        throw std::runtime_error(std::string("Parser found unexpected character ") + letter);
    }
}

NAMESPACE_END(pd)

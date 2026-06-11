#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct Member {
    int         id;
    std::string name;
    std::string email;
    std::string phone;
    int         age;
};

inline void to_json(nlohmann::json& j, const Member& m)
{
    j = nlohmann::json{
        {"id",    m.id},
        {"name",  m.name},
        {"email", m.email},
        {"phone", m.phone},
        {"age",   m.age}
    };
}

inline void from_json(const nlohmann::json& j, Member& m)
{
    m.id    = j["id"].get<int>();
    m.name  = j["name"].get<std::string>();
    m.email = j["email"].get<std::string>();
    m.phone = j["phone"].get<std::string>();
    m.age   = j["age"].get<int>();
}

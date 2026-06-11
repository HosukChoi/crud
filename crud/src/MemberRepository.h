#pragma once
#include <vector>
#include <optional>
#include <string>
#include "Member.h"
#include "JsonCrud.h"

class MemberRepository {
public:
    explicit MemberRepository(const std::string& filepath);

    bool create(Member& m);
    std::vector<Member> findAll() const;
    std::optional<Member> findById(int id) const;
    bool update(const Member& m);
    bool remove(int id);

private:
    JsonCrud<Member> crud_;
    bool emailExists(const std::string& email, int excludeId = -1) const;
};

#include "MemberRepository.h"
#include <iostream>

MemberRepository::MemberRepository(const std::string& filepath)
{
    crud_.load(filepath, "members");
}

bool MemberRepository::create(Member& m)
{
    if (emailExists(m.email))
    {
        std::cerr << "[Repository] 이미 사용 중인 이메일: " << m.email << "\n";
        return false;
    }

    m.id = crud_.nextId();
    crud_.create(m);
    crud_.bumpNextId();
    return true;
}

std::vector<Member> MemberRepository::findAll() const
{
    return crud_.readAll();
}

std::optional<Member> MemberRepository::findById(int id) const
{
    auto matches = crud_.readByKey("id", id);
    if (matches.empty()) return std::nullopt;
    return matches.front();
}

bool MemberRepository::update(const Member& m)
{
    if (emailExists(m.email, m.id))
    {
        std::cerr << "[Repository] 이미 사용 중인 이메일: " << m.email << "\n";
        return false;
    }

    bool ok = true;
    ok &= crud_.update("id", m.id, "name",  m.name);
    ok &= crud_.update("id", m.id, "email", m.email);
    ok &= crud_.update("id", m.id, "phone", m.phone);
    ok &= crud_.update("id", m.id, "age",   m.age);
    return ok;
}

bool MemberRepository::remove(int id)
{
    return crud_.remove("id", id);
}

bool MemberRepository::emailExists(const std::string& email, int excludeId) const
{
    for (const auto& m : crud_.readAll())
        if (m.email == email && m.id != excludeId)
            return true;
    return false;
}

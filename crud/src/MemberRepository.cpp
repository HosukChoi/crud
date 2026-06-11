#include "MemberRepository.h"
#include <iostream>

using json = nlohmann::json;

// -------------------------------------------------------
// 생성자: JsonCrud 통해 파일 로드
// -------------------------------------------------------
MemberRepository::MemberRepository(const std::string& filepath)
{
    crud_.load(filepath);
}

// -------------------------------------------------------
// JSON ↔ Member 변환 헬퍼
// -------------------------------------------------------
Member MemberRepository::fromJson(const json& j)
{
    Member m;
    m.id    = j["id"].get<int>();
    m.name  = j["name"].get<std::string>();
    m.email = j["email"].get<std::string>();
    m.phone = j["phone"].get<std::string>();
    m.age   = j["age"].get<int>();
    return m;
}

json MemberRepository::toJson(const Member& m)
{
    json j;
    j["id"]    = m.id;
    j["name"]  = m.name;
    j["email"] = m.email;
    j["phone"] = m.phone;
    j["age"]   = m.age;
    return j;
}

// -------------------------------------------------------
// CRUD
// -------------------------------------------------------
bool MemberRepository::create(Member& m)
{
    if (emailExists(m.email))
    {
        std::cerr << "[Repository] 이미 사용 중인 이메일: " << m.email << "\n";
        return false;
    }

    m.id = crud_.nextId();
    crud_.create(toJson(m));
    crud_.bumpNextId();
    return true;
}

std::vector<Member> MemberRepository::findAll() const
{
    std::vector<Member> result;
    for (const auto& j : crud_.readAll())
        result.push_back(fromJson(j));
    return result;
}

std::optional<Member> MemberRepository::findById(int id) const
{
    auto matches = crud_.readByKey("id", id);
    if (matches.empty()) return std::nullopt;
    return fromJson(matches.front());
}

bool MemberRepository::update(const Member& m)
{
    if (emailExists(m.email, m.id))
    {
        std::cerr << "[Repository] 이미 사용 중인 이메일: " << m.email << "\n";
        return false;
    }

    // 변경된 각 필드를 개별 update 호출로 반영
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

// -------------------------------------------------------
// 이메일 중복 확인 (excludeId: 수정 시 자기 자신 제외)
// -------------------------------------------------------
bool MemberRepository::emailExists(const std::string& email, int excludeId) const
{
    for (const auto& j : crud_.readAll())
    {
        if (j["email"].get<std::string>() == email &&
            j["id"].get<int>() != excludeId)
            return true;
    }
    return false;
}

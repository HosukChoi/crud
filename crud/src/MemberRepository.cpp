#include "MemberRepository.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// -------------------------------------------------------
// 생성자: 파일 경로 저장 후 즉시 로드
// -------------------------------------------------------
MemberRepository::MemberRepository(const std::string& filepath)
    : filepath_(filepath), nextId_(1)
{
    load();
}

// -------------------------------------------------------
// 파일 로드 (JSON → 내부 벡터)
// -------------------------------------------------------
void MemberRepository::load()
{
    std::ifstream ifs(filepath_);
    if (!ifs.is_open())
    {
        // 파일 없으면 빈 상태로 초기화
        nextId_ = 1;
        members_.clear();
        return;
    }

    json j;
    try
    {
        ifs >> j;
    }
    catch (const json::parse_error& e)
    {
        std::cerr << "[Repository] JSON 파싱 오류: " << e.what() << "\n";
        return;
    }

    nextId_ = j.value("next_id", 1);
    members_.clear();

    for (const auto& item : j["members"])
    {
        Member m;
        m.id    = item["id"].get<int>();
        m.name  = item["name"].get<std::string>();
        m.email = item["email"].get<std::string>();
        m.phone = item["phone"].get<std::string>();
        m.age   = item["age"].get<int>();
        members_.push_back(m);
    }
}

// -------------------------------------------------------
// 파일 저장 (내부 벡터 → JSON)
// -------------------------------------------------------
void MemberRepository::save() const
{
    json j;
    j["next_id"] = nextId_;
    j["members"] = json::array();

    for (const auto& m : members_)
    {
        json item;
        item["id"]    = m.id;
        item["name"]  = m.name;
        item["email"] = m.email;
        item["phone"] = m.phone;
        item["age"]   = m.age;
        j["members"].push_back(item);
    }

    std::ofstream ofs(filepath_);
    if (!ofs.is_open())
    {
        std::cerr << "[Repository] 파일 저장 실패: " << filepath_ << "\n";
        return;
    }
    ofs << j.dump(2) << "\n";
}

// -------------------------------------------------------
// CRUD 구현
// -------------------------------------------------------
bool MemberRepository::create(Member& m)
{
    if (emailExists(m.email))
    {
        std::cerr << "[Repository] 이미 사용 중인 이메일입니다: " << m.email << "\n";
        return false;
    }

    m.id = nextId_++;
    members_.push_back(m);
    save();
    return true;
}

std::vector<Member> MemberRepository::findAll() const
{
    return members_;
}

std::optional<Member> MemberRepository::findById(int id) const
{
    for (const auto& m : members_)
        if (m.id == id)
            return m;
    return std::nullopt;
}

bool MemberRepository::update(const Member& m)
{
    if (emailExists(m.email, m.id))
    {
        std::cerr << "[Repository] 이미 사용 중인 이메일입니다: " << m.email << "\n";
        return false;
    }

    for (auto& existing : members_)
    {
        if (existing.id == m.id)
        {
            existing = m;
            save();
            return true;
        }
    }
    return false;
}

bool MemberRepository::remove(int id)
{
    auto it = std::find_if(members_.begin(), members_.end(),
                           [id](const Member& m) { return m.id == id; });
    if (it == members_.end())
        return false;

    members_.erase(it);
    save();
    return true;
}

// -------------------------------------------------------
// 이메일 중복 확인 (excludeId: 수정 시 자기 자신 제외)
// -------------------------------------------------------
bool MemberRepository::emailExists(const std::string& email, int excludeId) const
{
    for (const auto& m : members_)
        if (m.email == email && m.id != excludeId)
            return true;
    return false;
}

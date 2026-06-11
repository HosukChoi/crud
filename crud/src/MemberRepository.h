#pragma once
#include <vector>
#include <optional>
#include <string>
#include "Member.h"
#include "JsonCrud.h"

class MemberRepository {
public:
    explicit MemberRepository(const std::string& filepath);

    // Create: 새 회원 등록 (성공 시 m.id가 채워짐)
    bool create(Member& m);

    // Read: 전체 목록 / ID 조회
    std::vector<Member> findAll() const;
    std::optional<Member> findById(int id) const;

    // Update: 기존 회원 정보 수정
    bool update(const Member& m);

    // Delete: ID로 삭제
    bool remove(int id);

private:
    JsonCrud crud_;

    static Member            fromJson(const nlohmann::json& j);
    static nlohmann::json    toJson(const Member& m);
    bool emailExists(const std::string& email, int excludeId = -1) const;
};

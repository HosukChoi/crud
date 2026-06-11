#pragma once
#include <vector>
#include <optional>
#include <string>
#include <memory>
#include "Member.h"
#include "JsonCrud.h"

class MemberRepository {
public:
    // 프로덕션용: JsonCrud<Member> 를 내부에서 생성
    explicit MemberRepository(const std::string& filepath);

    // 테스트용: ICrud<Member> 를 외부에서 주입 (mock 등)
    explicit MemberRepository(ICrud<Member>& crud);

    bool create(Member& m);
    std::vector<Member> findAll() const;
    std::optional<Member> findById(int id) const;
    bool update(const Member& m);
    bool remove(int id);

private:
    std::unique_ptr<JsonCrud<Member>> owned_;  // 내부 생성 시 소유권 보유
    ICrud<Member>&                    crud_;
    bool emailExists(const std::string& email, int excludeId = -1) const;
};

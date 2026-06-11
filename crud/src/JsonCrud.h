#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// JSON 파일 기반 범용 CRUD 클래스.
// 최상위 구조: { "next_id": N, "members": [ {...}, ... ] }
class JsonCrud {
public:
    // 파일 로드 (없으면 빈 상태로 초기화)
    void load(const std::string& filepath);

    // Create: 배열에 item 추가 후 파일 저장
    void create(const nlohmann::json& item);

    // Read: 전체 반환
    std::vector<nlohmann::json> readAll() const;

    // Read: 특정 키 == 값인 항목들 반환 (없으면 빈 벡터)
    std::vector<nlohmann::json> readByKey(const std::string& key,
                                          const nlohmann::json& value) const;

    // Update: keyField == keyValue 인 항목의 targetField 를 newValue 로 수정
    // 성공 시 true, 해당 항목 없으면 false
    bool update(const std::string& keyField, const nlohmann::json& keyValue,
                const std::string& targetField, const nlohmann::json& newValue);

    // Delete: keyField == keyValue 인 항목 삭제
    // 성공 시 true, 해당 항목 없으면 false
    bool remove(const std::string& keyField, const nlohmann::json& keyValue);

    // next_id 관리
    int  nextId() const;
    void bumpNextId();

private:
    void save() const;

    std::string        filepath_;
    nlohmann::json     root_;     // { "next_id": N, "members": [...] }
};

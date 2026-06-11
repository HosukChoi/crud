#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "ICrud.h"

// JSON 파일 기반 범용 CRUD 템플릿 클래스.
// T는 nlohmann ADL 직렬화 함수를 제공해야 합니다:
//   void to_json(nlohmann::json&, const T&)
//   void from_json(const nlohmann::json&, T&)
//
// 파일 구조: { "next_id": N, "<arrayKey>": [ {...}, ... ] }
template<typename T>
class JsonCrud : public ICrud<T> {
public:
    // 파일 로드. arrayKey 는 JSON 내 배열 필드 이름 (예: "members", "products")
    void load(const std::string& filepath, const std::string& arrayKey) override
    {
        filepath_ = filepath;
        arrayKey_ = arrayKey;

        std::ifstream ifs(filepath_);
        if (!ifs.is_open())
        {
            root_["next_id"] = 1;
            root_[arrayKey_] = nlohmann::json::array();
            return;
        }

        try { ifs >> root_; }
        catch (const nlohmann::json::parse_error& e)
        {
            std::cerr << "[JsonCrud] 파싱 오류: " << e.what() << "\n";
            root_["next_id"] = 1;
            root_[arrayKey_] = nlohmann::json::array();
        }

        if (!root_.contains(arrayKey_) || !root_[arrayKey_].is_array())
            root_[arrayKey_] = nlohmann::json::array();
        if (!root_.contains("next_id"))
            root_["next_id"] = 1;
    }

    // Create: T → json 변환(to_json ADL) 후 배열에 추가
    void create(const T& item) override
    {
        nlohmann::json j = item;
        root_[arrayKey_].push_back(j);
        save();
    }

    // Read: 전체 배열을 T 벡터로 반환 (from_json ADL)
    std::vector<T> readAll() const override
    {
        std::vector<T> result;
        for (const auto& j : root_[arrayKey_])
            result.push_back(j.template get<T>());
        return result;
    }

    // Read: key == value 인 항목들을 T 벡터로 반환
    std::vector<T> readByKey(const std::string& key,
                              const nlohmann::json& value) const override
    {
        std::vector<T> result;
        for (const auto& j : root_[arrayKey_])
            if (j.contains(key) && j[key] == value)
                result.push_back(j.template get<T>());
        return result;
    }

    // Update: keyField == keyValue 인 항목의 targetField 를 newValue 로 수정
    bool update(const std::string& keyField, const nlohmann::json& keyValue,
                const std::string& targetField, const nlohmann::json& newValue) override
    {
        bool found = false;
        for (auto& j : root_[arrayKey_])
        {
            if (j.contains(keyField) && j[keyField] == keyValue)
            {
                j[targetField] = newValue;
                found = true;
            }
        }
        if (found) save();
        return found;
    }

    // Delete: keyField == keyValue 인 항목 삭제
    bool remove(const std::string& keyField, const nlohmann::json& keyValue) override
    {
        auto& arr = root_[arrayKey_];
        const auto before = arr.size();
        arr.erase(
            std::remove_if(arr.begin(), arr.end(),
                [&](const nlohmann::json& j) {
                    return j.contains(keyField) && j[keyField] == keyValue;
                }),
            arr.end());
        if (arr.size() == before) return false;
        save();
        return true;
    }

    // next_id 관리
    int  nextId()    const override { return root_.value("next_id", 1); }
    void bumpNextId()      override { root_["next_id"] = nextId() + 1; save(); }

private:
    void save() const
    {
        std::ofstream ofs(filepath_);
        if (!ofs.is_open())
        {
            std::cerr << "[JsonCrud] 파일 저장 실패: " << filepath_ << "\n";
            return;
        }
        ofs << root_.dump(2) << "\n";
    }

    std::string     filepath_;
    std::string     arrayKey_;
    nlohmann::json  root_;
};

#include "JsonCrud.h"

#include <fstream>
#include <iostream>
#include <algorithm>

using json = nlohmann::json;

// -------------------------------------------------------
// load: JSON 파일 → root_
// -------------------------------------------------------
void JsonCrud::load(const std::string& filepath)
{
    filepath_ = filepath;

    std::ifstream ifs(filepath_);
    if (!ifs.is_open())
    {
        root_["next_id"] = 1;
        root_["members"] = json::array();
        return;
    }

    try
    {
        ifs >> root_;
    }
    catch (const json::parse_error& e)
    {
        std::cerr << "[JsonCrud] 파싱 오류: " << e.what() << "\n";
        root_["next_id"] = 1;
        root_["members"] = json::array();
    }

    if (!root_.contains("members") || !root_["members"].is_array())
        root_["members"] = json::array();

    if (!root_.contains("next_id"))
        root_["next_id"] = 1;
}

// -------------------------------------------------------
// save: root_ → JSON 파일
// -------------------------------------------------------
void JsonCrud::save() const
{
    std::ofstream ofs(filepath_);
    if (!ofs.is_open())
    {
        std::cerr << "[JsonCrud] 파일 저장 실패: " << filepath_ << "\n";
        return;
    }
    ofs << root_.dump(2) << "\n";
}

// -------------------------------------------------------
// create
// -------------------------------------------------------
void JsonCrud::create(const json& item)
{
    root_["members"].push_back(item);
    save();
}

// -------------------------------------------------------
// readAll
// -------------------------------------------------------
std::vector<json> JsonCrud::readAll() const
{
    std::vector<json> result;
    for (const auto& item : root_["members"])
        result.push_back(item);
    return result;
}

// -------------------------------------------------------
// readByKey
// -------------------------------------------------------
std::vector<json> JsonCrud::readByKey(const std::string& key,
                                       const json& value) const
{
    std::vector<json> result;
    for (const auto& item : root_["members"])
    {
        if (item.contains(key) && item[key] == value)
            result.push_back(item);
    }
    return result;
}

// -------------------------------------------------------
// update: keyField == keyValue 인 항목의 targetField 수정
// -------------------------------------------------------
bool JsonCrud::update(const std::string& keyField, const json& keyValue,
                      const std::string& targetField, const json& newValue)
{
    bool found = false;
    for (auto& item : root_["members"])
    {
        if (item.contains(keyField) && item[keyField] == keyValue)
        {
            item[targetField] = newValue;
            found = true;
        }
    }
    if (found) save();
    return found;
}

// -------------------------------------------------------
// remove: keyField == keyValue 인 항목 삭제
// -------------------------------------------------------
bool JsonCrud::remove(const std::string& keyField, const json& keyValue)
{
    auto& arr = root_["members"];
    const auto before = arr.size();

    arr.erase(
        std::remove_if(arr.begin(), arr.end(),
                       [&](const json& item) {
                           return item.contains(keyField) && item[keyField] == keyValue;
                       }),
        arr.end());

    if (arr.size() == before) return false;
    save();
    return true;
}

// -------------------------------------------------------
// next_id 관리
// -------------------------------------------------------
int JsonCrud::nextId() const
{
    return root_.value("next_id", 1);
}

void JsonCrud::bumpNextId()
{
    root_["next_id"] = nextId() + 1;
    save();
}

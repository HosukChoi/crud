#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// JsonCrud<T> 의 추상 인터페이스.
// 테스트에서 MockICrud<T> 를 생성하기 위해 사용합니다.
template<typename T>
class ICrud {
public:
    virtual ~ICrud() = default;

    virtual void load(const std::string& filepath, const std::string& arrayKey) = 0;

    virtual void           create(const T& item)                                        = 0;
    virtual std::vector<T> readAll()                                              const = 0;
    virtual std::vector<T> readByKey(const std::string& key,
                                     const nlohmann::json& value)                 const = 0;
    virtual bool           update(const std::string& keyField,
                                  const nlohmann::json& keyValue,
                                  const std::string&   targetField,
                                  const nlohmann::json& newValue)                       = 0;
    virtual bool           remove(const std::string& keyField,
                                  const nlohmann::json& keyValue)                       = 0;
    virtual int            nextId()                                               const = 0;
    virtual void           bumpNextId()                                                 = 0;
};

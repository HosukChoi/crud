# CRUD - JSON 기반 회원 관리 애플리케이션

## 프로젝트 개요

JSON 파일을 데이터 저장소로 사용하는 C++ 콘솔 기반 회원 정보 CRUD 애플리케이션.

## 기술 스택

- **언어**: C++20
- **빌드**: Visual Studio 2022 (v145 toolset), MSBuild
- **JSON 라이브러리**: [nlohmann/json](https://github.com/nlohmann/json) (단일 헤더)
- **데이터 저장소**: `data/members.json` (로컬 파일)
- **참고 저장소**: [HosukChoi/json_library_poc](https://github.com/HosukChoi/json_library_poc)

## 프로젝트 구조

```
crud/
├── CLAUDE.md
├── crud.slnx
└── crud/
    ├── crud.vcxproj
    ├── include/
    │   └── nlohmann/
    │       └── json.hpp          ← 별도 다운로드 필요
    ├── src/
    │   ├── main.cpp              # 콘솔 메뉴 UI
    │   ├── Member.h              # Member 데이터 구조체
    │   ├── MemberRepository.h    # Repository 인터페이스
    │   └── MemberRepository.cpp  # JSON 파일 I/O 및 CRUD 구현
    └── data/
        └── members.json          # 데이터 파일
```

## 초기 설정

### json.hpp 다운로드 (필수)

`json.hpp`는 저장소에 포함되지 않으므로 직접 다운로드해야 합니다.

```powershell
# crud/crud/include/nlohmann/ 디렉토리 생성 후 다운로드
New-Item -ItemType Directory -Force "crud\include\nlohmann"
Invoke-WebRequest `
  -Uri "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp" `
  -OutFile "crud\include\nlohmann\json.hpp"
```

## 빌드 및 실행

Visual Studio에서 `crud.slnx`를 열고 `F5` (또는 Ctrl+F5) 로 빌드 및 실행.

```powershell
# MSBuild 직접 사용 시
msbuild crud.slnx /p:Configuration=Debug /p:Platform=x64
.\crud\x64\Debug\crud.exe
```

> **중요**: 실행 시 작업 디렉토리는 `crud/crud/` 여야 합니다 (`data/members.json` 경로 기준).
> Visual Studio Debug 설정에서 Working Directory가 `$(ProjectDir)`로 지정되어 있습니다.

## 데이터 모델

| 필드 | 타입 | 설명 |
|------|------|------|
| `id` | int | 자동 증가 고유 식별자 (삭제 후 재사용 없음) |
| `name` | string | 이름 |
| `email` | string | 이메일 주소 (중복 불허) |
| `phone` | string | 전화번호 |
| `age` | int | 나이 |

### members.json 구조

```json
{
  "next_id": 4,
  "members": [
    { "id": 1, "name": "홍길동", "email": "hong@example.com", "phone": "010-1234-5678", "age": 30 }
  ]
}
```

## CRUD 기능

| 메뉴 | 기능 | 설명 |
|------|------|------|
| 1 | **Create** | 새 회원 등록 (이메일 중복 검증) |
| 2 | **Read All** | 전체 회원 목록 조회 |
| 3 | **Read One** | ID로 단일 회원 조회 |
| 4 | **Update** | 회원 정보 수정 (필드별 선택 수정) |
| 5 | **Delete** | 회원 삭제 (확인 프롬프트) |
| 0 | **Exit** | 종료 |

## JSON 라이브러리 사용 패턴

참조 저장소(json_library_poc)와 동일한 방식을 사용합니다.

```cpp
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// 파일 로드
std::ifstream ifs("data/members.json");
json j;
ifs >> j;

// 값 읽기
int id        = item["id"].get<int>();
std::string name = item["name"].get<std::string>();
int timeout   = j.value("timeout_ms", 0);  // 기본값 지정

// JSON 빌드 및 저장
json output;
output["next_id"] = 5;
output["members"] = json::array();
std::ofstream ofs("data/members.json");
ofs << output.dump(2) << "\n";
```

## 아키텍처

```
main.cpp  ──→  MemberRepository  ──→  JsonCrud<Member>  ──→  data/members.json
              (도메인 검증)            (JSON CRUD)
                     ↑                       ↑
                 Member.h            to_json / from_json
              (구조체 + ADL)
```

- **Member**: 데이터 구조체 + `to_json`/`from_json` ADL 직렬화 함수
- **JsonCrud\<T\>**: 범용 JSON CRUD 템플릿 클래스 (파일 I/O, 배열 관리)
- **MemberRepository**: 도메인 검증 (이메일 중복 등) + JsonCrud 위임
- **main.cpp**: 콘솔 메뉴 UI, 사용자 입력 처리

---

## JsonCrud\<T\> 사용 가이드

`src/JsonCrud.h` 하나만 포함하면 어떤 타입에도 적용 가능한 범용 JSON CRUD 클래스입니다.

### 인터페이스 요약

```cpp
template<typename T>
class JsonCrud {
    void load(const std::string& filepath, const std::string& arrayKey);

    void           create(const T& item);
    std::vector<T> readAll() const;
    std::vector<T> readByKey(const std::string& key, const nlohmann::json& value) const;
    bool           update(const std::string& keyField, const nlohmann::json& keyValue,
                          const std::string& targetField, const nlohmann::json& newValue);
    bool           remove(const std::string& keyField, const nlohmann::json& keyValue);

    int  nextId() const;
    void bumpNextId();
};
```

### 데이터 파일 구조

```json
{
  "next_id": 4,
  "<arrayKey>": [
    { "id": 1, "field1": "...", "field2": "..." }
  ]
}
```

### 새 타입 추가 체크리스트

새로운 도메인 타입(예: `Product`)을 JSON CRUD로 관리하려면 아래 순서로 작업합니다.

**1. 구조체 + ADL 직렬화 함수 정의** (`Product.h`)

```cpp
#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct Product {
    int         id;
    std::string name;
    int         price;
};

inline void to_json(nlohmann::json& j, const Product& p)
{
    j = nlohmann::json{{"id", p.id}, {"name", p.name}, {"price", p.price}};
}

inline void from_json(const nlohmann::json& j, Product& p)
{
    p.id    = j["id"].get<int>();
    p.name  = j["name"].get<std::string>();
    p.price = j["price"].get<int>();
}
```

**2. Repository 클래스 작성** (`ProductRepository.h / .cpp`)

```cpp
// ProductRepository.h
#include "Product.h"
#include "JsonCrud.h"

class ProductRepository {
public:
    explicit ProductRepository(const std::string& filepath);
    bool                  create(Product& p);
    std::vector<Product>  findAll() const;
    std::optional<Product> findById(int id) const;
    bool                  update(const Product& p);
    bool                  remove(int id);
private:
    JsonCrud<Product> crud_;
};

// ProductRepository.cpp
ProductRepository::ProductRepository(const std::string& filepath)
{
    crud_.load(filepath, "products");  // arrayKey를 타입에 맞게 지정
}
```

**3. vcxproj에 파일 등록**

```xml
<ClCompile Include="src\ProductRepository.cpp" />
<ClInclude Include="src\Product.h" />
<ClInclude Include="src\ProductRepository.h" />
```

### 기존 구현 참조

`Member` 타입이 완성된 구현 예시입니다. 새 타입 작업 시 참고하세요.

| 파일 | 역할 |
|------|------|
| `src/Member.h` | 구조체 + to_json/from_json 패턴 |
| `src/MemberRepository.h/.cpp` | Repository 패턴 (도메인 검증 포함) |
| `data/members.json` | 데이터 파일 구조 |

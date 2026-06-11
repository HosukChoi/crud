#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "JsonCrud.h"
#include "Member.h"

namespace fs = std::filesystem;

// 테스트용 임시 JSON 파일을 관리하는 Fixture
class JsonCrudTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        tempFile_ = "test_members_crud.json";
    }

    void TearDown() override
    {
        fs::remove(tempFile_);
    }

    // 파일에 초기 데이터를 직접 기록
    void writeFile(const std::string& content)
    {
        std::ofstream ofs(tempFile_);
        ofs << content;
    }

    std::string    tempFile_;
    JsonCrud<Member> crud_;
};

// -------------------------------------------------------
// load
// -------------------------------------------------------
TEST_F(JsonCrudTest, Load_InitializesEmpty_WhenFileNotFound)
{
    crud_.load("nonexistent_file.json", "members");

    EXPECT_TRUE(crud_.readAll().empty());
    EXPECT_EQ(crud_.nextId(), 1);
}

TEST_F(JsonCrudTest, Load_ReadsExistingData)
{
    writeFile(R"({
        "next_id": 3,
        "members": [
            {"id":1,"name":"홍길동","email":"hong@test.com","phone":"010-0000-0001","age":30},
            {"id":2,"name":"김철수","email":"kim@test.com", "phone":"010-0000-0002","age":25}
        ]
    })");

    crud_.load(tempFile_, "members");

    auto all = crud_.readAll();
    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0].name, "홍길동");
    EXPECT_EQ(all[1].name, "김철수");
    EXPECT_EQ(crud_.nextId(), 3);
}

TEST_F(JsonCrudTest, Load_InitializesEmpty_WhenJsonMalformed)
{
    writeFile("{ this is not valid json }");

    crud_.load(tempFile_, "members");

    EXPECT_TRUE(crud_.readAll().empty());
    EXPECT_EQ(crud_.nextId(), 1);
}

// -------------------------------------------------------
// create
// -------------------------------------------------------
TEST_F(JsonCrudTest, Create_AddsItem)
{
    crud_.load(tempFile_, "members");

    Member m{1, "홍길동", "hong@test.com", "010-0000-0001", 30};
    crud_.create(m);

    auto all = crud_.readAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].name,  "홍길동");
    EXPECT_EQ(all[0].email, "hong@test.com");
}

TEST_F(JsonCrudTest, Create_PersistsToFile)
{
    crud_.load(tempFile_, "members");
    Member m{1, "홍길동", "hong@test.com", "010-0000-0001", 30};
    crud_.create(m);

    // 새 인스턴스로 다시 로드해서 파일에 저장됐는지 확인
    JsonCrud<Member> crud2;
    crud2.load(tempFile_, "members");

    ASSERT_EQ(crud2.readAll().size(), 1u);
    EXPECT_EQ(crud2.readAll()[0].email, "hong@test.com");
}

// -------------------------------------------------------
// readAll
// -------------------------------------------------------
TEST_F(JsonCrudTest, ReadAll_ReturnsEmpty_WhenNoItems)
{
    crud_.load(tempFile_, "members");
    EXPECT_TRUE(crud_.readAll().empty());
}

TEST_F(JsonCrudTest, ReadAll_ReturnsAllItems)
{
    crud_.load(tempFile_, "members");
    crud_.create({1, "A", "a@test.com", "010-0001", 20});
    crud_.create({2, "B", "b@test.com", "010-0002", 25});
    crud_.create({3, "C", "c@test.com", "010-0003", 30});

    EXPECT_EQ(crud_.readAll().size(), 3u);
}

// -------------------------------------------------------
// readByKey
// -------------------------------------------------------
TEST_F(JsonCrudTest, ReadByKey_ReturnsMatchingItem)
{
    crud_.load(tempFile_, "members");
    crud_.create({1, "홍길동", "hong@test.com", "010-0000-0001", 30});
    crud_.create({2, "김철수", "kim@test.com",  "010-0000-0002", 25});

    auto result = crud_.readByKey("id", 2);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].name, "김철수");
}

TEST_F(JsonCrudTest, ReadByKey_ReturnsEmpty_WhenNoMatch)
{
    crud_.load(tempFile_, "members");
    crud_.create({1, "홍길동", "hong@test.com", "010-0000-0001", 30});

    auto result = crud_.readByKey("id", 999);
    EXPECT_TRUE(result.empty());
}

TEST_F(JsonCrudTest, ReadByKey_CanSearchByStringField)
{
    crud_.load(tempFile_, "members");
    crud_.create({1, "홍길동", "hong@test.com", "010-0000-0001", 30});

    auto result = crud_.readByKey("email", std::string("hong@test.com"));
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].id, 1);
}

// -------------------------------------------------------
// update
// -------------------------------------------------------
TEST_F(JsonCrudTest, Update_ModifiesTargetField)
{
    crud_.load(tempFile_, "members");
    crud_.create({1, "홍길동", "hong@test.com", "010-0000-0001", 30});

    bool ok = crud_.update("id", 1, "name", std::string("홍길순"));
    EXPECT_TRUE(ok);

    auto result = crud_.readByKey("id", 1);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].name, "홍길순");
}

TEST_F(JsonCrudTest, Update_ReturnsFalse_WhenKeyNotFound)
{
    crud_.load(tempFile_, "members");

    bool ok = crud_.update("id", 999, "name", std::string("없음"));
    EXPECT_FALSE(ok);
}

TEST_F(JsonCrudTest, Update_PersistsToFile)
{
    crud_.load(tempFile_, "members");
    crud_.create({1, "홍길동", "hong@test.com", "010-0000-0001", 30});
    crud_.update("id", 1, "age", 99);

    JsonCrud<Member> crud2;
    crud2.load(tempFile_, "members");
    EXPECT_EQ(crud2.readByKey("id", 1)[0].age, 99);
}

// -------------------------------------------------------
// remove
// -------------------------------------------------------
TEST_F(JsonCrudTest, Remove_DeletesItem)
{
    crud_.load(tempFile_, "members");
    crud_.create({1, "홍길동", "hong@test.com", "010-0000-0001", 30});
    crud_.create({2, "김철수", "kim@test.com",  "010-0000-0002", 25});

    bool ok = crud_.remove("id", 1);
    EXPECT_TRUE(ok);

    auto all = crud_.readAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].id, 2);
}

TEST_F(JsonCrudTest, Remove_ReturnsFalse_WhenKeyNotFound)
{
    crud_.load(tempFile_, "members");
    bool ok = crud_.remove("id", 999);
    EXPECT_FALSE(ok);
}

TEST_F(JsonCrudTest, Remove_PersistsToFile)
{
    crud_.load(tempFile_, "members");
    crud_.create({1, "홍길동", "hong@test.com", "010-0000-0001", 30});
    crud_.remove("id", 1);

    JsonCrud<Member> crud2;
    crud2.load(tempFile_, "members");
    EXPECT_TRUE(crud2.readAll().empty());
}

// -------------------------------------------------------
// nextId / bumpNextId
// -------------------------------------------------------
TEST_F(JsonCrudTest, NextId_DefaultsToOne)
{
    crud_.load(tempFile_, "members");
    EXPECT_EQ(crud_.nextId(), 1);
}

TEST_F(JsonCrudTest, BumpNextId_Increments)
{
    crud_.load(tempFile_, "members");
    crud_.bumpNextId();
    EXPECT_EQ(crud_.nextId(), 2);
    crud_.bumpNextId();
    EXPECT_EQ(crud_.nextId(), 3);
}

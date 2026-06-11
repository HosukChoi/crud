#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "MemberRepository.h"
#include "ICrud.h"

using ::testing::Return;
using ::testing::_;

// -------------------------------------------------------
// MockICrud: ICrud<Member> 의 gmock 구현
// -------------------------------------------------------
class MockICrud : public ICrud<Member> {
public:
    MOCK_METHOD(void, load,
        (const std::string& filepath, const std::string& arrayKey), (override));
    MOCK_METHOD(void, create,
        (const Member& item), (override));
    MOCK_METHOD(std::vector<Member>, readAll,
        (), (const, override));
    MOCK_METHOD(std::vector<Member>, readByKey,
        (const std::string& key, const nlohmann::json& value), (const, override));
    MOCK_METHOD(bool, update,
        (const std::string& keyField, const nlohmann::json& keyValue,
         const std::string& targetField, const nlohmann::json& newValue), (override));
    MOCK_METHOD(bool, remove,
        (const std::string& keyField, const nlohmann::json& keyValue), (override));
    MOCK_METHOD(int,  nextId,    (), (const, override));
    MOCK_METHOD(void, bumpNextId, (), (override));
};

// -------------------------------------------------------
// Fixture
// -------------------------------------------------------
class MemberRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        repo_ = std::make_unique<MemberRepository>(mockCrud_);
    }

    MockICrud                         mockCrud_;
    std::unique_ptr<MemberRepository> repo_;

    // 테스트용 샘플 멤버
    Member makeMember(int id, const std::string& email = "test@test.com")
    {
        return Member{id, "홍길동", email, "010-0000-0000", 30};
    }
};

// -------------------------------------------------------
// create
// -------------------------------------------------------
TEST_F(MemberRepositoryTest, Create_AssignsNextId)
{
    EXPECT_CALL(mockCrud_, readAll()).WillRepeatedly(Return(std::vector<Member>{}));
    EXPECT_CALL(mockCrud_, nextId()).WillOnce(Return(5));
    EXPECT_CALL(mockCrud_, create(_));
    EXPECT_CALL(mockCrud_, bumpNextId());

    Member m = makeMember(0);
    bool ok = repo_->create(m);

    EXPECT_TRUE(ok);
    EXPECT_EQ(m.id, 5);
}

TEST_F(MemberRepositoryTest, Create_RejectsDuplicateEmail)
{
    std::vector<Member> existing = {makeMember(1, "dup@test.com")};
    EXPECT_CALL(mockCrud_, readAll()).WillRepeatedly(Return(existing));
    // create / bumpNextId / nextId 는 호출되면 안 됨
    EXPECT_CALL(mockCrud_, create(_)).Times(0);
    EXPECT_CALL(mockCrud_, bumpNextId()).Times(0);

    Member m = makeMember(0, "dup@test.com");
    bool ok = repo_->create(m);

    EXPECT_FALSE(ok);
}

TEST_F(MemberRepositoryTest, Create_CallsBumpNextId)
{
    EXPECT_CALL(mockCrud_, readAll()).WillRepeatedly(Return(std::vector<Member>{}));
    EXPECT_CALL(mockCrud_, nextId()).WillOnce(Return(1));
    EXPECT_CALL(mockCrud_, create(_));
    EXPECT_CALL(mockCrud_, bumpNextId()).Times(1);

    Member m = makeMember(0);
    repo_->create(m);
}

// -------------------------------------------------------
// findAll
// -------------------------------------------------------
TEST_F(MemberRepositoryTest, FindAll_DelegatesToReadAll)
{
    std::vector<Member> expected = {makeMember(1), makeMember(2)};
    EXPECT_CALL(mockCrud_, readAll()).WillOnce(Return(expected));

    auto result = repo_->findAll();
    EXPECT_EQ(result.size(), 2u);
}

// -------------------------------------------------------
// findById
// -------------------------------------------------------
TEST_F(MemberRepositoryTest, FindById_ReturnsMember_WhenFound)
{
    std::vector<Member> found = {makeMember(3)};
    EXPECT_CALL(mockCrud_, readByKey("id", nlohmann::json(3))).WillOnce(Return(found));

    auto result = repo_->findById(3);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, 3);
}

TEST_F(MemberRepositoryTest, FindById_ReturnsNullopt_WhenNotFound)
{
    EXPECT_CALL(mockCrud_, readByKey("id", nlohmann::json(99)))
        .WillOnce(Return(std::vector<Member>{}));

    auto result = repo_->findById(99);
    EXPECT_FALSE(result.has_value());
}

// -------------------------------------------------------
// update
// -------------------------------------------------------
TEST_F(MemberRepositoryTest, Update_CallsCrudUpdateForEachField)
{
    // 이메일 중복 검사용 readAll → 자기 자신만 반환
    Member m = makeMember(1, "new@test.com");
    EXPECT_CALL(mockCrud_, readAll()).WillRepeatedly(Return(std::vector<Member>{m}));
    EXPECT_CALL(mockCrud_, update("id", nlohmann::json(1), "name",  nlohmann::json(m.name))).WillOnce(Return(true));
    EXPECT_CALL(mockCrud_, update("id", nlohmann::json(1), "email", nlohmann::json(m.email))).WillOnce(Return(true));
    EXPECT_CALL(mockCrud_, update("id", nlohmann::json(1), "phone", nlohmann::json(m.phone))).WillOnce(Return(true));
    EXPECT_CALL(mockCrud_, update("id", nlohmann::json(1), "age",   nlohmann::json(m.age))).WillOnce(Return(true));

    bool ok = repo_->update(m);
    EXPECT_TRUE(ok);
}

TEST_F(MemberRepositoryTest, Update_RejectsDuplicateEmail)
{
    // 다른 회원이 같은 이메일을 이미 사용 중인 경우
    std::vector<Member> others = {makeMember(2, "taken@test.com")};
    EXPECT_CALL(mockCrud_, readAll()).WillRepeatedly(Return(others));
    EXPECT_CALL(mockCrud_, update(_, _, _, _)).Times(0);

    Member m = makeMember(1, "taken@test.com");
    bool ok = repo_->update(m);
    EXPECT_FALSE(ok);
}

// -------------------------------------------------------
// remove
// -------------------------------------------------------
TEST_F(MemberRepositoryTest, Remove_DelegatesToCrud)
{
    EXPECT_CALL(mockCrud_, remove("id", nlohmann::json(7))).WillOnce(Return(true));

    bool ok = repo_->remove(7);
    EXPECT_TRUE(ok);
}

TEST_F(MemberRepositoryTest, Remove_ReturnsFalse_WhenNotFound)
{
    EXPECT_CALL(mockCrud_, remove("id", nlohmann::json(99))).WillOnce(Return(false));

    bool ok = repo_->remove(99);
    EXPECT_FALSE(ok);
}

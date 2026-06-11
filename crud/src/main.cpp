#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
#include "MemberRepository.h"

// -------------------------------------------------------
// 출력 유틸리티
// -------------------------------------------------------
static void printSeparator()
{
    std::cout << std::string(55, '-') << "\n";
}

static void printMember(const Member& m)
{
    std::cout << std::left
              << std::setw(5)  << m.id
              << std::setw(12) << m.name
              << std::setw(25) << m.email
              << std::setw(15) << m.phone
              << m.age << "세\n";
}

static void printHeader()
{
    printSeparator();
    std::cout << std::left
              << std::setw(5)  << "ID"
              << std::setw(12) << "이름"
              << std::setw(25) << "이메일"
              << std::setw(15) << "전화번호"
              << "나이\n";
    printSeparator();
}

// -------------------------------------------------------
// 입력 유틸리티
// -------------------------------------------------------
static int readInt(const std::string& prompt)
{
    int value;
    while (true)
    {
        std::cout << prompt;
        if (std::cin >> value)
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  숫자를 입력해주세요.\n";
    }
}

static std::string readLine(const std::string& prompt)
{
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value);
    return value;
}

// -------------------------------------------------------
// CRUD 메뉴 핸들러
// -------------------------------------------------------
static void handleCreate(MemberRepository& repo)
{
    std::cout << "\n[회원 등록]\n";
    Member m{};
    m.name  = readLine("  이름     : ");
    m.email = readLine("  이메일   : ");
    m.phone = readLine("  전화번호 : ");
    m.age   = readInt ("  나이     : ");

    if (repo.create(m))
        std::cout << "  등록 완료 (ID: " << m.id << ")\n";
    else
        std::cout << "  등록 실패\n";
}

static void handleReadAll(MemberRepository& repo)
{
    std::cout << "\n[전체 회원 목록]\n";
    auto members = repo.findAll();
    if (members.empty())
    {
        std::cout << "  등록된 회원이 없습니다.\n";
        return;
    }
    printHeader();
    for (const auto& m : members)
        printMember(m);
    printSeparator();
    std::cout << "  총 " << members.size() << "명\n";
}

static void handleReadOne(MemberRepository& repo)
{
    int id = readInt("\n[회원 조회] ID: ");
    auto result = repo.findById(id);
    if (!result)
    {
        std::cout << "  ID " << id << " 회원을 찾을 수 없습니다.\n";
        return;
    }
    printHeader();
    printMember(*result);
    printSeparator();
}

static void handleUpdate(MemberRepository& repo)
{
    int id = readInt("\n[회원 수정] ID: ");
    auto result = repo.findById(id);
    if (!result)
    {
        std::cout << "  ID " << id << " 회원을 찾을 수 없습니다.\n";
        return;
    }

    std::cout << "  현재 정보:\n";
    printHeader();
    printMember(*result);
    printSeparator();

    std::cout << "  새 정보 입력 (그대로 유지하려면 Enter):\n";
    Member m = *result;

    auto updated = readLine("  이름     [" + m.name + "]: ");
    if (!updated.empty()) m.name = updated;

    updated = readLine("  이메일   [" + m.email + "]: ");
    if (!updated.empty()) m.email = updated;

    updated = readLine("  전화번호 [" + m.phone + "]: ");
    if (!updated.empty()) m.phone = updated;

    std::cout << "  나이     [" << m.age << "]: ";
    std::string ageInput;
    std::getline(std::cin, ageInput);
    if (!ageInput.empty())
    {
        try { m.age = std::stoi(ageInput); }
        catch (...) { std::cout << "  나이 입력 오류, 기존 값 유지\n"; }
    }

    if (repo.update(m))
        std::cout << "  수정 완료\n";
    else
        std::cout << "  수정 실패\n";
}

static void handleDelete(MemberRepository& repo)
{
    int id = readInt("\n[회원 삭제] ID: ");
    auto result = repo.findById(id);
    if (!result)
    {
        std::cout << "  ID " << id << " 회원을 찾을 수 없습니다.\n";
        return;
    }

    std::cout << "  삭제 대상: " << result->name << " (" << result->email << ")\n";
    std::string confirm = readLine("  정말 삭제하시겠습니까? (y/N): ");
    if (confirm != "y" && confirm != "Y")
    {
        std::cout << "  삭제 취소\n";
        return;
    }

    if (repo.remove(id))
        std::cout << "  삭제 완료\n";
    else
        std::cout << "  삭제 실패\n";
}

// -------------------------------------------------------
// main
// -------------------------------------------------------
int main()
{
    const std::string dataPath = "data/members.json";
    MemberRepository repo(dataPath);

    std::cout << "==========================================\n";
    std::cout << "       회원 관리 시스템 (JSON CRUD)       \n";
    std::cout << "==========================================\n";

    while (true)
    {
        std::cout << "\n  1. 회원 등록\n"
                     "  2. 전체 조회\n"
                     "  3. ID로 조회\n"
                     "  4. 회원 수정\n"
                     "  5. 회원 삭제\n"
                     "  0. 종료\n";

        int choice = readInt("선택> ");
        switch (choice)
        {
        case 1: handleCreate(repo);  break;
        case 2: handleReadAll(repo); break;
        case 3: handleReadOne(repo); break;
        case 4: handleUpdate(repo);  break;
        case 5: handleDelete(repo);  break;
        case 0:
            std::cout << "종료합니다.\n";
            return 0;
        default:
            std::cout << "  올바른 메뉴를 선택해주세요.\n";
        }
    }
}

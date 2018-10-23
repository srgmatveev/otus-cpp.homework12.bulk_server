#include "gmock/gmock.h"
#include "bulk.cpp"
#include "bulk_storage.cpp"
#include "bulk_observer.cpp"
#include "metrics.cpp"
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>

using namespace testing;
class TestCommandsCollection : public Test
{
  public:
    std::ostringstream oss;
    std::size_t chunk_size{3};
    std::shared_ptr<BulkReadCmd> ptrBulkRead;
    std::shared_ptr<ToConsolePrint> ptrToConsolePrint;
    std::shared_ptr<ToFilePrint> ptrToFilePrint;
    void SetUp() override
    {
        ptrBulkRead = BulkReadCmd::create(chunk_size);

        ptrToConsolePrint = ToConsolePrint::create(oss, ptrBulkRead);
        //  ptrToFilePrint = ToFilePrint::create(ptrBulkRead, 2);
        // ptrBulkRead->subscribe(ptrToConsolePrint);
    }
};

TEST(Test_Bulk_Create_Case, Test_Bulk_Create)
{
    std::size_t chunk_size = 4;
    auto ptrBulkRead = BulkReadCmd::create(chunk_size);
    BulkStorage *testStorage = new BulkStorage;
    std::size_t numb_of_chunk = testStorage->create_bulk();
    std::size_t a = testStorage->get_timestamp(2867856);
    ASSERT_THAT(a, 0);
    std::vector<std::string> b = testStorage->get_commands(29789);
    ASSERT_THAT(b.size(), 0);
    if (testStorage)
        delete (testStorage);
    ASSERT_THAT(numb_of_chunk, 1);
}

TEST_F(TestCommandsCollection, Test_Observer_Storage)
{
    ptrToConsolePrint->subscribe_on_observable(ptrBulkRead);
    ASSERT_TRUE(ptrToConsolePrint.use_count() == 1);
    ptrToConsolePrint->subscribe_on_observable(ptrBulkRead);
    ASSERT_TRUE(ptrToConsolePrint.use_count() == 1);
    ptrToConsolePrint->unsubscribe_on_observable(ptrBulkRead);
    ptrToConsolePrint->unsubscribe_on_observable(ptrBulkRead);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append1)
{

    std::string testData{"cmd1\n"
                         "cmd2\n"
                         "cmd3\n"
                         "cmd4\n"
                         "cmd5\n"
                         "cmd6\n"
                         "cmd7\n"
                         "cmd8\n"};
    std::string resultData{"bulk: cmd1, cmd2, cmd3\n"
                           "bulk: cmd4, cmd5, cmd6\n"
                           "bulk: cmd7, cmd8\n"};

    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
    ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append2)
{
    std::string testData{"cmd1\n"
                         "cmd2\n"
                         "cmd3\n"
                         "cmd4\n"
                         "cmd5\n"
                         "cmd6\n"
                         "cmd7\n"
                         "cmd8\n"};
    std::string resultData{"bulk: cmd1, cmd2, cmd3\n"
                           "bulk: cmd4, cmd5, cmd6\n"
                           "bulk: cmd7, cmd8\n"};

    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append3)
{
    std::string testData{"cmd1\n"
                         "cmd2\n"
                         "cmd3\n"
                         "\n"
                         "cmd5\n"
                         "cmd6\n"
                         "cmd7\n"
                         "cmd8\n"};
    std::string resultData{"bulk: cmd1, cmd2, cmd3\n"
                           "bulk: , cmd5, cmd6\n"
                           "bulk: cmd7, cmd8\n"};

    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
    ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append4)
{
    std::string testData{"cmd1\n"
                         "cmd2\n"
                         "cmd3\n"
                         "{\n"
                         "cmd4\n"
                         "\n"
                         "cmd6\n"
                         "cmd7\n"
                         "}\n"
                         "cmd8\n"
                         "cmd9\n"
                         "cmd10\n"
                         "cmd11\n"};
    std::string resultData{
        "bulk: cmd1, cmd2, cmd3\n"
        "bulk: cmd4, , cmd6, cmd7\n"
        "bulk: cmd8, cmd9, cmd10\n"
        "bulk: cmd11\n"};

    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
    ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append5)
{
    std::string testData{"cmd1\n"
                         "cmd2\n"
                         "{\n"
                         "cmd3\n"
                         "cmd4\n"
                         "\n"
                         "cmd6\n"
                         "}\n"};
    std::string resultData{
        "bulk: cmd1, cmd2\n"
        "bulk: cmd3, cmd4, , cmd6\n"};
    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
    ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append6)
{
    std::string testData{"cmd1\n"
                         "cmd2\n"
                         "}\n"
                         "cmd3\n"
                         "cmd4\n"
                         "\n"
                         "cmd6\n"
                         "}\n"};
    std::string resultData{
        "bulk: cmd1, cmd2\n"
        "bulk: cmd3, cmd4, \n"
        "bulk: cmd6\n"};
    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
    ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append7)
{
    std::string testData{"{\n"
                         "cmd1\n"
                         "cmd2\n"
                         "{\n"
                         "cmd3\n"
                         "cmd4\n"
                         "}\n"
                         "\n"
                         "cmd6\n"
                         "}\n"};
    std::string resultData{
        "bulk: cmd1, cmd2, cmd3, cmd4, , cmd6\n"};
    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
    ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append8)
{
    std::string testData{"cmd1\n"
                         "cmd2\n"
                         "cmd3\n"
                         "{\n"
                         "cmd4\n"
                         "cmd5\n"
                         "cmd6\n"
                         "cmd7\n"};
    std::string resultData{
        "bulk: cmd1, cmd2, cmd3\n"};
    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
    ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append9)
{
    std::string testData{"cmd1\n"
                         "cmd2\n"
                         "{cmd_in_wrong_place\n"
                         "cmd3\n"
                         "cmd4\n"
                         "\n"
                         "cmd6\n"
                         "}\n"};
    std::string resultData{
        "bulk: cmd1, cmd2, {cmd_in_wrong_place\n"
        "bulk: cmd3, cmd4, \n"
        "bulk: cmd6\n"};
    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
    ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append10)
{
    std::string testData{"cmd1\n"
                         "cmd2\n"
                         "cmd3\n"
                         "{\n"
                         "cmd4\n"
                         "\n"
                         "cmd6\n"
                         "cmd7\n"
                         "}cmd_in_wrong_place\n"
                         "cmd8\n"};
    std::string resultData{
        "bulk: cmd1, cmd2, cmd3\n"};
    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
    ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append11)
{
    std::string testData{"cmd1\n"
                         "cmd2{\n"
                         "cmd3\n"
                         "cmd4\n"
                         "\n"};
    std::string resultData{
        "bulk: cmd1, cmd2{, cmd3\n"
        "bulk: cmd4, \n"};
    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
    ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

TEST_F(TestCommandsCollection, Test_Bulk_Append12)
{
    std::string testData{"{\n"
                         "}\n"};
    std::string resultData{};
    std::istringstream iss(testData);
    ptrBulkRead->process(iss);
    ptrToConsolePrint->stop();
    std::cout << oss.str() << std::endl;
    ASSERT_THAT(oss.str(), resultData);
}

#pragma once
#include <gmock/gmock.h>
#include <Port.h>
#include <FdSet.h>
class MockPort : public Port {
public:
    MOCK_METHOD(size_t, available, (), (const, noexcept, override));

    MOCK_METHOD(void, write, (std::string_view), (override));

    MOCK_METHOD(std::vector<char>, read, (size_t), (override));
    MOCK_METHOD(std::vector<char>, read, ());

    MOCK_METHOD(std::vector<char>, try_read, (), (override));

    MOCK_METHOD(void, add_to_fd, (FdSet&), (const, override));

    MOCK_METHOD(void, remove_from_fd, (FdSet&), (const, override));

    MOCK_METHOD(bool, is_in_fd, (const FdSet&), (const, override));
};
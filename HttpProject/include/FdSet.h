#pragma once
#include <optional>
#include <chrono>
#include "Networking.h"
/// Set of file descriptors.
/// When activity is detected on an FD that is part of the set
/// a flag is set, which can be queried
class FdSet {
    fd_set set;
public:
    /**
    * Removes all FDs from the set.
    * 
    * Clears all set flags
    */
    void reset() {}

    /// @return true if there is activity on the specified socket/port/fd
    bool is_set(unsigned long long fd) const { fd; return false; }

    /// Removes an fd from the set and clears its flags
    void remove(unsigned long long fd) { fd; }

    /// Adds an fd to the set
    /// Begins listening for activity on that fd
    void add(unsigned long long fd) { fd; }

    /**
    * Suspends program until there is activity on an fd set
    * @param ... sets the read and/or write and/or error set to wait for
    *   Cannot have duplicate set types and at most 3 sets can be specified
    */
    template<class ... Sets>
    static std::enable_if_t<true/*TODO*/> wait(Sets&& ... sets);

    /**
    * Suspends program until there is activity on an fd set or
    * the timeout is reached
    * @param ... sets the read and/or write and/or error set to wait for
    *   Cannot have duplicate set types and at most 3 sets can be specified
    */
    template<class ... Sets>
    static std::enable_if_t<true/*TODO*/> wait(std::chrono::microseconds timeout, Sets&& ... sets);
};

enum class SetType {
    Read, Write, Error
};

/// An FD set of FDs to wait for data to come in
struct ReadSet {
    const std::reference_wrapper<FdSet> fd;
    constexpr static auto type = SetType::Read;
};

/// An FD set of FDs to wait for an error to occur
struct ErrorSet {
    const std::reference_wrapper<FdSet> fd;
    constexpr static auto type = SetType::Error;
};

/// An FD set of FDs to wait to be able to write data
struct WriteSet {
    const std::reference_wrapper<FdSet> fd;
    constexpr static auto type = SetType::Write;
};
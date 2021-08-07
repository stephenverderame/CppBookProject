#include <optional>
#include <chrono>
/// Set of file descriptors.
/// When activity is detected on an FD that is part of the set
/// a flag is set, which can be queried
class FdSet {
    struct fd_set fd;
    // forward declaration
public:
    /**
    * Removes all FDs from the set.
    * 
    * Clears all set flags
    */
    void reset();

    /// @return true if there is activity on the specified socket/port/fd
    bool is_set(int fd) const;

    /// Removes an fd from the set and clears its flags
    void remove(int fd);

    /// Adds an fd to the set
    /// Begins listening for activity on that fd
    void add(int fd);

    using fd_opt_t = std::optional<const std::reference_wrapper<FdSet>>;

    /**
    * Suspends program until there is activity on an fd set
    * @param ... sets the read and/or write and/or error set to wait for
    */
    template<class ... Sets>
    static std::enable_if_t</*...*/> wait(Sets&& ... sets);

    /**
    * Suspends program until there is activity on an fd set or
    * the timeout is reached
    * @param ... sets the read and/or write and/or error set to wait for
    */
    template<class ... Sets>
    static std::enable_if_t</*...*/> wait(std::chrono::microseconds timeout, Sets&& ... sets);
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
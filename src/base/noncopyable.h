#ifndef BASE_NONCOPYABLE_H_
#define BASE_NONCOPYABLE_H_

struct noncopyable {
    noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

struct nonmovable {
    nonmovable() = default;
    nonmovable(nonmovable&&) = delete;
    nonmovable& operator=(nonmovable&&) = delete;
};

#endif

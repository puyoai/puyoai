#ifndef BASE_SLICE_H_
#define BASE_SLICE_H_

template<typename T>
struct Slice {
public:
    Slice() : data_(nullptr), size_(0) {}
    Slice(const T* data, size_t size) : data_(data), size_(size) {}

    size_t size() const { return size_; }

    const T* begin() const { return data_; }
    const T* cbegin() const { return data_; }
    const T* end() const { return data_ + size_; }
    const T* cend() const { return data_ + size_; }

    friend bool operator==(const Slice& lhs, const Slice& rhs)
    {
        if (lhs.size() != rhs.size())
            return false;

        for (size_t i = 0; i < lhs.size(); ++i) {
            if (lhs.data_[i] != rhs.data_[i])
                return false;
        }

        return true;
    }

private:
    const T* data_;
    size_t size_;
};

#endif

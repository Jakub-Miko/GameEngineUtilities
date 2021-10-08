#pragma once

template<typename T>
class Allocator {
public:
    using value_type = T;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;

    //     template <class U> struct rebind {typedef allocator<U> other;};

    Allocator() noexcept
    {

    }  

    template <class U> 
    Allocator(const Allocator<U>& asdas) noexcept
    {

    }

    Allocator(const Allocator<T>& asdas) noexcept
    {

    }

     
    Allocator<T>& operator=(const Allocator<T>&) noexcept
    {
        std::cout << "copy\n";
        return *this;
    }

    Allocator<T>& operator=(Allocator<T>&&) noexcept
    {
        std::cout << "move\n";
        return *this;
    }

    value_type* allocate(std::size_t n)
    {
        return static_cast<value_type*>(::operator new (n * sizeof(value_type)));
    }

    void deallocate(value_type* p, std::size_t) noexcept  
    {
        ::operator delete(p);
    }

    Allocator select_on_container_copy_construction() const {
        return *this;
    }



};


template <class T, class U>
bool operator==(Allocator<T> const&, Allocator<U> const&) noexcept
{
    return true;
}

template <class T, class U>
bool operator!=(Allocator<T> const& x, Allocator<U> const& y) noexcept
{
    return !(x == y);
}
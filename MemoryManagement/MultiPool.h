#pragma once 
#include <memory_resource>
#include <memory>
#include <type_traits>
#include "MemoryPoolDynamic.h"

template<typename Allocator = std::allocator<void>>
class MultiPool : public std::pmr::memory_resource {
	
	using Pool_type = MemoryPool<Allocator>;

	struct Pool_entry {
		size_t pool_size;
		Pool_type pool;

		Pool_entry(size_t pool_size, Pool_type pool) : pool_size(pool_size), pool(pool) {}
		Pool_entry(size_t pool_size, size_t default_pool_size, const Allocator& alloc) 
			: pool_size(pool_size), pool(pool_size, default_pool_size, alloc) {}

		bool operator==(size_t size) const {
			return pool_size == size;
		}

	};

	using Pool_entry_Allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Pool_entry>;

public:

	MultiPool(const Allocator& alloc = std::allocator<void>(), size_t default_pool_chunk_size = 256)
		: upstream_alloc(alloc), default_pool_chunk_size(default_pool_chunk_size), m_Pools(alloc)
	{

	}


	MultiPool(const MultiPool& ref) = delete;
	MultiPool(MultiPool&& ref)
		: upstream_alloc(std::move(ref.upstream_alloc)),
		default_pool_chunk_size(ref.default_pool_chunk_size),
		m_Pools(std::move(ref.m_Pools))
	{

	}

	MultiPool& operator=(const MultiPool& ref) = delete;
	MultiPool& operator=(MultiPool&& ref) {
		release();
		m_Pools = std::move(ref.m_Pools);
		upstream_alloc = std::move(ref.upstream_alloc);
		default_pool_chunk_size = default_pool_chunk_size;
	}

	~MultiPool() {
		release();
	}

	virtual void* do_allocate(size_t size, size_t alignment) override {
		Pool_type* pool = GetPool(size, alignment);
		if (pool) {
			return pool->allocate();
		}
		else {
			size_t size_req = get_closest_higher_pow_2(std::max(size, alignment));
			m_Pools.emplace_back(size_req, default_pool_chunk_size, upstream_alloc);
			pool = &(m_Pools.back().pool);
			return pool->allocate();
		}
	}


	virtual void do_deallocate(void* ptr, size_t size, size_t alignment) override {
		Pool_type* pool = GetPool(size, alignment);
		if (pool) {
			return pool->deallocate(ptr);
		}
		
		assert(false); //Invalid Deallocation
	}

	virtual bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
		return false;
	}

	Pool_type* GetPool(size_t size, size_t alignment) {
		size_t required_size = get_closest_higher_pow_2(std::max(size, alignment));
		auto pool = std::find(m_Pools.begin(), m_Pools.end(), required_size);
		if (pool != m_Pools.end()) {
			return &((*pool).pool);
		}

		return nullptr;
	}

	size_t get_closest_higher_pow_2(size_t num) {
		num--;

		num|= num >> 1;
		num|= num>> 2;
		num|= num>> 4;
		num|= num>> 8;
		num|= num>> 16;
		num|= num>> 32;

		num++;

		return num;
	}

	void clear() {
		for (auto& entry : m_Pools) {
			entry.pool.clear();
		}
	}

	void release() {
		for (auto& entry : m_Pools) {
			entry.pool.release();
		}
		m_Pools.clear();
	}

	bool owns_ptr(void* ptr, size_t size, size_t alignment) {
		Pool_type* pool = GetPool(size, alignment);
		if (pool) {
			return pool->owns_ptr(ptr);
		}
		return false;
	}

private:
	std::vector<Pool_entry, Pool_entry_Allocator> m_Pools;
	Allocator upstream_alloc;
	size_t default_pool_chunk_size;
};
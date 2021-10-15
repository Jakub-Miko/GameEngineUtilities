#pragma once 
#include <memory_resource>
#include <memory>
#include <type_traits>
#include "MemoryPoolDynamic.h"

template<typename Allocator = std::allocator<void>,bool stateful = false, bool deffered_deallocation = false>
class MultiPool : public std::pmr::memory_resource {
public:
	using Pool_type = MemoryPool<Allocator,stateful, deffered_deallocation>;
private:
	using Pool_entry_Allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Pool_type>;

	struct Pool_entry {
		size_t pool_size;
		Pool_type* pool;

		Pool_entry(size_t pool_size, Pool_type* pool) : pool_size(pool_size), pool(pool) {}
		Pool_entry(size_t pool_size, size_t default_pool_size, const Allocator& alloc) 
			: pool_size(pool_size)
		{
			Pool_entry_Allocator al(alloc);
			Pool_type* pool_ptr = std::allocator_traits<Pool_entry_Allocator>::allocate(al,1);
			std::allocator_traits<Pool_entry_Allocator>::construct(al, pool_ptr, pool_size, default_pool_size, alloc);
			pool = pool_ptr;
		}

		bool operator==(size_t size) const {
			return pool_size == size;
		}


	};

	using Pool_handle_Allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Pool_entry>;

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
			size_t size_req = get_Required_size(size, alignment);
			m_Pools.emplace_back(size_req, default_pool_chunk_size, upstream_alloc);
			pool = m_Pools.back().pool;
			return pool->allocate();
		}
	}


	virtual void do_deallocate(void* ptr, size_t size, size_t alignment) override {
		if constexpr (stateful) {
			deallocate_stateful(ptr, size, alignment);
		}
		else {
			Pool_type* pool = GetPool(size, alignment);
			if (pool) {
				return pool->deallocate(ptr);
			}

			assert(false); //Invalid Deallocation
		}
	}

	virtual bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
		return false;
	}

	static void deallocate_stateful(void* ptr, size_t size, size_t aligment) {
		size_t req = get_Required_size(size, aligment);
		auto chunk_st = Pool_type::GetChunkState(ptr, req);
		if (!Pool_type::deallocate_form_chunk(ptr, chunk_st)) {
			assert(false); //Invalid Chunk State
		}
	}


	Pool_type* GetPool(size_t size, size_t alignment) {
		size_t required_size = get_Required_size(size, alignment);

		auto pool = std::find(m_Pools.begin(), m_Pools.end(), required_size);
		if (pool != m_Pools.end()) {
			return (*pool).pool;
		}

		return nullptr;
	}


	static size_t get_Required_size(size_t size, size_t alignment) {
		size_t num;
		if constexpr (stateful) {
			num = std::max(size + sizeof(void*), alignment);
		}
		else {
			num = std::max(size, alignment);
		}
		
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

	//Should only be called in singlethread system, or from thread owning the pool!!!
	void FlushDeallocations() {
		for (auto& entry : m_Pools) {
			entry.pool->FlushDeallocations();
		}
	}

	void clear() {
		for (auto& entry : m_Pools) {
			entry.pool->clear();
		}
	}

	void release() {
		Pool_entry_Allocator al(upstream_alloc);
		for (auto& entry : m_Pools) {
			entry.pool->release();
			std::allocator_traits<Pool_entry_Allocator>::destroy(al, entry.pool);
			std::allocator_traits<Pool_entry_Allocator>::deallocate(al, entry.pool, 1);
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
	std::vector<Pool_entry, Pool_handle_Allocator> m_Pools;
	Allocator upstream_alloc;
	size_t default_pool_chunk_size;
};
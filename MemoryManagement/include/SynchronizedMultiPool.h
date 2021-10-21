#pragma once
#include "MultiPool.h"
#include <thread>
#include <mutex>

template<typename Allocator = std::allocator<void>, bool owning_thread_direct_deallocate = true>
class SynchronizedMultiPool : public std::pmr::memory_resource {
public:
	
	using MultiPool_type = MultiPool<Allocator, true, true>;
	using Pool_type = typename MultiPool_type::Pool_type;

	struct MultiPoolThreadBinding {
		std::thread::id thread_id;
		MultiPool_type multipool;


		MultiPoolThreadBinding(std::thread::id thread_id, MultiPool_type multipool)
			: thread_id(thread_id), multipool(multipool) {}

		MultiPoolThreadBinding(std::thread::id thread_id,
			const Allocator& alloc,
			size_t default_pool_chunk_size)
			: thread_id(thread_id), multipool(alloc, default_pool_chunk_size) {}

		bool operator==(std::thread::id ref) {
			return thread_id == ref;
		}

 	};

	using MultiPoolAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<MultiPoolThreadBinding>;
	
	SynchronizedMultiPool(const Allocator& alloc = std::allocator<void>(), size_t default_pool_chunk_size = 256) 
		: default_pool_chunk_size(default_pool_chunk_size), alloc(alloc), sync_mutex(), m_MultiPools(alloc)
	{
		m_MultiPools.reserve(10000);
	}


	SynchronizedMultiPool(const SynchronizedMultiPool& ref) = delete;
	SynchronizedMultiPool(SynchronizedMultiPool&& ref)
		: alloc(std::move(ref.alloc)),
		default_pool_chunk_size(ref.default_pool_chunk_size),
		m_MultiPools(std::move(ref.m_MultiPools)),
		sync_mutex()
	{

	}

	SynchronizedMultiPool& operator=(const SynchronizedMultiPool& ref) = delete;
	SynchronizedMultiPool& operator=(SynchronizedMultiPool&& ref) {
		release();
		alloc = std::move(ref.alloc);
		default_pool_chunk_size = ref.default_pool_chunk_size;
		m_MultiPools = std::move(ref.m_MultiPools);
	}

	~SynchronizedMultiPool() {
		release();
	}

	void InitializePools(std::thread::id thread) {
		std::lock_guard<std::mutex> lock(sync_mutex);
		m_MultiPools.emplace_back(thread, alloc, default_pool_chunk_size);
	}

	//TODO: Preinitialize the pool on construction to avoid locking at allocation.
	virtual void* do_allocate(size_t size, size_t alignment) override {
		std::thread::id thread_id = std::this_thread::get_id();
		auto multi_pool = std::find(m_MultiPools.begin(), m_MultiPools.end(), thread_id);
		if(multi_pool != m_MultiPools.end()) {
			return (*multi_pool).multipool.allocate(size, alignment);
		}
		assert(false); //Thread for pool isn't registered
	}

	//Build deallocation list
	//TODO: Add DirectDeallocation
	virtual void do_deallocate(void* ptr, size_t size, size_t alignment) override {
		if constexpr (owning_thread_direct_deallocate) {
			Pool_type::deallocate_stateful(ptr, size, alignment); // Rework !!!!!!!!!!!!!!!!!
		}
		else {
			MultiPool_type::deallocate_stateful(ptr, size, alignment);
		}

	}

	virtual bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
		return false;
	}

	//TODO: Remove lock when preinitialization is supported
	void FlushDeallocations() {
		std::thread::id thread_id = std::this_thread::get_id();
		auto multi_pool = std::find(m_MultiPools.begin(), m_MultiPools.end(), thread_id);
		if (multi_pool != m_MultiPools.end()) {
			return (*multi_pool).multipool.FlushDeallocations();
		}
	}

	void release() {
		for (auto& pool : m_MultiPools) {
			pool.multipool.release();
		}
		m_MultiPools.clear();
	}

	void clear() {
		for (auto& pool : m_MultiPools) {
			pool.multipool.clear();
		}
	}

private:
	std::vector<MultiPoolThreadBinding, MultiPoolAllocator> m_MultiPools;
	const Allocator& alloc;
	size_t default_pool_chunk_size;
	std::mutex sync_mutex;
};
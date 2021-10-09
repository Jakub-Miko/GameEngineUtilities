#pragma once
#include "MultiPool.h"
#include <thread>
#include <mutex>

template<typename Allocator = std::allocator<void>,bool stateful = false>
class SynchronizedMultiPool : public std::pmr::memory_resource {
public:
	
	using Pool_type = MultiPool<Allocator, stateful>;

	struct MultiPoolThreadBinding {
		std::thread::id thread_id;
		Pool_type multipool;
		

		MultiPoolThreadBinding(std::thread::id thread_id, Pool_type multipool)
			: thread_id(thread_id), multipool(multipool) {}

		MultiPoolThreadBinding(std::thread::id thread_id,
			const Allocator& alloc, 
			size_t default_pool_chunk_size)
			: thread_id(thread_id), multipool(alloc,default_pool_chunk_size) {}

		bool operator==(std::thread::id ref) {
			return thread_id == ref;
		}
	};

	using MultiPoolAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<MultiPoolThreadBinding>;
	
	SynchronizedMultiPool(const Allocator& alloc = std::allocator<void>(), size_t default_pool_chunk_size = 256) 
		: default_pool_chunk_size(default_pool_chunk_size), alloc(alloc), sync_mutex()
	{

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

	virtual void* do_allocate(size_t size, size_t alignment) override {
		std::thread::id thread_id = std::this_thread::get_id();
		auto multi_pool = std::find(m_MultiPools.begin(), m_MultiPools.end(), thread_id);
		if(multi_pool != m_MultiPools.end()) {
			return (*multi_pool).multipool.allocate(size, alignment);
		}
		else {
			std::unique_lock<std::mutex> lock(sync_mutex);
			m_MultiPools.emplace_back(thread_id, alloc, default_pool_chunk_size);
			lock.unlock();
			return do_allocate(size, alignment);
		}
	}

	//Build deallocation list
	virtual void do_deallocate(void* ptr, size_t size, size_t alignment) override {
		if constexpr (stateful) {
			Pool_type::deallocate_stateful(ptr, size, alignment);
		}
		else {

			for (auto& pool : m_MultiPools) {
				if (pool.multipool.owns_ptr(ptr, size, alignment)) {
					pool.multipool.deallocate(ptr, size, alignment);
					return;
				}
			}

			assert(false); // Invalid Deallocation!
		}
	}

	virtual bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
		return false;
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
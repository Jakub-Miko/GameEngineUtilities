#pragma once
#include <type_traits>
#include <memory>
#include <vector>
#include <cassert>
#include "MemoryManagementUtilities.h"

template<typename Allocator = std::allocator<void>, bool stateful = false, bool deffered_deallocation = false>
class MemoryPool {

	static_assert(!(deffered_deallocation == true && stateful == false), "deffered_deallocation is only supported for stateful pools.");

	struct free_block {
		free_block(free_block* ptr) : next(ptr) {}
		free_block* next;
	};

	using block_alloc_unit = unsigned char;

public:

	struct deallocation_list {
		deallocation_list() : m_lock() {}
		deallocation_list(const deallocation_list& ref) : m_lock(), head(ref.head), tail(ref.tail) {}
		deallocation_list(deallocation_list&& ref) : head(ref.head), tail(ref.tail), m_lock(std::move(lock)) {}
		deallocation_list& operator=(const deallocation_list& ref) = delete;
		deallocation_list& operator=(deallocation_list&& ref) = delete;

		void clear() {
			std::lock_guard<std::mutex> lock(m_lock);
			head = nullptr;
			tail = nullptr; 
			num_of_deallocs = 0;
		}

		void push_back(void* ptr) {
			std::lock_guard<std::mutex> lock(m_lock);
			new(ptr) free_block(head);
			if (!head) {
				tail = reinterpret_cast<free_block*>(ptr);
			}
			head = reinterpret_cast<free_block*>(ptr);
			num_of_deallocs++;
		}

	size_t num_of_deallocs = 0;
	free_block* head = nullptr;
	free_block* tail = nullptr;
	private:
		std::mutex m_lock;
	};

	template<bool deffered>
	struct Chunk_impl {
		Chunk_impl() : base(nullptr), capacity(0), block_size(0), available(0), next_available(0),freelist_head(nullptr) {}
		block_alloc_unit* base;
		size_t capacity;
		size_t block_size;
		size_t available;
		size_t next_available;
		free_block* freelist_head;
	};

	template<>
	struct Chunk_impl<true> {
		
		Chunk_impl() : base(nullptr), capacity(0), block_size(0), available(0), next_available(0), freelist_head(nullptr), dealloc_list() {}
		block_alloc_unit* base;
		size_t capacity;
		size_t block_size;
		size_t available;
		size_t next_available;
		free_block* freelist_head;
		deallocation_list dealloc_list;
	};

	using Chunk = typename Chunk_impl<deffered_deallocation>;

private:

	using ChunkPtr_Allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Chunk*>;
	using Chunk_Allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Chunk>;
	using Block_Allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<block_alloc_unit>;

public:

	MemoryPool(size_t block_size_a, size_t default_pool_size, const Allocator& alloc = Allocator())
		: m_Chunks(alloc), upstream_alloc(alloc), next_chunk_size(default_pool_size / block_size), block_size(block_size_a)
	{
		Block_Allocator b_alloc(alloc);
		Chunk_Allocator c_alloc(alloc);
		block_alloc_unit* data = std::allocator_traits<Block_Allocator>::allocate(b_alloc, next_chunk_size * block_size);
		Chunk* c_data = std::allocator_traits<Chunk_Allocator>::allocate(c_alloc, 1);
		Chunk chunk;
		chunk.base = data;
		chunk.capacity = next_chunk_size;
		chunk.next_available = 0;
		chunk.block_size = block_size;
		chunk.available = chunk.capacity;
		chunk.freelist_head = nullptr;
		std::allocator_traits<Chunk_Allocator>::construct(c_alloc, c_data, chunk);

		m_Chunks.push_back(c_data);
		next_chunk_size = next_chunk_size << 1;
		current_chunk = m_Chunks.back();
	}


	MemoryPool(const MemoryPool& ref) = delete;
	MemoryPool(MemoryPool&& ref) noexcept
		: m_Chunks(ref.m_Chunks), upstream_alloc(std::move(ref.upstream_alloc)), 
		next_chunk_size (ref.next_chunk_size), current_chunk(ref.current_chunk), block_size(ref.block_size)
	{
		ref.m_Chunks.clear();
		ref.next_chunk_size = 0;
		ref.current_chunk = nullptr;
	}

	MemoryPool& operator=(const MemoryPool& ref) = delete;

	//TODO:!!!!!!!!!!!!!!!!!!      revise       !!!!!!!!!!!!!!!!!!!!!!!!!
	MemoryPool& operator=(MemoryPool&& ref) noexcept {	
		release();
		m_Chunks = std::move(ref.m_Chunks);
		upstream_alloc = std::move(ref.upstream_alloc);
		next_chunk_size = ref.next_chunk_size;
		current_chunk = ref.current_chunk;
		block_size = ref.block_size;

		ref.m_Chunks.clear();
		ref.next_chunk_size = 0;
		ref.current_chunk = nullptr;
	}

	~MemoryPool() {
		release();
	}

	void* allocate() {
		if (current_chunk->freelist_head) {
			free_block* block = current_chunk->freelist_head;
			current_chunk->freelist_head = block->next;
			current_chunk->available--;
			if constexpr (stateful) {
				SetChunkState(reinterpret_cast<void*>(block), current_chunk);
			}
			return reinterpret_cast<void*>(block);
		}

		if (current_chunk->capacity > current_chunk->next_available) {
			current_chunk->available--;
			if constexpr (stateful) {
				SetChunkState(reinterpret_cast<void*>(current_chunk->base + (current_chunk->next_available * block_size)),
					current_chunk);
			}
			return reinterpret_cast<void*>(current_chunk->base + (current_chunk->next_available++ * block_size));
		} 
	
		size_t max = 0;
		Chunk* top = nullptr;
		for (auto chunk : m_Chunks) {
			if (chunk->available > max) {
				max = chunk->available;
				top = chunk;
			}
		}
		if (top) {
			MakeChunkCurrent(top);
			return allocate();
		}
		else {
			MakeChunkCurrent(Allocate_Chunk());
			return allocate();
		}

	}

	static void SetChunkState(void* ptr,Chunk* chunk) {
		*(reinterpret_cast<Chunk**>(static_cast<char*>(ptr) + chunk->block_size) - 1) = chunk;
		return;
	}

	static Chunk* GetChunkState(void* ptr, size_t size) {
		Chunk* ptr2 = *(reinterpret_cast<Chunk**>(static_cast<char*>(ptr) + size) - 1);
		return ptr2;
	}

	static void deallocate_stateful(void* ptr, size_t size, size_t aligment) {
		size_t req = get_pool_block_size(size, aligment, stateful);
		auto chunk_st = GetChunkState(ptr, req);
		if (!deallocate_form_chunk(ptr, chunk_st)) {
			assert(false); //Invalid Chunk State
		}
	}

	void deallocate(void* ptr){
		for (auto& chunk : m_Chunks) {
			if (is_ptr_in_chunk(chunk, ptr)) {
				chunk->freelist_head = new(ptr) free_block(chunk->freelist_head);
				chunk->available++;
				return;
			}
		}

		assert(false); // this pointer isn't owned by this instance of MemoryPool
		return;
	}

	bool owns_ptr(void* ptr) {
		for (auto chunk : m_Chunks) {
			if (is_ptr_in_chunk(chunk, ptr)) {
				return true;
			}
		}

		return false;
	}

	Chunk* GetChunk(void* ptr) {
		for (auto chunk : m_Chunks) {
			if (is_ptr_in_chunk(chunk, ptr)) {
				return chunk;
			}
		}

		return nullptr;
	}

	//Should only be called in singlethread system, or from thread owning the pool!!!
	void FlushDeallocations() {
		if constexpr (deffered_deallocation) {
			for (auto chunk : m_Chunks) {
				deallocation_list& dealloc_list = chunk->dealloc_list;
				dealloc_list.tail->next = chunk->freelist_head;
				chunk->freelist_head = dealloc_list.head;
				chunk->available += dealloc_list.num_of_deallocs;
				dealloc_list.clear();
			}
		}
		else {
			assert(false); //FlushDeallocations is only supported by deffered_deallocation enabled pool!!!
		}
	}

	void clear() {
		for (auto chunk : m_Chunks) {
			chunk->freelist_head = nullptr;
			chunk->next_available = 0;
			chunk->available = chunk.capacity;
			if constexpr (deffered_deallocation) {
				chunk.dealloc_list.clear();
			}
		}
	}

	void release() {
		Block_Allocator b_alloc(upstream_alloc);
		Chunk_Allocator c_alloc(upstream_alloc);
		for (auto chunk : m_Chunks) {
			std::allocator_traits<Block_Allocator>::deallocate(b_alloc, chunk->base, chunk->capacity * block_size);
			std::allocator_traits<Chunk_Allocator>::deallocate(c_alloc, chunk, 1);
		}
		m_Chunks.clear();
	}

	bool operator==(size_t size) {
		return size == block_size;
	}

	const Allocator& GetUpstreamAlloc() const {
		return upstream_alloc;
	}

	static bool deallocate_form_chunk(void* ptr, Chunk* chunk) {
		if constexpr (deffered_deallocation) {
			chunk->dealloc_list.push_back(ptr);
			return true;
		}
		else {
			if (is_ptr_in_chunk(chunk, ptr)) {
				chunk->freelist_head = new(ptr) free_block(chunk->freelist_head);
				chunk->available++;
				return true;
			}
			return false;
		}
	}

	size_t GetBlockCapacity() {
		if (stateful) {
			return block_size - sizeof(void*);
		}
		else {
			return block_size;
		}
	}

private:
	
	Chunk* Allocate_Chunk() {
		Block_Allocator b_alloc(upstream_alloc);
		Chunk_Allocator c_alloc(upstream_alloc);
		block_alloc_unit* data = std::allocator_traits<Block_Allocator>::allocate(b_alloc, next_chunk_size * block_size);
		Chunk* c_data = std::allocator_traits<Chunk_Allocator>::allocate(c_alloc, 1);
		Chunk chunk;
		chunk.base = data;
		chunk.capacity = next_chunk_size;
		chunk.next_available = 0;
		chunk.available = chunk.capacity;
		chunk.block_size = block_size;
		chunk.freelist_head = nullptr;
		std::allocator_traits<Chunk_Allocator>::construct(c_alloc, c_data, chunk);
		m_Chunks.push_back(c_data);
		next_chunk_size = next_chunk_size << 1;
		return m_Chunks.back();
	}

	void MakeChunkCurrent(Chunk* chunk) {
		current_chunk = chunk;
	}

	static bool is_ptr_in_chunk(Chunk* chunk,void* ptr) {
		return ptr >= reinterpret_cast<void*>(chunk->base) && ptr < reinterpret_cast<void*>(chunk->base + (chunk->capacity * chunk->block_size));
	}

private:
	size_t block_size;
	std::vector<Chunk*, ChunkPtr_Allocator> m_Chunks;
	size_t next_chunk_size;
	const Allocator upstream_alloc;
	Chunk* current_chunk;
};

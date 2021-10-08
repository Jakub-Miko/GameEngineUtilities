#pragma once
#include <type_traits>
#include <memory>
#include <vector>
#include <cassert>

template<typename Allocator = std::allocator<void>>
class MemoryPool {

	struct free_block {
		free_block(free_block* ptr) : next(ptr) {}
		free_block* next;
	};

	using block_alloc_unit = unsigned char;

public:

	struct Chunk {
		block_alloc_unit* base;
		size_t capacity;
		size_t available;
		size_t next_available;
		free_block* freelist_head;
	};

private:

	using Chunk_Allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Chunk>;
	using Block_Allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<block_alloc_unit>;

public:

	MemoryPool(size_t block_size, size_t default_pool_size, const Allocator& alloc = Allocator())
		: m_Chunks(alloc), upstream_alloc(alloc), next_chunk_size(default_pool_size / block_size), block_size(block_size)
	{
		Block_Allocator b_alloc = Block_Allocator(alloc);
		block_alloc_unit* data = std::allocator_traits<Block_Allocator>::allocate(b_alloc, next_chunk_size * block_size);
		Chunk chunk;
		chunk.base = data;
		chunk.capacity = next_chunk_size;
		chunk.next_available = 0;
		chunk.available = chunk.capacity;
		chunk.freelist_head = nullptr;
		m_Chunks.push_back(chunk);
		next_chunk_size = next_chunk_size << 1;
		current_chunk = &m_Chunks.back();
	}


	MemoryPool(const MemoryPool& ref) = delete;
	MemoryPool(MemoryPool&& ref) 
		: m_Chunks(ref.m_Chunks), upstream_alloc(std::move(ref.upstream_alloc)), 
		next_chunk_size (ref.next_chunk_size), current_chunk(ref.current_chunk), block_size(ref.block_size)
	{
		ref.m_Chunks.clear();
		ref.next_chunk_size = 0;
		ref.current_chunk = nullptr;
	}

	MemoryPool& operator=(const MemoryPool& ref) = delete;

	//TODO:!!!!!!!!!!!!!!!!!!      revise       !!!!!!!!!!!!!!!!!!!!!!!!!
	MemoryPool& operator=(MemoryPool&& ref) {
		
		m_Chunks.clear();
		
		m_Chunks = ref.m_Chunks;
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
			return reinterpret_cast<void*>(block);
		}

		if (current_chunk->capacity > current_chunk->next_available) {
			current_chunk->available--;
			return reinterpret_cast<void*>(current_chunk->base + (current_chunk->next_available++ * block_size));
		} 
	
		size_t max = 0;
		Chunk* top = nullptr;
		for (auto& chunk : m_Chunks) {
			if (chunk.available > max) {
				max = chunk.available;
				top = &chunk;
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


	void deallocate(void* ptr){
		for (auto& chunk : m_Chunks) {
			if (is_ptr_in_chunk(&chunk, ptr)) {
				chunk.freelist_head = new(ptr) free_block(chunk.freelist_head);
				chunk.available++;
				return;
			}
		}

		assert(false); // this pointer isn't owned by this instance of MemoryPool
		return;
	}

	bool owns_ptr(void* ptr) {
		for (auto& chunk : m_Chunks) {
			if (is_ptr_in_chunk(&chunk, ptr)) {
				return true;
			}
		}

		return false;
	}

	Chunk* GetChunk(void* ptr) {
		for (auto& chunk : m_Chunks) {
			if (is_ptr_in_chunk(&chunk, ptr)) {
				return &chunk;
			}
		}

		return nullptr;
	}

	void clear() {
		for (auto& chunk : m_Chunks) {
			chunk.freelist_head = nullptr;
			chunk.next_available = 0;
			chunk.available = chunk.capacity;
		}
	}

	void release() {
		Block_Allocator b_alloc = Block_Allocator(upstream_alloc);
		for (auto& chunk : m_Chunks) {
			std::allocator_traits< Block_Allocator>::deallocate(b_alloc, chunk.base, chunk.capacity * block_size);
		}
		m_Chunks.clear();
	}

	bool operator==(size_t size) {
		return size == block_size;
	}

private:
	
	Chunk* Allocate_Chunk() {
		Block_Allocator b_alloc = Block_Allocator(upstream_alloc);
		block_alloc_unit* data = std::allocator_traits<Block_Allocator>::allocate(b_alloc, next_chunk_size * block_size);
		Chunk chunk;
		chunk.base = data;
		chunk.capacity = next_chunk_size;
		chunk.next_available = 0;
		chunk.available = chunk.capacity;
		chunk.freelist_head = nullptr;
		m_Chunks.push_back(chunk);
		next_chunk_size = next_chunk_size << 1;
		return &(m_Chunks.back());
	}

	void MakeChunkCurrent(Chunk* chunk) {
		current_chunk = chunk;
	}

	bool is_ptr_in_chunk(Chunk* chunk,void* ptr) {
		return ptr >= reinterpret_cast<void*>(chunk->base) && ptr < reinterpret_cast<void*>(chunk->base + (chunk->capacity * block_size));
	}

private:
	size_t block_size;
	std::vector<Chunk, Chunk_Allocator> m_Chunks;
	size_t next_chunk_size;
	const Allocator upstream_alloc;
	Chunk* current_chunk;
};

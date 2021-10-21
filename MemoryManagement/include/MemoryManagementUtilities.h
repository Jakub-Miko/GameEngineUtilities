#pragma once


static size_t get_pool_block_size(size_t size, size_t alignment, bool is_stateful = false) {
	size_t num;
	if(is_stateful) {
		num = std::max(size + sizeof(void*), alignment);
	}
	else {
		num = std::max(size, alignment);
	}

	num--;

	num |= num >> 1;
	num |= num >> 2;
	num |= num >> 4;
	num |= num >> 8;
	num |= num >> 16;
	num |= num >> 32;

	num++;

	return num;
}
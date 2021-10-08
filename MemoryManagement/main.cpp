#include <iostream>

#include <Profiler.h>
#include <MultiPool.h>
#include <SynchronizedMultiPool.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>




int main() {
	{
		BEGIN_PROFILING("Profile", "C:/Users/mainm/Desktop/GameEngine/Utility/MemoryManagement/Profile_Result.json");
		using data_type = typename long long;
		
		{
			SynchronizedMultiPool<std::allocator<void>> mem;
			std::vector<data_type*> m_ptrs;
			m_ptrs.reserve(1024);
			PROFILE("Main");
			{
				PROFILE("Alloc custom pool");
				for (int i = 0; i < 1024; i++) {
					data_type* ptr = (data_type*)mem.allocate(sizeof(data_type), alignof(data_type));
					*ptr = 0xffffffffffffffff;
					m_ptrs.push_back(ptr);
				}
			}
			for (auto ptr : m_ptrs) {
				std::cout << *ptr << "\n";
			}
			{
				PROFILE("Dealloc custom pool");
				for (auto& ptr : m_ptrs) {
					mem.deallocate(ptr, sizeof(data_type), alignof(data_type));
				}
			}
		}
		{
			SynchronizedMultiPool<std::allocator<void>> mem;
			std::vector<data_type*> m_ptrs;
			m_ptrs.reserve(1024);
			PROFILE("Main");
			{
				PROFILE("Alloc custom pool");
				for (int i = 0; i < 1024; i++) {
					data_type* ptr = (data_type*)mem.allocate(sizeof(data_type), alignof(data_type));
					*ptr = 0xffffffffffffffff;
					m_ptrs.push_back(ptr);
				}
			}
			for (auto ptr : m_ptrs) {
				std::cout << *ptr << "\n";
			}
			{
				PROFILE("Dealloc custom pool");
				for (auto& ptr : m_ptrs) {
					mem.deallocate(ptr, sizeof(data_type), alignof(data_type));
				}
			}
		}
		{
			std::vector<data_type*> m_ptrs;
			std::allocator<data_type> alloc;
			m_ptrs.reserve(1024);
			PROFILE("Main");
			{
				PROFILE("Alloc new");
				for (int i = 0; i < 1024; i++) {
					data_type* ptr = std::allocator_traits<std::allocator<data_type>>::allocate(alloc,1);
					*ptr = 0xffffffffffffffff;
					m_ptrs.push_back(ptr);
				}
			}
			for (auto ptr : m_ptrs) {
				std::cout << *ptr << "\n";
			}
			{
				PROFILE("Dealloc new");
				for (auto& ptr : m_ptrs) {
					delete ptr;
				}
			}
		}
		{
			std::vector<data_type*> m_ptrs;
			std::allocator<data_type> alloc;
			m_ptrs.reserve(1024);
			PROFILE("Main");
			{
				PROFILE("Alloc new");
				for (int i = 0; i < 1024; i++) {
					data_type* ptr = std::allocator_traits<std::allocator<data_type>>::allocate(alloc, 1);
					*ptr = 0xffffffffffffffff;
					m_ptrs.push_back(ptr);
				}
			}
			for (auto ptr : m_ptrs) {
				std::cout << *ptr << "\n";
			}
			{
				PROFILE("Dealloc new");
				for (auto& ptr : m_ptrs) {
					delete ptr;
				}
			}
		}
		{
			std::pmr::unsynchronized_pool_resource mem;
			std::vector<data_type*> m_ptrs;
			m_ptrs.reserve(1024);
			PROFILE("Main");
			{
				PROFILE("Alloc pmr pool");
				for (int i = 0; i < 1024; i++) {
					data_type* ptr = (data_type*)mem.allocate(sizeof(data_type), alignof(data_type));
					*ptr = 0xffffffffffffffff;
					m_ptrs.push_back(ptr);
				}
			}
			for (auto ptr : m_ptrs) {
				std::cout << *ptr << "\n";
			}
			{
				PROFILE("Dealloc pmr pool");
				for (auto& ptr : m_ptrs) {
					mem.deallocate(ptr, sizeof(data_type), alignof(data_type));
				}
			}
		}
		{
			std::pmr::unsynchronized_pool_resource mem;
			std::vector<data_type*> m_ptrs;
			m_ptrs.reserve(1024);
			PROFILE("Main");
			{
				PROFILE("Alloc pmr pool");
				for (int i = 0; i < 1024; i++) {
					data_type* ptr = (data_type*)mem.allocate(sizeof(data_type), alignof(data_type));
					*ptr = 0xffffffffffffffff;
					m_ptrs.push_back(ptr);
				}
			}
			for (auto ptr : m_ptrs) {
				std::cout << *ptr << "\n";
			}
			{
				PROFILE("Dealloc pmr pool");
				for (auto& ptr : m_ptrs) {
					mem.deallocate(ptr, sizeof(data_type), alignof(data_type));
				}
			}
		}


	}
	END_PROFILING();
	_CrtDumpMemoryLeaks();
}
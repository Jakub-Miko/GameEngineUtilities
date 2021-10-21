#pragma once
// Valid generator for Ids
struct SequentialIdGenerator {
	using id_type = int;
	static int Id() {
		static int id = 0;
		return id++;
	}
};

// TypeId class, its class can implicitly be converterd to the id of generator type  
// Generator needs to be valid and needs to be supplied either as a template parameter or as a type_id_gen typedef of the Type
template<typename T, typename Generator = typename T::type_id_gen, typename dummy = decltype(Generator::Id(), typename Generator::id_type())>
struct TypeId {
	using Type = typename Generator::id_type;

	inline static const Type GetId() {
		static const Type id = Generator::Id();
		return id;
	}

	constexpr operator Type() const {
		return GetId();
	}

};

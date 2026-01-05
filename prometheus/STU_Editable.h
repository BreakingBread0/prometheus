#pragma once
#include "STU.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class STU_ObjectList;
class STU_PrimitiveList;
class STU_ResourceRefList;
class STU_Map;

class STU_Primitive {
public:
	void* value;
	uint type;

	STU_Primitive(void* value, uint type) : value(value), type(type) {
		owassert(value);
	}

	template <typename T>
	T get_value() const {
		return *(T*)value;
	}

	template<>
	const char* get_value<const char*>() const {
		return ((STUteString*)value)->read();
	}

	template<>
	std::string get_value<std::string>() const {
		return ((STUteString*)value)->read();
	}

	template<typename T>
	void set_value(T new_value) {
		*(T*)value = new_value;
	}

	template<>
	void set_value<std::string>(std::string new_value) {
		return set_value(new_value.c_str());
	}

	template<>
	void set_value<const char*>(const char* new_value) {
		STUteString* str = (STUteString*)value;
		str->set(new_value);
	}
};

#define DEFINE_ACCESSOR_FUNC(return_type, name) \
return_type name (STUArgumentInfo* arg); \
return_type name(uint hash); \
return_type name(const char* str_name);

class STU_Object {
public:
	void* value;
	STUInfo* struct_info;

	STU_Object(STUInfo* struct_info, void* value = nullptr) : struct_info(struct_info), value(value) {}

	//Always valid if this object is valid
	DEFINE_ACCESSOR_FUNC(STU_Primitive, get_argument_primitive);
	//May return an invalid object, check with valid()
	DEFINE_ACCESSOR_FUNC(STU_Object, get_argument_object);
	//Child object is always valid
	DEFINE_ACCESSOR_FUNC(STU_ObjectList, get_argument_objectlist);
	//Child object is always valid
	DEFINE_ACCESSOR_FUNC(STU_PrimitiveList, get_argument_primitivelist);
	//Child object is always valid (but may not have a resource)
	DEFINE_ACCESSOR_FUNC(STU_ResourceRefList, get_argument_resreflist);
	//Always valid if this object is valid
	DEFINE_ACCESSOR_FUNC(STUResourceReference*, get_argument_resource);
	//Always valid if this object is valid
	DEFINE_ACCESSOR_FUNC(STU_Map, get_argument_map);

	//Set object will only work on an non-inlined object!
	void set_object(STUArgumentInfo* arg, STU_Object object);
	void set_object(uint name_hash, STU_Object object);
	void set_object(const char* str_name, STU_Object object);

	//initializes all lists
	void initialize();
	void initialize_configVar();
	//Shallow
	void deallocate();
	inline bool valid() {
		return struct_info != nullptr && value != nullptr;
	}

	inline STU_Object get_runtime_root() {
		//ghetts... How can you differentiate between STUs with and without vfptrs?
		if (valid() && struct_info->instance_size >= 8) {
			for (auto arg : *struct_info) {
				if (arg.second->offset == 0)
					return *this;
			}
			auto base = (STUBase<>*)value;
			if (base->valid()) {
				return STU_Object(base->vfptr->GetSTUInfo(), value);
			}
		}
		return *this;
	}

	static STU_Object create(STUInfo* struct_info);
};

class STU_ListBase {
public:
	template <typename value_type, typename list_type = STU_ListBase>
	class ListIteratorBase {
	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;

		ListIteratorBase() {}
		ListIteratorBase(list_type* list) : _list(list) {
			if (!_list->valid() || _list->count() == 0)
				_list = nullptr;
		}
		ListIteratorBase& operator++() {
			if (++_current_pos >= _list->count()) {
				_list = nullptr;
				_current_pos = 0;
			}
			return *this;
		}

		// TODO this change changes behaviour, make sure it is still valid.
		ListIteratorBase& operator++(int) {
			auto tmp = *this;
			++(*this);
			return *tmp;
		}

		bool operator==(const ListIteratorBase& it) const {
			return it._current_pos == _current_pos && it._list == _list;
		}

		virtual value_type operator*() const = 0;
	protected:
		list_type* _list = nullptr;
		int _current_pos = 0;
	};

	STUBullshitListFull<__int64>* value;
	STUArgumentInfo* argument;

	STU_ListBase(STUArgumentInfo* argument, void* value = nullptr) : 
		argument(argument), 
		value((STUBullshitListFull<__int64>*)value) {
		if (!valid() || !this->value->valid()) {
			value = nullptr;
		}
	}

	bool valid() {
		return value != nullptr;
	}

	int count() {
		owassert(valid());
		return value->count();
	}

protected:
	void remove_at_index(int index, int item_size) {
		owassert(valid());
		owassert(index < value->count());

		int old_size = value->count() * item_size;
		void* new_buf = (void*)ow_memalloc(old_size - item_size);
		int item_offset = item_size * index;
		memcpy(new_buf, value->list(), item_offset);
		item_offset += item_size;
		memcpy((void*)((__int64)new_buf + item_offset), (void*)((__int64)value->list() + item_offset), old_size - item_offset);

		value->set_count(value->count() - 1);
		ow_dealloc((__int64)value->list());
		value->set_list(new_buf);
	}
};

class STU_ObjectList : public STU_ListBase {
public:
	class ListIterator : public STU_ListBase::ListIteratorBase<STU_Object, STU_ObjectList> {
	public:
		using STU_ListBase::ListIteratorBase<STU_Object, STU_ObjectList>::ListIteratorBase;

		STU_Object operator*() const override {
			if (!_list)
				throw std::out_of_range("Iterator out of range");
			return (*_list)[_current_pos];
		}
	};

	STU_ObjectList(STUArgumentInfo* argument, void* value = nullptr) : STU_ListBase(argument, value) {
		_type = argument->constraint->get_type_flag();
		owassert(_type == STU_ConstraintType_BSList_InlinedObject || _type == STU_ConstraintType_BSList_Object);
		owassert(_list_type = GetSTUInfoByHash(argument->constraint->get_stu_type()));
	}

	STU_Object operator[](int pos) {
		if (_type == STU_ConstraintType_BSList_InlinedObject)
			return STU_Object(_list_type, (void*)((__int64)value->list() + _list_type->instance_size * pos));
		return STU_Object(_list_type, (void*)(value->list()[pos]));
	}

	ListIterator begin() {
		return ListIterator(this);
	}

	ListIterator end() {
		return ListIterator();
	}

	//copied & value of object changed if type is inlined object
	void push_back_object(STU_Object& object) {
		owassert(valid());
		owassert(_type == STU_ConstraintType_BSList_Object);
		int old_size = value->count() * 8;
		__int64* new_buf = (__int64*)ow_memalloc(old_size + 8);
		memcpy(new_buf, value->list(), old_size);

		new_buf[value->count()] = (__int64)object.value;

		value->set_count(value->count() + 1);
		ow_dealloc((__int64)value->list());
		value->set_list(new_buf);
	}

	STU_Object push_back_inlinedObject() {
		owassert(valid());
		owassert(_type == STU_ConstraintType_BSList_InlinedObject);
		int old_size = value->count() * _list_type->instance_size;
		__int64 new_buf = ow_memalloc(old_size + _list_type->instance_size);
		memcpy((void*)new_buf, value->list(), old_size);

		STU_Object new_object(_list_type, (void*)(new_buf + old_size));
		new_object.initialize();

		value->set_count(value->count() + 1);
		ow_dealloc((__int64)value->list());
		value->set_list((void*)new_buf);

		return new_object;
	}

	//Not deallocating since the item may be referenced in multiple locations
	//example: STUStatescriptGraph->m_nodes and STUStatescriptGraph->m_graph->m_items
	void remove_at_index(int index) {
		if (_type == STU_ConstraintType_BSList_InlinedObject) {
			STU_ListBase::remove_at_index(index, _list_type->instance_size);
		}
		else {
			STU_ListBase::remove_at_index(index, 8);
		}
	}
private:
	STUConstraintType _type;
	STUInfo* _list_type;
};

class STU_PrimitiveList : public STU_ListBase {
public:
	class ListIterator : public STU_ListBase::ListIteratorBase<STU_Primitive, STU_PrimitiveList> {
	public:
		using STU_ListBase::ListIteratorBase<STU_Primitive, STU_PrimitiveList>::ListIteratorBase;

		STU_Primitive operator*() const override {
			if (!_list)
				throw std::out_of_range("Iterator out of range");
			return (*_list)[_current_pos];
		}
	};

	STU_PrimitiveList(STUArgumentInfo* argument, void* value = nullptr) : STU_ListBase(argument, value) {
		auto type = argument->constraint->get_type_flag();
		owassert(type == STU_ConstraintType_BSList_Primitive || type == STU_ConstraintType_BSList_Enum);
		auto stu_type = argument->constraint->get_stu_type();
		if (type == STU_ConstraintType_BSList_Primitive)
			_item_size = STU_NAME::Primitive::_primitive_size(stu_type);
		else
			_item_size = 4; //Enum has a size of 4
	}

	template <typename T>
	void push_back(T item) {
		owassert(sizeof(T) == _item_size);

		int old_size = value->count() * _item_size;
		void* new_buf = (void*)ow_memalloc(old_size + _item_size);
		memcpy(new_buf, value->list(), old_size);
		*(T*)((__int64)new_buf + old_size) = item;

		value->set_count(value->count() + 1);
		ow_dealloc((__int64)value->list());
		value->set_list(new_buf);
	}

	template<>
	void push_back<std::string>(std::string str) {
		push_back(str.c_str());
	}

	template<>
	void push_back<const char*>(const char* str) {
		STUteString teString{};
		teString.set(str);
		push_back(teString);
	}

	void remove_at_index(int index) {
		STU_ListBase::remove_at_index(index, _item_size);
	}

	STU_Primitive operator[](int pos) {
		return STU_Primitive((void*)((__int64)value->list() + _item_size * pos), _stu_type);
	}

	ListIterator begin() {
		return ListIterator(this);
	}

	ListIterator end() {
		return ListIterator();
	}
private:
	int _item_size;
	uint _stu_type;
};

class STU_ResourceRefList : public STU_ListBase {
public:
	class ListIterator : public STU_ListBase::ListIteratorBase<STUResourceReference*, STU_ResourceRefList> {
	public:
		using STU_ListBase::ListIteratorBase<STUResourceReference*, STU_ResourceRefList>::ListIteratorBase;

		STUResourceReference* operator*() const override {
			if (!_list)
				throw std::out_of_range("Iterator out of range");
			return (*_list)[_current_pos];
		}
	};

	STU_ResourceRefList(STUArgumentInfo* argument, void* value = nullptr) : STU_ListBase(argument, value) {
		auto type = argument->constraint->get_type_flag();
		owassert(type == STU_ConstraintType_BSList_NonSTUResourceRef || type == STU_ConstraintType_BSList_ResourceRef);
	}

	STUResourceReference* operator[](int pos) {
		return (STUResourceReference*)((__int64)value->list() + sizeof(STUResourceReference) * pos);
	}

	void remove_at_index(int index) {
		STU_ListBase::remove_at_index(index, sizeof(STUResourceReference));
	}

	ListIterator begin() {
		return ListIterator(this);
	}

	ListIterator end() {
		return ListIterator();
	}
};

//Only returns back values because i dont want to look at STU maps ever again in my life
class STU_Map {
public:
	class MapIterator {
	public:
		using value_type = std::pair<int, STU_Object>;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;

		MapIterator() {}
		MapIterator(STU_Map* list) : _list(list) {
			if (_list->count() == 0)
				_list = nullptr;
		}
		MapIterator& operator++() {
			if (++_current_pos >= _list->count()) {
				_list = nullptr;
				_current_pos = 0;
			}
			return *this;
		}

		MapIterator operator++(int) {
			MapIterator tmp = *this;
			++(*this);
			return tmp;
		}

		value_type operator*() const {
			if (!_list)
				throw std::out_of_range("Iterator out of range");
			return (*_list)[_current_pos];
		}

		bool operator==(const MapIterator& it) const {
			return it._current_pos == _current_pos && it._list == _list;
		}
	private:
		STU_Map* _list = nullptr;
		int _current_pos = 0;
	};

	int count() {
		owassert(valid());
		return value->count();
	}

	bool valid() {
		return value != nullptr;
	}

	STU_Map(STUArgumentInfo* argument, void* map) :
		_list_value_type(GetSTUInfoByHash(argument->constraint->get_stu_type())),
		value((STUBullshitMapFull<void*>*)map)
	{
		if (!valid() || !value->valid())
			value = nullptr;
		owassert(argument->constraint->get_type_flag() == STU_ConstraintType_Map);
	}

	MapIterator begin() {
		return MapIterator(this);
	}

	MapIterator end() {
		return MapIterator();
	}

	std::pair<int, STU_Object> operator[](int pos) {
		auto& item = value->list()[pos];
		return { item.key, STU_Object(_list_value_type, item.value) };
	}

	STUBullshitMapFull<void*>* value;
private:
	STUInfo* _list_value_type;
};


// void to_json(json& j, const STU_Primitive& p);
// void to_json(json& j, const STU_Object& p);
// void to_json(json& j, const STU_ResourceRefList& p);
// void to_json(json& j, const STU_ObjectList& p);
// void to_json(json& j, const STU_PrimitiveList& p);
// 
// void from_json(const json& j, STU_Primitive& p);
// void from_json(const json& j, STUResourceReference& stu);
// void from_json(const json& j, STU_Object& p);
// void from_json(const json& j, STU_ResourceRefList& p);
// void from_json(const json& j, STU_ObjectList& p);
// void from_json(const json& j, STU_PrimitiveList& p);
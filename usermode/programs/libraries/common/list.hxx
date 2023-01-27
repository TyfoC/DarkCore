#pragma once
#ifndef LIST_HXX
#define LIST_HXX

#include "memory-utils.hxx"

template <typename T>
class List {
	public:
	static constexpr size_t CountAllocate = 10;

	List() {
		Data_ = new T[CountAllocate];
		if (!Data_) return;
		AllocatedElementsCount_ = CountAllocate;
		ElementsCount_ = 0;
	}

	List(const T* values, size_t count) {
		size_t allocateCount = count - (count % CountAllocate) + CountAllocate;
		Data_ = new T[allocateCount];
		if (Data_) {
			AllocatedElementsCount_ = allocateCount;
			ElementsCount_ = count;
			MemoryUtils::Copy(Data_, values, count * sizeof(T));
		}
	}
	
	List(const List<T>& values) {
		Data_ = new T[values.AllocatedElementsCount_];
		if (Data_) {
			AllocatedElementsCount_ = values.AllocatedElementsCount_;
			ElementsCount_ = values.ElementsCount_;
			MemoryUtils::Copy(Data_, values.Data_, ElementsCount_ * sizeof(T));
		}
	}

	~List() {
		if (Data_ && AllocatedElementsCount_) delete[] Data_;
		ElementsCount_ = AllocatedElementsCount_ = 0;
	}

	bool Append(const T& value) {
		if (!Data_) return false;
		else if (ElementsCount_ == AllocatedElementsCount_) {
			T* tmp = new T[AllocatedElementsCount_ + CountAllocate];
			if (!tmp) return false;

			AllocatedElementsCount_ += CountAllocate;
			MemoryUtils::Copy(tmp, Data_, ElementsCount_ * sizeof(T));
			delete[] Data_;
			Data_ = tmp;
		}

		Data_[ElementsCount_++] = value;
		return true;
	}

	bool Insert(const T& value, size_t index) {
		if (!Data_) return false;
		
		if (index > ElementsCount_) return false;
		else if (ElementsCount_ == AllocatedElementsCount_) {
			T* tmp = new T[AllocatedElementsCount_ + CountAllocate];
			if (!tmp) return false;

			AllocatedElementsCount_ += CountAllocate;
			MemoryUtils::Copy(tmp, Data_, ElementsCount_ * sizeof(T));
			delete[] Data_;
			Data_ = tmp;
		}

		for (size_t i = ElementsCount_; i > index; i--) Data_[i] = Data_[i - 1];
		Data_[index] = value;
		++ElementsCount_;

		return true;
	}

	bool Remove(size_t index) {
		if (!Data_) return false;
		
		if (index >= ElementsCount_) return false;
		
		for (size_t i = index; i < ElementsCount_ - 1; i++) Data_[i] = Data_[i + 1];
		if (!(ElementsCount_ % (CountAllocate + 1))) {
			T* tmp = new T[AllocatedElementsCount_ - CountAllocate];
			if (!tmp) return false;

			AllocatedElementsCount_ -= CountAllocate;
			MemoryUtils::Copy(tmp, Data_, ElementsCount_ * sizeof(T));
			delete[] Data_;
			Data_ = tmp;
		}

		--ElementsCount_;
		return true;
	}

	size_t GetCount() {
		return ElementsCount_;
	}

	T& operator[](size_t index) {
		return Data_[index];
	}

	const T& operator[](size_t index) const {
		return Data_[index];
	}
	private:
	T* Data_ = 0;
	size_t ElementsCount_ = 0;
	size_t AllocatedElementsCount_ = 0;
};

#endif
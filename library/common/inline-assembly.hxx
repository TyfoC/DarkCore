#pragma once
#ifndef INLINE_ASSEMBLY_HXX
#define INLINE_ASSEMBLY_HXX

#include "typedefs.hxx"

#define INSERT_ASSEMBLY(...)						__asm__ __volatile__(__VA_ARGS__)

class InlineAssembly {
	public:
	enum RegisterFlagsBits {
		REG_FLAG_INTERRUPT_ENABLE =	0x0200,
	};

	enum RegisterCR0Bits {
		REG_CR0_PROTECTED_MODE =	0x01,
		REG_CR0_PAGING_ENABLE =		0x80000000
	};

	/**
	* @param port [in] port index
	* @param value [in] value
	*/
	static inline void SendPortByte(uint16_t port, uint8_t value) {
		INSERT_ASSEMBLY("outb %0, %1"::"a"(value), "d"(port));
	}

	/**
	 * @param port [in] port index
	 * @param value [in] value
	*/
	static inline void SendPortWord(uint16_t port, uint16_t value) {
		INSERT_ASSEMBLY("outw %0, %1"::"a"(value), "d"(port));
	}

	/**
	 * @param port [in] port index
	 * @param value [in] value
	*/
	static inline void SendPortLong(uint16_t port, uint32_t value) {
		INSERT_ASSEMBLY("outl %0, %1"::"a"(value), "d"(port));
	}

	/**
	 * @param port [in] port index
	 * @return readed value from port
	*/
	static inline uint8_t ReadPortByte(uint16_t port) {
		uint8_t value;
		INSERT_ASSEMBLY("inb %1, %0":"=a"(value):"d"(port));
		return value;
	}

	/**
	 * @param port [in] port index
	 * @return readed value from port
	*/
	static inline uint16_t ReadPortWord(uint16_t port) {
		uint16_t value;
		INSERT_ASSEMBLY("inw %1, %0":"=a"(value):"d"(port));
		return value;
	}

	/**
	 * @param port [in] port index
	 * @return readed value from port
	*/
	static inline uint32_t ReadPortLong(uint16_t port) {
		uint32_t value;
		INSERT_ASSEMBLY("inl %1, %0":"=a"(value):"d"(port));
		return value;
	}

	/**
	 * @param pointer [in] pointer to GDT register
	*/
	static inline void LoadGDTRegister(void* pointer) {
		INSERT_ASSEMBLY("lgdt %0"::"m"(*(psize_t)pointer));
	}

	/**
	 * @return pointer to GDT register
	*/
	static inline void* ReadGDTRegister() {
		void* pointer;
		INSERT_ASSEMBLY("sgdt %0":"=m"(pointer));
		return pointer;
	}

	/**
	 * @param pointer [in] pointer to IDT register
	*/
	static inline void LoadIDTRegister(void* pointer) {
		INSERT_ASSEMBLY("lidt %0"::"m"(*(psize_t)pointer));
	}

	/**
	 * @return pointer to IDT register
	*/
	static inline void* ReadIDTRegister() {
		void* pointer;
		INSERT_ASSEMBLY("sidt %0":"=m"(pointer));
		return pointer;
	}

	/**
	 * @param segmentValue [in] segment value
	*/
	static inline void WriteDataSegment(uint16_t segmentValue) {
		INSERT_ASSEMBLY("mov %0, %%ds"::"r"(segmentValue));
	}

	/**
	 * @param segmentValue [in] segment value
	*/
	static inline void WriteExtraSegment(uint16_t segmentValue) {
		INSERT_ASSEMBLY("mov %0, %%es"::"r"(segmentValue));
	}

	/**
	 * @param segmentValue [in] segment value
	*/
	static inline void WriteFSegment(uint16_t segmentValue) {
		INSERT_ASSEMBLY("mov %0, %%fs"::"r"(segmentValue));
	}

	/**
	 * @param segmentValue [in] segment value
	*/
	static inline void WriteGSegment(uint16_t segmentValue) {
		INSERT_ASSEMBLY("mov %0, %%gs"::"r"(segmentValue));
	}

	/**
	 * @param segmentValue [in] segment value
	*/
	static inline void WriteStackSegment(uint16_t segmentValue) {
		INSERT_ASSEMBLY("mov %0, %%ss"::"r"(segmentValue));
	}

	/**
	 * @return segment value
	*/
	static inline uint16_t ReadCodeSegment() {
		uint16_t segmentValue;
		INSERT_ASSEMBLY("mov %%cs, %0":"=r"(segmentValue));
		return segmentValue;
	}

	/**
	 * @return segment value
	*/
	static inline uint16_t ReadDataSegment() {
		uint16_t segmentValue;
		INSERT_ASSEMBLY("mov %%ds, %0":"=r"(segmentValue));
		return segmentValue;
	}

	/**
	 * @return segment value
	*/
	static inline uint16_t ReadExtraSegment() {
		uint16_t segmentValue;
		INSERT_ASSEMBLY("mov %%es, %0":"=r"(segmentValue));
		return segmentValue;
	}

	/**
	 * @return segment value
	*/
	static inline uint16_t ReadFSegment() {
		uint16_t segmentValue;
		INSERT_ASSEMBLY("mov %%fs, %0":"=r"(segmentValue));
		return segmentValue;
	}

	/**
	 * @return segment value
	*/
	static inline uint16_t ReadGSegment() {
		uint16_t segmentValue;
		INSERT_ASSEMBLY("mov %%gs, %0":"=r"(segmentValue));
		return segmentValue;
	}

	/**
	 * @return segment value
	*/
	static inline uint16_t ReadStackSegment() {
		uint16_t segmentValue;
		INSERT_ASSEMBLY("mov %%ss, %0":"=r"(segmentValue));
		return segmentValue;
	}

	/**
	 * @param registerValue [in] register value
	*/
	static inline void WriteFlagsRegister(size_t registerValue) {
		INSERT_ASSEMBLY("push %0; popf"::"r"(registerValue));
	}

	/**
	 * @return register value
	*/
	static inline size_t ReadFlagsRegister() {
		size_t registerValue;
		INSERT_ASSEMBLY("pushf; pop %0":"=r"(registerValue));
		return registerValue;
	}

	/**
	 *
	*/
	static inline void EnableIRQs() {
		INSERT_ASSEMBLY("sti");
	}

	/**
	 *
	*/
	static inline void DisableIRQs() {
		INSERT_ASSEMBLY("cli");
	}

	/**
	 * @param registerValue [in] register value
	*/
	static inline void WriteControlRegister0(size_t registerValue) {
		INSERT_ASSEMBLY("mov %0, %%cr0"::"r"(registerValue));
	}

	/**
	 * @param registerValue [in] register value
	*/
	static inline void WriteControlRegister3(size_t registerValue) {
		INSERT_ASSEMBLY("mov %0, %%cr3"::"r"(registerValue));
	}

	/**
	 * @param registerValue [in] register value
	*/
	static inline void WriteControlRegister4(size_t registerValue) {
		INSERT_ASSEMBLY("mov %0, %%cr4"::"r"(registerValue));
	}

	/**
	 * @return register value
	*/
	static inline size_t ReadControlRegister0() {
		size_t registerValue;
		INSERT_ASSEMBLY("mov %%cr0, %0":"=r"(registerValue));
		return registerValue;
	}

	/**
	 * @return register value
	*/
	static inline size_t ReadControlRegister2() {
		size_t registerValue;
		INSERT_ASSEMBLY("mov %%cr2, %0":"=r"(registerValue));
		return registerValue;
	}

	/**
	 * @return register value
	*/
	static inline size_t ReadControlRegister3() {
		size_t registerValue;
		INSERT_ASSEMBLY("mov %%cr3, %0":"=r"(registerValue));
		return registerValue;
	}

	/**
	 * @return register value
	*/
	static inline size_t ReadControlRegister4() {
		size_t registerValue;
		INSERT_ASSEMBLY("mov %%cr4, %0":"=r"(registerValue));
		return registerValue;
	}
};

#endif
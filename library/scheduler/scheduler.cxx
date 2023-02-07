#include "scheduler.hxx"

size_t TicksCount_ = 0;
static Scheduler::PThread Threads_ = 0;
static size_t ThreadsCount_ = 0, AllocatedThreadsCount_ = 0;
Scheduler::PThread CurrentThread_ = &Threads_[0];
size_t TaskSwitchingEnabled_ = true;

EXTERN_C void Schedule();
EXTERN_C void SwitchThreadNext();

void Scheduler::Run(IDT::PEntry table, VMM::PDirectory kernelDirectory) {
	TicksCount_ = VirtualTimer::GetTicksCount();
	VirtualTimer::SetFunctionSleep(Sleep);
	VirtualTimer::SetFunctionGetTicksCount(GetTicksCount);
	CurrentThread_ = &Threads_[0];

	const size_t pagesCount = AlignUp(AllocatedThreadsCount_ * sizeof(Thread), PMM::FrameSize) / PMM::FrameSize;
	VMM::MapPages(
		kernelDirectory,
		(size_t)Threads_ / PMM::FrameSize, (size_t)Threads_,
		pagesCount, VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE
	);
	table[32 + PIT::IRQIndex] = IDT::CreateEntry((size_t)Schedule, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	::SwitchThreadNext();
}

/**
 * After creating thread you still need to allocate pages!
*/
Scheduler::Thread Scheduler::CreateThread(bool suspended, size_t entryPoint, size_t stackSize, uint16_t dataSeg, uint16_t codeSeg, size_t flagsRegister) {
	Thread thread = {};

	thread.State = suspended ? STATE_SUSPENDED : STATE_READY_TO_RUN;
	thread.SleepTicksCount = 0;
	thread.TicksBeforeSwitchCount = Scheduler::TICKS_COUNT_USER_PRIORITY;
	thread.WorkedTicksCount = 0;

	bool pagingEnable = VMM::VirtualMemoryEnabled();
	if (pagingEnable) VMM::DisableVirtualMemory();
	thread.PagingDirectory = VMM::CreateDirectory();
	if (pagingEnable) VMM::EnableVirtualMemory();

	thread.Registers.AccumulatorRegister = 0;
	thread.Registers.CounterRegister = 0;
	thread.Registers.DataRegister = 0;
	thread.Registers.BaseRegister = 0;
	thread.Registers.StackPointer = (size_t)VMM::AllocateVirtualMemory(
		thread.PagingDirectory, stackSize,
		VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE | VMM::PAGE_FLAG_USER
	) + stackSize - 4;
	thread.Registers.BasePointer = thread.Registers.StackPointer;
	thread.Registers.SourceIndexRegister = 0;
	thread.Registers.DestinationIndexRegister = 0;
	thread.Registers.DataSegment = dataSeg;
	thread.Registers.ExtraSegment = dataSeg;
	thread.Registers.FSegment = dataSeg;
	thread.Registers.GSegment = dataSeg;
	thread.Registers.CodeSegment = codeSeg;
	thread.Registers.InstructionPointer = entryPoint;
	thread.Registers.FlagsRegister = flagsRegister;
	thread.Next = 0;
	thread.CurrentPathStringVirtualAddress = (size_t)VMM::AllocateVirtualMemory(thread.PagingDirectory, MaxPathLength, VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE);

	return thread;
}

void Scheduler::AddThread(const PThread thread) {
	bool pagingEnabled = VMM::VirtualMemoryEnabled();
	if (pagingEnabled) VMM::DisableVirtualMemory();

	if (ThreadsCount_ >= AllocatedThreadsCount_) {

		PThread nTCBs;
		if (!Threads_) nTCBs = (PThread)PMM::AllocatePhysicalMemory(sizeof(Thread) * CountAllocate);
		else nTCBs = (PThread)PMM::ReallocatePhysicalMemory(Threads_, sizeof(Thread) * (AllocatedThreadsCount_ + CountAllocate));

		if (!nTCBs) {
			if (pagingEnabled) VMM::EnableVirtualMemory();
			return;
		}

		Threads_ = nTCBs;
		AllocatedThreadsCount_ += CountAllocate;
	}

	Threads_[ThreadsCount_] = *thread;
	Threads_[ThreadsCount_].Next = &Threads_[0];
	if (ThreadsCount_) Threads_[ThreadsCount_ - 1].Next = &Threads_[ThreadsCount_];
	++ThreadsCount_;
	
	//	Map tasks to all page directories
	const size_t pagesCount = AlignUp(AllocatedThreadsCount_ * sizeof(Thread), PMM::FrameSize) / PMM::FrameSize;
	for (size_t i = 0; i < ThreadsCount_; i++) {
		VMM::MapPages(
			Threads_[i].PagingDirectory,
			(size_t)Threads_ / PMM::FrameSize, (size_t)Threads_,
			pagesCount, VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE
		);
	}

	if (pagingEnabled) VMM::EnableVirtualMemory();
}

void Scheduler::Sleep(size_t ticksCount) {
	CurrentThread_->State = STATE_SLEEPING;
	CurrentThread_->SleepTicksCount = ticksCount;
}

size_t Scheduler::GetTicksCount() {
	return TicksCount_;
}

void Scheduler::SwitchThreadNext() {
	::SwitchThreadNext();
}

Scheduler::PThread Scheduler::GetCurrentThread() {
	return CurrentThread_;
}

void Scheduler::RemoveCurrentThread() {
	INSERT_ASSEMBLY("cli");
	if ((size_t)CurrentThread_ > (size_t)&Threads_[0]) {
		size_t threadIndex = ((size_t)CurrentThread_ - (size_t)&Threads_[0]) / sizeof(Thread);
		ThreadsCount_ -= 1;
		for (size_t i = threadIndex; i < ThreadsCount_; i++) {
			Threads_[i] = Threads_[i + 1];
			Threads_[i].Next = &Threads_[i + 1];
		}
		Threads_[ThreadsCount_ - 1].Next = &Threads_[0];
		CurrentThread_ = &Threads_[0];
		INSERT_ASSEMBLY("sti");
		::SwitchThreadNext();
	}
	INSERT_ASSEMBLY("sti");
}

bool Scheduler::TaskSwitchingEnabled() {
	return TaskSwitchingEnabled_ == 1;
}

void Scheduler::DisableTaskSwitching() {
	TaskSwitchingEnabled_ = 0;
}

void Scheduler::EnableTaskSwitching() {
	TaskSwitchingEnabled_ = 1;
}
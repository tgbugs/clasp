
#include <sys/types.h>
#include <clasp/core/foundation.h>
#include <clasp/gctools/threadlocal.h>
#include <clasp/core/lisp.h>
#include <clasp/core/mpPackage.h>
#include <clasp/core/lispStream.h>

// from llvm/lib/Support/Unix/Threading.inc
#if defined(__linux__)
#include <sys/syscall.h> // For syscall codes
#include <unistd.h>      // For syscall()
#endif

uint64_t get_threadid() {
#if defined(__APPLE__)
  // Calling "mach_thread_self()" bumps the reference count on the thread
  // port, so we need to deallocate it. mach_task_self() doesn't bump the ref
  // count.
  thread_port_t Self = mach_thread_self();
  mach_port_deallocate(mach_task_self(), Self);
  return Self;
#elif defined(__FreeBSD__)
  return uint64_t(pthread_getthreadid_np());
#elif defined(__NetBSD__)
  return uint64_t(_lwp_self());
#elif defined(__ANDROID__)
  return uint64_t(gettid());
#elif defined(__linux__)
  return uint64_t(syscall(SYS_gettid));
#elif defined(LLVM_ON_WIN32)
  return uint64_t(::GetCurrentThreadId());
#else
  return uint64_t(pthread_self());
#endif
}


THREAD_LOCAL core::ThreadLocalState* my_thread;

namespace core {
unsigned int *BignumExportBuffer::getOrAllocate(const mpz_class &bignum, int nail) {
  size_t size = _lisp->integer_ordering()._mpz_import_size;
  size_t numb = (size << 3) - nail; // *8
  size_t count = (mpz_sizeinbase(bignum.get_mpz_t(), 2) + numb - 1) / numb;
  size_t bytes = count * size;
  if (bytes > this->bufferSize) {
    if (this->buffer) {
      free(this->buffer);
    }
    this->buffer = (unsigned int *)malloc(bytes);
  }
  return this->buffer;
};





};


namespace core {



ThreadLocalState::ThreadLocalState(void* stack_top) :  _DisableInterrupts(false), _StackTop(stack_top), _PendingInterrupts(_Nil<core::T_O>()) {
  my_thread = this;
#ifdef _TARGET_OS_DARWIN
  this->_Tid = 0;
#else
  this->_Tid = get_threadid();
#endif
  this->_InvocationHistoryStackTop = NULL;
  this->_BufferStr8NsPool.reset_(); // Can't use _Nil<core::T_O>(); - too early
  this->_BufferStrWNsPool.reset_();
}

void ThreadLocalState::initialize_thread(mp::Process_sp process) {
//  printf("%s:%d Initialize all ThreadLocalState things this->%p\n",__FILE__, __LINE__, (void*)this);
  this->_Bindings.reserve(1024);
  this->_Process = process;
  process->_ThreadInfo = this;
  this->_BFormatStringOutputStream = clasp_make_string_output_stream();
  this->_BignumRegister0 = Bignum_O::create( (gc::Fixnum) 0);
  this->_BignumRegister1 = Bignum_O::create( (gc::Fixnum) 0);
  this->_BignumRegister2 = Bignum_O::create( (gc::Fixnum) 0);
#if 1
  this->_SingleDispatchMethodCachePtr = gc::GC<Cache_O>::allocate();
  this->_SingleDispatchMethodCachePtr->setup(2, Lisp_O::SingleDispatchMethodCacheSize);
  this->_MethodCachePtr = gctools::GC<Cache_O>::allocate();
  this->_MethodCachePtr->setup(Lisp_O::MaxFunctionArguments, Lisp_O::ClosCacheSize);
  this->_SlotCachePtr = gctools::GC<Cache_O>::allocate();
  this->_SlotCachePtr->setup(Lisp_O::MaxClosSlots, Lisp_O::ClosCacheSize);
#endif
  this->_PendingInterrupts = _Nil<T_O>();
  this->_SparePendingInterruptRecords = cl__make_list(clasp_make_fixnum(16),_Nil<T_O>());
};

};

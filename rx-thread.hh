#pragma once

// Kernel threads
#if !defined(__STDC_NO_THREADS__)
  #include <thread>
  namespace rx {
  using thread = std::thread;
  using thread_id = std::thread::id;
    namespace this_thread {
      using std::this_thread::get_id;
      // std::thread::id std::this_thread::get_id();
    } // namespace this_thread
    using std::thread;
  } // namespace rx
#else
  #error "No C++11 thread implementation"
#endif // !defined(__STDC_NO_THREADS__)


// Thread-local storage
//
// Example:
//   RX_THREAD_LOCAL(Foo*, bar, NULL)
// Causes the following functions to be defined:
//   Foo* bar_get(); // defaults to NULL
//   Foo* bar_set(Foo*);
//
#if defined(__GNUC__) /*|| __has_feature(cxx_thread_local)*/
  #define RX_THREAD_LOCAL(T, name, default_value) \
    static __thread T name##_value = default_value; \
    inline T name##_set(T p) { return name##_value = p; } \
    inline T name##_get() { return name##_value; }
//#elif __cplusplus >= 201100L
// DISABLED: spotty compiler support
//   #define RX_THREAD_LOCAL(T, name, default_value) \
//     static thread_local T name##_value = default_value; \
//     inline T name##_set(T p) { return name##_value = p; } \
//     inline T name##_get() { return name##_value; }
#elif defined(_WIN32)
  #error "TODO: Windows thread-local storage"
  // See http://msdn.microsoft.com/en-us/library/windows/desktop/ms686991(v=vs.85).aspx
#else
  #ifndef _MULTI_THREADED
  #define _MULTI_THREADED
  #endif
  #include <pthread.h>
  #define RX_THREAD_LOCAL(T, name, default_value) \
    static pthread_key_t name##_key = 0; \
    struct name##_tls_initializer_st { \
      name##_tls_initializer_st() { \
        pthread_key_create(&(name##_key), default_value); \
      } \
    } name##_tls_initializer_; \
    inline T name##_set(T p) { pthread_setspecific(name##_key, (void*)p); return p; } \
    inline T name##_get() { return (T)pthread_getspecific(name##_key); }
#endif

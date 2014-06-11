# rx

Common C and C++11 utilities used in a variety of projects.


## Usage and API reference

Include the `rx.h` file where you want to use the fundational parts of rx. This includes:

- Shorthand type definitions (i8, u32, u64, etc)
- Includes a few common libc
- Build target information — implemented in `rx-target.h`
- Atomic operations, like CAS — implemented in `rx-atomic.h`
- C++11 automatic reference-counted objects — implemented in `rx-ref.hh`
- A memory efficient C++11 function container "rx::func" (API equivalent to std::function) — implemented in `rx-func.hh`

### Shorthand type definitions

- `i8`  — signed 8-bit integer
- `u8`  — unsigned 8-bit integer
- `i16` — signed 16-bit integer
- `u16` — unsigned 16-bit integer
- `i32` — signed 32-bit integer
- `u32` — unsigned 32-bit integer
- `i64` — signed 64-bit integer
- `u64` — unsigned 64-bit integer

### Build target information

- `#define RX_TARGET_ARCH_X86`          — truthy integer value if the target architecture is 32-bit x86
- `#define RX_TARGET_ARCH_X64`          — truthy integer value if the target architecture is 64-bit x86
- `#define RX_TARGET_ARCH_ARM`          — truthy integer value if the target architecture is ARM
- `#define RX_TARGET_ARCH_NAME`         — c-string of the architecture's name (e.g. `"x64"`)
- `#define RX_TARGET_ARCH_SIZE`         — integer value of the architecture's register size in bits (e.g. `64`)
- `#define RX_TARGET_ARCH_LE`           — truthy integer value if the target architecture uses little-endian byte-order
- `#define RX_TARGET_OS_WINDOWS`        — truthy integer value if the target OS is some version of Microsoft Windows
- `#define RX_TARGET_OS_POSIX`          — truthy integer value if the target OS is posix-compliant (Linux, Darwin, etc)
- `#define RX_TARGET_OS_LINUX`          — truthy integer value if the target OS is some version of Linux
- `#define RX_TARGET_OS_DARWIN`         — truthy integer value if the target OS is some version of Darwin (OS X, iOS, et al)
- `#define RX_TARGET_OS_OSX`            — truthy integer value if the target OS is some version of Apple OS X
- `#define RX_TARGET_OS_IOS`            — truthy integer value if the target OS is some version of Apple iOS
- `#define RX_TARGET_OS_IOS_SIMULATOR`  — truthy integer value if the target OS is Apple iOS Simulator
- `#define RX_TARGET_OS_BSD`            — truthy integer value if the target OS is some version of BSD (not including Darwin)
- `#define RX_TARGET_OS_UNKNOWN`        — truthy integer value if the target OS is unknown
- `#define RX_TARGET_OS_NAME`           — c-string of the OS's name (e.g. `"osx"`)

### Atomic operations

```cc
T rx_atomic_swap(T *ptr, T value);
  // Atomically swap integers or pointers in memory. Note that this is more than just CAS.
  // E.g: int old_value = rx_atomic_swap(&value, new_value);

void rx_atomic_add32(i32* operand, i32 delta);
  // Increment a 32-bit integer `operand` by `delta`. There's no return value.

T rx_atomic_add_fetch(T* operand, T delta);
  // Add `delta` to `operand` and return the resulting value of `operand`

T rx_atomic_sub_fetch(T* operand, T delta);
  // Subtract `delta` from `operand` and return the resulting value of `operand`

bool rx_atomic_cas_bool(T* ptr, T oldval, T newval);
  // If the current value of *ptr is oldval, then write newval into *ptr. Returns true if the
  // operation was successful and newval was written.

T rx_atomic_cas(T* ptr, T oldval, T newval);
  // If the current value of *ptr is oldval, then write newval into *ptr. Returns the contents of
  // *ptr before the operation.
```

### C++11 automatic reference-counted objects

- `typedef rx::refcount_t` — reference count value type
- `struct rx::ref_counted` — struct members needed for a vtable implementation
- `struct rx::ref_counted_novtable` — struct members needed for a non-vtable implementation
- `#define RX_REF_COUNT_INIT` — constant initializer for a countable `rx::refcount_t` value
- `#define RX_REF_COUNT_INIT` — constant initializer for a constant (never moving) `rx::refcount_t` value, used for objects that should never be deallocated from reference counting.
- `void rx::refcount_retain(volatile rx::refcount_t&)` — increment a counter
- `bool rx::refcount_release(volatile rx::refcount_t&)` — decrement a counter. Returns true if the counter reached `0`.
- `#define RX_REF_MIXIN(T)` — make a struct a ref object including a vtable, providing the struct's type with `T` (see example below)
- `#define RX_REF_MIXIN_NOVTABLE(T)` — make a struct a ref object without a vtable, providing the struct's type with `T` (see example below)
- `#define RX_REF_MIXIN_IMPL_VTABLE(T, O, I)` — see rx-ref.hh
- `#define RX_REF_MIXIN_IMPL_NOVTABLE(T, O, I)` — see rx-ref.hh

Example:

```cc
// foo.hh:
#include "rx.h"
struct foo {
  foo(const char* name);
  const char* name() const;
  RX_REF_MIXIN(foo)
};

// foo.cc:
struct foo::Imp : rx::ref_counted {
  const char* name;
};
foo::foo(const char* name) : self{new Imp{name}} {}
const char* foo::name() const { return self->name; }

// main.cc:
foo make_furball(bool b) {
  return foo{b};
}
int main() {
  foo cat; // here, cat == nullptr
  cat = make_furball("Busta Rhymes");  // here, cat != nullptr
  std::cout << cat.name() << "\n";
  return 0;
} // <- `cat` deallocated here
```

Example without using a vtable:

```cc
// foo.hh:
#include "rx.h"
struct foo {
  foo(const char* name);
  const char* name() const;
  RX_REF_MIXIN_NOVTABLE(foo)
};

// foo.cc:
struct foo::Imp : rx::ref_counted_novtable {
  const char* name;
};
foo::foo(const char* name) : self(new Imp{name}) {}
const char* foo::name() const { return self->name; }
void foo::__dealloc(Imp* p) { delete p; } // needed b/c we have no vtable
```

> Note: Make sure to use `RX_REF_MIXIN_NOVTABLE` only when there's no vtable, or the struct will have a—to rx::ref—misaligned header which will cause you all kinds of funky memory-related bugs. When in doubt, use `RX_REF_MIXIN`.



## Optional "add-ons"

### rx::Status

A universal status/error type which has a very low cost when there's no error (`rs::Status::OK()`).

- Defined by: `rx-status.hh`
- Requires: `rx.h`
- When representing "no error" (rx::Status::OK()) the code generated is simply a nullptr_t.
- When representing an error, a single memory allocation is incured at construction which itself
  represents both the error code (first byte) as well as any message (remaining bytes) terminated
  by a NUL sentinel byte.

```cc
struct rx::Status {
  typedef uint8_t Code;

  static Status OK();
  Status() noexcept;
  Status(std::nullptr_t) noexcept;
    // == OK (no error)

  Status(Code);
  Status(Code, const std::string& error_message);
  Status(const std::string& error_message);

  bool ok() const; // true when empty/OK/no error
  Code code() const; // 
  const char* message() const;

  // copyable, moveable, copy-assignable, move-assignable
}
```


### rx::thread

Platform-independent kernel thread interface

- Defined by: `rx-thread.hh`
- Requires: `rx.h`, C++11 `<thread>` implementation

> TODO: Documentation



## Using rx without C++ standard library

When used with C++11 without likning with the C++ standard library, you should define `RX_NO_STDLIBCXX` to a truthy value.

    #define RX_NO_STDLIBCXX 1

i.e.

    c++ -DRX_NO_STDLIBCXX=1 ...

Note that in this case you still need to link with libc as the `new` and `delete` keywords provided by rx in this case depend on malloc and free.


## MIT License

Copyright (c) 2012-2014 Rasmus Andersson <http://rsms.me/>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

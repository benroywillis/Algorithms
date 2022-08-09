#include <iostream>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

extern "C" {
int64_t halide_current_time_ns(void *ctx);
void halide_profiler_pipeline_end(void *, void *);
}

#ifdef _WIN32
__declspec(dllimport) float __cdecl roundf(float);
__declspec(dllimport) double __cdecl round(double);
#else
inline float asinh_f32(float x) {return asinhf(x);}
inline float acosh_f32(float x) {return acoshf(x);}
inline float atanh_f32(float x) {return atanhf(x);}
inline double asinh_f64(double x) {return asinh(x);}
inline double acosh_f64(double x) {return acosh(x);}
inline double atanh_f64(double x) {return atanh(x);}
#endif
inline float sqrt_f32(float x) {return sqrtf(x);}
inline float sin_f32(float x) {return sinf(x);}
inline float asin_f32(float x) {return asinf(x);}
inline float cos_f32(float x) {return cosf(x);}
inline float acos_f32(float x) {return acosf(x);}
inline float tan_f32(float x) {return tanf(x);}
inline float atan_f32(float x) {return atanf(x);}
inline float atan2_f32(float x, float y) {return atan2f(x, y);}
inline float sinh_f32(float x) {return sinhf(x);}
inline float cosh_f32(float x) {return coshf(x);}
inline float tanh_f32(float x) {return tanhf(x);}
inline float hypot_f32(float x, float y) {return hypotf(x, y);}
inline float exp_f32(float x) {return expf(x);}
inline float log_f32(float x) {return logf(x);}
inline float pow_f32(float x, float y) {return powf(x, y);}
inline float floor_f32(float x) {return floorf(x);}
inline float ceil_f32(float x) {return ceilf(x);}
inline float round_f32(float x) {return roundf(x);}

inline double sqrt_f64(double x) {return sqrt(x);}
inline double sin_f64(double x) {return sin(x);}
inline double asin_f64(double x) {return asin(x);}
inline double cos_f64(double x) {return cos(x);}
inline double acos_f64(double x) {return acos(x);}
inline double tan_f64(double x) {return tan(x);}
inline double atan_f64(double x) {return atan(x);}
inline double atan2_f64(double x, double y) {return atan2(x, y);}
inline double sinh_f64(double x) {return sinh(x);}
inline double cosh_f64(double x) {return cosh(x);}
inline double tanh_f64(double x) {return tanh(x);}
inline double hypot_f64(double x, double y) {return hypot(x, y);}
inline double exp_f64(double x) {return exp(x);}
inline double log_f64(double x) {return log(x);}
inline double pow_f64(double x, double y) {return pow(x, y);}
inline double floor_f64(double x) {return floor(x);}
inline double ceil_f64(double x) {return ceil(x);}
inline double round_f64(double x) {return round(x);}

inline float nan_f32() {return NAN;}
inline float neg_inf_f32() {return -INFINITY;}
inline float inf_f32() {return INFINITY;}
inline bool is_nan_f32(float x) {return x != x;}
inline bool is_nan_f64(double x) {return x != x;}

template<typename A, typename B>
inline A reinterpret(const B &b) {
    #if __cplusplus >= 201103L
    static_assert(sizeof(A) == sizeof(B), "type size mismatch");
    #endif
    A a;
    memcpy(&a, &b, sizeof(a));
    return a;
}
inline float float_from_bits(uint32_t bits) {
    return reinterpret<float, uint32_t>(bits);
}

template<typename T>
inline int halide_popcount(T a) {
    int bits_set = 0;
    while (a != 0) {
        bits_set += a & 1;
        a >>= 1;
    }
    return bits_set;
}

template<typename T>
inline int halide_count_leading_zeros(T a) {
    int leading_zeros = 0;
    int bit = sizeof(a) * 8 - 1;
    while (bit >= 0 && (a & (((T)1) << bit)) == 0) {
        leading_zeros++;
        bit--;
    }
    return leading_zeros;
}

template<typename T>
inline int halide_count_trailing_zeros(T a) {
    int trailing_zeros = 0;
    constexpr int bits = sizeof(a) * 8;
    int bit = 0;
    while (bit < bits && (a & (((T)1) << bit)) == 0) {
        trailing_zeros++;
        bit++;
    }
    return trailing_zeros;
}

template<typename T>
inline T halide_cpp_max(const T &a, const T &b) {return (a > b) ? a : b;}

template<typename T>
inline T halide_cpp_min(const T &a, const T &b) {return (a < b) ? a : b;}

template<typename A, typename B>
const B &return_second(const A &a, const B &b) {
    (void) a;
    return b;
}

template<typename A, typename B>
inline auto quiet_div(const A &a, const B &b) -> decltype(a / b) {
    return b == 0 ? static_cast<decltype(a / b)>(0) : (a / b);
}

template<typename A, typename B>
inline auto quiet_mod(const A &a, const B &b) -> decltype(a % b) {
    return b == 0 ? static_cast<decltype(a % b)>(0) : (a % b);
}

namespace {
class HalideFreeHelper {
    typedef void (*FreeFunction)(void *user_context, void *p);
    void * user_context;
    void *p;
    FreeFunction free_function;
public:
    HalideFreeHelper(void *user_context, void *p, FreeFunction free_function)
        : user_context(user_context), p(p), free_function(free_function) {}
    ~HalideFreeHelper() { free(); }
    void free() {
        if (p) {
            // TODO: do all free_functions guarantee to ignore a nullptr?
            free_function(user_context, p);
            p = nullptr;
        }
    }
};
} // namespace
#ifndef HALIDE_HALIDERUNTIME_H
#define HALIDE_HALIDERUNTIME_H

#ifndef COMPILING_HALIDE_RUNTIME
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#else
#include "runtime_internal.h"
#endif

#ifdef __cplusplus
// Forward declare type to allow naming typed handles.
// See Type.h for documentation.
template<typename T> struct halide_handle_traits;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Note that you should not use "inline" along with HALIDE_ALWAYS_INLINE;
// it is not necessary, and may produce warnings for some build configurations.
#ifdef _MSC_VER
#define HALIDE_ALWAYS_INLINE __forceinline
#define HALIDE_NEVER_INLINE __declspec(noinline)
#else
#define HALIDE_ALWAYS_INLINE __attribute__((always_inline)) inline
#define HALIDE_NEVER_INLINE __attribute__((noinline))
#endif

/** \file
 *
 * This file declares the routines used by Halide internally in its
 * runtime. On platforms that support weak linking, these can be
 * replaced with user-defined versions by defining an extern "C"
 * function with the same name and signature.
 *
 * When doing Just In Time (JIT) compilation methods on the Func being
 * compiled must be called instead. The corresponding methods are
 * documented below.
 *
 * All of these functions take a "void *user_context" parameter as their
 * first argument; if the Halide kernel that calls back to any of these
 * functions has been compiled with the UserContext feature set on its Target,
 * then the value of that pointer passed from the code that calls the
 * Halide kernel is piped through to the function.
 *
 * Some of these are also useful to call when using the default
 * implementation. E.g. halide_shutdown_thread_pool.
 *
 * Note that even on platforms with weak linking, some linker setups
 * may not respect the override you provide. E.g. if the override is
 * in a shared library and the halide object files are linked directly
 * into the output, the builtin versions of the runtime functions will
 * be called. See your linker documentation for more details. On
 * Linux, LD_DYNAMIC_WEAK=1 may help.
 *
 */

// Forward-declare to suppress warnings if compiling as C.
struct halide_buffer_t;
struct buffer_t;

/** Print a message to stderr. Main use is to support tracing
 * functionality, print, and print_when calls. Also called by the default
 * halide_error.  This function can be replaced in JITed code by using
 * halide_custom_print and providing an implementation of halide_print
 * in AOT code. See Func::set_custom_print.
 */
// @{
extern void halide_print(void *user_context, const char *);
extern void halide_default_print(void *user_context, const char *);
typedef void (*halide_print_t)(void *, const char *);
extern halide_print_t halide_set_custom_print(halide_print_t print);
// @}

/** Halide calls this function on runtime errors (for example bounds
 * checking failures). This function can be replaced in JITed code by
 * using Func::set_error_handler, or in AOT code by calling
 * halide_set_error_handler. In AOT code on platforms that support
 * weak linking (i.e. not Windows), you can also override it by simply
 * defining your own halide_error.
 */
// @{
extern void halide_error(void *user_context, const char *);
extern void halide_default_error(void *user_context, const char *);
typedef void (*halide_error_handler_t)(void *, const char *);
extern halide_error_handler_t halide_set_error_handler(halide_error_handler_t handler);
// @}

/** Cross-platform mutex. Must be initialized with zero and implementation
 * must treat zero as an unlocked mutex with no waiters, etc.
 */
struct halide_mutex {
    uintptr_t _private[1];
};

/** Cross platform condition variable. Must be initialized to 0. */
struct halide_cond {
    uintptr_t _private[1];
};

/** A basic set of mutex and condition variable functions, which call
 * platform specific code for mutual exclusion. Equivalent to posix
 * calls. */
//@{
extern void halide_mutex_lock(struct halide_mutex *mutex);
extern void halide_mutex_unlock(struct halide_mutex *mutex);
extern void halide_cond_signal(struct halide_cond *cond);
extern void halide_cond_broadcast(struct halide_cond *cond);
extern void halide_cond_signal(struct halide_cond *cond);
extern void halide_cond_wait(struct halide_cond *cond, struct halide_mutex *mutex);
//@}

/** Define halide_do_par_for to replace the default thread pool
 * implementation. halide_shutdown_thread_pool can also be called to
 * release resources used by the default thread pool on platforms
 * where it makes sense. (E.g. On Mac OS, Grand Central Dispatch is
 * used so %Halide does not own the threads backing the pool and they
 * cannot be released.)  See Func::set_custom_do_task and
 * Func::set_custom_do_par_for. Should return zero if all the jobs
 * return zero, or an arbitrarily chosen return value from one of the
 * jobs otherwise.
 */
//@{
typedef int (*halide_task_t)(void *user_context, int task_number, uint8_t *closure);
extern int halide_do_par_for(void *user_context,
                             halide_task_t task,
                             int min, int size, uint8_t *closure);
extern void halide_shutdown_thread_pool();
//@}

/** Set a custom method for performing a parallel for loop. Returns
 * the old do_par_for handler. */
typedef int (*halide_do_par_for_t)(void *, halide_task_t, int, int, uint8_t*);
extern halide_do_par_for_t halide_set_custom_do_par_for(halide_do_par_for_t do_par_for);

/** An opaque struct representing a semaphore. Used by the task system for async tasks. */
struct halide_semaphore_t {
    uint64_t _private[2];
};

/** A struct representing a semaphore and a number of items that must
 * be acquired from it. Used in halide_parallel_task_t below. */
struct halide_semaphore_acquire_t {
    struct halide_semaphore_t *semaphore;
    int count;
};
extern int halide_semaphore_init(struct halide_semaphore_t *, int n);
extern int halide_semaphore_release(struct halide_semaphore_t *, int n);
extern bool halide_semaphore_try_acquire(struct halide_semaphore_t *, int n);
typedef int (*halide_semaphore_init_t)(struct halide_semaphore_t *, int);
typedef int (*halide_semaphore_release_t)(struct halide_semaphore_t *, int);
typedef bool (*halide_semaphore_try_acquire_t)(struct halide_semaphore_t *, int);


/** A task representing a serial for loop evaluated over some range.
 * Note that task_parent is a pass through argument that should be
 * passed to any dependent taks that are invokved using halide_do_parallel_tasks
 * underneath this call. */
typedef int (*halide_loop_task_t)(void *user_context, int min, int extent,
                                  uint8_t *closure, void *task_parent);

/** A parallel task to be passed to halide_do_parallel_tasks. This
 * task may recursively call halide_do_parallel_tasks, and there may
 * be complex dependencies between seemingly unrelated tasks expressed
 * using semaphores. If you are using a custom task system, care must
 * be taken to avoid potential deadlock. This can be done by carefully
 * respecting the static metadata at the end of the task struct.*/
struct halide_parallel_task_t {
    // The function to call. It takes a user context, a min and
    // extent, a closure, and a task system pass through argument.
    halide_loop_task_t fn;

    // The closure to pass it
    uint8_t *closure;

    // The name of the function to be called. For debugging purposes only.
    const char *name;

    // An array of semaphores that must be acquired before the
    // function is called. Must be reacquired for every call made.
    struct halide_semaphore_acquire_t *semaphores;
    int num_semaphores;

    // The entire range the function should be called over. This range
    // may be sliced up and the function called multiple times.
    int min, extent;

    // A parallel task provides several pieces of metadata to prevent
    // unbounded resource usage or deadlock.

    // The first is the minimum number of execution contexts (call
    // stacks or threads) necessary for the function to run to
    // completion. This may be greater than one when there is nested
    // parallelism with internal producer-consumer relationships
    // (calling the function recursively spawns and blocks on parallel
    // sub-tasks that communicate with each other via semaphores). If
    // a parallel runtime calls the function when fewer than this many
    // threads are idle, it may need to create more threads to
    // complete the task, or else risk deadlock due to committing all
    // threads to tasks that cannot complete without more.
    //
    // FIXME: Note that extern stages are assumed to only require a
    // single thread to complete. If the extern stage is itself a
    // Halide pipeline, this may be an underestimate.
    int min_threads;

    // The calls to the function should be in serial order from min to min+extent-1, with only
    // one executing at a time. If false, any order is fine, and
    // concurrency is fine.
    bool serial;
};

/** Enqueue some number of the tasks described above and wait for them
 * to complete. While waiting, the calling threads assists with either
 * the tasks enqueued, or other non-blocking tasks in the task
 * system. Note that task_parent should be NULL for top-level calls
 * and the pass through argument if this call is being made from
 * another task. */
extern int halide_do_parallel_tasks(void *user_context, int num_tasks,
                                    struct halide_parallel_task_t *tasks,
                                    void *task_parent);

/** If you use the default do_par_for, you can still set a custom
 * handler to perform each individual task. Returns the old handler. */
//@{
typedef int (*halide_do_task_t)(void *, halide_task_t, int, uint8_t *);
extern halide_do_task_t halide_set_custom_do_task(halide_do_task_t do_task);
extern int halide_do_task(void *user_context, halide_task_t f, int idx,
                          uint8_t *closure);
//@}

/** The version of do_task called for loop tasks. By default calls the
 * loop task with the same arguments. */
// @{
  typedef int (*halide_do_loop_task_t)(void *, halide_loop_task_t, int, int, uint8_t *, void *);
extern halide_do_loop_task_t halide_set_custom_do_loop_task(halide_do_loop_task_t do_task);
extern int halide_do_loop_task(void *user_context, halide_loop_task_t f, int min, int extent,
                               uint8_t *closure, void *task_parent);
//@}

/** Provide an entire custom tasking runtime via function
 * pointers. Note that do_task and semaphore_try_acquire are only ever
 * called by halide_default_do_par_for and
 * halide_default_do_parallel_tasks, so it's only necessary to provide
 * those if you are mixing in the default implementations of
 * do_par_for and do_parallel_tasks. */
// @{
typedef int (*halide_do_parallel_tasks_t)(void *, int, struct halide_parallel_task_t *,
                                          void *task_parent);
extern void halide_set_custom_parallel_runtime(
    halide_do_par_for_t,
    halide_do_task_t,
    halide_do_loop_task_t,
    halide_do_parallel_tasks_t,
    halide_semaphore_init_t,
    halide_semaphore_try_acquire_t,
    halide_semaphore_release_t
    );
// @}

/** The default versions of the parallel runtime functions. */
// @{
extern int halide_default_do_par_for(void *user_context,
                                     halide_task_t task,
                                     int min, int size, uint8_t *closure);
extern int halide_default_do_parallel_tasks(void *user_context,
                                            int num_tasks,
                                            struct halide_parallel_task_t *tasks,
                                            void *task_parent);
extern int halide_default_do_task(void *user_context, halide_task_t f, int idx,
                                  uint8_t *closure);
extern int halide_default_do_loop_task(void *user_context, halide_loop_task_t f,
                                       int min, int extent,
                                       uint8_t *closure, void *task_parent);
extern int halide_default_semaphore_init(struct halide_semaphore_t *, int n);
extern int halide_default_semaphore_release(struct halide_semaphore_t *, int n);
extern bool halide_default_semaphore_try_acquire(struct halide_semaphore_t *, int n);
// @}

struct halide_thread;

/** Spawn a thread. Returns a handle to the thread for the purposes of
 * joining it. The thread must be joined in order to clean up any
 * resources associated with it. */
extern struct halide_thread *halide_spawn_thread(void (*f)(void *), void *closure);

/** Join a thread. */
extern void halide_join_thread(struct halide_thread *);

/** Set the number of threads used by Halide's thread pool. Returns
 * the old number.
 *
 * n < 0  : error condition
 * n == 0 : use a reasonable system default (typically, number of cpus online).
 * n == 1 : use exactly one thread; this will always enforce serial execution
 * n > 1  : use a pool of exactly n threads.
 *
 * (Note that this is only guaranteed when using the default implementations
 * of halide_do_par_for(); custom implementations may completely ignore values
 * passed to halide_set_num_threads().)
 */
extern int halide_set_num_threads(int n);

/** Halide calls these functions to allocate and free memory. To
 * replace in AOT code, use the halide_set_custom_malloc and
 * halide_set_custom_free, or (on platforms that support weak
 * linking), simply define these functions yourself. In JIT-compiled
 * code use Func::set_custom_allocator.
 *
 * If you override them, and find yourself wanting to call the default
 * implementation from within your override, use
 * halide_default_malloc/free.
 *
 * Note that halide_malloc must return a pointer aligned to the
 * maximum meaningful alignment for the platform for the purpose of
 * vector loads and stores. The default implementation uses 32-byte
 * alignment, which is safe for arm and x86. Additionally, it must be
 * safe to read at least 8 bytes before the start and beyond the
 * end.
 */
//@{
extern void *halide_malloc(void *user_context, size_t x);
extern void halide_free(void *user_context, void *ptr);
extern void *halide_default_malloc(void *user_context, size_t x);
extern void halide_default_free(void *user_context, void *ptr);
typedef void *(*halide_malloc_t)(void *, size_t);
typedef void (*halide_free_t)(void *, void *);
extern halide_malloc_t halide_set_custom_malloc(halide_malloc_t user_malloc);
extern halide_free_t halide_set_custom_free(halide_free_t user_free);
//@}

/** Halide calls these functions to interact with the underlying
 * system runtime functions. To replace in AOT code on platforms that
 * support weak linking, define these functions yourself, or use
 * the halide_set_custom_load_library() and halide_set_custom_get_library_symbol()
 * functions. In JIT-compiled code, use JITSharedRuntime::set_default_handlers().
 *
 * halide_load_library and halide_get_library_symbol are equivalent to
 * dlopen and dlsym. halide_get_symbol(sym) is equivalent to
 * dlsym(RTLD_DEFAULT, sym).
 */
//@{
extern void *halide_get_symbol(const char *name);
extern void *halide_load_library(const char *name);
extern void *halide_get_library_symbol(void *lib, const char *name);
extern void *halide_default_get_symbol(const char *name);
extern void *halide_default_load_library(const char *name);
extern void *halide_default_get_library_symbol(void *lib, const char *name);
typedef void *(*halide_get_symbol_t)(const char *name);
typedef void *(*halide_load_library_t)(const char *name);
typedef void *(*halide_get_library_symbol_t)(void *lib, const char *name);
extern halide_get_symbol_t halide_set_custom_get_symbol(halide_get_symbol_t user_get_symbol);
extern halide_load_library_t halide_set_custom_load_library(halide_load_library_t user_load_library);
extern halide_get_library_symbol_t halide_set_custom_get_library_symbol(halide_get_library_symbol_t user_get_library_symbol);
//@}

/** Called when debug_to_file is used inside %Halide code.  See
 * Func::debug_to_file for how this is called
 *
 * Cannot be replaced in JITted code at present.
 */
extern int32_t halide_debug_to_file(void *user_context, const char *filename,
                                    int32_t type_code,
                                    struct halide_buffer_t *buf);

/** Types in the halide type system. They can be ints, unsigned ints,
 * or floats (of various bit-widths), or a handle (which is always 64-bits).
 * Note that the int/uint/float values do not imply a specific bit width
 * (the bit width is expected to be encoded in a separate value).
 */
typedef enum halide_type_code_t
#if __cplusplus >= 201103L
: uint8_t
#endif
{
    halide_type_int = 0,   //!< signed integers
    halide_type_uint = 1,  //!< unsigned integers
    halide_type_float = 2, //!< IEEE floating point numbers
    halide_type_handle = 3, //!< opaque pointer type (void *)
    halide_type_bfloat = 4, //!< floating point numbers in the bfloat format
} halide_type_code_t;

// Note that while __attribute__ can go before or after the declaration,
// __declspec apparently is only allowed before.
#ifndef HALIDE_ATTRIBUTE_ALIGN
    #ifdef _MSC_VER
        #define HALIDE_ATTRIBUTE_ALIGN(x) __declspec(align(x))
    #else
        #define HALIDE_ATTRIBUTE_ALIGN(x) __attribute__((aligned(x)))
    #endif
#endif

/** A runtime tag for a type in the halide type system. Can be ints,
 * unsigned ints, or floats of various bit-widths (the 'bits'
 * field). Can also be vectors of the same (by setting the 'lanes'
 * field to something larger than one). This struct should be
 * exactly 32-bits in size. */
struct halide_type_t {
    /** The basic type code: signed integer, unsigned integer, or floating point. */
#if __cplusplus >= 201103L
    HALIDE_ATTRIBUTE_ALIGN(1) halide_type_code_t code; // halide_type_code_t
#else
    HALIDE_ATTRIBUTE_ALIGN(1) uint8_t code; // halide_type_code_t
#endif

    /** The number of bits of precision of a single scalar value of this type. */
    HALIDE_ATTRIBUTE_ALIGN(1) uint8_t bits;

    /** How many elements in a vector. This is 1 for scalar types. */
    HALIDE_ATTRIBUTE_ALIGN(2) uint16_t lanes;

#ifdef __cplusplus
    /** Construct a runtime representation of a Halide type from:
     * code: The fundamental type from an enum.
     * bits: The bit size of one element.
     * lanes: The number of vector elements in the type. */
    HALIDE_ALWAYS_INLINE halide_type_t(halide_type_code_t code, uint8_t bits, uint16_t lanes = 1)
        : code(code), bits(bits), lanes(lanes) {
    }

    /** Default constructor is required e.g. to declare halide_trace_event
     * instances. */
    HALIDE_ALWAYS_INLINE halide_type_t() : code((halide_type_code_t)0), bits(0), lanes(0) {}

    /** Compare two types for equality. */
    HALIDE_ALWAYS_INLINE bool operator==(const halide_type_t &other) const {
        return as_u32() == other.as_u32();
    }

    HALIDE_ALWAYS_INLINE bool operator!=(const halide_type_t &other) const {
        return !(*this == other);
    }

    HALIDE_ALWAYS_INLINE bool operator<(const halide_type_t &other) const {
        return as_u32() < other.as_u32();
    }

    /** Size in bytes for a single element, even if width is not 1, of this type. */
    HALIDE_ALWAYS_INLINE int bytes() const { return (bits + 7) / 8; }

    HALIDE_ALWAYS_INLINE uint32_t as_u32() const {
        uint32_t u;
        memcpy(&u, this, sizeof(u));
        return u;
    }
#endif
};

enum halide_trace_event_code_t {halide_trace_load = 0,
                                halide_trace_store = 1,
                                halide_trace_begin_realization = 2,
                                halide_trace_end_realization = 3,
                                halide_trace_produce = 4,
                                halide_trace_end_produce = 5,
                                halide_trace_consume = 6,
                                halide_trace_end_consume = 7,
                                halide_trace_begin_pipeline = 8,
                                halide_trace_end_pipeline = 9,
                                halide_trace_tag = 10 };

struct halide_trace_event_t {
    /** The name of the Func or Pipeline that this event refers to */
    const char *func;

    /** If the event type is a load or a store, this points to the
     * value being loaded or stored. Use the type field to safely cast
     * this to a concrete pointer type and retrieve it. For other
     * events this is null. */
    void *value;

    /** For loads and stores, an array which contains the location
     * being accessed. For vector loads or stores it is an array of
     * vectors of coordinates (the vector dimension is innermost).
     *
     * For realization or production-related events, this will contain
     * the mins and extents of the region being accessed, in the order
     * min0, extent0, min1, extent1, ...
     *
     * For pipeline-related events, this will be null.
     */
    int32_t *coordinates;

    /** For halide_trace_tag, this points to a read-only null-terminated string
     * of arbitrary text. For all other events, this will be null.
     */
    const char *trace_tag;

    /** If the event type is a load or a store, this is the type of
     * the data. Otherwise, the value is meaningless. */
    struct halide_type_t type;

    /** The type of event */
    enum halide_trace_event_code_t event;

    /* The ID of the parent event (see below for an explanation of
     * event ancestry). */
    int32_t parent_id;

    /** If this was a load or store of a Tuple-valued Func, this is
     * which tuple element was accessed. */
    int32_t value_index;

    /** The length of the coordinates array */
    int32_t dimensions;

#ifdef __cplusplus
    // If we don't explicitly mark the default ctor as inline,
    // certain build configurations can fail (notably iOS)
    HALIDE_ALWAYS_INLINE halide_trace_event_t() {}
#endif
};

/** Called when Funcs are marked as trace_load, trace_store, or
 * trace_realization. See Func::set_custom_trace. The default
 * implementation either prints events via halide_print, or if
 * HL_TRACE_FILE is defined, dumps the trace to that file in a
 * sequence of trace packets. The header for a trace packet is defined
 * below. If the trace is going to be large, you may want to make the
 * file a named pipe, and then read from that pipe into gzip.
 *
 * halide_trace returns a unique ID which will be passed to future
 * events that "belong" to the earlier event as the parent id. The
 * ownership hierarchy looks like:
 *
 * begin_pipeline
 * +--trace_tag (if any)
 * +--trace_tag (if any)
 * ...
 * +--begin_realization
 * |  +--produce
 * |  |  +--load/store
 * |  |  +--end_produce
 * |  +--consume
 * |  |  +--load
 * |  |  +--end_consume
 * |  +--end_realization
 * +--end_pipeline
 *
 * Threading means that ownership cannot be inferred from the ordering
 * of events. There can be many active realizations of a given
 * function, or many active productions for a single
 * realization. Within a single production, the ordering of events is
 * meaningful.
 *
 * Note that all trace_tag events (if any) will occur just after the begin_pipeline
 * event, but before any begin_realization events. All trace_tags for a given Func
 * will be emitted in the order added.
 */
// @}
extern int32_t halide_trace(void *user_context, const struct halide_trace_event_t *event);
extern int32_t halide_default_trace(void *user_context, const struct halide_trace_event_t *event);
typedef int32_t (*halide_trace_t)(void *user_context, const struct halide_trace_event_t *);
extern halide_trace_t halide_set_custom_trace(halide_trace_t trace);
// @}

/** The header of a packet in a binary trace. All fields are 32-bit. */
struct halide_trace_packet_t {
    /** The total size of this packet in bytes. Always a multiple of
     * four. Equivalently, the number of bytes until the next
     * packet. */
    uint32_t size;

    /** The id of this packet (for the purpose of parent_id). */
    int32_t id;

    /** The remaining fields are equivalent to those in halide_trace_event_t */
    // @{
    struct halide_type_t type;
    enum halide_trace_event_code_t event;
    int32_t parent_id;
    int32_t value_index;
    int32_t dimensions;
    // @}

    #ifdef __cplusplus
    // If we don't explicitly mark the default ctor as inline,
    // certain build configurations can fail (notably iOS)
    HALIDE_ALWAYS_INLINE halide_trace_packet_t() {}

    /** Get the coordinates array, assuming this packet is laid out in
     * memory as it was written. The coordinates array comes
     * immediately after the packet header. */
    HALIDE_ALWAYS_INLINE const int *coordinates() const {
        return (const int *)(this + 1);
    }

    HALIDE_ALWAYS_INLINE int *coordinates() {
        return (int *)(this + 1);
    }

    /** Get the value, assuming this packet is laid out in memory as
     * it was written. The packet comes immediately after the coordinates
     * array. */
    HALIDE_ALWAYS_INLINE const void *value() const {
        return (const void *)(coordinates() + dimensions);
    }

    HALIDE_ALWAYS_INLINE void *value() {
        return (void *)(coordinates() + dimensions);
    }

    /** Get the func name, assuming this packet is laid out in memory
     * as it was written. It comes after the value. */
    HALIDE_ALWAYS_INLINE const char *func() const {
        return (const char *)value() + type.lanes * type.bytes();
    }

    HALIDE_ALWAYS_INLINE char *func() {
        return (char *)value() + type.lanes * type.bytes();
    }

    /** Get the trace_tag (if any), assuming this packet is laid out in memory
     * as it was written. It comes after the func name. If there is no trace_tag,
     * this will return a pointer to an empty string. */
    HALIDE_ALWAYS_INLINE const char *trace_tag() const {
        const char *f = func();
        // strlen may not be available here
        while (*f++) {
            // nothing
        }
        return f;
    }

    HALIDE_ALWAYS_INLINE char *trace_tag() {
        char *f = func();
        // strlen may not be available here
        while (*f++) {
            // nothing
        }
        return f;
    }
    #endif
};



/** Set the file descriptor that Halide should write binary trace
 * events to. If called with 0 as the argument, Halide outputs trace
 * information to stdout in a human-readable format. If never called,
 * Halide checks the for existence of an environment variable called
 * HL_TRACE_FILE and opens that file. If HL_TRACE_FILE is not defined,
 * it outputs trace information to stdout in a human-readable
 * format. */
extern void halide_set_trace_file(int fd);

/** Halide calls this to retrieve the file descriptor to write binary
 * trace events to. The default implementation returns the value set
 * by halide_set_trace_file. Implement it yourself if you wish to use
 * a custom file descriptor per user_context. Return zero from your
 * implementation to tell Halide to print human-readable trace
 * information to stdout. */
extern int halide_get_trace_file(void *user_context);

/** If tracing is writing to a file. This call closes that file
 * (flushing the trace). Returns zero on success. */
extern int halide_shutdown_trace();

/** All Halide GPU or device backend implementations provide an
 * interface to be used with halide_device_malloc, etc. This is
 * accessed via the functions below.
 */

/** An opaque struct containing per-GPU API implementations of the
 * device functions. */
struct halide_device_interface_impl_t;

/** Each GPU API provides a halide_device_interface_t struct pointing
 * to the code that manages device allocations. You can access these
 * functions directly from the struct member function pointers, or by
 * calling the functions declared below. Note that the global
 * functions are not available when using Halide as a JIT compiler.
 * If you are using raw halide_buffer_t in that context you must use
 * the function pointers in the device_interface struct.
 *
 * The function pointers below are currently the same for every GPU
 * API; only the impl field varies. These top-level functions do the
 * bookkeeping that is common across all GPU APIs, and then dispatch
 * to more API-specific functions via another set of function pointers
 * hidden inside the impl field.
 */
struct halide_device_interface_t {
    int (*device_malloc)(void *user_context, struct halide_buffer_t *buf,
                         const struct halide_device_interface_t *device_interface);
    int (*device_free)(void *user_context, struct halide_buffer_t *buf);
    int (*device_sync)(void *user_context, struct halide_buffer_t *buf);
    void (*device_release)(void *user_context,
                          const struct halide_device_interface_t *device_interface);
    int (*copy_to_host)(void *user_context, struct halide_buffer_t *buf);
    int (*copy_to_device)(void *user_context, struct halide_buffer_t *buf,
                          const struct halide_device_interface_t *device_interface);
    int (*device_and_host_malloc)(void *user_context, struct halide_buffer_t *buf,
                                  const struct halide_device_interface_t *device_interface);
    int (*device_and_host_free)(void *user_context, struct halide_buffer_t *buf);
    int (*buffer_copy)(void *user_context, struct halide_buffer_t *src,
                       const struct halide_device_interface_t *dst_device_interface, struct halide_buffer_t *dst);
    int (*device_crop)(void *user_context, const struct halide_buffer_t *src,
                       struct halide_buffer_t *dst);
    int (*device_slice)(void *user_context, const struct halide_buffer_t *src,
                        int slice_dim, int slice_pos, struct halide_buffer_t *dst);
    int (*device_release_crop)(void *user_context, struct halide_buffer_t *buf);
    int (*wrap_native)(void *user_context, struct halide_buffer_t *buf, uint64_t handle,
                       const struct halide_device_interface_t *device_interface);
    int (*detach_native)(void *user_context, struct halide_buffer_t *buf);
    int (*compute_capability)(void *user_context, int *major, int *minor);
    const struct halide_device_interface_impl_t *impl;
};

/** Release all data associated with the given device interface, in
 * particular all resources (memory, texture, context handles)
 * allocated by Halide. Must be called explicitly when using AOT
 * compilation. This is *not* thread-safe with respect to actively
 * running Halide code. Ensure all pipelines are finished before
 * calling this. */
extern void halide_device_release(void *user_context,
                                  const struct halide_device_interface_t *device_interface);

/** Copy image data from device memory to host memory. This must be called
 * explicitly to copy back the results of a GPU-based filter. */
extern int halide_copy_to_host(void *user_context, struct halide_buffer_t *buf);

/** Copy image data from host memory to device memory. This should not
 * be called directly; Halide handles copying to the device
 * automatically.  If interface is NULL and the buf has a non-zero dev
 * field, the device associated with the dev handle will be
 * used. Otherwise if the dev field is 0 and interface is NULL, an
 * error is returned. */
extern int halide_copy_to_device(void *user_context, struct halide_buffer_t *buf,
                                 const struct halide_device_interface_t *device_interface);

/** Copy data from one buffer to another. The buffers may have
 * different shapes and sizes, but the destination buffer's shape must
 * be contained within the source buffer's shape. That is, for each
 * dimension, the min on the destination buffer must be greater than
 * or equal to the min on the source buffer, and min+extent on the
 * destination buffer must be less that or equal to min+extent on the
 * source buffer. The source data is pulled from either device or
 * host memory on the source, depending on the dirty flags. host is
 * preferred if both are valid. The dst_device_interface parameter
 * controls the destination memory space. NULL means host memory. */
extern int halide_buffer_copy(void *user_context, struct halide_buffer_t *src,
                              const struct halide_device_interface_t *dst_device_interface,
                              struct halide_buffer_t *dst);

/** Give the destination buffer a device allocation which is an alias
 * for the same coordinate range in the source buffer. Modifies the
 * device, device_interface, and the device_dirty flag only. Only
 * supported by some device APIs (others will return
 * halide_error_code_device_crop_unsupported). Call
 * halide_device_release_crop instead of halide_device_free to clean
 * up resources associated with the cropped view. Do not free the
 * device allocation on the source buffer while the destination buffer
 * still lives. Note that the two buffers do not share dirty flags, so
 * care must be taken to update them together as needed. Note that src
 * and dst are required to have the same number of dimensions.
 *
 * Note also that (in theory) device interfaces which support cropping may
 * still not support cropping a crop (instead, create a new crop of the parent
 * buffer); in practice, no known implementation has this limitation, although
 * it is possible that some future implementations may require it. */
extern int halide_device_crop(void *user_context,
                              const struct halide_buffer_t *src,
                              struct halide_buffer_t *dst);

/** Give the destination buffer a device allocation which is an alias
 * for a similar coordinate range in the source buffer, but with one dimension
 * sliced away in the dst. Modifies the device, device_interface, and the
 * device_dirty flag only. Only supported by some device APIs (others will return
 * halide_error_code_device_crop_unsupported). Call
 * halide_device_release_crop instead of halide_device_free to clean
 * up resources associated with the sliced view. Do not free the
 * device allocation on the source buffer while the destination buffer
 * still lives. Note that the two buffers do not share dirty flags, so
 * care must be taken to update them together as needed. Note that the dst buffer
 * must have exactly one fewer dimension than the src buffer, and that slice_dim
 * and slice_pos must be valid within src. */
extern int halide_device_slice(void *user_context,
                               const struct halide_buffer_t *src,
                               int slice_dim, int slice_pos,
                               struct halide_buffer_t *dst);

/** Release any resources associated with a cropped/sliced view of another
 * buffer. */
extern int halide_device_release_crop(void *user_context,
                                      struct halide_buffer_t *buf);

/** Wait for current GPU operations to complete. Calling this explicitly
 * should rarely be necessary, except maybe for profiling. */
extern int halide_device_sync(void *user_context, struct halide_buffer_t *buf);

/** Allocate device memory to back a halide_buffer_t. */
extern int halide_device_malloc(void *user_context, struct halide_buffer_t *buf,
                                const struct halide_device_interface_t *device_interface);

/** Free device memory. */
extern int halide_device_free(void *user_context, struct halide_buffer_t *buf);

/** Wrap or detach a native device handle, setting the device field
 * and device_interface field as appropriate for the given GPU
 * API. The meaning of the opaque handle is specific to the device
 * interface, so if you know the device interface in use, call the
 * more specific functions in the runtime headers for your specific
 * device API instead (e.g. HalideRuntimeCuda.h). */
// @{
extern int halide_device_wrap_native(void *user_context,
                                     struct halide_buffer_t *buf,
                                     uint64_t handle,
                                     const struct halide_device_interface_t *device_interface);
extern int halide_device_detach_native(void *user_context, struct halide_buffer_t *buf);
// @}

/** Versions of the above functions that accept legacy buffer_t structs. */
// @{
extern int halide_copy_to_host_legacy(void *user_context, struct buffer_t *buf);
extern int halide_copy_to_device_legacy(void *user_context, struct buffer_t *buf,
                                 const struct halide_device_interface_t *device_interface);
extern int halide_device_sync_legacy(void *user_context, struct buffer_t *buf);
extern int halide_device_malloc_legacy(void *user_context, struct buffer_t *buf,
                                const struct halide_device_interface_t *device_interface);
extern int halide_device_free_legacy(void *user_context, struct buffer_t *buf);
// @}

/** Selects which gpu device to use. 0 is usually the display
 * device. If never called, Halide uses the environment variable
 * HL_GPU_DEVICE. If that variable is unset, Halide uses the last
 * device. Set this to -1 to use the last device. */
extern void halide_set_gpu_device(int n);

/** Halide calls this to get the desired halide gpu device
 * setting. Implement this yourself to use a different gpu device per
 * user_context. The default implementation returns the value set by
 * halide_set_gpu_device, or the environment variable
 * HL_GPU_DEVICE. */
extern int halide_get_gpu_device(void *user_context);

/** Set the soft maximum amount of memory, in bytes, that the LRU
 *  cache will use to memoize Func results.  This is not a strict
 *  maximum in that concurrency and simultaneous use of memoized
 *  reults larger than the cache size can both cause it to
 *  temporariliy be larger than the size specified here.
 */
extern void halide_memoization_cache_set_size(int64_t size);

/** Given a cache key for a memoized result, currently constructed
 *  from the Func name and top-level Func name plus the arguments of
 *  the computation, determine if the result is in the cache and
 *  return it if so. (The internals of the cache key should be
 *  considered opaque by this function.) If this routine returns true,
 *  it is a cache miss. Otherwise, it will return false and the
 *  buffers passed in will be filled, via copying, with memoized
 *  data. The last argument is a list if halide_buffer_t pointers which
 *  represents the outputs of the memoized Func. If the Func does not
 *  return a Tuple, there will only be one halide_buffer_t in the list. The
 *  tuple_count parameters determines the length of the list.
 *
 * The return values are:
 * -1: Signals an error.
 *  0: Success and cache hit.
 *  1: Success and cache miss.
 */
extern int halide_memoization_cache_lookup(void *user_context, const uint8_t *cache_key, int32_t size,
                                           struct halide_buffer_t *realized_bounds,
                                           int32_t tuple_count, struct halide_buffer_t **tuple_buffers);

/** Given a cache key for a memoized result, currently constructed
 *  from the Func name and top-level Func name plus the arguments of
 *  the computation, store the result in the cache for futre access by
 *  halide_memoization_cache_lookup. (The internals of the cache key
 *  should be considered opaque by this function.) Data is copied out
 *  from the inputs and inputs are unmodified. The last argument is a
 *  list if halide_buffer_t pointers which represents the outputs of the
 *  memoized Func. If the Func does not return a Tuple, there will
 *  only be one halide_buffer_t in the list. The tuple_count parameters
 *  determines the length of the list.
 *
 * If there is a memory allocation failure, the store does not store
 * the data into the cache.
 */
extern int halide_memoization_cache_store(void *user_context, const uint8_t *cache_key, int32_t size,
                                          struct halide_buffer_t *realized_bounds,
                                          int32_t tuple_count,
                                          struct halide_buffer_t **tuple_buffers);

/** If halide_memoization_cache_lookup succeeds,
 * halide_memoization_cache_release must be called to signal the
 * storage is no longer being used by the caller. It will be passed
 * the host pointer of one the buffers returned by
 * halide_memoization_cache_lookup. That is
 * halide_memoization_cache_release will be called multiple times for
 * the case where halide_memoization_cache_lookup is handling multiple
 * buffers.  (This corresponds to memoizing a Tuple in Halide.) Note
 * that the host pointer must be sufficient to get to all information
 * the relase operation needs. The default Halide cache impleemntation
 * accomplishes this by storing extra data before the start of the user
 * modifiable host storage.
 *
 * This call is like free and does not have a failure return.
  */
extern void halide_memoization_cache_release(void *user_context, void *host);

/** Free all memory and resources associated with the memoization cache.
 * Must be called at a time when no other threads are accessing the cache.
 */
extern void halide_memoization_cache_cleanup();

/** Annotate that a given range of memory has been initialized;
 * only used when Target::MSAN is enabled.
 *
 * The default implementation uses the LLVM-provided AnnotateMemoryIsInitialized() function.
 */
extern int halide_msan_annotate_memory_is_initialized(void *user_context, const void *ptr, uint64_t len);

/** Mark the data pointed to by the buffer_t as initialized (but *not* the buffer_t itself),
 * using halide_msan_annotate_memory_is_initialized() for marking.
 *
 * The default implementation takes pains to only mark the active memory ranges
 * (skipping padding), and sorting into ranges to always mark the smallest number of
 * ranges, in monotonically increasing memory order.
 *
 * Most client code should never need to replace the default implementation.
 */
extern int halide_msan_annotate_buffer_is_initialized(void *user_context, struct halide_buffer_t *buffer);
extern void halide_msan_annotate_buffer_is_initialized_as_destructor(void *user_context, void *buffer);

/** The error codes that may be returned by a Halide pipeline. */
enum halide_error_code_t {
    /** There was no error. This is the value returned by Halide on success. */
    halide_error_code_success = 0,

    /** An uncategorized error occurred. Refer to the string passed to halide_error. */
    halide_error_code_generic_error = -1,

    /** A Func was given an explicit bound via Func::bound, but this
     * was not large enough to encompass the region that is used of
     * the Func by the rest of the pipeline. */
    halide_error_code_explicit_bounds_too_small = -2,

    /** The elem_size field of a halide_buffer_t does not match the size in
     * bytes of the type of that ImageParam. Probable type mismatch. */
    halide_error_code_bad_type = -3,

    /** A pipeline would access memory outside of the halide_buffer_t passed
     * in. */
    halide_error_code_access_out_of_bounds = -4,

    /** A halide_buffer_t was given that spans more than 2GB of memory. */
    halide_error_code_buffer_allocation_too_large = -5,

    /** A halide_buffer_t was given with extents that multiply to a number
     * greater than 2^31-1 */
    halide_error_code_buffer_extents_too_large = -6,

    /** Applying explicit constraints on the size of an input or
     * output buffer shrank the size of that buffer below what will be
     * accessed by the pipeline. */
    halide_error_code_constraints_make_required_region_smaller = -7,

    /** A constraint on a size or stride of an input or output buffer
     * was not met by the halide_buffer_t passed in. */
    halide_error_code_constraint_violated = -8,

    /** A scalar parameter passed in was smaller than its minimum
     * declared value. */
    halide_error_code_param_too_small = -9,

    /** A scalar parameter passed in was greater than its minimum
     * declared value. */
    halide_error_code_param_too_large = -10,

    /** A call to halide_malloc returned NULL. */
    halide_error_code_out_of_memory = -11,

    /** A halide_buffer_t pointer passed in was NULL. */
    halide_error_code_buffer_argument_is_null = -12,

    /** debug_to_file failed to open or write to the specified
     * file. */
    halide_error_code_debug_to_file_failed = -13,

    /** The Halide runtime encountered an error while trying to copy
     * from device to host. Turn on -debug in your target string to
     * see more details. */
    halide_error_code_copy_to_host_failed = -14,

    /** The Halide runtime encountered an error while trying to copy
     * from host to device. Turn on -debug in your target string to
     * see more details. */
    halide_error_code_copy_to_device_failed = -15,

    /** The Halide runtime encountered an error while trying to
     * allocate memory on device. Turn on -debug in your target string
     * to see more details. */
    halide_error_code_device_malloc_failed = -16,

    /** The Halide runtime encountered an error while trying to
     * synchronize with a device. Turn on -debug in your target string
     * to see more details. */
    halide_error_code_device_sync_failed = -17,

    /** The Halide runtime encountered an error while trying to free a
     * device allocation. Turn on -debug in your target string to see
     * more details. */
    halide_error_code_device_free_failed = -18,

    /** Buffer has a non-zero device but no device interface, which
     * violates a Halide invariant. */
    halide_error_code_no_device_interface = -19,

    /** An error occurred when attempting to initialize the Matlab
     * runtime. */
    halide_error_code_matlab_init_failed = -20,

    /** The type of an mxArray did not match the expected type. */
    halide_error_code_matlab_bad_param_type = -21,

    /** There is a bug in the Halide compiler. */
    halide_error_code_internal_error = -22,

    /** The Halide runtime encountered an error while trying to launch
     * a GPU kernel. Turn on -debug in your target string to see more
     * details. */
    halide_error_code_device_run_failed = -23,

    /** The Halide runtime encountered a host pointer that violated
     * the alignment set for it by way of a call to
     * set_host_alignment */
    halide_error_code_unaligned_host_ptr = -24,

    /** A fold_storage directive was used on a dimension that is not
     * accessed in a monotonically increasing or decreasing fashion. */
    halide_error_code_bad_fold = -25,

    /** A fold_storage directive was used with a fold factor that was
     * too small to store all the values of a producer needed by the
     * consumer. */
    halide_error_code_fold_factor_too_small = -26,

    /** User-specified require() expression was not satisfied. */
    halide_error_code_requirement_failed = -27,

    /** At least one of the buffer's extents are negative. */
    halide_error_code_buffer_extents_negative = -28,

    /** A compiled pipeline was passed the old deprecated buffer_t
     * struct, and it could not be upgraded to a halide_buffer_t. */
    halide_error_code_failed_to_upgrade_buffer_t = -29,

    /** A compiled pipeline was passed the old deprecated buffer_t
     * struct in bounds inference mode, but the returned information
     * can't be expressed in the old buffer_t. */
    halide_error_code_failed_to_downgrade_buffer_t = -30,

    /** A specialize_fail() schedule branch was selected at runtime. */
    halide_error_code_specialize_fail = -31,

    /** The Halide runtime encountered an error while trying to wrap a
     * native device handle.  Turn on -debug in your target string to
     * see more details. */
    halide_error_code_device_wrap_native_failed = -32,

    /** The Halide runtime encountered an error while trying to detach
     * a native device handle.  Turn on -debug in your target string
     * to see more details. */
    halide_error_code_device_detach_native_failed = -33,

    /** The host field on an input or output was null, the device
     * field was not zero, and the pipeline tries to use the buffer on
     * the host. You may be passing a GPU-only buffer to a pipeline
     * which is scheduled to use it on the CPU. */
    halide_error_code_host_is_null = -34,

    /** A folded buffer was passed to an extern stage, but the region
     * touched wraps around the fold boundary. */
    halide_error_code_bad_extern_fold = -35,

    /** Buffer has a non-null device_interface but device is 0, which
     * violates a Halide invariant. */
    halide_error_code_device_interface_no_device= -36,

    /** Buffer has both host and device dirty bits set, which violates
     * a Halide invariant. */
    halide_error_code_host_and_device_dirty = -37,

    /** The halide_buffer_t * passed to a halide runtime routine is
     * nullptr and this is not allowed. */
    halide_error_code_buffer_is_null = -38,

    /** The Halide runtime encountered an error while trying to copy
     * from one buffer to another. Turn on -debug in your target
     * string to see more details. */
    halide_error_code_device_buffer_copy_failed = -39,

    /** Attempted to make cropped/sliced alias of a buffer with a device
     * field, but the device_interface does not support cropping. */
    halide_error_code_device_crop_unsupported = -40,

    /** Cropping/slicing a buffer failed for some other reason. Turn on -debug
     * in your target string. */
    halide_error_code_device_crop_failed = -41,

    /** An operation on a buffer required an allocation on a
     * particular device interface, but a device allocation already
     * existed on a different device interface. Free the old one
     * first. */
    halide_error_code_incompatible_device_interface = -42,

    /** The dimensions field of a halide_buffer_t does not match the dimensions of that ImageParam. */
    halide_error_code_bad_dimensions = -43,

    /** An expression that would perform an integer division or modulo
     * by zero was evaluated. */
    halide_error_code_integer_division_by_zero = -44,

};

/** Halide calls the functions below on various error conditions. The
 * default implementations construct an error message, call
 * halide_error, then return the matching error code above. On
 * platforms that support weak linking, you can override these to
 * catch the errors individually. */

/** A call into an extern stage for the purposes of bounds inference
 * failed. Returns the error code given by the extern stage. */
extern int halide_error_bounds_inference_call_failed(void *user_context, const char *extern_stage_name, int result);

/** A call to an extern stage failed. Returned the error code given by
 * the extern stage. */
extern int halide_error_extern_stage_failed(void *user_context, const char *extern_stage_name, int result);

/** Various other error conditions. See the enum above for a
 * description of each. */
// @{
extern int halide_error_explicit_bounds_too_small(void *user_context, const char *func_name, const char *var_name,
                                                      int min_bound, int max_bound, int min_required, int max_required);
extern int halide_error_bad_type(void *user_context, const char *func_name,
                                 uint32_t type_given, uint32_t correct_type); // N.B. The last two args are the bit representation of a halide_type_t
extern int halide_error_bad_dimensions(void *user_context, const char *func_name,
                                       int32_t dimensions_given, int32_t correct_dimensions);
extern int halide_error_access_out_of_bounds(void *user_context, const char *func_name,
                                             int dimension, int min_touched, int max_touched,
                                             int min_valid, int max_valid);
extern int halide_error_buffer_allocation_too_large(void *user_context, const char *buffer_name,
                                                    uint64_t allocation_size, uint64_t max_size);
extern int halide_error_buffer_extents_negative(void *user_context, const char *buffer_name, int dimension, int extent);
extern int halide_error_buffer_extents_too_large(void *user_context, const char *buffer_name,
                                                 int64_t actual_size, int64_t max_size);
extern int halide_error_constraints_make_required_region_smaller(void *user_context, const char *buffer_name,
                                                                 int dimension,
                                                                 int constrained_min, int constrained_extent,
                                                                 int required_min, int required_extent);
extern int halide_error_constraint_violated(void *user_context, const char *var, int val,
                                            const char *constrained_var, int constrained_val);
extern int halide_error_param_too_small_i64(void *user_context, const char *param_name,
                                            int64_t val, int64_t min_val);
extern int halide_error_param_too_small_u64(void *user_context, const char *param_name,
                                            uint64_t val, uint64_t min_val);
extern int halide_error_param_too_small_f64(void *user_context, const char *param_name,
                                            double val, double min_val);
extern int halide_error_param_too_large_i64(void *user_context, const char *param_name,
                                            int64_t val, int64_t max_val);
extern int halide_error_param_too_large_u64(void *user_context, const char *param_name,
                                            uint64_t val, uint64_t max_val);
extern int halide_error_param_too_large_f64(void *user_context, const char *param_name,
                                            double val, double max_val);
extern int halide_error_out_of_memory(void *user_context);
extern int halide_error_buffer_argument_is_null(void *user_context, const char *buffer_name);
extern int halide_error_debug_to_file_failed(void *user_context, const char *func,
                                             const char *filename, int error_code);
extern int halide_error_unaligned_host_ptr(void *user_context, const char *func_name, int alignment);
extern int halide_error_host_is_null(void *user_context, const char *func_name);
extern int halide_error_failed_to_upgrade_buffer_t(void *user_context,
                                                   const char *input_name,
                                                   const char *reason);
extern int halide_error_failed_to_downgrade_buffer_t(void *user_context,
                                                     const char *input_name,
                                                     const char *reason);
extern int halide_error_bad_fold(void *user_context, const char *func_name, const char *var_name,
                                 const char *loop_name);
extern int halide_error_bad_extern_fold(void *user_context, const char *func_name,
                                        int dim, int min, int extent, int valid_min, int fold_factor);

extern int halide_error_fold_factor_too_small(void *user_context, const char *func_name, const char *var_name,
                                              int fold_factor, const char *loop_name, int required_extent);
extern int halide_error_requirement_failed(void *user_context, const char *condition, const char *message);
extern int halide_error_specialize_fail(void *user_context, const char *message);
extern int halide_error_no_device_interface(void *user_context);
extern int halide_error_device_interface_no_device(void *user_context);
extern int halide_error_host_and_device_dirty(void *user_context);
extern int halide_error_buffer_is_null(void *user_context, const char *routine);
extern int halide_error_integer_division_by_zero(void *user_context);
// @}

/** Optional features a compilation Target can have.
 * Be sure to keep this in sync with the Feature enum in Target.h and the implementation of
 * get_runtime_compatible_target in Target.cpp if you add a new feature.
 */
typedef enum halide_target_feature_t {
    halide_target_feature_jit = 0,  ///< Generate code that will run immediately inside the calling process.
    halide_target_feature_debug,  ///< Turn on debug info and output for runtime code.
    halide_target_feature_no_asserts,  ///< Disable all runtime checks, for slightly tighter code.
    halide_target_feature_no_bounds_query, ///< Disable the bounds querying functionality.

    halide_target_feature_sse41,  ///< Use SSE 4.1 and earlier instructions. Only relevant on x86.
    halide_target_feature_avx,  ///< Use AVX 1 instructions. Only relevant on x86.
    halide_target_feature_avx2,  ///< Use AVX 2 instructions. Only relevant on x86.
    halide_target_feature_fma,  ///< Enable x86 FMA instruction
    halide_target_feature_fma4,  ///< Enable x86 (AMD) FMA4 instruction set
    halide_target_feature_f16c,  ///< Enable x86 16-bit float support

    halide_target_feature_armv7s,  ///< Generate code for ARMv7s. Only relevant for 32-bit ARM.
    halide_target_feature_no_neon,  ///< Avoid using NEON instructions. Only relevant for 32-bit ARM.

    halide_target_feature_vsx,  ///< Use VSX instructions. Only relevant on POWERPC.
    halide_target_feature_power_arch_2_07,  ///< Use POWER ISA 2.07 new instructions. Only relevant on POWERPC.

    halide_target_feature_cuda,  ///< Enable the CUDA runtime. Defaults to compute capability 2.0 (Fermi)
    halide_target_feature_cuda_capability30,  ///< Enable CUDA compute capability 3.0 (Kepler)
    halide_target_feature_cuda_capability32,  ///< Enable CUDA compute capability 3.2 (Tegra K1)
    halide_target_feature_cuda_capability35,  ///< Enable CUDA compute capability 3.5 (Kepler)
    halide_target_feature_cuda_capability50,  ///< Enable CUDA compute capability 5.0 (Maxwell)

    halide_target_feature_opencl,  ///< Enable the OpenCL runtime.
    halide_target_feature_cl_doubles,  ///< Enable double support on OpenCL targets

    halide_target_feature_opengl,  ///< Enable the OpenGL runtime.
    halide_target_feature_openglcompute, ///< Enable OpenGL Compute runtime.

    halide_target_feature_user_context,  ///< Generated code takes a user_context pointer as first argument

    halide_target_feature_matlab,  ///< Generate a mexFunction compatible with Matlab mex libraries. See tools/mex_halide.m.

    halide_target_feature_profile, ///< Launch a sampling profiler alongside the Halide pipeline that monitors and reports the runtime used by each Func
    halide_target_feature_no_runtime, ///< Do not include a copy of the Halide runtime in any generated object file or assembly

    halide_target_feature_metal, ///< Enable the (Apple) Metal runtime.
    halide_target_feature_mingw, ///< For Windows compile to MinGW toolset rather then Visual Studio

    halide_target_feature_c_plus_plus_mangling, ///< Generate C++ mangled names for result function, et al

    halide_target_feature_large_buffers, ///< Enable 64-bit buffer indexing to support buffers > 2GB. Ignored if bits != 64.

    halide_target_feature_hvx_64, ///< Enable HVX 64 byte mode.
    halide_target_feature_hvx_128, ///< Enable HVX 128 byte mode.
    halide_target_feature_hvx_v62, ///< Enable Hexagon v62 architecture.
    halide_target_feature_fuzz_float_stores, ///< On every floating point store, set the last bit of the mantissa to zero. Pipelines for which the output is very different with this feature enabled may also produce very different output on different processors.
    halide_target_feature_soft_float_abi, ///< Enable soft float ABI. This only enables the soft float ABI calling convention, which does not necessarily use soft floats.
    halide_target_feature_msan, ///< Enable hooks for MSAN support.
    halide_target_feature_avx512, ///< Enable the base AVX512 subset supported by all AVX512 architectures. The specific feature sets are AVX-512F and AVX512-CD. See https://en.wikipedia.org/wiki/AVX-512 for a description of each AVX subset.
    halide_target_feature_avx512_knl, ///< Enable the AVX512 features supported by Knight's Landing chips, such as the Xeon Phi x200. This includes the base AVX512 set, and also AVX512-CD and AVX512-ER.
    halide_target_feature_avx512_skylake, ///< Enable the AVX512 features supported by Skylake Xeon server processors. This adds AVX512-VL, AVX512-BW, and AVX512-DQ to the base set. The main difference from the base AVX512 set is better support for small integer ops. Note that this does not include the Knight's Landing features. Note also that these features are not available on Skylake desktop and mobile processors.
    halide_target_feature_avx512_cannonlake, ///< Enable the AVX512 features expected to be supported by future Cannonlake processors. This includes all of the Skylake features, plus AVX512-IFMA and AVX512-VBMI.
    halide_target_feature_hvx_use_shared_object, ///< Deprecated
    halide_target_feature_trace_loads, ///< Trace all loads done by the pipeline. Equivalent to calling Func::trace_loads on every non-inlined Func.
    halide_target_feature_trace_stores, ///< Trace all stores done by the pipeline. Equivalent to calling Func::trace_stores on every non-inlined Func.
    halide_target_feature_trace_realizations, ///< Trace all realizations done by the pipeline. Equivalent to calling Func::trace_realizations on every non-inlined Func.
    halide_target_feature_trace_pipeline, ///< Trace the pipeline.
    halide_target_feature_cuda_capability61,  ///< Enable CUDA compute capability 6.1 (Pascal)
    halide_target_feature_hvx_v65, ///< Enable Hexagon v65 architecture.
    halide_target_feature_hvx_v66, ///< Enable Hexagon v66 architecture.
    halide_target_feature_cl_half,  ///< Enable half support on OpenCL targets
    halide_target_feature_strict_float, ///< Turn off all non-IEEE floating-point optimization. Currently applies only to LLVM targets.
    halide_target_feature_legacy_buffer_wrappers,  ///< Emit legacy wrapper code for buffer_t (vs halide_buffer_t) when AOT-compiled.
    halide_target_feature_tsan, ///< Enable hooks for TSAN support.
    halide_target_feature_asan, ///< Enable hooks for ASAN support.
    halide_target_feature_d3d12compute, ///< Enable Direct3D 12 Compute runtime.
    halide_target_feature_check_unsafe_promises, ///< Insert assertions for promises.
    halide_target_feature_hexagon_dma, ///< Enable Hexagon DMA buffers.
    halide_target_feature_embed_bitcode,  ///< Emulate clang -fembed-bitcode flag.
    halide_target_feature_enable_llvm_loop_opt,  ///< Enable loop vectorization + unrolling in LLVM. Overrides halide_target_feature_disable_llvm_loop_opt. (Ignored for non-LLVM targets.)
    halide_target_feature_disable_llvm_loop_opt,  ///< Disable loop vectorization + unrolling in LLVM. (Ignored for non-LLVM targets.)
    halide_target_feature_wasm_simd128,  ///< Enable +simd128 instructions for WebAssembly codegen.
    halide_target_feature_wasm_signext,  ///< Enable +sign-ext instructions for WebAssembly codegen.
    halide_target_feature_sve, ///< Enable ARM Scalable Vector Extensions
    halide_target_feature_sve2, ///< Enable ARM Scalable Vector Extensions v2
    halide_target_feature_egl,            ///< Force use of EGL support.

    halide_target_feature_end ///< A sentinel. Every target is considered to have this feature, and setting this feature does nothing.
} halide_target_feature_t;

/** This function is called internally by Halide in some situations to determine
 * if the current execution environment can support the given set of
 * halide_target_feature_t flags. The implementation must do the following:
 *
 * -- If there are flags set in features that the function knows *cannot* be supported, return 0.
 * -- Otherwise, return 1.
 * -- Note that any flags set in features that the function doesn't know how to test should be ignored;
 * this implies that a return value of 1 means "not known to be bad" rather than "known to be good".
 *
 * In other words: a return value of 0 means "It is not safe to use code compiled with these features",
 * while a return value of 1 means "It is not obviously unsafe to use code compiled with these features".
 *
 * The default implementation simply calls halide_default_can_use_target_features.
 *
 * Note that `features` points to an array of `count` uint64_t; this array must contain enough
 * bits to represent all the currently known features. Any excess bits must be set to zero.
 */
// @{
extern int halide_can_use_target_features(int count, const uint64_t *features);
typedef int (*halide_can_use_target_features_t)(int count, const uint64_t *features);
extern halide_can_use_target_features_t halide_set_custom_can_use_target_features(halide_can_use_target_features_t);
// @}

/**
 * This is the default implementation of halide_can_use_target_features; it is provided
 * for convenience of user code that may wish to extend halide_can_use_target_features
 * but continue providing existing support, e.g.
 *
 *     int halide_can_use_target_features(int count, const uint64_t *features) {
 *          if (features[halide_target_somefeature >> 6] & (1LL << (halide_target_somefeature & 63))) {
 *              if (!can_use_somefeature()) {
 *                  return 0;
 *              }
 *          }
 *          return halide_default_can_use_target_features(count, features);
 *     }
 */
extern int halide_default_can_use_target_features(int count, const uint64_t *features);


typedef struct halide_dimension_t {
    int32_t min, extent, stride;

    // Per-dimension flags. None are defined yet (This is reserved for future use).
    uint32_t flags;

#ifdef __cplusplus
    HALIDE_ALWAYS_INLINE halide_dimension_t() : min(0), extent(0), stride(0), flags(0) {}
    HALIDE_ALWAYS_INLINE halide_dimension_t(int32_t m, int32_t e, int32_t s, uint32_t f = 0) :
        min(m), extent(e), stride(s), flags(f) {}

    HALIDE_ALWAYS_INLINE bool operator==(const halide_dimension_t &other) const {
        return (min == other.min) &&
            (extent == other.extent) &&
            (stride == other.stride) &&
            (flags == other.flags);
    }

    HALIDE_ALWAYS_INLINE bool operator!=(const halide_dimension_t &other) const {
        return !(*this == other);
    }
#endif
} halide_dimension_t;

#ifdef __cplusplus
} // extern "C"
#endif

typedef enum {halide_buffer_flag_host_dirty = 1,
              halide_buffer_flag_device_dirty = 2} halide_buffer_flags;

/**
 * The raw representation of an image passed around by generated
 * Halide code. It includes some stuff to track whether the image is
 * not actually in main memory, but instead on a device (like a
 * GPU). For a more convenient C++ wrapper, use Halide::Buffer<T>. */
typedef struct halide_buffer_t {
    /** A device-handle for e.g. GPU memory used to back this buffer. */
    uint64_t device;

    /** The interface used to interpret the above handle. */
    const struct halide_device_interface_t *device_interface;

    /** A pointer to the start of the data in main memory. In terms of
     * the Halide coordinate system, this is the address of the min
     * coordinates (defined below). */
    uint8_t* host;

    /** flags with various meanings. */
    uint64_t flags;

    /** The type of each buffer element. */
    struct halide_type_t type;

    /** The dimensionality of the buffer. */
    int32_t dimensions;

    /** The shape of the buffer. Halide does not own this array - you
     * must manage the memory for it yourself. */
    halide_dimension_t *dim;

    /** Pads the buffer up to a multiple of 8 bytes */
    void *padding;

#ifdef __cplusplus
    /** Convenience methods for accessing the flags */
    // @{
    HALIDE_ALWAYS_INLINE bool get_flag(halide_buffer_flags flag) const {
        return (flags & flag) != 0;
    }

    HALIDE_ALWAYS_INLINE void set_flag(halide_buffer_flags flag, bool value) {
        if (value) {
            flags |= flag;
        } else {
            flags &= ~flag;
        }
    }

    HALIDE_ALWAYS_INLINE bool host_dirty() const {
        return get_flag(halide_buffer_flag_host_dirty);
    }

    HALIDE_ALWAYS_INLINE bool device_dirty() const {
        return get_flag(halide_buffer_flag_device_dirty);
    }

    HALIDE_ALWAYS_INLINE void set_host_dirty(bool v = true) {
        set_flag(halide_buffer_flag_host_dirty, v);
    }

    HALIDE_ALWAYS_INLINE void set_device_dirty(bool v = true) {
        set_flag(halide_buffer_flag_device_dirty, v);
    }
    // @}

    /** The total number of elements this buffer represents. Equal to
     * the product of the extents */
    HALIDE_ALWAYS_INLINE size_t number_of_elements() const {
        size_t s = 1;
        for (int i = 0; i < dimensions; i++) {
            s *= dim[i].extent;
        }
        return s;
    }

    /** A pointer to the element with the lowest address. If all
     * strides are positive, equal to the host pointer. */
    HALIDE_ALWAYS_INLINE uint8_t *begin() const {
        ptrdiff_t index = 0;
        for (int i = 0; i < dimensions; i++) {
            if (dim[i].stride < 0) {
                index += dim[i].stride * (dim[i].extent - 1);
            }
        }
        return host + index * type.bytes();
    }

    /** A pointer to one beyond the element with the highest address. */
    HALIDE_ALWAYS_INLINE uint8_t *end() const {
        ptrdiff_t index = 0;
        for (int i = 0; i < dimensions; i++) {
            if (dim[i].stride > 0) {
                index += dim[i].stride * (dim[i].extent - 1);
            }
        }
        index += 1;
        return host + index * type.bytes();
    }

    /** The total number of bytes spanned by the data in memory. */
    HALIDE_ALWAYS_INLINE size_t size_in_bytes() const {
        return (size_t)(end() - begin());
    }

    /** A pointer to the element at the given location. */
    HALIDE_ALWAYS_INLINE uint8_t *address_of(const int *pos) const {
        ptrdiff_t index = 0;
        for (int i = 0; i < dimensions; i++) {
            index += dim[i].stride * (pos[i] - dim[i].min);
        }
        return host + index * type.bytes();
    }

    /** Attempt to call device_sync for the buffer. If the buffer
     * has no device_interface (or no device_sync), this is a quiet no-op.
     * Calling this explicitly should rarely be necessary, except for profiling. */
    HALIDE_ALWAYS_INLINE int device_sync(void *ctx = NULL) {
        if (device_interface && device_interface->device_sync) {
            return device_interface->device_sync(ctx, this);
        }
        return 0;
    }

    /** Check if an input buffer passed extern stage is a querying
     * bounds. Compared to doing the host pointer check directly,
     * this both adds clarity to code and will facilitate moving to
     * another representation for bounds query arguments. */
    HALIDE_ALWAYS_INLINE bool is_bounds_query() const {
        return host == NULL && device == 0;
    }

#endif
} halide_buffer_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HALIDE_ATTRIBUTE_DEPRECATED
#ifdef HALIDE_ALLOW_DEPRECATED
#define HALIDE_ATTRIBUTE_DEPRECATED(x)
#else
#ifdef _MSC_VER
#define HALIDE_ATTRIBUTE_DEPRECATED(x) __declspec(deprecated(x))
#else
#define HALIDE_ATTRIBUTE_DEPRECATED(x) __attribute__((deprecated(x)))
#endif
#endif
#endif

/** The old buffer_t, included for compatibility with old code. Don't
 * use it. */
#ifndef BUFFER_T_DEFINED
#define BUFFER_T_DEFINED
typedef struct buffer_t {
    uint64_t dev;
    uint8_t* host;
    int32_t extent[4];
    int32_t stride[4];
    int32_t min[4];
    int32_t elem_size;
    HALIDE_ATTRIBUTE_ALIGN(1) bool host_dirty;
    HALIDE_ATTRIBUTE_ALIGN(1) bool dev_dirty;
    HALIDE_ATTRIBUTE_ALIGN(1) uint8_t _padding[10 - sizeof(void *)];
} buffer_t;
#endif // BUFFER_T_DEFINED

/** Copies host pointer, mins, extents, strides, and device state from
 * an old-style buffer_t into a new-style halide_buffer_t. If bounds_query_only is nonzero,
 * the copy is only done if the old_buf has null host and dev (ie, a bounds query is being
 * performed); otherwise new_buf is left untouched. (This is used for input buffers to avoid
 * benign data races.) The dimensions and type fields of the new buffer_t should already be
 * set. Returns an error code if the upgrade could not be performed. */
extern int halide_upgrade_buffer_t(void *user_context, const char *name,
                                   const buffer_t *old_buf, halide_buffer_t *new_buf,
                                   int bounds_query_only);

/** Copies the host pointer, mins, extents, strides, and device state
 * from a halide_buffer_t to a buffer_t. Also sets elem_size. Useful
 * for backporting the results of bounds inference. */
extern int halide_downgrade_buffer_t(void *user_context, const char *name,
                                     const halide_buffer_t *new_buf, buffer_t *old_buf);

/** Copies the dirty flags and device allocation state from a new
 * buffer_t back to a legacy buffer_t. */
extern int halide_downgrade_buffer_t_device_fields(void *user_context, const char *name,
                                                   const halide_buffer_t *new_buf, buffer_t *old_buf);

/** halide_scalar_value_t is a simple union able to represent all the well-known
 * scalar values in a filter argument. Note that it isn't tagged with a type;
 * you must ensure you know the proper type before accessing. Most user
 * code will never need to create instances of this struct; its primary use
 * is to hold def/min/max values in a halide_filter_argument_t. (Note that
 * this is conceptually just a union; it's wrapped in a struct to ensure
 * that it doesn't get anonymized by LLVM.)
 */
struct halide_scalar_value_t {
    union {
        bool b;
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float f32;
        double f64;
        void *handle;
    } u;
    #ifdef __cplusplus
    HALIDE_ALWAYS_INLINE halide_scalar_value_t() {u.u64 = 0;}
    #endif
};

enum halide_argument_kind_t {
    halide_argument_kind_input_scalar = 0,
    halide_argument_kind_input_buffer = 1,
    halide_argument_kind_output_buffer = 2
};

/*
    These structs must be robust across different compilers and settings; when
    modifying them, strive for the following rules:

    1) All fields are explicitly sized. I.e. must use int32_t and not "int"
    2) All fields must land on an alignment boundary that is the same as their size
    3) Explicit padding is added to make that so
    4) The sizeof the struct is padded out to a multiple of the largest natural size thing in the struct
    5) don't forget that 32 and 64 bit pointers are different sizes
*/

/**
 * Obsolete version of halide_filter_argument_t; only present in
 * code that wrote halide_filter_metadata_t version 0.
 */
struct halide_filter_argument_t_v0 {
    const char *name;
    int32_t kind;
    int32_t dimensions;
    struct halide_type_t type;
    const struct halide_scalar_value_t *def, *min, *max;
};

/**
 * halide_filter_argument_t is essentially a plain-C-struct equivalent to
 * Halide::Argument; most user code will never need to create one.
 */
struct halide_filter_argument_t {
    const char *name;       // name of the argument; will never be null or empty.
    int32_t kind;           // actually halide_argument_kind_t
    int32_t dimensions;     // always zero for scalar arguments
    struct halide_type_t type;
    // These pointers should always be null for buffer arguments,
    // and *may* be null for scalar arguments. (A null value means
    // there is no def/min/max/estimate specified for this argument.)
    const struct halide_scalar_value_t *scalar_def, *scalar_min, *scalar_max, *scalar_estimate;
    // This pointer should always be null for scalar arguments,
    // and *may* be null for buffer arguments. If not null, it should always
    // point to an array of dimensions*2 pointers, which will be the (min, extent)
    // estimates for each dimension of the buffer. (Note that any of the pointers
    // may be null as well.)
    int64_t const* const* buffer_estimates;
};

struct halide_filter_metadata_t {
#ifdef __cplusplus
    static const int32_t VERSION = 1;
#endif

    /** version of this metadata; currently always 1. */
    int32_t version;

    /** The number of entries in the arguments field. This is always >= 1. */
    int32_t num_arguments;

    /** An array of the filters input and output arguments; this will never be
     * null. The order of arguments is not guaranteed (input and output arguments
     * may come in any order); however, it is guaranteed that all arguments
     * will have a unique name within a given filter. */
    const struct halide_filter_argument_t* arguments;

    /** The Target for which the filter was compiled. This is always
     * a canonical Target string (ie a product of Target::to_string). */
    const char* target;

    /** The function name of the filter. */
    const char* name;
};

/** halide_register_argv_and_metadata() is a **user-defined** function that
 * must be provided in order to use the registration.cc files produced
 * by Generators when the 'registration' output is requested. Each registration.cc
 * file provides a static initializer that calls this function with the given
 * filter's argv-call variant, its metadata, and (optionally) and additional
 * textual data that the build system chooses to tack on for its own purposes.
 * Note that this will be called at static-initializer time (i.e., before
 * main() is called), and in an unpredictable order. Note that extra_key_value_pairs
 * may be nullptr; if it's not null, it's expected to be a null-terminated list
 * of strings, with an even number of entries. */
void halide_register_argv_and_metadata(
    int (*filter_argv_call)(void **),
    const struct halide_filter_metadata_t *filter_metadata,
    const char * const *extra_key_value_pairs
);

/** The functions below here are relevant for pipelines compiled with
 * the -profile target flag, which runs a sampling profiler thread
 * alongside the pipeline. */

/** Per-Func state tracked by the sampling profiler. */
struct halide_profiler_func_stats {
    /** Total time taken evaluating this Func (in nanoseconds). */
    uint64_t time;

    /** The current memory allocation of this Func. */
    uint64_t memory_current;

    /** The peak memory allocation of this Func. */
    uint64_t memory_peak;

    /** The total memory allocation of this Func. */
    uint64_t memory_total;

    /** The peak stack allocation of this Func's threads. */
    uint64_t stack_peak;

    /** The average number of thread pool worker threads active while computing this Func. */
    uint64_t active_threads_numerator, active_threads_denominator;

    /** The name of this Func. A global constant string. */
    const char *name;

    /** The total number of memory allocation of this Func. */
    int num_allocs;
};

/** Per-pipeline state tracked by the sampling profiler. These exist
 * in a linked list. */
struct halide_profiler_pipeline_stats {
    /** Total time spent inside this pipeline (in nanoseconds) */
    uint64_t time;

    /** The current memory allocation of funcs in this pipeline. */
    uint64_t memory_current;

    /** The peak memory allocation of funcs in this pipeline. */
    uint64_t memory_peak;

    /** The total memory allocation of funcs in this pipeline. */
    uint64_t memory_total;

    /** The average number of thread pool worker threads doing useful
     * work while computing this pipeline. */
    uint64_t active_threads_numerator, active_threads_denominator;

    /** The name of this pipeline. A global constant string. */
    const char *name;

    /** An array containing states for each Func in this pipeline. */
    struct halide_profiler_func_stats *funcs;

    /** The next pipeline_stats pointer. It's a void * because types
     * in the Halide runtime may not currently be recursive. */
    void *next;

    /** The number of funcs in this pipeline. */
    int num_funcs;

    /** An internal base id used to identify the funcs in this pipeline. */
    int first_func_id;

    /** The number of times this pipeline has been run. */
    int runs;

    /** The total number of samples taken inside of this pipeline. */
    int samples;

    /** The total number of memory allocation of funcs in this pipeline. */
    int num_allocs;
};

/** The global state of the profiler. */

struct halide_profiler_state {
    /** Guards access to the fields below. If not locked, the sampling
     * profiler thread is free to modify things below (including
     * reordering the linked list of pipeline stats). */
    struct halide_mutex lock;

    /** The amount of time the profiler thread sleeps between samples
     * in milliseconds. Defaults to 1 */
    int sleep_time;

    /** An internal id used for bookkeeping. */
    int first_free_id;

    /** The id of the current running Func. Set by the pipeline, read
     * periodically by the profiler thread. */
    int current_func;

    /** The number of threads currently doing work. */
    int active_threads;

    /** A linked list of stats gathered for each pipeline. */
    struct halide_profiler_pipeline_stats *pipelines;

    /** Retrieve remote profiler state. Used so that the sampling
     * profiler can follow along with execution that occurs elsewhere,
     * e.g. on a DSP. If null, it reads from the int above instead. */
    void (*get_remote_profiler_state)(int *func, int *active_workers);

    /** Sampling thread reference to be joined at shutdown. */
    struct halide_thread *sampling_thread;
};

/** Profiler func ids with special meanings. */
enum {
    /// current_func takes on this value when not inside Halide code
    halide_profiler_outside_of_halide = -1,
    /// Set current_func to this value to tell the profiling thread to
    /// halt. It will start up again next time you run a pipeline with
    /// profiling enabled.
    halide_profiler_please_stop = -2
};

/** Get a pointer to the global profiler state for programmatic
 * inspection. Lock it before using to pause the profiler. */
extern struct halide_profiler_state *halide_profiler_get_state();

/** Get a pointer to the pipeline state associated with pipeline_name.
 * This function grabs the global profiler state's lock on entry. */
extern struct halide_profiler_pipeline_stats *halide_profiler_get_pipeline_state(const char *pipeline_name);

/** Reset profiler state cheaply. May leave threads running or some
 * memory allocated but all accumluated statistics are reset.
 * WARNING: Do NOT call this method while any halide pipeline is
 * running; halide_profiler_memory_allocate/free and
 * halide_profiler_stack_peak_update update the profiler pipeline's
 * state without grabbing the global profiler state's lock. */
extern void halide_profiler_reset();

/** Reset all profiler state.
 * WARNING: Do NOT call this method while any halide pipeline is
 * running; halide_profiler_memory_allocate/free and
 * halide_profiler_stack_peak_update update the profiler pipeline's
 * state without grabbing the global profiler state's lock. */
void halide_profiler_shutdown();

/** Print out timing statistics for everything run since the last
 * reset. Also happens at process exit. */
extern void halide_profiler_report(void *user_context);

/// \name "Float16" functions
/// These functions operate of bits (``uint16_t``) representing a half
/// precision floating point number (IEEE-754 2008 binary16).
//{@

/** Read bits representing a half precision floating point number and return
 *  the float that represents the same value */
extern float halide_float16_bits_to_float(uint16_t);

/** Read bits representing a half precision floating point number and return
 *  the double that represents the same value */
extern double halide_float16_bits_to_double(uint16_t);

// TODO: Conversion functions to half

//@}

// Allocating and freeing device memory is often very slow. The
// methods below give Halide's runtime permission to hold onto device
// memory to service future requests instead of returning it to the
// underlying device API. The API does not manage an allocation pool,
// all it does is provide access to a shared counter that acts as a
// limit on the unused memory not yet returned to the underlying
// device API. It makes callbacks to participants when memory needs to
// be released because the limit is about to be exceeded (either
// because the limit has been reduced, or because the memory owned by
// some participant becomes unused).

/** Tell Halide whether or not it is permitted to hold onto device
 * allocations to service future requests instead of returning them
 * eagerly to the underlying device API. Many device allocators are
 * quite slow, so it can be beneficial to set this to true. The
 * default value for now is false.
 *
 * Note that if enabled, the eviction policy is very simplistic. The
 * 32 most-recently used allocations are preserved, regardless of
 * their size. Additionally, if a call to cuMalloc results in an
 * out-of-memory error, the entire cache is flushed and the allocation
 * is retried. See https://github.com/halide/Halide/issues/4093
 *
 * If set to false, releases all unused device allocations back to the
 * underlying device APIs. For finer-grained control, see specific
 * methods in each device api runtime. */
extern int halide_reuse_device_allocations(void *user_context, bool);

/** Determines whether on device_free the memory is returned
 * immediately to the device API, or placed on a free list for future
 * use. Override and switch based on the user_context for
 * finer-grained control. By default just returns the value most
 * recently set by the method above. */
extern bool halide_can_reuse_device_allocations(void *user_context);

struct halide_device_allocation_pool {
    int (*release_unused)(void *user_context);
    struct halide_device_allocation_pool *next;
};

/** Register a callback to be informed when
 * halide_reuse_device_allocations(false) is called, and all unused
 * device allocations must be released. The object passed should have
 * global lifetime, and its next field will be clobbered. */
extern void halide_register_device_allocation_pool(struct halide_device_allocation_pool *);

#ifdef __cplusplus
} // End extern "C"
#endif

#ifdef __cplusplus

namespace {
template<typename T> struct check_is_pointer;
template<typename T> struct check_is_pointer<T *> {};
}

/** Construct the halide equivalent of a C type */
template<typename T>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of() {
    // Create a compile-time error if T is not a pointer (without
    // using any includes - this code goes into the runtime).
    check_is_pointer<T> check;
    (void)check;
    return halide_type_t(halide_type_handle, 64);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<float>() {
    return halide_type_t(halide_type_float, 32);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<double>() {
    return halide_type_t(halide_type_float, 64);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<bool>() {
    return halide_type_t(halide_type_uint, 1);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint8_t>() {
    return halide_type_t(halide_type_uint, 8);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint16_t>() {
    return halide_type_t(halide_type_uint, 16);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint32_t>() {
    return halide_type_t(halide_type_uint, 32);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint64_t>() {
    return halide_type_t(halide_type_uint, 64);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int8_t>() {
    return halide_type_t(halide_type_int, 8);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int16_t>() {
    return halide_type_t(halide_type_int, 16);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int32_t>() {
    return halide_type_t(halide_type_int, 32);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int64_t>() {
    return halide_type_t(halide_type_int, 64);
}

#endif

#endif // HALIDE_HALIDERUNTIME_H

#ifdef COMPILING_HALIDE_RUNTIME
#include "HalideRuntime.h"
#define HALIDE_BUFFER_HELPER_ATTRS __attribute__((always_inline, weak))
#else
#define HALIDE_BUFFER_HELPER_ATTRS inline
#endif

// Structs are annoying to deal with from within Halide Stmts. These
// utility functions are for dealing with buffer_t in that
// context. They are not intended for use outside of Halide code, and
// not exposed in HalideRuntime.h. The symbols are private to the
// module and should be inlined and then stripped. This blob of code
// also gets copy-pasted into C outputs.

extern "C" {

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_dimensions(const halide_buffer_t *buf) {
    return buf->dimensions;
}

HALIDE_BUFFER_HELPER_ATTRS
uint8_t *_halide_buffer_get_host(const halide_buffer_t *buf) {
    return buf->host;
}

HALIDE_BUFFER_HELPER_ATTRS
uint64_t _halide_buffer_get_device(const halide_buffer_t *buf) {
    return buf->device;
}

HALIDE_BUFFER_HELPER_ATTRS
const struct halide_device_interface_t *_halide_buffer_get_device_interface(const halide_buffer_t *buf) {
    return buf->device_interface;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_min(const halide_buffer_t *buf, int d) {
    return buf->dim[d].min;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_max(const halide_buffer_t *buf, int d) {
    return buf->dim[d].min + buf->dim[d].extent - 1;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_extent(const halide_buffer_t *buf, int d) {
    return buf->dim[d].extent;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_stride(const halide_buffer_t *buf, int d) {
    return buf->dim[d].stride;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_set_host_dirty(halide_buffer_t *buf, bool val) {
    buf->set_host_dirty(val);
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_set_device_dirty(halide_buffer_t *buf, bool val) {
    buf->set_device_dirty(val);
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
bool _halide_buffer_get_host_dirty(const halide_buffer_t *buf) {
    return buf->host_dirty();
}

HALIDE_BUFFER_HELPER_ATTRS
bool _halide_buffer_get_device_dirty(const halide_buffer_t *buf) {
    return buf->device_dirty();
}

HALIDE_BUFFER_HELPER_ATTRS
halide_dimension_t *_halide_buffer_get_shape(halide_buffer_t *buf) {
    return buf->dim;
}

HALIDE_BUFFER_HELPER_ATTRS
bool _halide_buffer_is_bounds_query(const halide_buffer_t *buf) {
    return buf->host == NULL && buf->device == 0;
}

HALIDE_BUFFER_HELPER_ATTRS
uint32_t _halide_buffer_get_type(const halide_buffer_t *buf) {
    return buf->type.as_u32();
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_init(halide_buffer_t *dst,
                                     halide_dimension_t *dst_shape,
                                     void *host,
                                     uint64_t device,
                                     const halide_device_interface_t *device_interface,
                                     int type_code, int type_bits,
                                     int dimensions,
                                     halide_dimension_t *shape,
                                     uint64_t flags) {
    dst->host = (uint8_t *)host;
    dst->device = device;
    dst->device_interface = device_interface;
    dst->type.code = (halide_type_code_t)type_code;
    dst->type.bits = (uint8_t)type_bits;
    dst->type.lanes = 1;
    dst->dimensions = dimensions;
    dst->dim = dst_shape;
    if (shape != dst->dim) {
        for (int i = 0; i < dimensions; i++) {
            dst->dim[i] = shape[i];
        }
    }
    dst->flags = flags;
    return dst;
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_init_from_buffer(halide_buffer_t *dst,
                                                 halide_dimension_t *dst_shape,
                                                 const halide_buffer_t *src) {
    dst->host = src->host;
    dst->device = src->device;
    dst->device_interface = src->device_interface;
    dst->type = src->type;
    dst->dimensions = src->dimensions;
    dst->dim = dst_shape;
    dst->flags = src->flags;
    for (int i = 0; i < dst->dimensions; i++) {
        dst->dim[i] = src->dim[i];
    }
    return dst;
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_crop(void *user_context,
                                     halide_buffer_t *dst,
                                     halide_dimension_t *dst_shape,
                                     const halide_buffer_t *src,
                                     const int *min, const int *extent) {
    *dst = *src;
    dst->dim = dst_shape;
    int64_t offset = 0;
    for (int i = 0; i < dst->dimensions; i++) {
        dst->dim[i] = src->dim[i];
        dst->dim[i].min = min[i];
        dst->dim[i].extent = extent[i];
        offset += (min[i] - src->dim[i].min) * src->dim[i].stride;
    }
    if (dst->host) {
        dst->host += offset * src->type.bytes();
    }
    dst->device_interface = 0;
    dst->device = 0;
    if (src->device_interface) {
        src->device_interface->device_crop(user_context, src, dst);
    }
    return dst;
}


// Called on return from an extern stage where the output buffer was a
// crop of some other larger buffer. This happens for extern stages
// with distinct store_at/compute_at levels. Each call to the stage
// only fills in part of the buffer.
HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_retire_crop_after_extern_stage(void *user_context,
                                                   void *obj) {
    halide_buffer_t **buffers = (halide_buffer_t **)obj;
    halide_buffer_t *crop = buffers[0];
    halide_buffer_t *parent = buffers[1];

    if (crop->device) {
        if (!parent->device) {
            // We have been given a device allocation by the extern
            // stage. It only represents the cropped region, so we
            // can't just give it to the parent.
            if (crop->device_dirty()) {
                crop->device_interface->copy_to_host(user_context, crop);
            }
            crop->device_interface->device_free(user_context, crop);
        } else {
            // We are a crop of an existing device allocation.
            if (crop->device_dirty()) {
                parent->set_device_dirty();
            }
            crop->device_interface->device_release_crop(user_context, crop);
        }
    }
    if (crop->host_dirty()) {
        parent->set_host_dirty();
    }
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_retire_crops_after_extern_stage(void *user_context,
                                                    void *obj) {
    halide_buffer_t **buffers = (halide_buffer_t **)obj;
    while (*buffers) {
        _halide_buffer_retire_crop_after_extern_stage(user_context, buffers);
        buffers += 2;
    }
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_set_bounds(halide_buffer_t *buf,
                                           int dim, int min, int extent) {
    buf->dim[dim].min = min;
    buf->dim[dim].extent = extent;
    return buf;
}

}

#undef HALIDE_BUFFER_HELPER_ATTRS


// ll suffix in OpenCL is reserver for 128-bit integers.
#if defined __OPENCL_VERSION__
#define ADD_INT64_T_SUFFIX(x) x##l
#define ADD_UINT64_T_SUFFIX(x) x##ul
// HLSL doesn't have any suffixes.
#elif defined HLSL_VERSION
#define ADD_INT64_T_SUFFIX(x) x
#define ADD_UINT64_T_SUFFIX(x) x
#else
#define ADD_INT64_T_SUFFIX(x) x##ll
#define ADD_UINT64_T_SUFFIX(x) x##ull
#endif

#ifndef HALIDE_FUNCTION_ATTRS
#define HALIDE_FUNCTION_ATTRS
#endif



#if !defined(__has_attribute)
    #define __has_attribute(x) 0
#endif

#if !defined(__has_builtin)
    #define __has_builtin(x) 0
#endif

template <typename ElementType_, size_t Lanes_>
class CppVector {
public:
    typedef ElementType_ ElementType;
    static const size_t Lanes = Lanes_;
    typedef CppVector<ElementType, Lanes> Vec;
    typedef CppVector<uint8_t, Lanes> Mask;

    CppVector &operator=(const Vec &src) {
        if (this != &src) {
            for (size_t i = 0; i < Lanes; i++) {
                elements[i] = src[i];
            }
        }
        return *this;
    }

    /* not-explicit */ CppVector(const Vec &src) {
        for (size_t i = 0; i < Lanes; i++) {
            elements[i] = src[i];
        }
    }

    CppVector() {
        for (size_t i = 0; i < Lanes; i++) {
            elements[i] = 0;
        }
    }

    static Vec broadcast(const ElementType &v) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = v;
        }
        return r;
    }

    static Vec ramp(const ElementType &base, const ElementType &stride) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = base + stride * i;
        }
        return r;
    }

    static Vec load(const void *base, int32_t offset) {
        Vec r(empty);
        memcpy(&r.elements[0], ((const ElementType*)base + offset), sizeof(r.elements));
        return r;
    }

    // gather
    static Vec load(const void *base, const CppVector<int32_t, Lanes> &offset) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = ((const ElementType*)base)[offset[i]];
        }
        return r;
    }

    void store(void *base, int32_t offset) const {
        memcpy(((ElementType*)base + offset), &this->elements[0], sizeof(this->elements));
    }

    // scatter
    void store(void *base, const CppVector<int32_t, Lanes> &offset) const {
        for (size_t i = 0; i < Lanes; i++) {
            ((ElementType*)base)[offset[i]] = elements[i];
        }
    }

    static Vec shuffle(const Vec &a, const int32_t indices[Lanes]) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            if (indices[i] < 0) {
                continue;
            }
            r.elements[i] = a[indices[i]];
        }
        return r;
    }

    template<size_t InputLanes>
    static Vec concat(size_t count, const CppVector<ElementType, InputLanes> vecs[]) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = vecs[i / InputLanes][i % InputLanes];
        }
        return r;
    }

    Vec replace(size_t i, const ElementType &b) const {
        Vec r = *this;
        r.elements[i] = b;
        return r;
    }

    ElementType operator[](size_t i) const {
        return elements[i];
    }

    Vec operator~() const {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = ~elements[i];
        }
        return r;
    }
    Vec operator!() const {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = !r.elements[i];
        }
        return r;
    }

    friend Vec operator+(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] + b[i];
        }
        return r;
    }
    friend Vec operator-(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] - b[i];
        }
        return r;
    }
    friend Vec operator*(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] * b[i];
        }
        return r;
    }
    friend Vec operator/(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] / b[i];
        }
        return r;
    }
    friend Vec operator%(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] % b[i];
        }
        return r;
    }
    template <typename OtherElementType>
    friend Vec operator<<(const Vec &a, const CppVector<OtherElementType, Lanes> &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] << b[i];
        }
        return r;
    }
    template <typename OtherElementType>
    friend Vec operator>>(const Vec &a, const CppVector<OtherElementType, Lanes> &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] >> b[i];
        }
        return r;
    }
    friend Vec operator&(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] & b[i];
        }
        return r;
    }
    friend Vec operator|(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] | b[i];
        }
        return r;
    }

    friend Vec operator&&(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] && b[i];
        }
        return r;
    }
    friend Vec operator||(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] || b[i];
        }
        return r;
    }

    friend Vec operator+(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] + b;
        }
        return r;
    }
    friend Vec operator-(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] - b;
        }
        return r;
    }
    friend Vec operator*(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] * b;
        }
        return r;
    }
    friend Vec operator/(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] / b;
        }
        return r;
    }
    friend Vec operator%(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] % b;
        }
        return r;
    }
    friend Vec operator>>(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] >> b;
        }
        return r;
    }
    friend Vec operator<<(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] << b;
        }
        return r;
    }
    friend Vec operator&(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] & b;
        }
        return r;
    }
    friend Vec operator|(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] | b;
        }
        return r;
    }
    friend Vec operator&&(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] && b;
        }
        return r;
    }
    friend Vec operator||(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] || b;
        }
        return r;
    }

    friend Vec operator+(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a + b[i];
        }
        return r;
    }
    friend Vec operator-(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a - b[i];
        }
        return r;
    }
    friend Vec operator*(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a * b[i];
        }
        return r;
    }
    friend Vec operator/(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a / b[i];
        }
        return r;
    }
    friend Vec operator%(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a % b[i];
        }
        return r;
    }
    friend Vec operator>>(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a >> b[i];
        }
        return r;
    }
    friend Vec operator<<(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a << b[i];
        }
        return r;
    }
    friend Vec operator&(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a & b[i];
        }
        return r;
    }
    friend Vec operator|(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a | b[i];
        }
        return r;
    }
    friend Vec operator&&(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a && b[i];
        }
        return r;
    }
    friend Vec operator||(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a || b[i];
        }
        return r;
    }

    friend Mask operator<(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] < b[i] ? 0xff : 0x00;
        }
        return r;
    }

    friend Mask operator<=(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] <= b[i] ? 0xff : 0x00;
        }
        return r;
    }

    friend Mask operator>(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] > b[i] ? 0xff : 0x00;
        }
        return r;
    }

    friend Mask operator>=(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] >= b[i] ? 0xff : 0x00;
        }
        return r;
    }

    friend Mask operator==(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] == b[i] ? 0xff : 0x00;
        }
        return r;
    }

    friend Mask operator!=(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = a[i] != b[i] ? 0xff : 0x00;
        }
        return r;
    }

    static Vec select(const Mask &cond, const Vec &true_value, const Vec &false_value) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = cond[i] ? true_value[i] : false_value[i];
        }
        return r;
    }

    template <typename OtherVec>
    static Vec convert_from(const OtherVec &src) {
        #if __cplusplus >= 201103L
        static_assert(Vec::Lanes == OtherVec::Lanes, "Lanes mismatch");
        #endif
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = static_cast<typename Vec::ElementType>(src[i]);
        }
        return r;
    }

    static Vec max(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = ::halide_cpp_max(a[i], b[i]);
        }
        return r;
    }

    static Vec min(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.elements[i] = ::halide_cpp_min(a[i], b[i]);
        }
        return r;
    }

private:
    template <typename, size_t> friend class CppVector;
    ElementType elements[Lanes];

    // Leave vector uninitialized for cases where we overwrite every entry
    enum Empty { empty };
    CppVector(Empty) {}
};


#if __has_attribute(ext_vector_type) || __has_attribute(vector_size)
template <typename ElementType_, size_t Lanes_>
class NativeVector {
public:
    typedef ElementType_ ElementType;
    static const size_t Lanes = Lanes_;
    typedef NativeVector<ElementType, Lanes> Vec;
    typedef NativeVector<uint8_t, Lanes> Mask;

#if __has_attribute(ext_vector_type)
    typedef ElementType_ NativeVectorType __attribute__((ext_vector_type(Lanes), aligned(sizeof(ElementType))));
#elif __has_attribute(vector_size) || __GNUC__
    typedef ElementType_ NativeVectorType __attribute__((vector_size(Lanes * sizeof(ElementType)), aligned(sizeof(ElementType))));
#endif

    NativeVector &operator=(const Vec &src) {
        if (this != &src) {
            native_vector = src.native_vector;
        }
        return *this;
    }

    /* not-explicit */ NativeVector(const Vec &src) {
        native_vector = src.native_vector;
    }

    NativeVector() {
        native_vector = (NativeVectorType){};
    }

    static Vec broadcast(const ElementType &v) {
        Vec zero; // Zero-initialized native vector.
        return zero + v;
    }

    // TODO: this should be improved by taking advantage of native operator support.
    static Vec ramp(const ElementType &base, const ElementType &stride) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = base + stride * i;
        }
        return r;
    }

    // TODO: could this be improved by taking advantage of native operator support?
    static Vec load(const void *base, int32_t offset) {
        Vec r(empty);
        // Note: do not use sizeof(NativeVectorType) here; if it's an unusual type
        // (e.g. uint8x48, which could be produced by concat()), the actual implementation
        // might be larger (e.g. it might really be a uint8x64). Only copy the amount
        // that is in the logical type, to avoid possible overreads.
        memcpy(&r.native_vector, ((const ElementType*)base + offset), sizeof(ElementType) * Lanes);
        return r;
    }

    // gather
    // TODO: could this be improved by taking advantage of native operator support?
    static Vec load(const void *base, const NativeVector<int32_t, Lanes> &offset) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = ((const ElementType*)base)[offset[i]];
        }
        return r;
    }

    // TODO: could this be improved by taking advantage of native operator support?
    void store(void *base, int32_t offset) const {
        // Note: do not use sizeof(NativeVectorType) here; if it's an unusual type
        // (e.g. uint8x48, which could be produced by concat()), the actual implementation
        // might be larger (e.g. it might really be a uint8x64). Only copy the amount
        // that is in the logical type, to avoid possible overwrites.
        memcpy(((ElementType*)base + offset), &native_vector, sizeof(ElementType) * Lanes);
    }

    // scatter
    // TODO: could this be improved by taking advantage of native operator support?
    void store(void *base, const NativeVector<int32_t, Lanes> &offset) const {
        for (size_t i = 0; i < Lanes; i++) {
            ((ElementType*)base)[offset[i]] = native_vector[i];
        }
    }

    // TODO: this should be improved by taking advantage of native operator support.
    static Vec shuffle(const Vec &a, const int32_t indices[Lanes]) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            if (indices[i] < 0) {
                continue;
            }
            r.native_vector[i] = a[indices[i]];
        }
        return r;
    }

    // TODO: this should be improved by taking advantage of native operator support.
    template<size_t InputLanes>
    static Vec concat(size_t count, const NativeVector<ElementType, InputLanes> vecs[]) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = vecs[i / InputLanes][i % InputLanes];
        }
        return r;
    }

    // TODO: this should be improved by taking advantage of native operator support.
    Vec replace(size_t i, const ElementType &b) const {
        Vec r = *this;
        r.native_vector[i] = b;
        return r;
    }

    ElementType operator[](size_t i) const {
        return native_vector[i];
    }

    Vec operator~() const {
        return Vec(from_native_vector, ~native_vector);
    }
    Vec operator!() const {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = !(*this)[i];
        }
        return r;
    }

    friend Vec operator+(const Vec &a, const Vec &b) {
        return Vec(from_native_vector, a.native_vector + b.native_vector);
    }
    friend Vec operator-(const Vec &a, const Vec &b) {
        return Vec(from_native_vector, a.native_vector - b.native_vector);
    }
    friend Vec operator*(const Vec &a, const Vec &b) {
        return Vec(from_native_vector, a.native_vector * b.native_vector);
    }
    friend Vec operator/(const Vec &a, const Vec &b) {
        return Vec(from_native_vector, a.native_vector / b.native_vector);
    }
    friend Vec operator%(const Vec &a, const Vec &b) {
        return Vec(from_native_vector, a.native_vector % b.native_vector);
    }
    friend Vec operator&(const Vec &a, const Vec &b) {
        return Vec(from_native_vector, a.native_vector & b.native_vector);
    }
    friend Vec operator|(const Vec &a, const Vec &b) {
        return Vec(from_native_vector, a.native_vector | b.native_vector);
    }
    friend Vec operator&&(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a.native_vector[i] && b.native_vector[i];
        }
        return r;
    }
    friend Vec operator||(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a.native_vector[i] || b.native_vector[i];
        }
        return r;
    }

    friend Vec operator+(const Vec &a, const ElementType &b) {
        return Vec(from_native_vector, a.native_vector + b);
    }
    friend Vec operator-(const Vec &a, const ElementType &b) {
        return Vec(from_native_vector, a.native_vector - b);
    }
    friend Vec operator*(const Vec &a, const ElementType &b) {
        return Vec(from_native_vector, a.native_vector * b);
    }
    friend Vec operator/(const Vec &a, const ElementType &b) {
        return Vec(from_native_vector, a.native_vector / b);
    }
    friend Vec operator%(const Vec &a, const ElementType &b) {
        return Vec(from_native_vector, a.native_vector % b);
    }
    friend Vec operator<<(const Vec &a, const ElementType &b) {
        return Vec(from_native_vector, a.native_vector << b);
    }
    friend Vec operator>>(const Vec &a, const ElementType &b) {
        return Vec(from_native_vector, a.native_vector >> b);
    }
    friend Vec operator&(const Vec &a, const ElementType &b) {
        return Vec(from_native_vector, a.native_vector & b);
    }
    friend Vec operator|(const Vec &a, const ElementType &b) {
        return Vec(from_native_vector, a.native_vector | b);
    }
    friend Vec operator&&(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a.native_vector[i] && b;
        }
        return r;
    }
    friend Vec operator||(const Vec &a, const ElementType &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a.native_vector[i] || b;
        }
        return r;
    }

    friend Vec operator+(const ElementType &a, const Vec &b) {
        return Vec(from_native_vector, a + b.native_vector);
    }
    friend Vec operator-(const ElementType &a, const Vec &b) {
        return Vec(from_native_vector, a - b.native_vector);
    }
    friend Vec operator*(const ElementType &a, const Vec &b) {
        return Vec(from_native_vector, a * b.native_vector);
    }
    friend Vec operator/(const ElementType &a, const Vec &b) {
        return Vec(from_native_vector, a / b.native_vector);
    }
    friend Vec operator%(const ElementType &a, const Vec &b) {
        return Vec(from_native_vector, a % b.native_vector);
    }
    friend Vec operator<<(const ElementType &a, const Vec &b) {
        return Vec(from_native_vector, a << b.native_vector);
    }
    friend Vec operator>>(const ElementType &a, const Vec &b) {
        return Vec(from_native_vector, a >> b.native_vector);
    }
    friend Vec operator&(const ElementType &a, const Vec &b) {
        return Vec(from_native_vector, a & b.native_vector);
    }
    friend Vec operator|(const ElementType &a, const Vec &b) {
        return Vec(from_native_vector, a | b.native_vector);
    }
    friend Vec operator&&(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a && b.native_vector[i];
        }
        return r;
    }
    friend Vec operator||(const ElementType &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a || b.native_vector[i];
        }
        return r;
    }

    // TODO: this should be improved by taking advantage of native operator support.
    friend Mask operator<(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a[i] < b[i] ? 0xff : 0x00;
        }
        return r;
    }

    // TODO: this should be improved by taking advantage of native operator support.
    friend Mask operator<=(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a[i] <= b[i] ? 0xff : 0x00;
        }
        return r;
    }

    // TODO: this should be improved by taking advantage of native operator support.
    friend Mask operator>(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a[i] > b[i] ? 0xff : 0x00;
        }
        return r;
    }

    // TODO: this should be improved by taking advantage of native operator support.
    friend Mask operator>=(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a[i] >= b[i] ? 0xff : 0x00;
        }
        return r;
    }

    // TODO: this should be improved by taking advantage of native operator support.
    friend Mask operator==(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a[i] == b[i] ? 0xff : 0x00;
        }
        return r;
    }

    // TODO: this should be improved by taking advantage of native operator support.
    friend Mask operator!=(const Vec &a, const Vec &b) {
        Mask r;
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = a[i] != b[i] ? 0xff : 0x00;
        }
        return r;
    }

    // TODO: this should be improved by taking advantage of native operator support.
    static Vec select(const Mask &cond, const Vec &true_value, const Vec &false_value) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = cond[i] ? true_value[i] : false_value[i];
        }
        return r;
    }

    template <typename OtherVec>
    static Vec convert_from(const OtherVec &src) {
        #if __cplusplus >= 201103L
        static_assert(Vec::Lanes == OtherVec::Lanes, "Lanes mismatch");
        #endif
#if 0 // __has_builtin(__builtin_convertvector)
        // Disabled (for now) because __builtin_convertvector appears to have
        // different float->int rounding behavior in at least some situations;
        // for now we'll use the much-slower-but-correct explicit C++ code.
        // (https://github.com/halide/Halide/issues/2080)
        return Vec(from_native_vector, __builtin_convertvector(src.native_vector, NativeVectorType));
#else
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = static_cast<typename Vec::ElementType>(src.native_vector[i]);
        }
        return r;
#endif
    }

    // TODO: this should be improved by taking advantage of native operator support.
    static Vec max(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = ::halide_cpp_max(a[i], b[i]);
        }
        return r;
    }

    // TODO: this should be improved by taking advantage of native operator support.
    static Vec min(const Vec &a, const Vec &b) {
        Vec r(empty);
        for (size_t i = 0; i < Lanes; i++) {
            r.native_vector[i] = ::halide_cpp_min(a[i], b[i]);
        }
        return r;
    }

private:
    template<typename, size_t> friend class NativeVector;

    template <typename ElementType, typename OtherElementType, size_t Lanes>
    friend NativeVector<ElementType, Lanes> operator<<(
                    const NativeVector<ElementType, Lanes> &a,
                    const NativeVector<OtherElementType, Lanes> &b);

    template <typename ElementType, typename OtherElementType, size_t Lanes>
    friend NativeVector<ElementType, Lanes> operator>>(
                    const NativeVector<ElementType, Lanes> &a,
                    const NativeVector<OtherElementType, Lanes> &b);

    NativeVectorType native_vector;

    // Leave vector uninitialized for cases where we overwrite every entry
    enum Empty { empty };
    inline NativeVector(Empty) {}

    // Syntactic sugar to avoid ctor overloading issues
    enum FromNativeVector { from_native_vector };
    inline NativeVector(FromNativeVector, const NativeVectorType &src) {
        native_vector = src;
    }
};

template <typename ElementType, typename OtherElementType, size_t Lanes>
NativeVector<ElementType, Lanes> operator<<(const NativeVector<ElementType, Lanes> &a,
                    const NativeVector<OtherElementType, Lanes> &b) {
    return NativeVector<ElementType, Lanes>(
                  NativeVector<ElementType, Lanes>::from_native_vector,
                  a.native_vector << b.native_vector);
}

template <typename ElementType, typename OtherElementType, size_t Lanes>
NativeVector<ElementType, Lanes> operator>>(const NativeVector<ElementType, Lanes> &a,
                    const NativeVector<OtherElementType, Lanes> &b) {
    return NativeVector<ElementType, Lanes>(
                  NativeVector<ElementType, Lanes>::from_native_vector,
                  a.native_vector >> b.native_vector);
}
#endif  // __has_attribute(ext_vector_type) || __has_attribute(vector_size)


// Dec. 1, 2018: Apparently emscripten compilation runs with the __has_attribute true,
// then fails to handle the vector intrinsics later.
#if !defined(__EMSCRIPTEN__) && (__has_attribute(ext_vector_type) || __has_attribute(vector_size))
    #if __GNUC__ && !__clang__
        // GCC only allows powers-of-two; fall back to CppVector for other widths
        #define halide_cpp_use_native_vector(type, lanes) ((lanes & (lanes - 1)) == 0)
    #else
        #define halide_cpp_use_native_vector(type, lanes) (true)
    #endif
#else
    // No NativeVector available
    #define halide_cpp_use_native_vector(type, lanes) (false)
#endif  // __has_attribute(ext_vector_type) || __has_attribute(vector_size)

// Failsafe to allow forcing non-native vectors in case of unruly compilers
#if HALIDE_CPP_ALWAYS_USE_CPP_VECTORS
    #undef halide_cpp_use_native_vector
    #define halide_cpp_use_native_vector(type, lanes) (false)
#endif

#if halide_cpp_use_native_vector(uint8_t, 4)
typedef NativeVector<uint8_t, 4> uint8x4_t;
#else
typedef CppVector<uint8_t, 4> uint8x4_t;
#endif
#if halide_cpp_use_native_vector(int32_t, 4)
typedef NativeVector<int32_t, 4> int32x4_t;
#else
typedef CppVector<int32_t, 4> int32x4_t;
#endif
#if halide_cpp_use_native_vector(uint32_t, 4)
typedef NativeVector<uint32_t, 4> uint32x4_t;
#else
typedef CppVector<uint32_t, 4> uint32x4_t;
#endif
#if halide_cpp_use_native_vector(uint8_t, 8)
typedef NativeVector<uint8_t, 8> uint8x8_t;
#else
typedef CppVector<uint8_t, 8> uint8x8_t;
#endif
#if halide_cpp_use_native_vector(uint16_t, 8)
typedef NativeVector<uint16_t, 8> uint16x8_t;
#else
typedef CppVector<uint16_t, 8> uint16x8_t;
#endif
#if halide_cpp_use_native_vector(int32_t, 8)
typedef NativeVector<int32_t, 8> int32x8_t;
#else
typedef CppVector<int32_t, 8> int32x8_t;
#endif
#if halide_cpp_use_native_vector(uint32_t, 8)
typedef NativeVector<uint32_t, 8> uint32x8_t;
#else
typedef CppVector<uint32_t, 8> uint32x8_t;
#endif
#if halide_cpp_use_native_vector(uint8_t, 16)
typedef NativeVector<uint8_t, 16> uint8x16_t;
#else
typedef CppVector<uint8_t, 16> uint8x16_t;
#endif
#if halide_cpp_use_native_vector(int32_t, 16)
typedef NativeVector<int32_t, 16> int32x16_t;
#else
typedef CppVector<int32_t, 16> int32x16_t;
#endif
#if halide_cpp_use_native_vector(uint32_t, 16)
typedef NativeVector<uint32_t, 16> uint32x16_t;
#else
typedef CppVector<uint32_t, 16> uint32x16_t;
#endif
#if halide_cpp_use_native_vector(int64_t, 16)
typedef NativeVector<int64_t, 16> int64x16_t;
#else
typedef CppVector<int64_t, 16> int64x16_t;
#endif
#if halide_cpp_use_native_vector(uint64_t, 16)
typedef NativeVector<uint64_t, 16> uint64x16_t;
#else
typedef CppVector<uint64_t, 16> uint64x16_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

int MatrixMultiply(struct halide_buffer_t *_mat_a_buffer, struct halide_buffer_t *_mat_b_buffer, struct halide_buffer_t *_bias_buffer, int16_t _mat_a_offset, int16_t _mat_b_offset, int32_t _output_multiplier, int32_t _output_shift, int32_t _output_offset, uint8_t _output_min, uint8_t _output_max, struct halide_buffer_t *_output_buffer) HALIDE_FUNCTION_ATTRS {
 void * const _ucon = nullptr;
 uint64_t _1 = (uint64_t)(_output_buffer);
 uint64_t _2 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
 bool _3 = _1 != _2;
 if (!_3)  {
  int32_t _4 = halide_error_buffer_argument_is_null(_ucon, "output");
  return _4;
 }
 uint64_t _5 = (uint64_t)(_mat_b_buffer);
 uint64_t _6 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
 bool _7 = _5 != _6;
 if (!_7)  {
  int32_t _8 = halide_error_buffer_argument_is_null(_ucon, "mat_b");
  return _8;
 }
 uint64_t _9 = (uint64_t)(_mat_a_buffer);
 uint64_t _10 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
 bool _11 = _9 != _10;
 if (!_11)  {
  int32_t _12 = halide_error_buffer_argument_is_null(_ucon, "mat_a");
  return _12;
 }
 uint64_t _13 = (uint64_t)(_bias_buffer);
 uint64_t _14 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
 bool _15 = _13 != _14;
 if (!_15)  {
  int32_t _16 = halide_error_buffer_argument_is_null(_ucon, "bias");
  return _16;
 }
 void *_17 = _halide_buffer_get_host(_bias_buffer);
 void * _bias = _17;
 uint32_t _18 = _halide_buffer_get_type(_bias_buffer);
 int32_t _19 = _halide_buffer_get_dimensions(_bias_buffer);
 int32_t _20 = _halide_buffer_get_min(_bias_buffer, 0);
 int32_t _21 = _halide_buffer_get_extent(_bias_buffer, 0);
 int32_t _22 = _halide_buffer_get_stride(_bias_buffer, 0);
 void *_23 = _halide_buffer_get_host(_mat_a_buffer);
 void * _mat_a = _23;
 uint32_t _24 = _halide_buffer_get_type(_mat_a_buffer);
 int32_t _25 = _halide_buffer_get_dimensions(_mat_a_buffer);
 int32_t _26 = _halide_buffer_get_min(_mat_a_buffer, 0);
 int32_t _27 = _halide_buffer_get_extent(_mat_a_buffer, 0);
 int32_t _28 = _halide_buffer_get_stride(_mat_a_buffer, 0);
 int32_t _29 = _halide_buffer_get_min(_mat_a_buffer, 1);
 int32_t _30 = _halide_buffer_get_extent(_mat_a_buffer, 1);
 int32_t _31 = _halide_buffer_get_stride(_mat_a_buffer, 1);
 void *_32 = _halide_buffer_get_host(_mat_b_buffer);
 void * _mat_b = _32;
 uint32_t _33 = _halide_buffer_get_type(_mat_b_buffer);
 int32_t _34 = _halide_buffer_get_dimensions(_mat_b_buffer);
 int32_t _35 = _halide_buffer_get_min(_mat_b_buffer, 0);
 int32_t _36 = _halide_buffer_get_extent(_mat_b_buffer, 0);
 int32_t _37 = _halide_buffer_get_stride(_mat_b_buffer, 0);
 int32_t _38 = _halide_buffer_get_min(_mat_b_buffer, 1);
 int32_t _39 = _halide_buffer_get_extent(_mat_b_buffer, 1);
 int32_t _40 = _halide_buffer_get_stride(_mat_b_buffer, 1);
 void *_41 = _halide_buffer_get_host(_output_buffer);
 void * _output = _41;
 uint32_t _42 = _halide_buffer_get_type(_output_buffer);
 int32_t _43 = _halide_buffer_get_dimensions(_output_buffer);
 int32_t _44 = _halide_buffer_get_min(_output_buffer, 0);
 int32_t _45 = _halide_buffer_get_extent(_output_buffer, 0);
 int32_t _46 = _halide_buffer_get_stride(_output_buffer, 0);
 int32_t _47 = _halide_buffer_get_min(_output_buffer, 1);
 int32_t _48 = _halide_buffer_get_extent(_output_buffer, 1);
 int32_t _49 = _halide_buffer_get_stride(_output_buffer, 1);
 bool _50 = _halide_buffer_is_bounds_query(_bias_buffer);
 if (_50)
 {
  struct halide_dimension_t *_51 = _halide_buffer_get_shape(_bias_buffer);
  uint64_t _52 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
  void *_53 = (void *)(_52);
  struct halide_device_interface_t *_54 = (struct halide_device_interface_t *)(_52);
  int32_t _55 = _45 >> 4;
  int32_t _56 = _55 * 16;
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
  } s0 = {
   0,
   _56,
   1,
   0
  };
  struct halide_dimension_t *_57 = (struct halide_dimension_t *)(&s0);
  struct halide_buffer_t *_58 = _halide_buffer_init(_bias_buffer, _51, _53, _52, _54, 0, 32, 1, _57, _52);
  (void)_58;
 } // if _50
 bool _59 = _halide_buffer_is_bounds_query(_mat_a_buffer);
 if (_59)
 {
  struct halide_dimension_t *_60 = _halide_buffer_get_shape(_mat_a_buffer);
  uint64_t _61 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
  void *_62 = (void *)(_61);
  struct halide_device_interface_t *_63 = (struct halide_device_interface_t *)(_61);
  int32_t _64 = _48 >> 2;
  int32_t _65 = _64 * 4;
  int32_t _66 = _27 >> 2;
  int32_t _67 = _66 * 4;
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
   const int32_t f_4;
   const int32_t f_5;
   const int32_t f_6;
   const int32_t f_7;
  } s1 = {
   0,
   _27,
   1,
   0,
   0,
   _65,
   _67,
   0
  };
  struct halide_dimension_t *_68 = (struct halide_dimension_t *)(&s1);
  struct halide_buffer_t *_69 = _halide_buffer_init(_mat_a_buffer, _60, _62, _61, _63, 1, 8, 2, _68, _61);
  (void)_69;
 } // if _59
 bool _70 = _halide_buffer_is_bounds_query(_mat_b_buffer);
 if (_70)
 {
  struct halide_dimension_t *_71 = _halide_buffer_get_shape(_mat_b_buffer);
  uint64_t _72 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
  void *_73 = (void *)(_72);
  struct halide_device_interface_t *_74 = (struct halide_device_interface_t *)(_72);
  int32_t _75 = _45 >> 4;
  int32_t _76 = _75 * 16;
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
   const int32_t f_4;
   const int32_t f_5;
   const int32_t f_6;
   const int32_t f_7;
  } s2 = {
   0,
   _76,
   1,
   0,
   0,
   _27,
   _76,
   0
  };
  struct halide_dimension_t *_77 = (struct halide_dimension_t *)(&s2);
  struct halide_buffer_t *_78 = _halide_buffer_init(_mat_b_buffer, _71, _73, _72, _74, 1, 8, 2, _77, _72);
  (void)_78;
 } // if _70
 bool _79 = _halide_buffer_is_bounds_query(_output_buffer);
 if (_79)
 {
  struct halide_dimension_t *_80 = _halide_buffer_get_shape(_output_buffer);
  uint64_t _81 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
  void *_82 = (void *)(_81);
  struct halide_device_interface_t *_83 = (struct halide_device_interface_t *)(_81);
  int32_t _84 = _45 >> 4;
  int32_t _85 = _84 * 16;
  int32_t _86 = _48 >> 2;
  int32_t _87 = _86 * 4;
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
   const int32_t f_4;
   const int32_t f_5;
   const int32_t f_6;
   const int32_t f_7;
  } s3 = {
   0,
   _85,
   1,
   0,
   0,
   _87,
   _85,
   0
  };
  struct halide_dimension_t *_88 = (struct halide_dimension_t *)(&s3);
  struct halide_buffer_t *_89 = _halide_buffer_init(_output_buffer, _80, _82, _81, _83, 1, 8, 2, _88, _81);
  (void)_89;
 } // if _79
 bool _90 = _halide_buffer_is_bounds_query(_output_buffer);
 bool _91 = _halide_buffer_is_bounds_query(_mat_b_buffer);
 bool _92 = _halide_buffer_is_bounds_query(_bias_buffer);
 bool _93 = _halide_buffer_is_bounds_query(_mat_a_buffer);
 bool _94 = _92 || _93;
 bool _95 = _91 || _94;
 bool _96 = _90 || _95;
 bool _97 = !(_96);
 if (_97)
 {
  uint32_t _98 = (uint32_t)(ADD_UINT64_T_SUFFIX(73728));
  bool _99 = _18 == _98;
  if (!_99)   {
   uint32_t _100 = (uint32_t)(ADD_UINT64_T_SUFFIX(73728));
   int32_t _101 = halide_error_bad_type(_ucon, "Input buffer bias", _18, _100);
   return _101;
  }
  bool _102 = _19 == 1;
  if (!_102)   {
   int32_t _103 = halide_error_bad_dimensions(_ucon, "Input buffer bias", _19, 1);
   return _103;
  }
  uint32_t _104 = (uint32_t)(ADD_UINT64_T_SUFFIX(67585));
  bool _105 = _24 == _104;
  if (!_105)   {
   uint32_t _106 = (uint32_t)(ADD_UINT64_T_SUFFIX(67585));
   int32_t _107 = halide_error_bad_type(_ucon, "Input buffer mat_a", _24, _106);
   return _107;
  }
  bool _108 = _25 == 2;
  if (!_108)   {
   int32_t _109 = halide_error_bad_dimensions(_ucon, "Input buffer mat_a", _25, 2);
   return _109;
  }
  uint32_t _110 = (uint32_t)(ADD_UINT64_T_SUFFIX(67585));
  bool _111 = _33 == _110;
  if (!_111)   {
   uint32_t _112 = (uint32_t)(ADD_UINT64_T_SUFFIX(67585));
   int32_t _113 = halide_error_bad_type(_ucon, "Input buffer mat_b", _33, _112);
   return _113;
  }
  bool _114 = _34 == 2;
  if (!_114)   {
   int32_t _115 = halide_error_bad_dimensions(_ucon, "Input buffer mat_b", _34, 2);
   return _115;
  }
  uint32_t _116 = (uint32_t)(ADD_UINT64_T_SUFFIX(67585));
  bool _117 = _42 == _116;
  if (!_117)   {
   uint32_t _118 = (uint32_t)(ADD_UINT64_T_SUFFIX(67585));
   int32_t _119 = halide_error_bad_type(_ucon, "Output buffer output", _42, _118);
   return _119;
  }
  bool _120 = _43 == 2;
  if (!_120)   {
   int32_t _121 = halide_error_bad_dimensions(_ucon, "Output buffer output", _43, 2);
   return _121;
  }
  bool _122 = _20 <= 0;
  int32_t _123 = _45 >> 4;
  int32_t _124 = _123 * 16;
  int32_t _125 = _21 + _20;
  bool _126 = _124 <= _125;
  bool _127 = _122 && _126;
  if (!_127)   {
   int32_t _128 = _45 >> 4;
   int32_t _129 = _128 * 16;
   int32_t _130 = _129 + -1;
   int32_t _131 = _21 + _20;
   int32_t _132 = _131 + -1;
   int32_t _133 = halide_error_access_out_of_bounds(_ucon, "Input buffer bias", 0, 0, _130, _20, _132);
   return _133;
  }
  bool _134 = 0 <= _21;
  if (!_134)   {
   int32_t _135 = halide_error_buffer_extents_negative(_ucon, "Input buffer bias", 0, _21);
   return _135;
  }
  bool _136 = _26 <= 0;
  bool _137 = 0 <= _26;
  bool _138 = _136 && _137;
  if (!_138)   {
   int32_t _139 = _27 + -1;
   int32_t _140 = _27 + _26;
   int32_t _141 = _140 + -1;
   int32_t _142 = halide_error_access_out_of_bounds(_ucon, "Input buffer mat_a", 0, 0, _139, _26, _141);
   return _142;
  }
  bool _143 = 0 <= _27;
  if (!_143)   {
   int32_t _144 = halide_error_buffer_extents_negative(_ucon, "Input buffer mat_a", 0, _27);
   return _144;
  }
  bool _145 = _29 <= 0;
  int32_t _146 = _48 >> 2;
  int32_t _147 = _146 * 4;
  int32_t _148 = _30 + _29;
  bool _149 = _147 <= _148;
  bool _150 = _145 && _149;
  if (!_150)   {
   int32_t _151 = _48 >> 2;
   int32_t _152 = _151 * 4;
   int32_t _153 = _152 + -1;
   int32_t _154 = _30 + _29;
   int32_t _155 = _154 + -1;
   int32_t _156 = halide_error_access_out_of_bounds(_ucon, "Input buffer mat_a", 1, 0, _153, _29, _155);
   return _156;
  }
  bool _157 = 0 <= _30;
  if (!_157)   {
   int32_t _158 = halide_error_buffer_extents_negative(_ucon, "Input buffer mat_a", 1, _30);
   return _158;
  }
  bool _159 = _35 <= 0;
  int32_t _160 = _45 >> 4;
  int32_t _161 = _160 * 16;
  int32_t _162 = _36 + _35;
  bool _163 = _161 <= _162;
  bool _164 = _159 && _163;
  if (!_164)   {
   int32_t _165 = _45 >> 4;
   int32_t _166 = _165 * 16;
   int32_t _167 = _166 + -1;
   int32_t _168 = _36 + _35;
   int32_t _169 = _168 + -1;
   int32_t _170 = halide_error_access_out_of_bounds(_ucon, "Input buffer mat_b", 0, 0, _167, _35, _169);
   return _170;
  }
  bool _171 = 0 <= _36;
  if (!_171)   {
   int32_t _172 = halide_error_buffer_extents_negative(_ucon, "Input buffer mat_b", 0, _36);
   return _172;
  }
  bool _173 = _38 <= 0;
  int32_t _174 = _39 + _38;
  bool _175 = _27 <= _174;
  bool _176 = _173 && _175;
  if (!_176)   {
   int32_t _177 = _27 + -1;
   int32_t _178 = _39 + _38;
   int32_t _179 = _178 + -1;
   int32_t _180 = halide_error_access_out_of_bounds(_ucon, "Input buffer mat_b", 1, 0, _177, _38, _179);
   return _180;
  }
  bool _181 = 0 <= _39;
  if (!_181)   {
   int32_t _182 = halide_error_buffer_extents_negative(_ucon, "Input buffer mat_b", 1, _39);
   return _182;
  }
  bool _183 = _44 <= 0;
  int32_t _184 = _45 & 15;
  int32_t _185 = _184 + _44;
  bool _186 = 0 <= _185;
  bool _187 = _183 && _186;
  if (!_187)   {
   int32_t _188 = _45 >> 4;
   int32_t _189 = _188 * 16;
   int32_t _190 = _189 + -1;
   int32_t _191 = _45 + _44;
   int32_t _192 = _191 + -1;
   int32_t _193 = halide_error_access_out_of_bounds(_ucon, "Output buffer output", 0, 0, _190, _44, _192);
   return _193;
  }
  bool _194 = 0 <= _45;
  if (!_194)   {
   int32_t _195 = halide_error_buffer_extents_negative(_ucon, "Output buffer output", 0, _45);
   return _195;
  }
  bool _196 = _47 <= 0;
  int32_t _197 = _48 & 3;
  int32_t _198 = _197 + _47;
  bool _199 = 0 <= _198;
  bool _200 = _196 && _199;
  if (!_200)   {
   int32_t _201 = _48 >> 2;
   int32_t _202 = _201 * 4;
   int32_t _203 = _202 + -1;
   int32_t _204 = _48 + _47;
   int32_t _205 = _204 + -1;
   int32_t _206 = halide_error_access_out_of_bounds(_ucon, "Output buffer output", 1, 0, _203, _47, _205);
   return _206;
  }
  bool _207 = 0 <= _48;
  if (!_207)   {
   int32_t _208 = halide_error_buffer_extents_negative(_ucon, "Output buffer output", 1, _48);
   return _208;
  }
  bool _209 = _22 == 1;
  if (!_209)   {
   int32_t _210 = halide_error_constraint_violated(_ucon, "bias.stride.0", _22, "1", 1);
   return _210;
  }
  bool _211 = _20 == 0;
  if (!_211)   {
   int32_t _212 = halide_error_constraint_violated(_ucon, "bias.min.0", _20, "0", 0);
   return _212;
  }
  bool _213 = _28 == 1;
  if (!_213)   {
   int32_t _214 = halide_error_constraint_violated(_ucon, "mat_a.stride.0", _28, "1", 1);
   return _214;
  }
  int32_t _215 = _31 & 3;
  bool _216 = _215 == 0;
  if (!_216)   {
   int32_t _217 = _31 >> 2;
   int32_t _218 = _217 * 4;
   int32_t _219 = halide_error_constraint_violated(_ucon, "mat_a.stride.1", _31, "((mat_a.stride.1/4)*4)", _218);
   return _219;
  }
  bool _220 = _29 == 0;
  if (!_220)   {
   int32_t _221 = halide_error_constraint_violated(_ucon, "mat_a.min.1", _29, "0", 0);
   return _221;
  }
  int32_t _222 = _30 & 3;
  bool _223 = _222 == 0;
  if (!_223)   {
   int32_t _224 = _30 >> 2;
   int32_t _225 = _224 * 4;
   int32_t _226 = halide_error_constraint_violated(_ucon, "mat_a.extent.1", _30, "((mat_a.extent.1/4)*4)", _225);
   return _226;
  }
  bool _227 = _37 == 1;
  if (!_227)   {
   int32_t _228 = halide_error_constraint_violated(_ucon, "mat_b.stride.0", _37, "1", 1);
   return _228;
  }
  bool _229 = _35 == 0;
  if (!_229)   {
   int32_t _230 = halide_error_constraint_violated(_ucon, "mat_b.min.0", _35, "0", 0);
   return _230;
  }
  int32_t _231 = _36 & 15;
  bool _232 = _231 == 0;
  if (!_232)   {
   int32_t _233 = _36 >> 4;
   int32_t _234 = _233 * 16;
   int32_t _235 = halide_error_constraint_violated(_ucon, "mat_b.extent.0", _36, "((mat_b.extent.0/16)*16)", _234);
   return _235;
  }
  bool _236 = _38 == 0;
  if (!_236)   {
   int32_t _237 = halide_error_constraint_violated(_ucon, "mat_b.min.1", _38, "0", 0);
   return _237;
  }
  bool _238 = _46 == 1;
  if (!_238)   {
   int32_t _239 = halide_error_constraint_violated(_ucon, "output.stride.0", _46, "1", 1);
   return _239;
  }
  bool _240 = _44 == 0;
  if (!_240)   {
   int32_t _241 = halide_error_constraint_violated(_ucon, "output.min.0", _44, "0", 0);
   return _241;
  }
  int32_t _242 = _45 & 15;
  bool _243 = _242 == 0;
  if (!_243)   {
   int32_t _244 = _45 >> 4;
   int32_t _245 = _244 * 16;
   int32_t _246 = halide_error_constraint_violated(_ucon, "output.extent.0", _45, "((output.extent.0/16)*16)", _245);
   return _246;
  }
  int32_t _247 = _49 & 3;
  bool _248 = _247 == 0;
  if (!_248)   {
   int32_t _249 = _49 >> 2;
   int32_t _250 = _249 * 4;
   int32_t _251 = halide_error_constraint_violated(_ucon, "output.stride.1", _49, "((output.stride.1/4)*4)", _250);
   return _251;
  }
  bool _252 = _47 == 0;
  if (!_252)   {
   int32_t _253 = halide_error_constraint_violated(_ucon, "output.min.1", _47, "0", 0);
   return _253;
  }
  int32_t _254 = _48 & 3;
  bool _255 = _254 == 0;
  if (!_255)   {
   int32_t _256 = _48 >> 2;
   int32_t _257 = _256 * 4;
   int32_t _258 = halide_error_constraint_violated(_ucon, "output.extent.1", _48, "((output.extent.1/4)*4)", _257);
   return _258;
  }
  int32_t _259 = _30 >> 2;
  int32_t _260 = _259 * 4;
  int64_t _261 = (int64_t)(_260);
  int64_t _262 = (int64_t)(_27);
  int64_t _263 = _261 * _262;
  int64_t _264 = (int64_t)(_39);
  int32_t _265 = _36 >> 4;
  int32_t _266 = _265 * 16;
  int64_t _267 = (int64_t)(_266);
  int64_t _268 = _264 * _267;
  int32_t _269 = _48 >> 2;
  int32_t _270 = _269 * 4;
  int64_t _271 = (int64_t)(_270);
  int32_t _272 = _45 >> 4;
  int32_t _273 = _272 * 16;
  int64_t _274 = (int64_t)(_273);
  int64_t _275 = _271 * _274;
  int64_t _276 = (int64_t)(_21);
  int64_t _277 = (int64_t)(ADD_INT64_T_SUFFIX(0));
  int64_t _278 = _277 - _276;
  bool _279 = _276 > _277;
  int64_t _280 = (int64_t)(_279 ? _276 : _278);
  uint64_t _281 = (uint64_t)(_280);
  uint64_t _282 = _281;
  uint64_t _283 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
  bool _284 = _282 <= _283;
  if (!_284)   {
   int64_t _285 = (int64_t)(_21);
   int64_t _286 = (int64_t)(ADD_INT64_T_SUFFIX(0));
   int64_t _287 = _286 - _285;
   bool _288 = _285 > _286;
   int64_t _289 = (int64_t)(_288 ? _285 : _287);
   uint64_t _290 = (uint64_t)(_289);
   uint64_t _291 = _290;
   uint64_t _292 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
   int32_t _293 = halide_error_buffer_allocation_too_large(_ucon, "bias", _291, _292);
   return _293;
  }
  int64_t _294 = (int64_t)(_27);
  int64_t _295 = (int64_t)(ADD_INT64_T_SUFFIX(0));
  int64_t _296 = _295 - _294;
  bool _297 = _294 > _295;
  int64_t _298 = (int64_t)(_297 ? _294 : _296);
  uint64_t _299 = (uint64_t)(_298);
  uint64_t _300 = _299;
  uint64_t _301 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
  bool _302 = _300 <= _301;
  if (!_302)   {
   int64_t _303 = (int64_t)(_27);
   int64_t _304 = (int64_t)(ADD_INT64_T_SUFFIX(0));
   int64_t _305 = _304 - _303;
   bool _306 = _303 > _304;
   int64_t _307 = (int64_t)(_306 ? _303 : _305);
   uint64_t _308 = (uint64_t)(_307);
   uint64_t _309 = _308;
   uint64_t _310 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
   int32_t _311 = halide_error_buffer_allocation_too_large(_ucon, "mat_a", _309, _310);
   return _311;
  }
  int32_t _312 = _30 >> 2;
  int32_t _313 = _312 * 4;
  int64_t _314 = (int64_t)(_313);
  int32_t _315 = _31 >> 2;
  int32_t _316 = _315 * 4;
  int64_t _317 = (int64_t)(_316);
  int64_t _318 = _314 * _317;
  int64_t _319 = (int64_t)(ADD_INT64_T_SUFFIX(0));
  int64_t _320 = _319 - _318;
  bool _321 = _318 > _319;
  int64_t _322 = (int64_t)(_321 ? _318 : _320);
  uint64_t _323 = (uint64_t)(_322);
  uint64_t _324 = _323;
  uint64_t _325 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
  bool _326 = _324 <= _325;
  if (!_326)   {
   int32_t _327 = _30 >> 2;
   int32_t _328 = _327 * 4;
   int64_t _329 = (int64_t)(_328);
   int32_t _330 = _31 >> 2;
   int32_t _331 = _330 * 4;
   int64_t _332 = (int64_t)(_331);
   int64_t _333 = _329 * _332;
   int64_t _334 = (int64_t)(ADD_INT64_T_SUFFIX(0));
   int64_t _335 = _334 - _333;
   bool _336 = _333 > _334;
   int64_t _337 = (int64_t)(_336 ? _333 : _335);
   uint64_t _338 = (uint64_t)(_337);
   uint64_t _339 = _338;
   uint64_t _340 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
   int32_t _341 = halide_error_buffer_allocation_too_large(_ucon, "mat_a", _339, _340);
   return _341;
  }
  int64_t _342 = (int64_t)(ADD_INT64_T_SUFFIX(2147483647));
  bool _343 = _263 <= _342;
  if (!_343)   {
   int64_t _344 = (int64_t)(ADD_INT64_T_SUFFIX(2147483647));
   int32_t _345 = halide_error_buffer_extents_too_large(_ucon, "mat_a", _263, _344);
   return _345;
  }
  int32_t _346 = _36 >> 4;
  int32_t _347 = _346 * 16;
  int64_t _348 = (int64_t)(_347);
  int64_t _349 = (int64_t)(ADD_INT64_T_SUFFIX(0));
  int64_t _350 = _349 - _348;
  bool _351 = _348 > _349;
  int64_t _352 = (int64_t)(_351 ? _348 : _350);
  uint64_t _353 = (uint64_t)(_352);
  uint64_t _354 = _353;
  uint64_t _355 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
  bool _356 = _354 <= _355;
  if (!_356)   {
   int32_t _357 = _36 >> 4;
   int32_t _358 = _357 * 16;
   int64_t _359 = (int64_t)(_358);
   int64_t _360 = (int64_t)(ADD_INT64_T_SUFFIX(0));
   int64_t _361 = _360 - _359;
   bool _362 = _359 > _360;
   int64_t _363 = (int64_t)(_362 ? _359 : _361);
   uint64_t _364 = (uint64_t)(_363);
   uint64_t _365 = _364;
   uint64_t _366 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
   int32_t _367 = halide_error_buffer_allocation_too_large(_ucon, "mat_b", _365, _366);
   return _367;
  }
  int64_t _368 = (int64_t)(_39);
  int64_t _369 = (int64_t)(_40);
  int64_t _370 = _368 * _369;
  int64_t _371 = (int64_t)(ADD_INT64_T_SUFFIX(0));
  int64_t _372 = _371 - _370;
  bool _373 = _370 > _371;
  int64_t _374 = (int64_t)(_373 ? _370 : _372);
  uint64_t _375 = (uint64_t)(_374);
  uint64_t _376 = _375;
  uint64_t _377 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
  bool _378 = _376 <= _377;
  if (!_378)   {
   int64_t _379 = (int64_t)(_39);
   int64_t _380 = (int64_t)(_40);
   int64_t _381 = _379 * _380;
   int64_t _382 = (int64_t)(ADD_INT64_T_SUFFIX(0));
   int64_t _383 = _382 - _381;
   bool _384 = _381 > _382;
   int64_t _385 = (int64_t)(_384 ? _381 : _383);
   uint64_t _386 = (uint64_t)(_385);
   uint64_t _387 = _386;
   uint64_t _388 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
   int32_t _389 = halide_error_buffer_allocation_too_large(_ucon, "mat_b", _387, _388);
   return _389;
  }
  int64_t _390 = (int64_t)(ADD_INT64_T_SUFFIX(2147483647));
  bool _391 = _268 <= _390;
  if (!_391)   {
   int64_t _392 = (int64_t)(ADD_INT64_T_SUFFIX(2147483647));
   int32_t _393 = halide_error_buffer_extents_too_large(_ucon, "mat_b", _268, _392);
   return _393;
  }
  int32_t _394 = _45 >> 4;
  int32_t _395 = _394 * 16;
  int64_t _396 = (int64_t)(_395);
  int64_t _397 = (int64_t)(ADD_INT64_T_SUFFIX(0));
  int64_t _398 = _397 - _396;
  bool _399 = _396 > _397;
  int64_t _400 = (int64_t)(_399 ? _396 : _398);
  uint64_t _401 = (uint64_t)(_400);
  uint64_t _402 = _401;
  uint64_t _403 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
  bool _404 = _402 <= _403;
  if (!_404)   {
   int32_t _405 = _45 >> 4;
   int32_t _406 = _405 * 16;
   int64_t _407 = (int64_t)(_406);
   int64_t _408 = (int64_t)(ADD_INT64_T_SUFFIX(0));
   int64_t _409 = _408 - _407;
   bool _410 = _407 > _408;
   int64_t _411 = (int64_t)(_410 ? _407 : _409);
   uint64_t _412 = (uint64_t)(_411);
   uint64_t _413 = _412;
   uint64_t _414 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
   int32_t _415 = halide_error_buffer_allocation_too_large(_ucon, "output", _413, _414);
   return _415;
  }
  int32_t _416 = _48 >> 2;
  int32_t _417 = _416 * 4;
  int64_t _418 = (int64_t)(_417);
  int32_t _419 = _49 >> 2;
  int32_t _420 = _419 * 4;
  int64_t _421 = (int64_t)(_420);
  int64_t _422 = _418 * _421;
  int64_t _423 = (int64_t)(ADD_INT64_T_SUFFIX(0));
  int64_t _424 = _423 - _422;
  bool _425 = _422 > _423;
  int64_t _426 = (int64_t)(_425 ? _422 : _424);
  uint64_t _427 = (uint64_t)(_426);
  uint64_t _428 = _427;
  uint64_t _429 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
  bool _430 = _428 <= _429;
  if (!_430)   {
   int32_t _431 = _48 >> 2;
   int32_t _432 = _431 * 4;
   int64_t _433 = (int64_t)(_432);
   int32_t _434 = _49 >> 2;
   int32_t _435 = _434 * 4;
   int64_t _436 = (int64_t)(_435);
   int64_t _437 = _433 * _436;
   int64_t _438 = (int64_t)(ADD_INT64_T_SUFFIX(0));
   int64_t _439 = _438 - _437;
   bool _440 = _437 > _438;
   int64_t _441 = (int64_t)(_440 ? _437 : _439);
   uint64_t _442 = (uint64_t)(_441);
   uint64_t _443 = _442;
   uint64_t _444 = (uint64_t)(ADD_UINT64_T_SUFFIX(2147483647));
   int32_t _445 = halide_error_buffer_allocation_too_large(_ucon, "output", _443, _444);
   return _445;
  }
  int64_t _446 = (int64_t)(ADD_INT64_T_SUFFIX(2147483647));
  bool _447 = _275 <= _446;
  if (!_447)   {
   int64_t _448 = (int64_t)(ADD_INT64_T_SUFFIX(2147483647));
   int32_t _449 = halide_error_buffer_extents_too_large(_ucon, "output", _275, _448);
   return _449;
  }
  uint64_t _450 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
  void *_451 = (void *)(_450);
  bool _452 = _bias != _451;
  if (!_452)   {
   int32_t _453 = halide_error_host_is_null(_ucon, "Input buffer bias");
   return _453;
  }
  uint64_t _454 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
  void *_455 = (void *)(_454);
  bool _456 = _mat_a != _455;
  if (!_456)   {
   int32_t _457 = halide_error_host_is_null(_ucon, "Input buffer mat_a");
   return _457;
  }
  uint64_t _458 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
  void *_459 = (void *)(_458);
  bool _460 = _mat_b != _459;
  if (!_460)   {
   int32_t _461 = halide_error_host_is_null(_ucon, "Input buffer mat_b");
   return _461;
  }
  uint64_t _462 = (uint64_t)(ADD_UINT64_T_SUFFIX(0));
  void *_463 = (void *)(_462);
  bool _464 = _output != _463;
  if (!_464)   {
   int32_t _465 = halide_error_host_is_null(_ucon, "Output buffer output");
   return _465;
  }
  bool _466 = _output_offset <= 255;
  if (!_466)   {
   int64_t _467 = (int64_t)(_output_offset);
   int64_t _468 = (int64_t)(ADD_INT64_T_SUFFIX(255));
   int32_t _469 = halide_error_param_too_large_i64(_ucon, "output_offset", _467, _468);
   return _469;
  }
  bool _470 = 0 <= _output_offset;
  if (!_470)   {
   int64_t _471 = (int64_t)(_output_offset);
   int64_t _472 = (int64_t)(ADD_INT64_T_SUFFIX(0));
   int32_t _473 = halide_error_param_too_small_i64(_ucon, "output_offset", _471, _472);
   return _473;
  }
  int16_t _474 = (int16_t)(ADD_INT64_T_SUFFIX(-255));
  int16_t _475 = ::halide_cpp_max(_mat_b_offset, _474);
  int16_t _476 = (int16_t)(ADD_INT64_T_SUFFIX(0));
  bool _477 = _475 <= _476;
  if (!_477)   {
   int16_t _478 = (int16_t)(ADD_INT64_T_SUFFIX(-255));
   int16_t _479 = ::halide_cpp_max(_mat_b_offset, _478);
   int64_t _480 = (int64_t)(_479);
   int64_t _481 = (int64_t)(ADD_INT64_T_SUFFIX(0));
   int32_t _482 = halide_error_param_too_large_i64(_ucon, "mat_b_offset", _480, _481);
   return _482;
  }
  int16_t _483 = (int16_t)(ADD_INT64_T_SUFFIX(-255));
  bool _484 = _483 <= _mat_b_offset;
  if (!_484)   {
   int64_t _485 = (int64_t)(_mat_b_offset);
   int64_t _486 = (int64_t)(ADD_INT64_T_SUFFIX(-255));
   int32_t _487 = halide_error_param_too_small_i64(_ucon, "mat_b_offset", _485, _486);
   return _487;
  }
  int16_t _488 = (int16_t)(ADD_INT64_T_SUFFIX(-255));
  int16_t _489 = ::halide_cpp_max(_mat_a_offset, _488);
  int16_t _490 = (int16_t)(ADD_INT64_T_SUFFIX(0));
  bool _491 = _489 <= _490;
  if (!_491)   {
   int16_t _492 = (int16_t)(ADD_INT64_T_SUFFIX(-255));
   int16_t _493 = ::halide_cpp_max(_mat_a_offset, _492);
   int64_t _494 = (int64_t)(_493);
   int64_t _495 = (int64_t)(ADD_INT64_T_SUFFIX(0));
   int32_t _496 = halide_error_param_too_large_i64(_ucon, "mat_a_offset", _494, _495);
   return _496;
  }
  int16_t _497 = (int16_t)(ADD_INT64_T_SUFFIX(-255));
  bool _498 = _497 <= _mat_a_offset;
  if (!_498)   {
   int64_t _499 = (int64_t)(_mat_a_offset);
   int64_t _500 = (int64_t)(ADD_INT64_T_SUFFIX(-255));
   int32_t _501 = halide_error_param_too_small_i64(_ucon, "mat_a_offset", _499, _500);
   return _501;
  }
  {
   int32_t _502 = _45 >> 4;
   int32_t _503 = ::halide_cpp_max(_502, 1);
   int32_t _504 = _503 * 16;
   int64_t _505 = _504;
   if ((_505 > ((int64_t(1) << 31) - 1)) || ((_505 * sizeof(uint32_t )) > ((int64_t(1) << 31) - 1)))
   {
    halide_error(_ucon, "32-bit signed overflow computing size of allocation column_sums_b\n");
    return -1;
   } // overflow test column_sums_b
   int64_t _506 = _505;
   uint32_t *_column_sums_b = (uint32_t  *)halide_malloc(_ucon, sizeof(uint32_t )*_506);
   if (!_column_sums_b)
   {
    return halide_error_out_of_memory(_ucon);
   }
   HalideFreeHelper _column_sums_b_free(_ucon, _column_sums_b, halide_free);
   // produce column_sums_b
   int32_t _507 = _45 >> 4;
   int32_t _508 = ::halide_cpp_min(_507, 1);
   for (int _column_sums_b_s0_x_x = 0; _column_sums_b_s0_x_x < 0 + _507; _column_sums_b_s0_x_x++)
   {
    int32_t _509 = _507 + -1;
    int32_t _510 = ::halide_cpp_min(_509, _column_sums_b_s0_x_x);
    {
     uint32_t _sum__1[16];
     // produce sum$1
     uint32_t _511 = (uint32_t)(ADD_UINT64_T_SUFFIX(0));
     uint32x16_t _512 = uint32x16_t::broadcast(_511);
     _512.store(_sum__1, 0);
     int32_t _513 = _510 * 16;
     for (int _sum__1_s1_fk__x = 0; _sum__1_s1_fk__x < 0 + _27; _sum__1_s1_fk__x++)
     {
      uint32x16_t _514 = uint32x16_t::load(_sum__1, 0);
      int32_t _515 = _40 * _sum__1_s1_fk__x;
      int32_t _516 = _515 + _513;
      uint8x16_t _517 = uint8x16_t::load(_mat_b, _516);
      uint32x16_t _518 = uint32x16_t::convert_from<uint8x16_t>(_517);
      uint32x16_t _519 = _514 + _518;
      _519.store(_sum__1, 0);
     } // for _sum__1_s1_fk__x
     // consume sum$1
     uint32x16_t _520 = uint32x16_t::load(_sum__1, 0);
     int32_t _521 = _510 - _508;
     int32_t _522 = _521 * 16;
     int32_t _523 = _522 + 16;
     _520.store(_column_sums_b, _523);
    } // alloc _sum__1
   } // for _column_sums_b_s0_x_x
   {
    int32_t _524 = _45 >> 4;
    int32_t _525 = _524 * 16;
    int32_t _526 = ::halide_cpp_max(_525, 4);
    int64_t _527 = _526;
    int32_t _528 = _48 >> 2;
    int32_t _529 = _528 * 4;
    int64_t _530 = _527 * _529;
    if ((_530 > ((int64_t(1) << 31) - 1)) || ((_530 * sizeof(uint32_t )) > ((int64_t(1) << 31) - 1)))
    {
     halide_error(_ucon, "32-bit signed overflow computing size of allocation multiplied_no_offsets\n");
     return -1;
    } // overflow test multiplied_no_offsets
    int64_t _531 = _530;
    uint32_t *_multiplied_no_offsets = (uint32_t  *)halide_malloc(_ucon, sizeof(uint32_t )*_531);
    if (!_multiplied_no_offsets)
    {
     return halide_error_out_of_memory(_ucon);
    }
    HalideFreeHelper _multiplied_no_offsets_free(_ucon, _multiplied_no_offsets, halide_free);
    // produce multiplied_no_offsets
    int32_t _532 = _45 >> 4;
    int32_t _533 = _532 * 16;
    int32_t _534 = ::halide_cpp_max(_533, 4);
    int32_t _535 = _48 >> 2;
    int32_t _536 = _535 * 4;
    int32_t _537 = _532 * 4;
    int32_t _538 = ::halide_cpp_min(_533, 4);
    int32_t _539 = 4 - _538;
    for (int _multiplied_no_offsets_s0_y = 0; _multiplied_no_offsets_s0_y < 0 + _536; _multiplied_no_offsets_s0_y++)
    {
     int32_t _540 = _multiplied_no_offsets_s0_y * _534;
     int32_t _541 = _540 + _539;
     for (int _multiplied_no_offsets_s0_x_x = 0; _multiplied_no_offsets_s0_x_x < 0 + _537; _multiplied_no_offsets_s0_x_x++)
     {
      uint32_t _542 = (uint32_t)(ADD_UINT64_T_SUFFIX(0));
      uint32x4_t _543 = uint32x4_t::broadcast(_542);
      int32_t _544 = _multiplied_no_offsets_s0_x_x * 4;
      int32_t _545 = _544 + _541;
      _543.store(_multiplied_no_offsets, _545);
     } // for _multiplied_no_offsets_s0_x_x
    } // for _multiplied_no_offsets_s0_y
    int32_t _546 = _45 >> 5;
    int32_t _547 = _546 * 32;
    int32_t _548 = _45 >> 4;
    int32_t _549 = _548 * 16;
    int32_t _550 = ::halide_cpp_min(_549, 4);
    int32_t _551 = _547 - _550;
    int32_t _552 = _40 * 2;
    int32_t _553 = _40 * 3;
    int32_t _554 = _549 + -31;
    int32_t _555 = _549 + -23;
    int32_t _556 = _27 >> 2;
    int32_t _557 = _27 >> 7;
    int32_t _558 = _557 * 32;
    int32_t _559 = _546 * 2;
    bool _560 = _559 < _548;
    int32_t _561 = _549 + -1;
    bool _562 = _547 < _561;
    bool _563 = _560 && _562;
    int32_t _564 = _549 + -2;
    bool _565 = _547 < _564;
    bool _566 = _563 && _565;
    int32_t _567 = _549 + -3;
    bool _568 = _547 < _567;
    bool _569 = _566 && _568;
    int32_t _570 = _549 + -4;
    bool _571 = _547 < _570;
    bool _572 = _569 && _571;
    int32_t _573 = _549 + -5;
    bool _574 = _547 < _573;
    bool _575 = _572 && _574;
    int32_t _576 = _549 + -6;
    bool _577 = _547 < _576;
    bool _578 = _575 && _577;
    int32_t _579 = _549 + -7;
    bool _580 = _547 < _579;
    bool _581 = _578 && _580;
    int32_t _582 = _549 + -15;
    bool _583 = _547 < _582;
    bool _584 = _547 < _555;
    bool _585 = _547 < _554;
    int32_t _586 = _45 & 31;
    bool _587 = 15 < _586;
    int32_t _588 = _27 & 127;
    bool _589 = 3 < _588;
    int32_t _590 = ::halide_cpp_max(_549, 4);
    int32_t _591 = _48 + 28;
    int32_t _592 = _591 >> 5;
    int32_t _593 = _45 + 16;
    int32_t _594 = _593 >> 5;
    int32_t _595 = _27 + 124;
    int32_t _596 = _595 >> 7;
    int32_t _597 = _48 >> 5;
    int32_t _598 = _48 >> 2;
    int32_t _599 = _548 - _559;
    int32_t _600 = 28 - _550;
    int32_t _601 = 20 - _550;
    int32_t _602 = 12 - _550;
    int32_t _603 = 4 - _550;
    int32_t _604 = _551 + 28;
    int32_t _605 = _551 + 20;
    int32_t _606 = _551 + 12;
    int32_t _607 = _551 + 4;
    #pragma omp parallel for
    for (int _multiplied_no_offsets_s1_y_y = 0; _multiplied_no_offsets_s1_y_y < 0 + _592; _multiplied_no_offsets_s1_y_y++)
    {
     bool _608 = _multiplied_no_offsets_s1_y_y < _597;
     if (_608)
     {
      int32_t _609 = _556 - _558;
      int32_t _610 = ::halide_cpp_min(_609, 32);
      int32_t _611 = ::halide_cpp_max(_610, 0);
      int32_t _612 = _multiplied_no_offsets_s1_y_y * 8;
      for (int _multiplied_no_offsets_s1_x_x = 0; _multiplied_no_offsets_s1_x_x < 0 + _546; _multiplied_no_offsets_s1_x_x++)
      {
       int32_t _613 = _multiplied_no_offsets_s1_x_x * 32;
       int32_t _614 = _multiplied_no_offsets_s1_x_x * 8;
       int32_t _615 = _600 + _613;
       int32_t _616 = _601 + _613;
       int32_t _617 = _602 + _613;
       int32_t _618 = _603 + _613;
       for (int _multiplied_no_offsets_s1_k__x_k__x = 0; _multiplied_no_offsets_s1_k__x_k__x < 0 + _557; _multiplied_no_offsets_s1_k__x_k__x++)
       {
        int32_t _619 = _multiplied_no_offsets_s1_k__x_k__x * 32;
        for (int _multiplied_no_offsets_s1_y_yi_yi = 0; _multiplied_no_offsets_s1_y_yi_yi < 0 + 8; _multiplied_no_offsets_s1_y_yi_yi++)
        {
         int32_t _620 = _multiplied_no_offsets_s1_y_yi_yi + _612;
         int32_t _621 = _620 * 4;
         int32_t _622 = _621 + 1;
         int32_t _623 = _622 * _590;
         int32_t _624 = _621 + 2;
         int32_t _625 = _624 * _590;
         int32_t _626 = _621 + 3;
         int32_t _627 = _626 * _590;
         int32_t _628 = _590 * _620;
         int32_t _629 = _628 * 4;
         int32_t _630 = _615 + _629;
         int32_t _631 = _616 + _629;
         int32_t _632 = _617 + _629;
         int32_t _633 = _618 + _629;
         int32_t _634 = _615 + _627;
         int32_t _635 = _616 + _627;
         int32_t _636 = _617 + _627;
         int32_t _637 = _618 + _627;
         int32_t _638 = _615 + _625;
         int32_t _639 = _616 + _625;
         int32_t _640 = _617 + _625;
         int32_t _641 = _618 + _625;
         int32_t _642 = _615 + _623;
         int32_t _643 = _616 + _623;
         int32_t _644 = _617 + _623;
         int32_t _645 = _618 + _623;
         for (int _multiplied_no_offsets_s1_k__x_rki = 0; _multiplied_no_offsets_s1_k__x_rki < 0 + 32; _multiplied_no_offsets_s1_k__x_rki++)
         {
          int32_t _646 = _590 * _620;
          int32_t _647 = _646 * 4;
          int32_t _648 = _647 + _618;
          uint32x8_t _649 = uint32x8_t::load(_multiplied_no_offsets, _648);
          int32_t _650 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _651 = _650 * _40;
          int32_t _652 = _651 + _614;
          int32_t _653 = _652 * 4;
          uint8x8_t _654 = uint8x8_t::load(_mat_b, _653);
          uint16x8_t _655 = uint16x8_t::convert_from<uint8x8_t>(_654);
          int32_t _656 = _31 * _620;
          int32_t _657 = _656 + _619;
          int32_t _658 = _657 + _multiplied_no_offsets_s1_k__x_rki;
          int32_t _659 = _658 * 4;
          uint8_t _660 = ((const uint8_t *)_mat_a)[_659];
          uint16_t _661 = (uint16_t)(_660);
          uint16x8_t _662 = uint16x8_t::broadcast(_661);
          uint16x8_t _663 = _655 * _662;
          uint32x8_t _664 = uint32x8_t::convert_from<uint16x8_t>(_663);
          uint32x8_t _665 = _649 + _664;
          int32_t _666 = _653 + _40;
          uint8x8_t _667 = uint8x8_t::load(_mat_b, _666);
          uint16x8_t _668 = uint16x8_t::convert_from<uint8x8_t>(_667);
          int32_t _669 = _659 + 1;
          uint8_t _670 = ((const uint8_t *)_mat_a)[_669];
          uint16_t _671 = (uint16_t)(_670);
          uint16x8_t _672 = uint16x8_t::broadcast(_671);
          uint16x8_t _673 = _668 * _672;
          uint32x8_t _674 = uint32x8_t::convert_from<uint16x8_t>(_673);
          uint32x8_t _675 = _665 + _674;
          int32_t _676 = _652 * 2;
          int32_t _677 = _676 + _40;
          int32_t _678 = _677 * 2;
          uint8x8_t _679 = uint8x8_t::load(_mat_b, _678);
          uint16x8_t _680 = uint16x8_t::convert_from<uint8x8_t>(_679);
          int32_t _681 = _659 + 2;
          uint8_t _682 = ((const uint8_t *)_mat_a)[_681];
          uint16_t _683 = (uint16_t)(_682);
          uint16x8_t _684 = uint16x8_t::broadcast(_683);
          uint16x8_t _685 = _680 * _684;
          uint32x8_t _686 = uint32x8_t::convert_from<uint16x8_t>(_685);
          uint32x8_t _687 = _675 + _686;
          int32_t _688 = _653 + _553;
          uint8x8_t _689 = uint8x8_t::load(_mat_b, _688);
          uint16x8_t _690 = uint16x8_t::convert_from<uint8x8_t>(_689);
          int32_t _691 = _659 + 3;
          uint8_t _692 = ((const uint8_t *)_mat_a)[_691];
          uint16_t _693 = (uint16_t)(_692);
          uint16x8_t _694 = uint16x8_t::broadcast(_693);
          uint16x8_t _695 = _690 * _694;
          uint32x8_t _696 = uint32x8_t::convert_from<uint16x8_t>(_695);
          uint32x8_t _697 = _687 + _696;
          _697.store(_multiplied_no_offsets, _633);
          int32_t _698 = _620 * 4;
          int32_t _699 = _698 + 1;
          int32_t _700 = _699 * _31;
          int32_t _701 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _702 = _701 * 4;
          int32_t _703 = _700 + _702;
          int32_t _704 = _699 * _590;
          int32_t _705 = _704 + _618;
          uint32x8_t _706 = uint32x8_t::load(_multiplied_no_offsets, _705);
          int32_t _707 = _701 * _40;
          int32_t _708 = _707 + _614;
          int32_t _709 = _708 * 4;
          uint8x8_t _710 = uint8x8_t::load(_mat_b, _709);
          uint16x8_t _711 = uint16x8_t::convert_from<uint8x8_t>(_710);
          uint8_t _712 = ((const uint8_t *)_mat_a)[_703];
          uint16_t _713 = (uint16_t)(_712);
          uint16x8_t _714 = uint16x8_t::broadcast(_713);
          uint16x8_t _715 = _711 * _714;
          uint32x8_t _716 = uint32x8_t::convert_from<uint16x8_t>(_715);
          uint32x8_t _717 = _706 + _716;
          int32_t _718 = _709 + _40;
          uint8x8_t _719 = uint8x8_t::load(_mat_b, _718);
          uint16x8_t _720 = uint16x8_t::convert_from<uint8x8_t>(_719);
          int32_t _721 = _703 + 1;
          uint8_t _722 = ((const uint8_t *)_mat_a)[_721];
          uint16_t _723 = (uint16_t)(_722);
          uint16x8_t _724 = uint16x8_t::broadcast(_723);
          uint16x8_t _725 = _720 * _724;
          uint32x8_t _726 = uint32x8_t::convert_from<uint16x8_t>(_725);
          uint32x8_t _727 = _717 + _726;
          int32_t _728 = _708 * 2;
          int32_t _729 = _728 + _40;
          int32_t _730 = _729 * 2;
          uint8x8_t _731 = uint8x8_t::load(_mat_b, _730);
          uint16x8_t _732 = uint16x8_t::convert_from<uint8x8_t>(_731);
          int32_t _733 = _703 + 2;
          uint8_t _734 = ((const uint8_t *)_mat_a)[_733];
          uint16_t _735 = (uint16_t)(_734);
          uint16x8_t _736 = uint16x8_t::broadcast(_735);
          uint16x8_t _737 = _732 * _736;
          uint32x8_t _738 = uint32x8_t::convert_from<uint16x8_t>(_737);
          uint32x8_t _739 = _727 + _738;
          int32_t _740 = _709 + _553;
          uint8x8_t _741 = uint8x8_t::load(_mat_b, _740);
          uint16x8_t _742 = uint16x8_t::convert_from<uint8x8_t>(_741);
          int32_t _743 = _703 + 3;
          uint8_t _744 = ((const uint8_t *)_mat_a)[_743];
          uint16_t _745 = (uint16_t)(_744);
          uint16x8_t _746 = uint16x8_t::broadcast(_745);
          uint16x8_t _747 = _742 * _746;
          uint32x8_t _748 = uint32x8_t::convert_from<uint16x8_t>(_747);
          uint32x8_t _749 = _739 + _748;
          _749.store(_multiplied_no_offsets, _645);
          int32_t _750 = _620 * 4;
          int32_t _751 = _750 + 2;
          int32_t _752 = _751 * _31;
          int32_t _753 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _754 = _753 * 4;
          int32_t _755 = _752 + _754;
          int32_t _756 = _751 * _590;
          int32_t _757 = _756 + _618;
          uint32x8_t _758 = uint32x8_t::load(_multiplied_no_offsets, _757);
          int32_t _759 = _753 * _40;
          int32_t _760 = _759 + _614;
          int32_t _761 = _760 * 4;
          uint8x8_t _762 = uint8x8_t::load(_mat_b, _761);
          uint16x8_t _763 = uint16x8_t::convert_from<uint8x8_t>(_762);
          uint8_t _764 = ((const uint8_t *)_mat_a)[_755];
          uint16_t _765 = (uint16_t)(_764);
          uint16x8_t _766 = uint16x8_t::broadcast(_765);
          uint16x8_t _767 = _763 * _766;
          uint32x8_t _768 = uint32x8_t::convert_from<uint16x8_t>(_767);
          uint32x8_t _769 = _758 + _768;
          int32_t _770 = _761 + _40;
          uint8x8_t _771 = uint8x8_t::load(_mat_b, _770);
          uint16x8_t _772 = uint16x8_t::convert_from<uint8x8_t>(_771);
          int32_t _773 = _755 + 1;
          uint8_t _774 = ((const uint8_t *)_mat_a)[_773];
          uint16_t _775 = (uint16_t)(_774);
          uint16x8_t _776 = uint16x8_t::broadcast(_775);
          uint16x8_t _777 = _772 * _776;
          uint32x8_t _778 = uint32x8_t::convert_from<uint16x8_t>(_777);
          uint32x8_t _779 = _769 + _778;
          int32_t _780 = _760 * 2;
          int32_t _781 = _780 + _40;
          int32_t _782 = _781 * 2;
          uint8x8_t _783 = uint8x8_t::load(_mat_b, _782);
          uint16x8_t _784 = uint16x8_t::convert_from<uint8x8_t>(_783);
          int32_t _785 = _755 + 2;
          uint8_t _786 = ((const uint8_t *)_mat_a)[_785];
          uint16_t _787 = (uint16_t)(_786);
          uint16x8_t _788 = uint16x8_t::broadcast(_787);
          uint16x8_t _789 = _784 * _788;
          uint32x8_t _790 = uint32x8_t::convert_from<uint16x8_t>(_789);
          uint32x8_t _791 = _779 + _790;
          int32_t _792 = _761 + _553;
          uint8x8_t _793 = uint8x8_t::load(_mat_b, _792);
          uint16x8_t _794 = uint16x8_t::convert_from<uint8x8_t>(_793);
          int32_t _795 = _755 + 3;
          uint8_t _796 = ((const uint8_t *)_mat_a)[_795];
          uint16_t _797 = (uint16_t)(_796);
          uint16x8_t _798 = uint16x8_t::broadcast(_797);
          uint16x8_t _799 = _794 * _798;
          uint32x8_t _800 = uint32x8_t::convert_from<uint16x8_t>(_799);
          uint32x8_t _801 = _791 + _800;
          _801.store(_multiplied_no_offsets, _641);
          int32_t _802 = _620 * 4;
          int32_t _803 = _802 + 3;
          int32_t _804 = _803 * _31;
          int32_t _805 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _806 = _805 * 4;
          int32_t _807 = _804 + _806;
          int32_t _808 = _803 * _590;
          int32_t _809 = _808 + _618;
          uint32x8_t _810 = uint32x8_t::load(_multiplied_no_offsets, _809);
          int32_t _811 = _805 * _40;
          int32_t _812 = _811 + _614;
          int32_t _813 = _812 * 4;
          uint8x8_t _814 = uint8x8_t::load(_mat_b, _813);
          uint16x8_t _815 = uint16x8_t::convert_from<uint8x8_t>(_814);
          uint8_t _816 = ((const uint8_t *)_mat_a)[_807];
          uint16_t _817 = (uint16_t)(_816);
          uint16x8_t _818 = uint16x8_t::broadcast(_817);
          uint16x8_t _819 = _815 * _818;
          uint32x8_t _820 = uint32x8_t::convert_from<uint16x8_t>(_819);
          uint32x8_t _821 = _810 + _820;
          int32_t _822 = _813 + _40;
          uint8x8_t _823 = uint8x8_t::load(_mat_b, _822);
          uint16x8_t _824 = uint16x8_t::convert_from<uint8x8_t>(_823);
          int32_t _825 = _807 + 1;
          uint8_t _826 = ((const uint8_t *)_mat_a)[_825];
          uint16_t _827 = (uint16_t)(_826);
          uint16x8_t _828 = uint16x8_t::broadcast(_827);
          uint16x8_t _829 = _824 * _828;
          uint32x8_t _830 = uint32x8_t::convert_from<uint16x8_t>(_829);
          uint32x8_t _831 = _821 + _830;
          int32_t _832 = _812 * 2;
          int32_t _833 = _832 + _40;
          int32_t _834 = _833 * 2;
          uint8x8_t _835 = uint8x8_t::load(_mat_b, _834);
          uint16x8_t _836 = uint16x8_t::convert_from<uint8x8_t>(_835);
          int32_t _837 = _807 + 2;
          uint8_t _838 = ((const uint8_t *)_mat_a)[_837];
          uint16_t _839 = (uint16_t)(_838);
          uint16x8_t _840 = uint16x8_t::broadcast(_839);
          uint16x8_t _841 = _836 * _840;
          uint32x8_t _842 = uint32x8_t::convert_from<uint16x8_t>(_841);
          uint32x8_t _843 = _831 + _842;
          int32_t _844 = _813 + _553;
          uint8x8_t _845 = uint8x8_t::load(_mat_b, _844);
          uint16x8_t _846 = uint16x8_t::convert_from<uint8x8_t>(_845);
          int32_t _847 = _807 + 3;
          uint8_t _848 = ((const uint8_t *)_mat_a)[_847];
          uint16_t _849 = (uint16_t)(_848);
          uint16x8_t _850 = uint16x8_t::broadcast(_849);
          uint16x8_t _851 = _846 * _850;
          uint32x8_t _852 = uint32x8_t::convert_from<uint16x8_t>(_851);
          uint32x8_t _853 = _843 + _852;
          _853.store(_multiplied_no_offsets, _637);
          int32_t _854 = _590 * _620;
          int32_t _855 = _854 * 4;
          int32_t _856 = _855 + _617;
          uint32x8_t _857 = uint32x8_t::load(_multiplied_no_offsets, _856);
          int32_t _858 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _859 = _858 * _40;
          int32_t _860 = _859 + _614;
          int32_t _861 = _860 * 4;
          int32_t _862 = _861 + 8;
          uint8x8_t _863 = uint8x8_t::load(_mat_b, _862);
          uint16x8_t _864 = uint16x8_t::convert_from<uint8x8_t>(_863);
          int32_t _865 = _31 * _620;
          int32_t _866 = _865 + _619;
          int32_t _867 = _866 + _multiplied_no_offsets_s1_k__x_rki;
          int32_t _868 = _867 * 4;
          uint8_t _869 = ((const uint8_t *)_mat_a)[_868];
          uint16_t _870 = (uint16_t)(_869);
          uint16x8_t _871 = uint16x8_t::broadcast(_870);
          uint16x8_t _872 = _864 * _871;
          uint32x8_t _873 = uint32x8_t::convert_from<uint16x8_t>(_872);
          uint32x8_t _874 = _857 + _873;
          int32_t _875 = _861 + _40;
          int32_t _876 = _875 + 8;
          uint8x8_t _877 = uint8x8_t::load(_mat_b, _876);
          uint16x8_t _878 = uint16x8_t::convert_from<uint8x8_t>(_877);
          int32_t _879 = _868 + 1;
          uint8_t _880 = ((const uint8_t *)_mat_a)[_879];
          uint16_t _881 = (uint16_t)(_880);
          uint16x8_t _882 = uint16x8_t::broadcast(_881);
          uint16x8_t _883 = _878 * _882;
          uint32x8_t _884 = uint32x8_t::convert_from<uint16x8_t>(_883);
          uint32x8_t _885 = _874 + _884;
          int32_t _886 = _860 * 2;
          int32_t _887 = _886 + _40;
          int32_t _888 = _887 * 2;
          int32_t _889 = _888 + 8;
          uint8x8_t _890 = uint8x8_t::load(_mat_b, _889);
          uint16x8_t _891 = uint16x8_t::convert_from<uint8x8_t>(_890);
          int32_t _892 = _868 + 2;
          uint8_t _893 = ((const uint8_t *)_mat_a)[_892];
          uint16_t _894 = (uint16_t)(_893);
          uint16x8_t _895 = uint16x8_t::broadcast(_894);
          uint16x8_t _896 = _891 * _895;
          uint32x8_t _897 = uint32x8_t::convert_from<uint16x8_t>(_896);
          uint32x8_t _898 = _885 + _897;
          int32_t _899 = _861 + _553;
          int32_t _900 = _899 + 8;
          uint8x8_t _901 = uint8x8_t::load(_mat_b, _900);
          uint16x8_t _902 = uint16x8_t::convert_from<uint8x8_t>(_901);
          int32_t _903 = _868 + 3;
          uint8_t _904 = ((const uint8_t *)_mat_a)[_903];
          uint16_t _905 = (uint16_t)(_904);
          uint16x8_t _906 = uint16x8_t::broadcast(_905);
          uint16x8_t _907 = _902 * _906;
          uint32x8_t _908 = uint32x8_t::convert_from<uint16x8_t>(_907);
          uint32x8_t _909 = _898 + _908;
          _909.store(_multiplied_no_offsets, _632);
          int32_t _910 = _620 * 4;
          int32_t _911 = _910 + 1;
          int32_t _912 = _911 * _31;
          int32_t _913 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _914 = _913 * 4;
          int32_t _915 = _912 + _914;
          int32_t _916 = _911 * _590;
          int32_t _917 = _916 + _617;
          uint32x8_t _918 = uint32x8_t::load(_multiplied_no_offsets, _917);
          int32_t _919 = _913 * _40;
          int32_t _920 = _919 + _614;
          int32_t _921 = _920 * 4;
          int32_t _922 = _921 + 8;
          uint8x8_t _923 = uint8x8_t::load(_mat_b, _922);
          uint16x8_t _924 = uint16x8_t::convert_from<uint8x8_t>(_923);
          uint8_t _925 = ((const uint8_t *)_mat_a)[_915];
          uint16_t _926 = (uint16_t)(_925);
          uint16x8_t _927 = uint16x8_t::broadcast(_926);
          uint16x8_t _928 = _924 * _927;
          uint32x8_t _929 = uint32x8_t::convert_from<uint16x8_t>(_928);
          uint32x8_t _930 = _918 + _929;
          int32_t _931 = _921 + _40;
          int32_t _932 = _931 + 8;
          uint8x8_t _933 = uint8x8_t::load(_mat_b, _932);
          uint16x8_t _934 = uint16x8_t::convert_from<uint8x8_t>(_933);
          int32_t _935 = _915 + 1;
          uint8_t _936 = ((const uint8_t *)_mat_a)[_935];
          uint16_t _937 = (uint16_t)(_936);
          uint16x8_t _938 = uint16x8_t::broadcast(_937);
          uint16x8_t _939 = _934 * _938;
          uint32x8_t _940 = uint32x8_t::convert_from<uint16x8_t>(_939);
          uint32x8_t _941 = _930 + _940;
          int32_t _942 = _920 * 2;
          int32_t _943 = _942 + _40;
          int32_t _944 = _943 * 2;
          int32_t _945 = _944 + 8;
          uint8x8_t _946 = uint8x8_t::load(_mat_b, _945);
          uint16x8_t _947 = uint16x8_t::convert_from<uint8x8_t>(_946);
          int32_t _948 = _915 + 2;
          uint8_t _949 = ((const uint8_t *)_mat_a)[_948];
          uint16_t _950 = (uint16_t)(_949);
          uint16x8_t _951 = uint16x8_t::broadcast(_950);
          uint16x8_t _952 = _947 * _951;
          uint32x8_t _953 = uint32x8_t::convert_from<uint16x8_t>(_952);
          uint32x8_t _954 = _941 + _953;
          int32_t _955 = _921 + _553;
          int32_t _956 = _955 + 8;
          uint8x8_t _957 = uint8x8_t::load(_mat_b, _956);
          uint16x8_t _958 = uint16x8_t::convert_from<uint8x8_t>(_957);
          int32_t _959 = _915 + 3;
          uint8_t _960 = ((const uint8_t *)_mat_a)[_959];
          uint16_t _961 = (uint16_t)(_960);
          uint16x8_t _962 = uint16x8_t::broadcast(_961);
          uint16x8_t _963 = _958 * _962;
          uint32x8_t _964 = uint32x8_t::convert_from<uint16x8_t>(_963);
          uint32x8_t _965 = _954 + _964;
          _965.store(_multiplied_no_offsets, _644);
          int32_t _966 = _620 * 4;
          int32_t _967 = _966 + 2;
          int32_t _968 = _967 * _31;
          int32_t _969 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _970 = _969 * 4;
          int32_t _971 = _968 + _970;
          int32_t _972 = _967 * _590;
          int32_t _973 = _972 + _617;
          uint32x8_t _974 = uint32x8_t::load(_multiplied_no_offsets, _973);
          int32_t _975 = _969 * _40;
          int32_t _976 = _975 + _614;
          int32_t _977 = _976 * 4;
          int32_t _978 = _977 + 8;
          uint8x8_t _979 = uint8x8_t::load(_mat_b, _978);
          uint16x8_t _980 = uint16x8_t::convert_from<uint8x8_t>(_979);
          uint8_t _981 = ((const uint8_t *)_mat_a)[_971];
          uint16_t _982 = (uint16_t)(_981);
          uint16x8_t _983 = uint16x8_t::broadcast(_982);
          uint16x8_t _984 = _980 * _983;
          uint32x8_t _985 = uint32x8_t::convert_from<uint16x8_t>(_984);
          uint32x8_t _986 = _974 + _985;
          int32_t _987 = _977 + _40;
          int32_t _988 = _987 + 8;
          uint8x8_t _989 = uint8x8_t::load(_mat_b, _988);
          uint16x8_t _990 = uint16x8_t::convert_from<uint8x8_t>(_989);
          int32_t _991 = _971 + 1;
          uint8_t _992 = ((const uint8_t *)_mat_a)[_991];
          uint16_t _993 = (uint16_t)(_992);
          uint16x8_t _994 = uint16x8_t::broadcast(_993);
          uint16x8_t _995 = _990 * _994;
          uint32x8_t _996 = uint32x8_t::convert_from<uint16x8_t>(_995);
          uint32x8_t _997 = _986 + _996;
          int32_t _998 = _976 * 2;
          int32_t _999 = _998 + _40;
          int32_t _1000 = _999 * 2;
          int32_t _1001 = _1000 + 8;
          uint8x8_t _1002 = uint8x8_t::load(_mat_b, _1001);
          uint16x8_t _1003 = uint16x8_t::convert_from<uint8x8_t>(_1002);
          int32_t _1004 = _971 + 2;
          uint8_t _1005 = ((const uint8_t *)_mat_a)[_1004];
          uint16_t _1006 = (uint16_t)(_1005);
          uint16x8_t _1007 = uint16x8_t::broadcast(_1006);
          uint16x8_t _1008 = _1003 * _1007;
          uint32x8_t _1009 = uint32x8_t::convert_from<uint16x8_t>(_1008);
          uint32x8_t _1010 = _997 + _1009;
          int32_t _1011 = _977 + _553;
          int32_t _1012 = _1011 + 8;
          uint8x8_t _1013 = uint8x8_t::load(_mat_b, _1012);
          uint16x8_t _1014 = uint16x8_t::convert_from<uint8x8_t>(_1013);
          int32_t _1015 = _971 + 3;
          uint8_t _1016 = ((const uint8_t *)_mat_a)[_1015];
          uint16_t _1017 = (uint16_t)(_1016);
          uint16x8_t _1018 = uint16x8_t::broadcast(_1017);
          uint16x8_t _1019 = _1014 * _1018;
          uint32x8_t _1020 = uint32x8_t::convert_from<uint16x8_t>(_1019);
          uint32x8_t _1021 = _1010 + _1020;
          _1021.store(_multiplied_no_offsets, _640);
          int32_t _1022 = _620 * 4;
          int32_t _1023 = _1022 + 3;
          int32_t _1024 = _1023 * _31;
          int32_t _1025 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _1026 = _1025 * 4;
          int32_t _1027 = _1024 + _1026;
          int32_t _1028 = _1023 * _590;
          int32_t _1029 = _1028 + _617;
          uint32x8_t _1030 = uint32x8_t::load(_multiplied_no_offsets, _1029);
          int32_t _1031 = _1025 * _40;
          int32_t _1032 = _1031 + _614;
          int32_t _1033 = _1032 * 4;
          int32_t _1034 = _1033 + 8;
          uint8x8_t _1035 = uint8x8_t::load(_mat_b, _1034);
          uint16x8_t _1036 = uint16x8_t::convert_from<uint8x8_t>(_1035);
          uint8_t _1037 = ((const uint8_t *)_mat_a)[_1027];
          uint16_t _1038 = (uint16_t)(_1037);
          uint16x8_t _1039 = uint16x8_t::broadcast(_1038);
          uint16x8_t _1040 = _1036 * _1039;
          uint32x8_t _1041 = uint32x8_t::convert_from<uint16x8_t>(_1040);
          uint32x8_t _1042 = _1030 + _1041;
          int32_t _1043 = _1033 + _40;
          int32_t _1044 = _1043 + 8;
          uint8x8_t _1045 = uint8x8_t::load(_mat_b, _1044);
          uint16x8_t _1046 = uint16x8_t::convert_from<uint8x8_t>(_1045);
          int32_t _1047 = _1027 + 1;
          uint8_t _1048 = ((const uint8_t *)_mat_a)[_1047];
          uint16_t _1049 = (uint16_t)(_1048);
          uint16x8_t _1050 = uint16x8_t::broadcast(_1049);
          uint16x8_t _1051 = _1046 * _1050;
          uint32x8_t _1052 = uint32x8_t::convert_from<uint16x8_t>(_1051);
          uint32x8_t _1053 = _1042 + _1052;
          int32_t _1054 = _1032 * 2;
          int32_t _1055 = _1054 + _40;
          int32_t _1056 = _1055 * 2;
          int32_t _1057 = _1056 + 8;
          uint8x8_t _1058 = uint8x8_t::load(_mat_b, _1057);
          uint16x8_t _1059 = uint16x8_t::convert_from<uint8x8_t>(_1058);
          int32_t _1060 = _1027 + 2;
          uint8_t _1061 = ((const uint8_t *)_mat_a)[_1060];
          uint16_t _1062 = (uint16_t)(_1061);
          uint16x8_t _1063 = uint16x8_t::broadcast(_1062);
          uint16x8_t _1064 = _1059 * _1063;
          uint32x8_t _1065 = uint32x8_t::convert_from<uint16x8_t>(_1064);
          uint32x8_t _1066 = _1053 + _1065;
          int32_t _1067 = _1033 + _553;
          int32_t _1068 = _1067 + 8;
          uint8x8_t _1069 = uint8x8_t::load(_mat_b, _1068);
          uint16x8_t _1070 = uint16x8_t::convert_from<uint8x8_t>(_1069);
          int32_t _1071 = _1027 + 3;
          uint8_t _1072 = ((const uint8_t *)_mat_a)[_1071];
          uint16_t _1073 = (uint16_t)(_1072);
          uint16x8_t _1074 = uint16x8_t::broadcast(_1073);
          uint16x8_t _1075 = _1070 * _1074;
          uint32x8_t _1076 = uint32x8_t::convert_from<uint16x8_t>(_1075);
          uint32x8_t _1077 = _1066 + _1076;
          _1077.store(_multiplied_no_offsets, _636);
          int32_t _1078 = _590 * _620;
          int32_t _1079 = _1078 * 4;
          int32_t _1080 = _1079 + _616;
          uint32x8_t _1081 = uint32x8_t::load(_multiplied_no_offsets, _1080);
          int32_t _1082 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _1083 = _1082 * _40;
          int32_t _1084 = _1083 + _614;
          int32_t _1085 = _1084 * 4;
          int32_t _1086 = _1085 + 16;
          uint8x8_t _1087 = uint8x8_t::load(_mat_b, _1086);
          uint16x8_t _1088 = uint16x8_t::convert_from<uint8x8_t>(_1087);
          int32_t _1089 = _31 * _620;
          int32_t _1090 = _1089 + _619;
          int32_t _1091 = _1090 + _multiplied_no_offsets_s1_k__x_rki;
          int32_t _1092 = _1091 * 4;
          uint8_t _1093 = ((const uint8_t *)_mat_a)[_1092];
          uint16_t _1094 = (uint16_t)(_1093);
          uint16x8_t _1095 = uint16x8_t::broadcast(_1094);
          uint16x8_t _1096 = _1088 * _1095;
          uint32x8_t _1097 = uint32x8_t::convert_from<uint16x8_t>(_1096);
          uint32x8_t _1098 = _1081 + _1097;
          int32_t _1099 = _1085 + _40;
          int32_t _1100 = _1099 + 16;
          uint8x8_t _1101 = uint8x8_t::load(_mat_b, _1100);
          uint16x8_t _1102 = uint16x8_t::convert_from<uint8x8_t>(_1101);
          int32_t _1103 = _1092 + 1;
          uint8_t _1104 = ((const uint8_t *)_mat_a)[_1103];
          uint16_t _1105 = (uint16_t)(_1104);
          uint16x8_t _1106 = uint16x8_t::broadcast(_1105);
          uint16x8_t _1107 = _1102 * _1106;
          uint32x8_t _1108 = uint32x8_t::convert_from<uint16x8_t>(_1107);
          uint32x8_t _1109 = _1098 + _1108;
          int32_t _1110 = _1084 * 2;
          int32_t _1111 = _1110 + _40;
          int32_t _1112 = _1111 * 2;
          int32_t _1113 = _1112 + 16;
          uint8x8_t _1114 = uint8x8_t::load(_mat_b, _1113);
          uint16x8_t _1115 = uint16x8_t::convert_from<uint8x8_t>(_1114);
          int32_t _1116 = _1092 + 2;
          uint8_t _1117 = ((const uint8_t *)_mat_a)[_1116];
          uint16_t _1118 = (uint16_t)(_1117);
          uint16x8_t _1119 = uint16x8_t::broadcast(_1118);
          uint16x8_t _1120 = _1115 * _1119;
          uint32x8_t _1121 = uint32x8_t::convert_from<uint16x8_t>(_1120);
          uint32x8_t _1122 = _1109 + _1121;
          int32_t _1123 = _1085 + _553;
          int32_t _1124 = _1123 + 16;
          uint8x8_t _1125 = uint8x8_t::load(_mat_b, _1124);
          uint16x8_t _1126 = uint16x8_t::convert_from<uint8x8_t>(_1125);
          int32_t _1127 = _1092 + 3;
          uint8_t _1128 = ((const uint8_t *)_mat_a)[_1127];
          uint16_t _1129 = (uint16_t)(_1128);
          uint16x8_t _1130 = uint16x8_t::broadcast(_1129);
          uint16x8_t _1131 = _1126 * _1130;
          uint32x8_t _1132 = uint32x8_t::convert_from<uint16x8_t>(_1131);
          uint32x8_t _1133 = _1122 + _1132;
          _1133.store(_multiplied_no_offsets, _631);
          int32_t _1134 = _620 * 4;
          int32_t _1135 = _1134 + 1;
          int32_t _1136 = _1135 * _31;
          int32_t _1137 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _1138 = _1137 * 4;
          int32_t _1139 = _1136 + _1138;
          int32_t _1140 = _1135 * _590;
          int32_t _1141 = _1140 + _616;
          uint32x8_t _1142 = uint32x8_t::load(_multiplied_no_offsets, _1141);
          int32_t _1143 = _1137 * _40;
          int32_t _1144 = _1143 + _614;
          int32_t _1145 = _1144 * 4;
          int32_t _1146 = _1145 + 16;
          uint8x8_t _1147 = uint8x8_t::load(_mat_b, _1146);
          uint16x8_t _1148 = uint16x8_t::convert_from<uint8x8_t>(_1147);
          uint8_t _1149 = ((const uint8_t *)_mat_a)[_1139];
          uint16_t _1150 = (uint16_t)(_1149);
          uint16x8_t _1151 = uint16x8_t::broadcast(_1150);
          uint16x8_t _1152 = _1148 * _1151;
          uint32x8_t _1153 = uint32x8_t::convert_from<uint16x8_t>(_1152);
          uint32x8_t _1154 = _1142 + _1153;
          int32_t _1155 = _1145 + _40;
          int32_t _1156 = _1155 + 16;
          uint8x8_t _1157 = uint8x8_t::load(_mat_b, _1156);
          uint16x8_t _1158 = uint16x8_t::convert_from<uint8x8_t>(_1157);
          int32_t _1159 = _1139 + 1;
          uint8_t _1160 = ((const uint8_t *)_mat_a)[_1159];
          uint16_t _1161 = (uint16_t)(_1160);
          uint16x8_t _1162 = uint16x8_t::broadcast(_1161);
          uint16x8_t _1163 = _1158 * _1162;
          uint32x8_t _1164 = uint32x8_t::convert_from<uint16x8_t>(_1163);
          uint32x8_t _1165 = _1154 + _1164;
          int32_t _1166 = _1144 * 2;
          int32_t _1167 = _1166 + _40;
          int32_t _1168 = _1167 * 2;
          int32_t _1169 = _1168 + 16;
          uint8x8_t _1170 = uint8x8_t::load(_mat_b, _1169);
          uint16x8_t _1171 = uint16x8_t::convert_from<uint8x8_t>(_1170);
          int32_t _1172 = _1139 + 2;
          uint8_t _1173 = ((const uint8_t *)_mat_a)[_1172];
          uint16_t _1174 = (uint16_t)(_1173);
          uint16x8_t _1175 = uint16x8_t::broadcast(_1174);
          uint16x8_t _1176 = _1171 * _1175;
          uint32x8_t _1177 = uint32x8_t::convert_from<uint16x8_t>(_1176);
          uint32x8_t _1178 = _1165 + _1177;
          int32_t _1179 = _1145 + _553;
          int32_t _1180 = _1179 + 16;
          uint8x8_t _1181 = uint8x8_t::load(_mat_b, _1180);
          uint16x8_t _1182 = uint16x8_t::convert_from<uint8x8_t>(_1181);
          int32_t _1183 = _1139 + 3;
          uint8_t _1184 = ((const uint8_t *)_mat_a)[_1183];
          uint16_t _1185 = (uint16_t)(_1184);
          uint16x8_t _1186 = uint16x8_t::broadcast(_1185);
          uint16x8_t _1187 = _1182 * _1186;
          uint32x8_t _1188 = uint32x8_t::convert_from<uint16x8_t>(_1187);
          uint32x8_t _1189 = _1178 + _1188;
          _1189.store(_multiplied_no_offsets, _643);
          int32_t _1190 = _620 * 4;
          int32_t _1191 = _1190 + 2;
          int32_t _1192 = _1191 * _31;
          int32_t _1193 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _1194 = _1193 * 4;
          int32_t _1195 = _1192 + _1194;
          int32_t _1196 = _1191 * _590;
          int32_t _1197 = _1196 + _616;
          uint32x8_t _1198 = uint32x8_t::load(_multiplied_no_offsets, _1197);
          int32_t _1199 = _1193 * _40;
          int32_t _1200 = _1199 + _614;
          int32_t _1201 = _1200 * 4;
          int32_t _1202 = _1201 + 16;
          uint8x8_t _1203 = uint8x8_t::load(_mat_b, _1202);
          uint16x8_t _1204 = uint16x8_t::convert_from<uint8x8_t>(_1203);
          uint8_t _1205 = ((const uint8_t *)_mat_a)[_1195];
          uint16_t _1206 = (uint16_t)(_1205);
          uint16x8_t _1207 = uint16x8_t::broadcast(_1206);
          uint16x8_t _1208 = _1204 * _1207;
          uint32x8_t _1209 = uint32x8_t::convert_from<uint16x8_t>(_1208);
          uint32x8_t _1210 = _1198 + _1209;
          int32_t _1211 = _1201 + _40;
          int32_t _1212 = _1211 + 16;
          uint8x8_t _1213 = uint8x8_t::load(_mat_b, _1212);
          uint16x8_t _1214 = uint16x8_t::convert_from<uint8x8_t>(_1213);
          int32_t _1215 = _1195 + 1;
          uint8_t _1216 = ((const uint8_t *)_mat_a)[_1215];
          uint16_t _1217 = (uint16_t)(_1216);
          uint16x8_t _1218 = uint16x8_t::broadcast(_1217);
          uint16x8_t _1219 = _1214 * _1218;
          uint32x8_t _1220 = uint32x8_t::convert_from<uint16x8_t>(_1219);
          uint32x8_t _1221 = _1210 + _1220;
          int32_t _1222 = _1200 * 2;
          int32_t _1223 = _1222 + _40;
          int32_t _1224 = _1223 * 2;
          int32_t _1225 = _1224 + 16;
          uint8x8_t _1226 = uint8x8_t::load(_mat_b, _1225);
          uint16x8_t _1227 = uint16x8_t::convert_from<uint8x8_t>(_1226);
          int32_t _1228 = _1195 + 2;
          uint8_t _1229 = ((const uint8_t *)_mat_a)[_1228];
          uint16_t _1230 = (uint16_t)(_1229);
          uint16x8_t _1231 = uint16x8_t::broadcast(_1230);
          uint16x8_t _1232 = _1227 * _1231;
          uint32x8_t _1233 = uint32x8_t::convert_from<uint16x8_t>(_1232);
          uint32x8_t _1234 = _1221 + _1233;
          int32_t _1235 = _1201 + _553;
          int32_t _1236 = _1235 + 16;
          uint8x8_t _1237 = uint8x8_t::load(_mat_b, _1236);
          uint16x8_t _1238 = uint16x8_t::convert_from<uint8x8_t>(_1237);
          int32_t _1239 = _1195 + 3;
          uint8_t _1240 = ((const uint8_t *)_mat_a)[_1239];
          uint16_t _1241 = (uint16_t)(_1240);
          uint16x8_t _1242 = uint16x8_t::broadcast(_1241);
          uint16x8_t _1243 = _1238 * _1242;
          uint32x8_t _1244 = uint32x8_t::convert_from<uint16x8_t>(_1243);
          uint32x8_t _1245 = _1234 + _1244;
          _1245.store(_multiplied_no_offsets, _639);
          int32_t _1246 = _620 * 4;
          int32_t _1247 = _1246 + 3;
          int32_t _1248 = _1247 * _31;
          int32_t _1249 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _1250 = _1249 * 4;
          int32_t _1251 = _1248 + _1250;
          int32_t _1252 = _1247 * _590;
          int32_t _1253 = _1252 + _616;
          uint32x8_t _1254 = uint32x8_t::load(_multiplied_no_offsets, _1253);
          int32_t _1255 = _1249 * _40;
          int32_t _1256 = _1255 + _614;
          int32_t _1257 = _1256 * 4;
          int32_t _1258 = _1257 + 16;
          uint8x8_t _1259 = uint8x8_t::load(_mat_b, _1258);
          uint16x8_t _1260 = uint16x8_t::convert_from<uint8x8_t>(_1259);
          uint8_t _1261 = ((const uint8_t *)_mat_a)[_1251];
          uint16_t _1262 = (uint16_t)(_1261);
          uint16x8_t _1263 = uint16x8_t::broadcast(_1262);
          uint16x8_t _1264 = _1260 * _1263;
          uint32x8_t _1265 = uint32x8_t::convert_from<uint16x8_t>(_1264);
          uint32x8_t _1266 = _1254 + _1265;
          int32_t _1267 = _1257 + _40;
          int32_t _1268 = _1267 + 16;
          uint8x8_t _1269 = uint8x8_t::load(_mat_b, _1268);
          uint16x8_t _1270 = uint16x8_t::convert_from<uint8x8_t>(_1269);
          int32_t _1271 = _1251 + 1;
          uint8_t _1272 = ((const uint8_t *)_mat_a)[_1271];
          uint16_t _1273 = (uint16_t)(_1272);
          uint16x8_t _1274 = uint16x8_t::broadcast(_1273);
          uint16x8_t _1275 = _1270 * _1274;
          uint32x8_t _1276 = uint32x8_t::convert_from<uint16x8_t>(_1275);
          uint32x8_t _1277 = _1266 + _1276;
          int32_t _1278 = _1256 * 2;
          int32_t _1279 = _1278 + _40;
          int32_t _1280 = _1279 * 2;
          int32_t _1281 = _1280 + 16;
          uint8x8_t _1282 = uint8x8_t::load(_mat_b, _1281);
          uint16x8_t _1283 = uint16x8_t::convert_from<uint8x8_t>(_1282);
          int32_t _1284 = _1251 + 2;
          uint8_t _1285 = ((const uint8_t *)_mat_a)[_1284];
          uint16_t _1286 = (uint16_t)(_1285);
          uint16x8_t _1287 = uint16x8_t::broadcast(_1286);
          uint16x8_t _1288 = _1283 * _1287;
          uint32x8_t _1289 = uint32x8_t::convert_from<uint16x8_t>(_1288);
          uint32x8_t _1290 = _1277 + _1289;
          int32_t _1291 = _1257 + _553;
          int32_t _1292 = _1291 + 16;
          uint8x8_t _1293 = uint8x8_t::load(_mat_b, _1292);
          uint16x8_t _1294 = uint16x8_t::convert_from<uint8x8_t>(_1293);
          int32_t _1295 = _1251 + 3;
          uint8_t _1296 = ((const uint8_t *)_mat_a)[_1295];
          uint16_t _1297 = (uint16_t)(_1296);
          uint16x8_t _1298 = uint16x8_t::broadcast(_1297);
          uint16x8_t _1299 = _1294 * _1298;
          uint32x8_t _1300 = uint32x8_t::convert_from<uint16x8_t>(_1299);
          uint32x8_t _1301 = _1290 + _1300;
          _1301.store(_multiplied_no_offsets, _635);
          int32_t _1302 = _590 * _620;
          int32_t _1303 = _1302 * 4;
          int32_t _1304 = _1303 + _615;
          uint32x8_t _1305 = uint32x8_t::load(_multiplied_no_offsets, _1304);
          int32_t _1306 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _1307 = _1306 * _40;
          int32_t _1308 = _1307 + _614;
          int32_t _1309 = _1308 * 4;
          int32_t _1310 = _1309 + 24;
          uint8x8_t _1311 = uint8x8_t::load(_mat_b, _1310);
          uint16x8_t _1312 = uint16x8_t::convert_from<uint8x8_t>(_1311);
          int32_t _1313 = _31 * _620;
          int32_t _1314 = _1313 + _619;
          int32_t _1315 = _1314 + _multiplied_no_offsets_s1_k__x_rki;
          int32_t _1316 = _1315 * 4;
          uint8_t _1317 = ((const uint8_t *)_mat_a)[_1316];
          uint16_t _1318 = (uint16_t)(_1317);
          uint16x8_t _1319 = uint16x8_t::broadcast(_1318);
          uint16x8_t _1320 = _1312 * _1319;
          uint32x8_t _1321 = uint32x8_t::convert_from<uint16x8_t>(_1320);
          uint32x8_t _1322 = _1305 + _1321;
          int32_t _1323 = _1309 + _40;
          int32_t _1324 = _1323 + 24;
          uint8x8_t _1325 = uint8x8_t::load(_mat_b, _1324);
          uint16x8_t _1326 = uint16x8_t::convert_from<uint8x8_t>(_1325);
          int32_t _1327 = _1316 + 1;
          uint8_t _1328 = ((const uint8_t *)_mat_a)[_1327];
          uint16_t _1329 = (uint16_t)(_1328);
          uint16x8_t _1330 = uint16x8_t::broadcast(_1329);
          uint16x8_t _1331 = _1326 * _1330;
          uint32x8_t _1332 = uint32x8_t::convert_from<uint16x8_t>(_1331);
          uint32x8_t _1333 = _1322 + _1332;
          int32_t _1334 = _1308 * 2;
          int32_t _1335 = _1334 + _40;
          int32_t _1336 = _1335 * 2;
          int32_t _1337 = _1336 + 24;
          uint8x8_t _1338 = uint8x8_t::load(_mat_b, _1337);
          uint16x8_t _1339 = uint16x8_t::convert_from<uint8x8_t>(_1338);
          int32_t _1340 = _1316 + 2;
          uint8_t _1341 = ((const uint8_t *)_mat_a)[_1340];
          uint16_t _1342 = (uint16_t)(_1341);
          uint16x8_t _1343 = uint16x8_t::broadcast(_1342);
          uint16x8_t _1344 = _1339 * _1343;
          uint32x8_t _1345 = uint32x8_t::convert_from<uint16x8_t>(_1344);
          uint32x8_t _1346 = _1333 + _1345;
          int32_t _1347 = _1309 + _553;
          int32_t _1348 = _1347 + 24;
          uint8x8_t _1349 = uint8x8_t::load(_mat_b, _1348);
          uint16x8_t _1350 = uint16x8_t::convert_from<uint8x8_t>(_1349);
          int32_t _1351 = _1316 + 3;
          uint8_t _1352 = ((const uint8_t *)_mat_a)[_1351];
          uint16_t _1353 = (uint16_t)(_1352);
          uint16x8_t _1354 = uint16x8_t::broadcast(_1353);
          uint16x8_t _1355 = _1350 * _1354;
          uint32x8_t _1356 = uint32x8_t::convert_from<uint16x8_t>(_1355);
          uint32x8_t _1357 = _1346 + _1356;
          _1357.store(_multiplied_no_offsets, _630);
          int32_t _1358 = _620 * 4;
          int32_t _1359 = _1358 + 1;
          int32_t _1360 = _1359 * _31;
          int32_t _1361 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _1362 = _1361 * 4;
          int32_t _1363 = _1360 + _1362;
          int32_t _1364 = _1359 * _590;
          int32_t _1365 = _1364 + _615;
          uint32x8_t _1366 = uint32x8_t::load(_multiplied_no_offsets, _1365);
          int32_t _1367 = _1361 * _40;
          int32_t _1368 = _1367 + _614;
          int32_t _1369 = _1368 * 4;
          int32_t _1370 = _1369 + 24;
          uint8x8_t _1371 = uint8x8_t::load(_mat_b, _1370);
          uint16x8_t _1372 = uint16x8_t::convert_from<uint8x8_t>(_1371);
          uint8_t _1373 = ((const uint8_t *)_mat_a)[_1363];
          uint16_t _1374 = (uint16_t)(_1373);
          uint16x8_t _1375 = uint16x8_t::broadcast(_1374);
          uint16x8_t _1376 = _1372 * _1375;
          uint32x8_t _1377 = uint32x8_t::convert_from<uint16x8_t>(_1376);
          uint32x8_t _1378 = _1366 + _1377;
          int32_t _1379 = _1369 + _40;
          int32_t _1380 = _1379 + 24;
          uint8x8_t _1381 = uint8x8_t::load(_mat_b, _1380);
          uint16x8_t _1382 = uint16x8_t::convert_from<uint8x8_t>(_1381);
          int32_t _1383 = _1363 + 1;
          uint8_t _1384 = ((const uint8_t *)_mat_a)[_1383];
          uint16_t _1385 = (uint16_t)(_1384);
          uint16x8_t _1386 = uint16x8_t::broadcast(_1385);
          uint16x8_t _1387 = _1382 * _1386;
          uint32x8_t _1388 = uint32x8_t::convert_from<uint16x8_t>(_1387);
          uint32x8_t _1389 = _1378 + _1388;
          int32_t _1390 = _1368 * 2;
          int32_t _1391 = _1390 + _40;
          int32_t _1392 = _1391 * 2;
          int32_t _1393 = _1392 + 24;
          uint8x8_t _1394 = uint8x8_t::load(_mat_b, _1393);
          uint16x8_t _1395 = uint16x8_t::convert_from<uint8x8_t>(_1394);
          int32_t _1396 = _1363 + 2;
          uint8_t _1397 = ((const uint8_t *)_mat_a)[_1396];
          uint16_t _1398 = (uint16_t)(_1397);
          uint16x8_t _1399 = uint16x8_t::broadcast(_1398);
          uint16x8_t _1400 = _1395 * _1399;
          uint32x8_t _1401 = uint32x8_t::convert_from<uint16x8_t>(_1400);
          uint32x8_t _1402 = _1389 + _1401;
          int32_t _1403 = _1369 + _553;
          int32_t _1404 = _1403 + 24;
          uint8x8_t _1405 = uint8x8_t::load(_mat_b, _1404);
          uint16x8_t _1406 = uint16x8_t::convert_from<uint8x8_t>(_1405);
          int32_t _1407 = _1363 + 3;
          uint8_t _1408 = ((const uint8_t *)_mat_a)[_1407];
          uint16_t _1409 = (uint16_t)(_1408);
          uint16x8_t _1410 = uint16x8_t::broadcast(_1409);
          uint16x8_t _1411 = _1406 * _1410;
          uint32x8_t _1412 = uint32x8_t::convert_from<uint16x8_t>(_1411);
          uint32x8_t _1413 = _1402 + _1412;
          _1413.store(_multiplied_no_offsets, _642);
          int32_t _1414 = _620 * 4;
          int32_t _1415 = _1414 + 2;
          int32_t _1416 = _1415 * _31;
          int32_t _1417 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _1418 = _1417 * 4;
          int32_t _1419 = _1416 + _1418;
          int32_t _1420 = _1415 * _590;
          int32_t _1421 = _1420 + _615;
          uint32x8_t _1422 = uint32x8_t::load(_multiplied_no_offsets, _1421);
          int32_t _1423 = _1417 * _40;
          int32_t _1424 = _1423 + _614;
          int32_t _1425 = _1424 * 4;
          int32_t _1426 = _1425 + 24;
          uint8x8_t _1427 = uint8x8_t::load(_mat_b, _1426);
          uint16x8_t _1428 = uint16x8_t::convert_from<uint8x8_t>(_1427);
          uint8_t _1429 = ((const uint8_t *)_mat_a)[_1419];
          uint16_t _1430 = (uint16_t)(_1429);
          uint16x8_t _1431 = uint16x8_t::broadcast(_1430);
          uint16x8_t _1432 = _1428 * _1431;
          uint32x8_t _1433 = uint32x8_t::convert_from<uint16x8_t>(_1432);
          uint32x8_t _1434 = _1422 + _1433;
          int32_t _1435 = _1425 + _40;
          int32_t _1436 = _1435 + 24;
          uint8x8_t _1437 = uint8x8_t::load(_mat_b, _1436);
          uint16x8_t _1438 = uint16x8_t::convert_from<uint8x8_t>(_1437);
          int32_t _1439 = _1419 + 1;
          uint8_t _1440 = ((const uint8_t *)_mat_a)[_1439];
          uint16_t _1441 = (uint16_t)(_1440);
          uint16x8_t _1442 = uint16x8_t::broadcast(_1441);
          uint16x8_t _1443 = _1438 * _1442;
          uint32x8_t _1444 = uint32x8_t::convert_from<uint16x8_t>(_1443);
          uint32x8_t _1445 = _1434 + _1444;
          int32_t _1446 = _1424 * 2;
          int32_t _1447 = _1446 + _40;
          int32_t _1448 = _1447 * 2;
          int32_t _1449 = _1448 + 24;
          uint8x8_t _1450 = uint8x8_t::load(_mat_b, _1449);
          uint16x8_t _1451 = uint16x8_t::convert_from<uint8x8_t>(_1450);
          int32_t _1452 = _1419 + 2;
          uint8_t _1453 = ((const uint8_t *)_mat_a)[_1452];
          uint16_t _1454 = (uint16_t)(_1453);
          uint16x8_t _1455 = uint16x8_t::broadcast(_1454);
          uint16x8_t _1456 = _1451 * _1455;
          uint32x8_t _1457 = uint32x8_t::convert_from<uint16x8_t>(_1456);
          uint32x8_t _1458 = _1445 + _1457;
          int32_t _1459 = _1425 + _553;
          int32_t _1460 = _1459 + 24;
          uint8x8_t _1461 = uint8x8_t::load(_mat_b, _1460);
          uint16x8_t _1462 = uint16x8_t::convert_from<uint8x8_t>(_1461);
          int32_t _1463 = _1419 + 3;
          uint8_t _1464 = ((const uint8_t *)_mat_a)[_1463];
          uint16_t _1465 = (uint16_t)(_1464);
          uint16x8_t _1466 = uint16x8_t::broadcast(_1465);
          uint16x8_t _1467 = _1462 * _1466;
          uint32x8_t _1468 = uint32x8_t::convert_from<uint16x8_t>(_1467);
          uint32x8_t _1469 = _1458 + _1468;
          _1469.store(_multiplied_no_offsets, _638);
          int32_t _1470 = _620 * 4;
          int32_t _1471 = _1470 + 3;
          int32_t _1472 = _1471 * _31;
          int32_t _1473 = _multiplied_no_offsets_s1_k__x_rki + _619;
          int32_t _1474 = _1473 * 4;
          int32_t _1475 = _1472 + _1474;
          int32_t _1476 = _1471 * _590;
          int32_t _1477 = _1476 + _615;
          uint32x8_t _1478 = uint32x8_t::load(_multiplied_no_offsets, _1477);
          int32_t _1479 = _1473 * _40;
          int32_t _1480 = _1479 + _614;
          int32_t _1481 = _1480 * 4;
          int32_t _1482 = _1481 + 24;
          uint8x8_t _1483 = uint8x8_t::load(_mat_b, _1482);
          uint16x8_t _1484 = uint16x8_t::convert_from<uint8x8_t>(_1483);
          uint8_t _1485 = ((const uint8_t *)_mat_a)[_1475];
          uint16_t _1486 = (uint16_t)(_1485);
          uint16x8_t _1487 = uint16x8_t::broadcast(_1486);
          uint16x8_t _1488 = _1484 * _1487;
          uint32x8_t _1489 = uint32x8_t::convert_from<uint16x8_t>(_1488);
          uint32x8_t _1490 = _1478 + _1489;
          int32_t _1491 = _1481 + _40;
          int32_t _1492 = _1491 + 24;
          uint8x8_t _1493 = uint8x8_t::load(_mat_b, _1492);
          uint16x8_t _1494 = uint16x8_t::convert_from<uint8x8_t>(_1493);
          int32_t _1495 = _1475 + 1;
          uint8_t _1496 = ((const uint8_t *)_mat_a)[_1495];
          uint16_t _1497 = (uint16_t)(_1496);
          uint16x8_t _1498 = uint16x8_t::broadcast(_1497);
          uint16x8_t _1499 = _1494 * _1498;
          uint32x8_t _1500 = uint32x8_t::convert_from<uint16x8_t>(_1499);
          uint32x8_t _1501 = _1490 + _1500;
          int32_t _1502 = _1480 * 2;
          int32_t _1503 = _1502 + _40;
          int32_t _1504 = _1503 * 2;
          int32_t _1505 = _1504 + 24;
          uint8x8_t _1506 = uint8x8_t::load(_mat_b, _1505);
          uint16x8_t _1507 = uint16x8_t::convert_from<uint8x8_t>(_1506);
          int32_t _1508 = _1475 + 2;
          uint8_t _1509 = ((const uint8_t *)_mat_a)[_1508];
          uint16_t _1510 = (uint16_t)(_1509);
          uint16x8_t _1511 = uint16x8_t::broadcast(_1510);
          uint16x8_t _1512 = _1507 * _1511;
          uint32x8_t _1513 = uint32x8_t::convert_from<uint16x8_t>(_1512);
          uint32x8_t _1514 = _1501 + _1513;
          int32_t _1515 = _1481 + _553;
          int32_t _1516 = _1515 + 24;
          uint8x8_t _1517 = uint8x8_t::load(_mat_b, _1516);
          uint16x8_t _1518 = uint16x8_t::convert_from<uint8x8_t>(_1517);
          int32_t _1519 = _1475 + 3;
          uint8_t _1520 = ((const uint8_t *)_mat_a)[_1519];
          uint16_t _1521 = (uint16_t)(_1520);
          uint16x8_t _1522 = uint16x8_t::broadcast(_1521);
          uint16x8_t _1523 = _1518 * _1522;
          uint32x8_t _1524 = uint32x8_t::convert_from<uint16x8_t>(_1523);
          uint32x8_t _1525 = _1514 + _1524;
          _1525.store(_multiplied_no_offsets, _634);
         } // for _multiplied_no_offsets_s1_k__x_rki
        } // for _multiplied_no_offsets_s1_y_yi_yi
       } // for _multiplied_no_offsets_s1_k__x_k__x
       if (_589)
       {
        int32_t _1526 = _multiplied_no_offsets_s1_x_x * 32;
        int32_t _1527 = _multiplied_no_offsets_s1_x_x * 8;
        int32_t _1528 = _600 + _1526;
        int32_t _1529 = _601 + _1526;
        int32_t _1530 = _602 + _1526;
        int32_t _1531 = _603 + _1526;
        for (int _multiplied_no_offsets_s1_y_yi_yi = 0; _multiplied_no_offsets_s1_y_yi_yi < 0 + 8; _multiplied_no_offsets_s1_y_yi_yi++)
        {
         int32_t _1532 = _multiplied_no_offsets_s1_y_yi_yi + _612;
         int32_t _1533 = _1532 * 4;
         int32_t _1534 = _1533 + 1;
         int32_t _1535 = _1534 * _590;
         int32_t _1536 = _1533 + 2;
         int32_t _1537 = _1536 * _590;
         int32_t _1538 = _1533 + 3;
         int32_t _1539 = _1538 * _590;
         int32_t _1540 = _590 * _1532;
         int32_t _1541 = _1540 * 4;
         int32_t _1542 = _1528 + _1541;
         int32_t _1543 = _1529 + _1541;
         int32_t _1544 = _1530 + _1541;
         int32_t _1545 = _1531 + _1541;
         int32_t _1546 = _1528 + _1539;
         int32_t _1547 = _1529 + _1539;
         int32_t _1548 = _1530 + _1539;
         int32_t _1549 = _1531 + _1539;
         int32_t _1550 = _1528 + _1537;
         int32_t _1551 = _1529 + _1537;
         int32_t _1552 = _1530 + _1537;
         int32_t _1553 = _1531 + _1537;
         int32_t _1554 = _1528 + _1535;
         int32_t _1555 = _1529 + _1535;
         int32_t _1556 = _1530 + _1535;
         int32_t _1557 = _1531 + _1535;
         for (int _multiplied_no_offsets_s1_k__x_rki = 0; _multiplied_no_offsets_s1_k__x_rki < 0 + _611; _multiplied_no_offsets_s1_k__x_rki++)
         {
          int32_t _1558 = _590 * _1532;
          int32_t _1559 = _1558 * 4;
          int32_t _1560 = _1559 + _1531;
          uint32x8_t _1561 = uint32x8_t::load(_multiplied_no_offsets, _1560);
          int32_t _1562 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _1563 = _1562 * _40;
          int32_t _1564 = _1563 + _1527;
          int32_t _1565 = _1564 * 4;
          uint8x8_t _1566 = uint8x8_t::load(_mat_b, _1565);
          uint16x8_t _1567 = uint16x8_t::convert_from<uint8x8_t>(_1566);
          int32_t _1568 = _31 * _1532;
          int32_t _1569 = _1568 + _558;
          int32_t _1570 = _1569 + _multiplied_no_offsets_s1_k__x_rki;
          int32_t _1571 = _1570 * 4;
          uint8_t _1572 = ((const uint8_t *)_mat_a)[_1571];
          uint16_t _1573 = (uint16_t)(_1572);
          uint16x8_t _1574 = uint16x8_t::broadcast(_1573);
          uint16x8_t _1575 = _1567 * _1574;
          uint32x8_t _1576 = uint32x8_t::convert_from<uint16x8_t>(_1575);
          uint32x8_t _1577 = _1561 + _1576;
          int32_t _1578 = _1565 + _40;
          uint8x8_t _1579 = uint8x8_t::load(_mat_b, _1578);
          uint16x8_t _1580 = uint16x8_t::convert_from<uint8x8_t>(_1579);
          int32_t _1581 = _1571 + 1;
          uint8_t _1582 = ((const uint8_t *)_mat_a)[_1581];
          uint16_t _1583 = (uint16_t)(_1582);
          uint16x8_t _1584 = uint16x8_t::broadcast(_1583);
          uint16x8_t _1585 = _1580 * _1584;
          uint32x8_t _1586 = uint32x8_t::convert_from<uint16x8_t>(_1585);
          uint32x8_t _1587 = _1577 + _1586;
          int32_t _1588 = _1564 * 2;
          int32_t _1589 = _1588 + _40;
          int32_t _1590 = _1589 * 2;
          uint8x8_t _1591 = uint8x8_t::load(_mat_b, _1590);
          uint16x8_t _1592 = uint16x8_t::convert_from<uint8x8_t>(_1591);
          int32_t _1593 = _1571 + 2;
          uint8_t _1594 = ((const uint8_t *)_mat_a)[_1593];
          uint16_t _1595 = (uint16_t)(_1594);
          uint16x8_t _1596 = uint16x8_t::broadcast(_1595);
          uint16x8_t _1597 = _1592 * _1596;
          uint32x8_t _1598 = uint32x8_t::convert_from<uint16x8_t>(_1597);
          uint32x8_t _1599 = _1587 + _1598;
          int32_t _1600 = _1565 + _553;
          uint8x8_t _1601 = uint8x8_t::load(_mat_b, _1600);
          uint16x8_t _1602 = uint16x8_t::convert_from<uint8x8_t>(_1601);
          int32_t _1603 = _1571 + 3;
          uint8_t _1604 = ((const uint8_t *)_mat_a)[_1603];
          uint16_t _1605 = (uint16_t)(_1604);
          uint16x8_t _1606 = uint16x8_t::broadcast(_1605);
          uint16x8_t _1607 = _1602 * _1606;
          uint32x8_t _1608 = uint32x8_t::convert_from<uint16x8_t>(_1607);
          uint32x8_t _1609 = _1599 + _1608;
          _1609.store(_multiplied_no_offsets, _1545);
          int32_t _1610 = _1532 * 4;
          int32_t _1611 = _1610 + 1;
          int32_t _1612 = _1611 * _31;
          int32_t _1613 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _1614 = _1613 * 4;
          int32_t _1615 = _1612 + _1614;
          int32_t _1616 = _1611 * _590;
          int32_t _1617 = _1616 + _1531;
          uint32x8_t _1618 = uint32x8_t::load(_multiplied_no_offsets, _1617);
          int32_t _1619 = _1613 * _40;
          int32_t _1620 = _1619 + _1527;
          int32_t _1621 = _1620 * 4;
          uint8x8_t _1622 = uint8x8_t::load(_mat_b, _1621);
          uint16x8_t _1623 = uint16x8_t::convert_from<uint8x8_t>(_1622);
          uint8_t _1624 = ((const uint8_t *)_mat_a)[_1615];
          uint16_t _1625 = (uint16_t)(_1624);
          uint16x8_t _1626 = uint16x8_t::broadcast(_1625);
          uint16x8_t _1627 = _1623 * _1626;
          uint32x8_t _1628 = uint32x8_t::convert_from<uint16x8_t>(_1627);
          uint32x8_t _1629 = _1618 + _1628;
          int32_t _1630 = _1621 + _40;
          uint8x8_t _1631 = uint8x8_t::load(_mat_b, _1630);
          uint16x8_t _1632 = uint16x8_t::convert_from<uint8x8_t>(_1631);
          int32_t _1633 = _1615 + 1;
          uint8_t _1634 = ((const uint8_t *)_mat_a)[_1633];
          uint16_t _1635 = (uint16_t)(_1634);
          uint16x8_t _1636 = uint16x8_t::broadcast(_1635);
          uint16x8_t _1637 = _1632 * _1636;
          uint32x8_t _1638 = uint32x8_t::convert_from<uint16x8_t>(_1637);
          uint32x8_t _1639 = _1629 + _1638;
          int32_t _1640 = _1620 * 2;
          int32_t _1641 = _1640 + _40;
          int32_t _1642 = _1641 * 2;
          uint8x8_t _1643 = uint8x8_t::load(_mat_b, _1642);
          uint16x8_t _1644 = uint16x8_t::convert_from<uint8x8_t>(_1643);
          int32_t _1645 = _1615 + 2;
          uint8_t _1646 = ((const uint8_t *)_mat_a)[_1645];
          uint16_t _1647 = (uint16_t)(_1646);
          uint16x8_t _1648 = uint16x8_t::broadcast(_1647);
          uint16x8_t _1649 = _1644 * _1648;
          uint32x8_t _1650 = uint32x8_t::convert_from<uint16x8_t>(_1649);
          uint32x8_t _1651 = _1639 + _1650;
          int32_t _1652 = _1621 + _553;
          uint8x8_t _1653 = uint8x8_t::load(_mat_b, _1652);
          uint16x8_t _1654 = uint16x8_t::convert_from<uint8x8_t>(_1653);
          int32_t _1655 = _1615 + 3;
          uint8_t _1656 = ((const uint8_t *)_mat_a)[_1655];
          uint16_t _1657 = (uint16_t)(_1656);
          uint16x8_t _1658 = uint16x8_t::broadcast(_1657);
          uint16x8_t _1659 = _1654 * _1658;
          uint32x8_t _1660 = uint32x8_t::convert_from<uint16x8_t>(_1659);
          uint32x8_t _1661 = _1651 + _1660;
          _1661.store(_multiplied_no_offsets, _1557);
          int32_t _1662 = _1532 * 4;
          int32_t _1663 = _1662 + 2;
          int32_t _1664 = _1663 * _31;
          int32_t _1665 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _1666 = _1665 * 4;
          int32_t _1667 = _1664 + _1666;
          int32_t _1668 = _1663 * _590;
          int32_t _1669 = _1668 + _1531;
          uint32x8_t _1670 = uint32x8_t::load(_multiplied_no_offsets, _1669);
          int32_t _1671 = _1665 * _40;
          int32_t _1672 = _1671 + _1527;
          int32_t _1673 = _1672 * 4;
          uint8x8_t _1674 = uint8x8_t::load(_mat_b, _1673);
          uint16x8_t _1675 = uint16x8_t::convert_from<uint8x8_t>(_1674);
          uint8_t _1676 = ((const uint8_t *)_mat_a)[_1667];
          uint16_t _1677 = (uint16_t)(_1676);
          uint16x8_t _1678 = uint16x8_t::broadcast(_1677);
          uint16x8_t _1679 = _1675 * _1678;
          uint32x8_t _1680 = uint32x8_t::convert_from<uint16x8_t>(_1679);
          uint32x8_t _1681 = _1670 + _1680;
          int32_t _1682 = _1673 + _40;
          uint8x8_t _1683 = uint8x8_t::load(_mat_b, _1682);
          uint16x8_t _1684 = uint16x8_t::convert_from<uint8x8_t>(_1683);
          int32_t _1685 = _1667 + 1;
          uint8_t _1686 = ((const uint8_t *)_mat_a)[_1685];
          uint16_t _1687 = (uint16_t)(_1686);
          uint16x8_t _1688 = uint16x8_t::broadcast(_1687);
          uint16x8_t _1689 = _1684 * _1688;
          uint32x8_t _1690 = uint32x8_t::convert_from<uint16x8_t>(_1689);
          uint32x8_t _1691 = _1681 + _1690;
          int32_t _1692 = _1672 * 2;
          int32_t _1693 = _1692 + _40;
          int32_t _1694 = _1693 * 2;
          uint8x8_t _1695 = uint8x8_t::load(_mat_b, _1694);
          uint16x8_t _1696 = uint16x8_t::convert_from<uint8x8_t>(_1695);
          int32_t _1697 = _1667 + 2;
          uint8_t _1698 = ((const uint8_t *)_mat_a)[_1697];
          uint16_t _1699 = (uint16_t)(_1698);
          uint16x8_t _1700 = uint16x8_t::broadcast(_1699);
          uint16x8_t _1701 = _1696 * _1700;
          uint32x8_t _1702 = uint32x8_t::convert_from<uint16x8_t>(_1701);
          uint32x8_t _1703 = _1691 + _1702;
          int32_t _1704 = _1673 + _553;
          uint8x8_t _1705 = uint8x8_t::load(_mat_b, _1704);
          uint16x8_t _1706 = uint16x8_t::convert_from<uint8x8_t>(_1705);
          int32_t _1707 = _1667 + 3;
          uint8_t _1708 = ((const uint8_t *)_mat_a)[_1707];
          uint16_t _1709 = (uint16_t)(_1708);
          uint16x8_t _1710 = uint16x8_t::broadcast(_1709);
          uint16x8_t _1711 = _1706 * _1710;
          uint32x8_t _1712 = uint32x8_t::convert_from<uint16x8_t>(_1711);
          uint32x8_t _1713 = _1703 + _1712;
          _1713.store(_multiplied_no_offsets, _1553);
          int32_t _1714 = _1532 * 4;
          int32_t _1715 = _1714 + 3;
          int32_t _1716 = _1715 * _31;
          int32_t _1717 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _1718 = _1717 * 4;
          int32_t _1719 = _1716 + _1718;
          int32_t _1720 = _1715 * _590;
          int32_t _1721 = _1720 + _1531;
          uint32x8_t _1722 = uint32x8_t::load(_multiplied_no_offsets, _1721);
          int32_t _1723 = _1717 * _40;
          int32_t _1724 = _1723 + _1527;
          int32_t _1725 = _1724 * 4;
          uint8x8_t _1726 = uint8x8_t::load(_mat_b, _1725);
          uint16x8_t _1727 = uint16x8_t::convert_from<uint8x8_t>(_1726);
          uint8_t _1728 = ((const uint8_t *)_mat_a)[_1719];
          uint16_t _1729 = (uint16_t)(_1728);
          uint16x8_t _1730 = uint16x8_t::broadcast(_1729);
          uint16x8_t _1731 = _1727 * _1730;
          uint32x8_t _1732 = uint32x8_t::convert_from<uint16x8_t>(_1731);
          uint32x8_t _1733 = _1722 + _1732;
          int32_t _1734 = _1725 + _40;
          uint8x8_t _1735 = uint8x8_t::load(_mat_b, _1734);
          uint16x8_t _1736 = uint16x8_t::convert_from<uint8x8_t>(_1735);
          int32_t _1737 = _1719 + 1;
          uint8_t _1738 = ((const uint8_t *)_mat_a)[_1737];
          uint16_t _1739 = (uint16_t)(_1738);
          uint16x8_t _1740 = uint16x8_t::broadcast(_1739);
          uint16x8_t _1741 = _1736 * _1740;
          uint32x8_t _1742 = uint32x8_t::convert_from<uint16x8_t>(_1741);
          uint32x8_t _1743 = _1733 + _1742;
          int32_t _1744 = _1724 * 2;
          int32_t _1745 = _1744 + _40;
          int32_t _1746 = _1745 * 2;
          uint8x8_t _1747 = uint8x8_t::load(_mat_b, _1746);
          uint16x8_t _1748 = uint16x8_t::convert_from<uint8x8_t>(_1747);
          int32_t _1749 = _1719 + 2;
          uint8_t _1750 = ((const uint8_t *)_mat_a)[_1749];
          uint16_t _1751 = (uint16_t)(_1750);
          uint16x8_t _1752 = uint16x8_t::broadcast(_1751);
          uint16x8_t _1753 = _1748 * _1752;
          uint32x8_t _1754 = uint32x8_t::convert_from<uint16x8_t>(_1753);
          uint32x8_t _1755 = _1743 + _1754;
          int32_t _1756 = _1725 + _553;
          uint8x8_t _1757 = uint8x8_t::load(_mat_b, _1756);
          uint16x8_t _1758 = uint16x8_t::convert_from<uint8x8_t>(_1757);
          int32_t _1759 = _1719 + 3;
          uint8_t _1760 = ((const uint8_t *)_mat_a)[_1759];
          uint16_t _1761 = (uint16_t)(_1760);
          uint16x8_t _1762 = uint16x8_t::broadcast(_1761);
          uint16x8_t _1763 = _1758 * _1762;
          uint32x8_t _1764 = uint32x8_t::convert_from<uint16x8_t>(_1763);
          uint32x8_t _1765 = _1755 + _1764;
          _1765.store(_multiplied_no_offsets, _1549);
          int32_t _1766 = _590 * _1532;
          int32_t _1767 = _1766 * 4;
          int32_t _1768 = _1767 + _1530;
          uint32x8_t _1769 = uint32x8_t::load(_multiplied_no_offsets, _1768);
          int32_t _1770 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _1771 = _1770 * _40;
          int32_t _1772 = _1771 + _1527;
          int32_t _1773 = _1772 * 4;
          int32_t _1774 = _1773 + 8;
          uint8x8_t _1775 = uint8x8_t::load(_mat_b, _1774);
          uint16x8_t _1776 = uint16x8_t::convert_from<uint8x8_t>(_1775);
          int32_t _1777 = _31 * _1532;
          int32_t _1778 = _1777 + _558;
          int32_t _1779 = _1778 + _multiplied_no_offsets_s1_k__x_rki;
          int32_t _1780 = _1779 * 4;
          uint8_t _1781 = ((const uint8_t *)_mat_a)[_1780];
          uint16_t _1782 = (uint16_t)(_1781);
          uint16x8_t _1783 = uint16x8_t::broadcast(_1782);
          uint16x8_t _1784 = _1776 * _1783;
          uint32x8_t _1785 = uint32x8_t::convert_from<uint16x8_t>(_1784);
          uint32x8_t _1786 = _1769 + _1785;
          int32_t _1787 = _1773 + _40;
          int32_t _1788 = _1787 + 8;
          uint8x8_t _1789 = uint8x8_t::load(_mat_b, _1788);
          uint16x8_t _1790 = uint16x8_t::convert_from<uint8x8_t>(_1789);
          int32_t _1791 = _1780 + 1;
          uint8_t _1792 = ((const uint8_t *)_mat_a)[_1791];
          uint16_t _1793 = (uint16_t)(_1792);
          uint16x8_t _1794 = uint16x8_t::broadcast(_1793);
          uint16x8_t _1795 = _1790 * _1794;
          uint32x8_t _1796 = uint32x8_t::convert_from<uint16x8_t>(_1795);
          uint32x8_t _1797 = _1786 + _1796;
          int32_t _1798 = _1772 * 2;
          int32_t _1799 = _1798 + _40;
          int32_t _1800 = _1799 * 2;
          int32_t _1801 = _1800 + 8;
          uint8x8_t _1802 = uint8x8_t::load(_mat_b, _1801);
          uint16x8_t _1803 = uint16x8_t::convert_from<uint8x8_t>(_1802);
          int32_t _1804 = _1780 + 2;
          uint8_t _1805 = ((const uint8_t *)_mat_a)[_1804];
          uint16_t _1806 = (uint16_t)(_1805);
          uint16x8_t _1807 = uint16x8_t::broadcast(_1806);
          uint16x8_t _1808 = _1803 * _1807;
          uint32x8_t _1809 = uint32x8_t::convert_from<uint16x8_t>(_1808);
          uint32x8_t _1810 = _1797 + _1809;
          int32_t _1811 = _1773 + _553;
          int32_t _1812 = _1811 + 8;
          uint8x8_t _1813 = uint8x8_t::load(_mat_b, _1812);
          uint16x8_t _1814 = uint16x8_t::convert_from<uint8x8_t>(_1813);
          int32_t _1815 = _1780 + 3;
          uint8_t _1816 = ((const uint8_t *)_mat_a)[_1815];
          uint16_t _1817 = (uint16_t)(_1816);
          uint16x8_t _1818 = uint16x8_t::broadcast(_1817);
          uint16x8_t _1819 = _1814 * _1818;
          uint32x8_t _1820 = uint32x8_t::convert_from<uint16x8_t>(_1819);
          uint32x8_t _1821 = _1810 + _1820;
          _1821.store(_multiplied_no_offsets, _1544);
          int32_t _1822 = _1532 * 4;
          int32_t _1823 = _1822 + 1;
          int32_t _1824 = _1823 * _31;
          int32_t _1825 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _1826 = _1825 * 4;
          int32_t _1827 = _1824 + _1826;
          int32_t _1828 = _1823 * _590;
          int32_t _1829 = _1828 + _1530;
          uint32x8_t _1830 = uint32x8_t::load(_multiplied_no_offsets, _1829);
          int32_t _1831 = _1825 * _40;
          int32_t _1832 = _1831 + _1527;
          int32_t _1833 = _1832 * 4;
          int32_t _1834 = _1833 + 8;
          uint8x8_t _1835 = uint8x8_t::load(_mat_b, _1834);
          uint16x8_t _1836 = uint16x8_t::convert_from<uint8x8_t>(_1835);
          uint8_t _1837 = ((const uint8_t *)_mat_a)[_1827];
          uint16_t _1838 = (uint16_t)(_1837);
          uint16x8_t _1839 = uint16x8_t::broadcast(_1838);
          uint16x8_t _1840 = _1836 * _1839;
          uint32x8_t _1841 = uint32x8_t::convert_from<uint16x8_t>(_1840);
          uint32x8_t _1842 = _1830 + _1841;
          int32_t _1843 = _1833 + _40;
          int32_t _1844 = _1843 + 8;
          uint8x8_t _1845 = uint8x8_t::load(_mat_b, _1844);
          uint16x8_t _1846 = uint16x8_t::convert_from<uint8x8_t>(_1845);
          int32_t _1847 = _1827 + 1;
          uint8_t _1848 = ((const uint8_t *)_mat_a)[_1847];
          uint16_t _1849 = (uint16_t)(_1848);
          uint16x8_t _1850 = uint16x8_t::broadcast(_1849);
          uint16x8_t _1851 = _1846 * _1850;
          uint32x8_t _1852 = uint32x8_t::convert_from<uint16x8_t>(_1851);
          uint32x8_t _1853 = _1842 + _1852;
          int32_t _1854 = _1832 * 2;
          int32_t _1855 = _1854 + _40;
          int32_t _1856 = _1855 * 2;
          int32_t _1857 = _1856 + 8;
          uint8x8_t _1858 = uint8x8_t::load(_mat_b, _1857);
          uint16x8_t _1859 = uint16x8_t::convert_from<uint8x8_t>(_1858);
          int32_t _1860 = _1827 + 2;
          uint8_t _1861 = ((const uint8_t *)_mat_a)[_1860];
          uint16_t _1862 = (uint16_t)(_1861);
          uint16x8_t _1863 = uint16x8_t::broadcast(_1862);
          uint16x8_t _1864 = _1859 * _1863;
          uint32x8_t _1865 = uint32x8_t::convert_from<uint16x8_t>(_1864);
          uint32x8_t _1866 = _1853 + _1865;
          int32_t _1867 = _1833 + _553;
          int32_t _1868 = _1867 + 8;
          uint8x8_t _1869 = uint8x8_t::load(_mat_b, _1868);
          uint16x8_t _1870 = uint16x8_t::convert_from<uint8x8_t>(_1869);
          int32_t _1871 = _1827 + 3;
          uint8_t _1872 = ((const uint8_t *)_mat_a)[_1871];
          uint16_t _1873 = (uint16_t)(_1872);
          uint16x8_t _1874 = uint16x8_t::broadcast(_1873);
          uint16x8_t _1875 = _1870 * _1874;
          uint32x8_t _1876 = uint32x8_t::convert_from<uint16x8_t>(_1875);
          uint32x8_t _1877 = _1866 + _1876;
          _1877.store(_multiplied_no_offsets, _1556);
          int32_t _1878 = _1532 * 4;
          int32_t _1879 = _1878 + 2;
          int32_t _1880 = _1879 * _31;
          int32_t _1881 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _1882 = _1881 * 4;
          int32_t _1883 = _1880 + _1882;
          int32_t _1884 = _1879 * _590;
          int32_t _1885 = _1884 + _1530;
          uint32x8_t _1886 = uint32x8_t::load(_multiplied_no_offsets, _1885);
          int32_t _1887 = _1881 * _40;
          int32_t _1888 = _1887 + _1527;
          int32_t _1889 = _1888 * 4;
          int32_t _1890 = _1889 + 8;
          uint8x8_t _1891 = uint8x8_t::load(_mat_b, _1890);
          uint16x8_t _1892 = uint16x8_t::convert_from<uint8x8_t>(_1891);
          uint8_t _1893 = ((const uint8_t *)_mat_a)[_1883];
          uint16_t _1894 = (uint16_t)(_1893);
          uint16x8_t _1895 = uint16x8_t::broadcast(_1894);
          uint16x8_t _1896 = _1892 * _1895;
          uint32x8_t _1897 = uint32x8_t::convert_from<uint16x8_t>(_1896);
          uint32x8_t _1898 = _1886 + _1897;
          int32_t _1899 = _1889 + _40;
          int32_t _1900 = _1899 + 8;
          uint8x8_t _1901 = uint8x8_t::load(_mat_b, _1900);
          uint16x8_t _1902 = uint16x8_t::convert_from<uint8x8_t>(_1901);
          int32_t _1903 = _1883 + 1;
          uint8_t _1904 = ((const uint8_t *)_mat_a)[_1903];
          uint16_t _1905 = (uint16_t)(_1904);
          uint16x8_t _1906 = uint16x8_t::broadcast(_1905);
          uint16x8_t _1907 = _1902 * _1906;
          uint32x8_t _1908 = uint32x8_t::convert_from<uint16x8_t>(_1907);
          uint32x8_t _1909 = _1898 + _1908;
          int32_t _1910 = _1888 * 2;
          int32_t _1911 = _1910 + _40;
          int32_t _1912 = _1911 * 2;
          int32_t _1913 = _1912 + 8;
          uint8x8_t _1914 = uint8x8_t::load(_mat_b, _1913);
          uint16x8_t _1915 = uint16x8_t::convert_from<uint8x8_t>(_1914);
          int32_t _1916 = _1883 + 2;
          uint8_t _1917 = ((const uint8_t *)_mat_a)[_1916];
          uint16_t _1918 = (uint16_t)(_1917);
          uint16x8_t _1919 = uint16x8_t::broadcast(_1918);
          uint16x8_t _1920 = _1915 * _1919;
          uint32x8_t _1921 = uint32x8_t::convert_from<uint16x8_t>(_1920);
          uint32x8_t _1922 = _1909 + _1921;
          int32_t _1923 = _1889 + _553;
          int32_t _1924 = _1923 + 8;
          uint8x8_t _1925 = uint8x8_t::load(_mat_b, _1924);
          uint16x8_t _1926 = uint16x8_t::convert_from<uint8x8_t>(_1925);
          int32_t _1927 = _1883 + 3;
          uint8_t _1928 = ((const uint8_t *)_mat_a)[_1927];
          uint16_t _1929 = (uint16_t)(_1928);
          uint16x8_t _1930 = uint16x8_t::broadcast(_1929);
          uint16x8_t _1931 = _1926 * _1930;
          uint32x8_t _1932 = uint32x8_t::convert_from<uint16x8_t>(_1931);
          uint32x8_t _1933 = _1922 + _1932;
          _1933.store(_multiplied_no_offsets, _1552);
          int32_t _1934 = _1532 * 4;
          int32_t _1935 = _1934 + 3;
          int32_t _1936 = _1935 * _31;
          int32_t _1937 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _1938 = _1937 * 4;
          int32_t _1939 = _1936 + _1938;
          int32_t _1940 = _1935 * _590;
          int32_t _1941 = _1940 + _1530;
          uint32x8_t _1942 = uint32x8_t::load(_multiplied_no_offsets, _1941);
          int32_t _1943 = _1937 * _40;
          int32_t _1944 = _1943 + _1527;
          int32_t _1945 = _1944 * 4;
          int32_t _1946 = _1945 + 8;
          uint8x8_t _1947 = uint8x8_t::load(_mat_b, _1946);
          uint16x8_t _1948 = uint16x8_t::convert_from<uint8x8_t>(_1947);
          uint8_t _1949 = ((const uint8_t *)_mat_a)[_1939];
          uint16_t _1950 = (uint16_t)(_1949);
          uint16x8_t _1951 = uint16x8_t::broadcast(_1950);
          uint16x8_t _1952 = _1948 * _1951;
          uint32x8_t _1953 = uint32x8_t::convert_from<uint16x8_t>(_1952);
          uint32x8_t _1954 = _1942 + _1953;
          int32_t _1955 = _1945 + _40;
          int32_t _1956 = _1955 + 8;
          uint8x8_t _1957 = uint8x8_t::load(_mat_b, _1956);
          uint16x8_t _1958 = uint16x8_t::convert_from<uint8x8_t>(_1957);
          int32_t _1959 = _1939 + 1;
          uint8_t _1960 = ((const uint8_t *)_mat_a)[_1959];
          uint16_t _1961 = (uint16_t)(_1960);
          uint16x8_t _1962 = uint16x8_t::broadcast(_1961);
          uint16x8_t _1963 = _1958 * _1962;
          uint32x8_t _1964 = uint32x8_t::convert_from<uint16x8_t>(_1963);
          uint32x8_t _1965 = _1954 + _1964;
          int32_t _1966 = _1944 * 2;
          int32_t _1967 = _1966 + _40;
          int32_t _1968 = _1967 * 2;
          int32_t _1969 = _1968 + 8;
          uint8x8_t _1970 = uint8x8_t::load(_mat_b, _1969);
          uint16x8_t _1971 = uint16x8_t::convert_from<uint8x8_t>(_1970);
          int32_t _1972 = _1939 + 2;
          uint8_t _1973 = ((const uint8_t *)_mat_a)[_1972];
          uint16_t _1974 = (uint16_t)(_1973);
          uint16x8_t _1975 = uint16x8_t::broadcast(_1974);
          uint16x8_t _1976 = _1971 * _1975;
          uint32x8_t _1977 = uint32x8_t::convert_from<uint16x8_t>(_1976);
          uint32x8_t _1978 = _1965 + _1977;
          int32_t _1979 = _1945 + _553;
          int32_t _1980 = _1979 + 8;
          uint8x8_t _1981 = uint8x8_t::load(_mat_b, _1980);
          uint16x8_t _1982 = uint16x8_t::convert_from<uint8x8_t>(_1981);
          int32_t _1983 = _1939 + 3;
          uint8_t _1984 = ((const uint8_t *)_mat_a)[_1983];
          uint16_t _1985 = (uint16_t)(_1984);
          uint16x8_t _1986 = uint16x8_t::broadcast(_1985);
          uint16x8_t _1987 = _1982 * _1986;
          uint32x8_t _1988 = uint32x8_t::convert_from<uint16x8_t>(_1987);
          uint32x8_t _1989 = _1978 + _1988;
          _1989.store(_multiplied_no_offsets, _1548);
          int32_t _1990 = _590 * _1532;
          int32_t _1991 = _1990 * 4;
          int32_t _1992 = _1991 + _1529;
          uint32x8_t _1993 = uint32x8_t::load(_multiplied_no_offsets, _1992);
          int32_t _1994 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _1995 = _1994 * _40;
          int32_t _1996 = _1995 + _1527;
          int32_t _1997 = _1996 * 4;
          int32_t _1998 = _1997 + 16;
          uint8x8_t _1999 = uint8x8_t::load(_mat_b, _1998);
          uint16x8_t _2000 = uint16x8_t::convert_from<uint8x8_t>(_1999);
          int32_t _2001 = _31 * _1532;
          int32_t _2002 = _2001 + _558;
          int32_t _2003 = _2002 + _multiplied_no_offsets_s1_k__x_rki;
          int32_t _2004 = _2003 * 4;
          uint8_t _2005 = ((const uint8_t *)_mat_a)[_2004];
          uint16_t _2006 = (uint16_t)(_2005);
          uint16x8_t _2007 = uint16x8_t::broadcast(_2006);
          uint16x8_t _2008 = _2000 * _2007;
          uint32x8_t _2009 = uint32x8_t::convert_from<uint16x8_t>(_2008);
          uint32x8_t _2010 = _1993 + _2009;
          int32_t _2011 = _1997 + _40;
          int32_t _2012 = _2011 + 16;
          uint8x8_t _2013 = uint8x8_t::load(_mat_b, _2012);
          uint16x8_t _2014 = uint16x8_t::convert_from<uint8x8_t>(_2013);
          int32_t _2015 = _2004 + 1;
          uint8_t _2016 = ((const uint8_t *)_mat_a)[_2015];
          uint16_t _2017 = (uint16_t)(_2016);
          uint16x8_t _2018 = uint16x8_t::broadcast(_2017);
          uint16x8_t _2019 = _2014 * _2018;
          uint32x8_t _2020 = uint32x8_t::convert_from<uint16x8_t>(_2019);
          uint32x8_t _2021 = _2010 + _2020;
          int32_t _2022 = _1996 * 2;
          int32_t _2023 = _2022 + _40;
          int32_t _2024 = _2023 * 2;
          int32_t _2025 = _2024 + 16;
          uint8x8_t _2026 = uint8x8_t::load(_mat_b, _2025);
          uint16x8_t _2027 = uint16x8_t::convert_from<uint8x8_t>(_2026);
          int32_t _2028 = _2004 + 2;
          uint8_t _2029 = ((const uint8_t *)_mat_a)[_2028];
          uint16_t _2030 = (uint16_t)(_2029);
          uint16x8_t _2031 = uint16x8_t::broadcast(_2030);
          uint16x8_t _2032 = _2027 * _2031;
          uint32x8_t _2033 = uint32x8_t::convert_from<uint16x8_t>(_2032);
          uint32x8_t _2034 = _2021 + _2033;
          int32_t _2035 = _1997 + _553;
          int32_t _2036 = _2035 + 16;
          uint8x8_t _2037 = uint8x8_t::load(_mat_b, _2036);
          uint16x8_t _2038 = uint16x8_t::convert_from<uint8x8_t>(_2037);
          int32_t _2039 = _2004 + 3;
          uint8_t _2040 = ((const uint8_t *)_mat_a)[_2039];
          uint16_t _2041 = (uint16_t)(_2040);
          uint16x8_t _2042 = uint16x8_t::broadcast(_2041);
          uint16x8_t _2043 = _2038 * _2042;
          uint32x8_t _2044 = uint32x8_t::convert_from<uint16x8_t>(_2043);
          uint32x8_t _2045 = _2034 + _2044;
          _2045.store(_multiplied_no_offsets, _1543);
          int32_t _2046 = _1532 * 4;
          int32_t _2047 = _2046 + 1;
          int32_t _2048 = _2047 * _31;
          int32_t _2049 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _2050 = _2049 * 4;
          int32_t _2051 = _2048 + _2050;
          int32_t _2052 = _2047 * _590;
          int32_t _2053 = _2052 + _1529;
          uint32x8_t _2054 = uint32x8_t::load(_multiplied_no_offsets, _2053);
          int32_t _2055 = _2049 * _40;
          int32_t _2056 = _2055 + _1527;
          int32_t _2057 = _2056 * 4;
          int32_t _2058 = _2057 + 16;
          uint8x8_t _2059 = uint8x8_t::load(_mat_b, _2058);
          uint16x8_t _2060 = uint16x8_t::convert_from<uint8x8_t>(_2059);
          uint8_t _2061 = ((const uint8_t *)_mat_a)[_2051];
          uint16_t _2062 = (uint16_t)(_2061);
          uint16x8_t _2063 = uint16x8_t::broadcast(_2062);
          uint16x8_t _2064 = _2060 * _2063;
          uint32x8_t _2065 = uint32x8_t::convert_from<uint16x8_t>(_2064);
          uint32x8_t _2066 = _2054 + _2065;
          int32_t _2067 = _2057 + _40;
          int32_t _2068 = _2067 + 16;
          uint8x8_t _2069 = uint8x8_t::load(_mat_b, _2068);
          uint16x8_t _2070 = uint16x8_t::convert_from<uint8x8_t>(_2069);
          int32_t _2071 = _2051 + 1;
          uint8_t _2072 = ((const uint8_t *)_mat_a)[_2071];
          uint16_t _2073 = (uint16_t)(_2072);
          uint16x8_t _2074 = uint16x8_t::broadcast(_2073);
          uint16x8_t _2075 = _2070 * _2074;
          uint32x8_t _2076 = uint32x8_t::convert_from<uint16x8_t>(_2075);
          uint32x8_t _2077 = _2066 + _2076;
          int32_t _2078 = _2056 * 2;
          int32_t _2079 = _2078 + _40;
          int32_t _2080 = _2079 * 2;
          int32_t _2081 = _2080 + 16;
          uint8x8_t _2082 = uint8x8_t::load(_mat_b, _2081);
          uint16x8_t _2083 = uint16x8_t::convert_from<uint8x8_t>(_2082);
          int32_t _2084 = _2051 + 2;
          uint8_t _2085 = ((const uint8_t *)_mat_a)[_2084];
          uint16_t _2086 = (uint16_t)(_2085);
          uint16x8_t _2087 = uint16x8_t::broadcast(_2086);
          uint16x8_t _2088 = _2083 * _2087;
          uint32x8_t _2089 = uint32x8_t::convert_from<uint16x8_t>(_2088);
          uint32x8_t _2090 = _2077 + _2089;
          int32_t _2091 = _2057 + _553;
          int32_t _2092 = _2091 + 16;
          uint8x8_t _2093 = uint8x8_t::load(_mat_b, _2092);
          uint16x8_t _2094 = uint16x8_t::convert_from<uint8x8_t>(_2093);
          int32_t _2095 = _2051 + 3;
          uint8_t _2096 = ((const uint8_t *)_mat_a)[_2095];
          uint16_t _2097 = (uint16_t)(_2096);
          uint16x8_t _2098 = uint16x8_t::broadcast(_2097);
          uint16x8_t _2099 = _2094 * _2098;
          uint32x8_t _2100 = uint32x8_t::convert_from<uint16x8_t>(_2099);
          uint32x8_t _2101 = _2090 + _2100;
          _2101.store(_multiplied_no_offsets, _1555);
          int32_t _2102 = _1532 * 4;
          int32_t _2103 = _2102 + 2;
          int32_t _2104 = _2103 * _31;
          int32_t _2105 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _2106 = _2105 * 4;
          int32_t _2107 = _2104 + _2106;
          int32_t _2108 = _2103 * _590;
          int32_t _2109 = _2108 + _1529;
          uint32x8_t _2110 = uint32x8_t::load(_multiplied_no_offsets, _2109);
          int32_t _2111 = _2105 * _40;
          int32_t _2112 = _2111 + _1527;
          int32_t _2113 = _2112 * 4;
          int32_t _2114 = _2113 + 16;
          uint8x8_t _2115 = uint8x8_t::load(_mat_b, _2114);
          uint16x8_t _2116 = uint16x8_t::convert_from<uint8x8_t>(_2115);
          uint8_t _2117 = ((const uint8_t *)_mat_a)[_2107];
          uint16_t _2118 = (uint16_t)(_2117);
          uint16x8_t _2119 = uint16x8_t::broadcast(_2118);
          uint16x8_t _2120 = _2116 * _2119;
          uint32x8_t _2121 = uint32x8_t::convert_from<uint16x8_t>(_2120);
          uint32x8_t _2122 = _2110 + _2121;
          int32_t _2123 = _2113 + _40;
          int32_t _2124 = _2123 + 16;
          uint8x8_t _2125 = uint8x8_t::load(_mat_b, _2124);
          uint16x8_t _2126 = uint16x8_t::convert_from<uint8x8_t>(_2125);
          int32_t _2127 = _2107 + 1;
          uint8_t _2128 = ((const uint8_t *)_mat_a)[_2127];
          uint16_t _2129 = (uint16_t)(_2128);
          uint16x8_t _2130 = uint16x8_t::broadcast(_2129);
          uint16x8_t _2131 = _2126 * _2130;
          uint32x8_t _2132 = uint32x8_t::convert_from<uint16x8_t>(_2131);
          uint32x8_t _2133 = _2122 + _2132;
          int32_t _2134 = _2112 * 2;
          int32_t _2135 = _2134 + _40;
          int32_t _2136 = _2135 * 2;
          int32_t _2137 = _2136 + 16;
          uint8x8_t _2138 = uint8x8_t::load(_mat_b, _2137);
          uint16x8_t _2139 = uint16x8_t::convert_from<uint8x8_t>(_2138);
          int32_t _2140 = _2107 + 2;
          uint8_t _2141 = ((const uint8_t *)_mat_a)[_2140];
          uint16_t _2142 = (uint16_t)(_2141);
          uint16x8_t _2143 = uint16x8_t::broadcast(_2142);
          uint16x8_t _2144 = _2139 * _2143;
          uint32x8_t _2145 = uint32x8_t::convert_from<uint16x8_t>(_2144);
          uint32x8_t _2146 = _2133 + _2145;
          int32_t _2147 = _2113 + _553;
          int32_t _2148 = _2147 + 16;
          uint8x8_t _2149 = uint8x8_t::load(_mat_b, _2148);
          uint16x8_t _2150 = uint16x8_t::convert_from<uint8x8_t>(_2149);
          int32_t _2151 = _2107 + 3;
          uint8_t _2152 = ((const uint8_t *)_mat_a)[_2151];
          uint16_t _2153 = (uint16_t)(_2152);
          uint16x8_t _2154 = uint16x8_t::broadcast(_2153);
          uint16x8_t _2155 = _2150 * _2154;
          uint32x8_t _2156 = uint32x8_t::convert_from<uint16x8_t>(_2155);
          uint32x8_t _2157 = _2146 + _2156;
          _2157.store(_multiplied_no_offsets, _1551);
          int32_t _2158 = _1532 * 4;
          int32_t _2159 = _2158 + 3;
          int32_t _2160 = _2159 * _31;
          int32_t _2161 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _2162 = _2161 * 4;
          int32_t _2163 = _2160 + _2162;
          int32_t _2164 = _2159 * _590;
          int32_t _2165 = _2164 + _1529;
          uint32x8_t _2166 = uint32x8_t::load(_multiplied_no_offsets, _2165);
          int32_t _2167 = _2161 * _40;
          int32_t _2168 = _2167 + _1527;
          int32_t _2169 = _2168 * 4;
          int32_t _2170 = _2169 + 16;
          uint8x8_t _2171 = uint8x8_t::load(_mat_b, _2170);
          uint16x8_t _2172 = uint16x8_t::convert_from<uint8x8_t>(_2171);
          uint8_t _2173 = ((const uint8_t *)_mat_a)[_2163];
          uint16_t _2174 = (uint16_t)(_2173);
          uint16x8_t _2175 = uint16x8_t::broadcast(_2174);
          uint16x8_t _2176 = _2172 * _2175;
          uint32x8_t _2177 = uint32x8_t::convert_from<uint16x8_t>(_2176);
          uint32x8_t _2178 = _2166 + _2177;
          int32_t _2179 = _2169 + _40;
          int32_t _2180 = _2179 + 16;
          uint8x8_t _2181 = uint8x8_t::load(_mat_b, _2180);
          uint16x8_t _2182 = uint16x8_t::convert_from<uint8x8_t>(_2181);
          int32_t _2183 = _2163 + 1;
          uint8_t _2184 = ((const uint8_t *)_mat_a)[_2183];
          uint16_t _2185 = (uint16_t)(_2184);
          uint16x8_t _2186 = uint16x8_t::broadcast(_2185);
          uint16x8_t _2187 = _2182 * _2186;
          uint32x8_t _2188 = uint32x8_t::convert_from<uint16x8_t>(_2187);
          uint32x8_t _2189 = _2178 + _2188;
          int32_t _2190 = _2168 * 2;
          int32_t _2191 = _2190 + _40;
          int32_t _2192 = _2191 * 2;
          int32_t _2193 = _2192 + 16;
          uint8x8_t _2194 = uint8x8_t::load(_mat_b, _2193);
          uint16x8_t _2195 = uint16x8_t::convert_from<uint8x8_t>(_2194);
          int32_t _2196 = _2163 + 2;
          uint8_t _2197 = ((const uint8_t *)_mat_a)[_2196];
          uint16_t _2198 = (uint16_t)(_2197);
          uint16x8_t _2199 = uint16x8_t::broadcast(_2198);
          uint16x8_t _2200 = _2195 * _2199;
          uint32x8_t _2201 = uint32x8_t::convert_from<uint16x8_t>(_2200);
          uint32x8_t _2202 = _2189 + _2201;
          int32_t _2203 = _2169 + _553;
          int32_t _2204 = _2203 + 16;
          uint8x8_t _2205 = uint8x8_t::load(_mat_b, _2204);
          uint16x8_t _2206 = uint16x8_t::convert_from<uint8x8_t>(_2205);
          int32_t _2207 = _2163 + 3;
          uint8_t _2208 = ((const uint8_t *)_mat_a)[_2207];
          uint16_t _2209 = (uint16_t)(_2208);
          uint16x8_t _2210 = uint16x8_t::broadcast(_2209);
          uint16x8_t _2211 = _2206 * _2210;
          uint32x8_t _2212 = uint32x8_t::convert_from<uint16x8_t>(_2211);
          uint32x8_t _2213 = _2202 + _2212;
          _2213.store(_multiplied_no_offsets, _1547);
          int32_t _2214 = _590 * _1532;
          int32_t _2215 = _2214 * 4;
          int32_t _2216 = _2215 + _1528;
          uint32x8_t _2217 = uint32x8_t::load(_multiplied_no_offsets, _2216);
          int32_t _2218 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _2219 = _2218 * _40;
          int32_t _2220 = _2219 + _1527;
          int32_t _2221 = _2220 * 4;
          int32_t _2222 = _2221 + 24;
          uint8x8_t _2223 = uint8x8_t::load(_mat_b, _2222);
          uint16x8_t _2224 = uint16x8_t::convert_from<uint8x8_t>(_2223);
          int32_t _2225 = _31 * _1532;
          int32_t _2226 = _2225 + _558;
          int32_t _2227 = _2226 + _multiplied_no_offsets_s1_k__x_rki;
          int32_t _2228 = _2227 * 4;
          uint8_t _2229 = ((const uint8_t *)_mat_a)[_2228];
          uint16_t _2230 = (uint16_t)(_2229);
          uint16x8_t _2231 = uint16x8_t::broadcast(_2230);
          uint16x8_t _2232 = _2224 * _2231;
          uint32x8_t _2233 = uint32x8_t::convert_from<uint16x8_t>(_2232);
          uint32x8_t _2234 = _2217 + _2233;
          int32_t _2235 = _2221 + _40;
          int32_t _2236 = _2235 + 24;
          uint8x8_t _2237 = uint8x8_t::load(_mat_b, _2236);
          uint16x8_t _2238 = uint16x8_t::convert_from<uint8x8_t>(_2237);
          int32_t _2239 = _2228 + 1;
          uint8_t _2240 = ((const uint8_t *)_mat_a)[_2239];
          uint16_t _2241 = (uint16_t)(_2240);
          uint16x8_t _2242 = uint16x8_t::broadcast(_2241);
          uint16x8_t _2243 = _2238 * _2242;
          uint32x8_t _2244 = uint32x8_t::convert_from<uint16x8_t>(_2243);
          uint32x8_t _2245 = _2234 + _2244;
          int32_t _2246 = _2220 * 2;
          int32_t _2247 = _2246 + _40;
          int32_t _2248 = _2247 * 2;
          int32_t _2249 = _2248 + 24;
          uint8x8_t _2250 = uint8x8_t::load(_mat_b, _2249);
          uint16x8_t _2251 = uint16x8_t::convert_from<uint8x8_t>(_2250);
          int32_t _2252 = _2228 + 2;
          uint8_t _2253 = ((const uint8_t *)_mat_a)[_2252];
          uint16_t _2254 = (uint16_t)(_2253);
          uint16x8_t _2255 = uint16x8_t::broadcast(_2254);
          uint16x8_t _2256 = _2251 * _2255;
          uint32x8_t _2257 = uint32x8_t::convert_from<uint16x8_t>(_2256);
          uint32x8_t _2258 = _2245 + _2257;
          int32_t _2259 = _2221 + _553;
          int32_t _2260 = _2259 + 24;
          uint8x8_t _2261 = uint8x8_t::load(_mat_b, _2260);
          uint16x8_t _2262 = uint16x8_t::convert_from<uint8x8_t>(_2261);
          int32_t _2263 = _2228 + 3;
          uint8_t _2264 = ((const uint8_t *)_mat_a)[_2263];
          uint16_t _2265 = (uint16_t)(_2264);
          uint16x8_t _2266 = uint16x8_t::broadcast(_2265);
          uint16x8_t _2267 = _2262 * _2266;
          uint32x8_t _2268 = uint32x8_t::convert_from<uint16x8_t>(_2267);
          uint32x8_t _2269 = _2258 + _2268;
          _2269.store(_multiplied_no_offsets, _1542);
          int32_t _2270 = _1532 * 4;
          int32_t _2271 = _2270 + 1;
          int32_t _2272 = _2271 * _31;
          int32_t _2273 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _2274 = _2273 * 4;
          int32_t _2275 = _2272 + _2274;
          int32_t _2276 = _2271 * _590;
          int32_t _2277 = _2276 + _1528;
          uint32x8_t _2278 = uint32x8_t::load(_multiplied_no_offsets, _2277);
          int32_t _2279 = _2273 * _40;
          int32_t _2280 = _2279 + _1527;
          int32_t _2281 = _2280 * 4;
          int32_t _2282 = _2281 + 24;
          uint8x8_t _2283 = uint8x8_t::load(_mat_b, _2282);
          uint16x8_t _2284 = uint16x8_t::convert_from<uint8x8_t>(_2283);
          uint8_t _2285 = ((const uint8_t *)_mat_a)[_2275];
          uint16_t _2286 = (uint16_t)(_2285);
          uint16x8_t _2287 = uint16x8_t::broadcast(_2286);
          uint16x8_t _2288 = _2284 * _2287;
          uint32x8_t _2289 = uint32x8_t::convert_from<uint16x8_t>(_2288);
          uint32x8_t _2290 = _2278 + _2289;
          int32_t _2291 = _2281 + _40;
          int32_t _2292 = _2291 + 24;
          uint8x8_t _2293 = uint8x8_t::load(_mat_b, _2292);
          uint16x8_t _2294 = uint16x8_t::convert_from<uint8x8_t>(_2293);
          int32_t _2295 = _2275 + 1;
          uint8_t _2296 = ((const uint8_t *)_mat_a)[_2295];
          uint16_t _2297 = (uint16_t)(_2296);
          uint16x8_t _2298 = uint16x8_t::broadcast(_2297);
          uint16x8_t _2299 = _2294 * _2298;
          uint32x8_t _2300 = uint32x8_t::convert_from<uint16x8_t>(_2299);
          uint32x8_t _2301 = _2290 + _2300;
          int32_t _2302 = _2280 * 2;
          int32_t _2303 = _2302 + _40;
          int32_t _2304 = _2303 * 2;
          int32_t _2305 = _2304 + 24;
          uint8x8_t _2306 = uint8x8_t::load(_mat_b, _2305);
          uint16x8_t _2307 = uint16x8_t::convert_from<uint8x8_t>(_2306);
          int32_t _2308 = _2275 + 2;
          uint8_t _2309 = ((const uint8_t *)_mat_a)[_2308];
          uint16_t _2310 = (uint16_t)(_2309);
          uint16x8_t _2311 = uint16x8_t::broadcast(_2310);
          uint16x8_t _2312 = _2307 * _2311;
          uint32x8_t _2313 = uint32x8_t::convert_from<uint16x8_t>(_2312);
          uint32x8_t _2314 = _2301 + _2313;
          int32_t _2315 = _2281 + _553;
          int32_t _2316 = _2315 + 24;
          uint8x8_t _2317 = uint8x8_t::load(_mat_b, _2316);
          uint16x8_t _2318 = uint16x8_t::convert_from<uint8x8_t>(_2317);
          int32_t _2319 = _2275 + 3;
          uint8_t _2320 = ((const uint8_t *)_mat_a)[_2319];
          uint16_t _2321 = (uint16_t)(_2320);
          uint16x8_t _2322 = uint16x8_t::broadcast(_2321);
          uint16x8_t _2323 = _2318 * _2322;
          uint32x8_t _2324 = uint32x8_t::convert_from<uint16x8_t>(_2323);
          uint32x8_t _2325 = _2314 + _2324;
          _2325.store(_multiplied_no_offsets, _1554);
          int32_t _2326 = _1532 * 4;
          int32_t _2327 = _2326 + 2;
          int32_t _2328 = _2327 * _31;
          int32_t _2329 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _2330 = _2329 * 4;
          int32_t _2331 = _2328 + _2330;
          int32_t _2332 = _2327 * _590;
          int32_t _2333 = _2332 + _1528;
          uint32x8_t _2334 = uint32x8_t::load(_multiplied_no_offsets, _2333);
          int32_t _2335 = _2329 * _40;
          int32_t _2336 = _2335 + _1527;
          int32_t _2337 = _2336 * 4;
          int32_t _2338 = _2337 + 24;
          uint8x8_t _2339 = uint8x8_t::load(_mat_b, _2338);
          uint16x8_t _2340 = uint16x8_t::convert_from<uint8x8_t>(_2339);
          uint8_t _2341 = ((const uint8_t *)_mat_a)[_2331];
          uint16_t _2342 = (uint16_t)(_2341);
          uint16x8_t _2343 = uint16x8_t::broadcast(_2342);
          uint16x8_t _2344 = _2340 * _2343;
          uint32x8_t _2345 = uint32x8_t::convert_from<uint16x8_t>(_2344);
          uint32x8_t _2346 = _2334 + _2345;
          int32_t _2347 = _2337 + _40;
          int32_t _2348 = _2347 + 24;
          uint8x8_t _2349 = uint8x8_t::load(_mat_b, _2348);
          uint16x8_t _2350 = uint16x8_t::convert_from<uint8x8_t>(_2349);
          int32_t _2351 = _2331 + 1;
          uint8_t _2352 = ((const uint8_t *)_mat_a)[_2351];
          uint16_t _2353 = (uint16_t)(_2352);
          uint16x8_t _2354 = uint16x8_t::broadcast(_2353);
          uint16x8_t _2355 = _2350 * _2354;
          uint32x8_t _2356 = uint32x8_t::convert_from<uint16x8_t>(_2355);
          uint32x8_t _2357 = _2346 + _2356;
          int32_t _2358 = _2336 * 2;
          int32_t _2359 = _2358 + _40;
          int32_t _2360 = _2359 * 2;
          int32_t _2361 = _2360 + 24;
          uint8x8_t _2362 = uint8x8_t::load(_mat_b, _2361);
          uint16x8_t _2363 = uint16x8_t::convert_from<uint8x8_t>(_2362);
          int32_t _2364 = _2331 + 2;
          uint8_t _2365 = ((const uint8_t *)_mat_a)[_2364];
          uint16_t _2366 = (uint16_t)(_2365);
          uint16x8_t _2367 = uint16x8_t::broadcast(_2366);
          uint16x8_t _2368 = _2363 * _2367;
          uint32x8_t _2369 = uint32x8_t::convert_from<uint16x8_t>(_2368);
          uint32x8_t _2370 = _2357 + _2369;
          int32_t _2371 = _2337 + _553;
          int32_t _2372 = _2371 + 24;
          uint8x8_t _2373 = uint8x8_t::load(_mat_b, _2372);
          uint16x8_t _2374 = uint16x8_t::convert_from<uint8x8_t>(_2373);
          int32_t _2375 = _2331 + 3;
          uint8_t _2376 = ((const uint8_t *)_mat_a)[_2375];
          uint16_t _2377 = (uint16_t)(_2376);
          uint16x8_t _2378 = uint16x8_t::broadcast(_2377);
          uint16x8_t _2379 = _2374 * _2378;
          uint32x8_t _2380 = uint32x8_t::convert_from<uint16x8_t>(_2379);
          uint32x8_t _2381 = _2370 + _2380;
          _2381.store(_multiplied_no_offsets, _1550);
          int32_t _2382 = _1532 * 4;
          int32_t _2383 = _2382 + 3;
          int32_t _2384 = _2383 * _31;
          int32_t _2385 = _multiplied_no_offsets_s1_k__x_rki + _558;
          int32_t _2386 = _2385 * 4;
          int32_t _2387 = _2384 + _2386;
          int32_t _2388 = _2383 * _590;
          int32_t _2389 = _2388 + _1528;
          uint32x8_t _2390 = uint32x8_t::load(_multiplied_no_offsets, _2389);
          int32_t _2391 = _2385 * _40;
          int32_t _2392 = _2391 + _1527;
          int32_t _2393 = _2392 * 4;
          int32_t _2394 = _2393 + 24;
          uint8x8_t _2395 = uint8x8_t::load(_mat_b, _2394);
          uint16x8_t _2396 = uint16x8_t::convert_from<uint8x8_t>(_2395);
          uint8_t _2397 = ((const uint8_t *)_mat_a)[_2387];
          uint16_t _2398 = (uint16_t)(_2397);
          uint16x8_t _2399 = uint16x8_t::broadcast(_2398);
          uint16x8_t _2400 = _2396 * _2399;
          uint32x8_t _2401 = uint32x8_t::convert_from<uint16x8_t>(_2400);
          uint32x8_t _2402 = _2390 + _2401;
          int32_t _2403 = _2393 + _40;
          int32_t _2404 = _2403 + 24;
          uint8x8_t _2405 = uint8x8_t::load(_mat_b, _2404);
          uint16x8_t _2406 = uint16x8_t::convert_from<uint8x8_t>(_2405);
          int32_t _2407 = _2387 + 1;
          uint8_t _2408 = ((const uint8_t *)_mat_a)[_2407];
          uint16_t _2409 = (uint16_t)(_2408);
          uint16x8_t _2410 = uint16x8_t::broadcast(_2409);
          uint16x8_t _2411 = _2406 * _2410;
          uint32x8_t _2412 = uint32x8_t::convert_from<uint16x8_t>(_2411);
          uint32x8_t _2413 = _2402 + _2412;
          int32_t _2414 = _2392 * 2;
          int32_t _2415 = _2414 + _40;
          int32_t _2416 = _2415 * 2;
          int32_t _2417 = _2416 + 24;
          uint8x8_t _2418 = uint8x8_t::load(_mat_b, _2417);
          uint16x8_t _2419 = uint16x8_t::convert_from<uint8x8_t>(_2418);
          int32_t _2420 = _2387 + 2;
          uint8_t _2421 = ((const uint8_t *)_mat_a)[_2420];
          uint16_t _2422 = (uint16_t)(_2421);
          uint16x8_t _2423 = uint16x8_t::broadcast(_2422);
          uint16x8_t _2424 = _2419 * _2423;
          uint32x8_t _2425 = uint32x8_t::convert_from<uint16x8_t>(_2424);
          uint32x8_t _2426 = _2413 + _2425;
          int32_t _2427 = _2393 + _553;
          int32_t _2428 = _2427 + 24;
          uint8x8_t _2429 = uint8x8_t::load(_mat_b, _2428);
          uint16x8_t _2430 = uint16x8_t::convert_from<uint8x8_t>(_2429);
          int32_t _2431 = _2387 + 3;
          uint8_t _2432 = ((const uint8_t *)_mat_a)[_2431];
          uint16_t _2433 = (uint16_t)(_2432);
          uint16x8_t _2434 = uint16x8_t::broadcast(_2433);
          uint16x8_t _2435 = _2430 * _2434;
          uint32x8_t _2436 = uint32x8_t::convert_from<uint16x8_t>(_2435);
          uint32x8_t _2437 = _2426 + _2436;
          _2437.store(_multiplied_no_offsets, _1546);
         } // for _multiplied_no_offsets_s1_k__x_rki
        } // for _multiplied_no_offsets_s1_y_yi_yi
       } // if _589
      } // for _multiplied_no_offsets_s1_x_x
      if (_587)
      {
       int32_t _2438 = _599 * 16;
       int32_t _2439 = ::halide_cpp_min(_2438, 8);
       int32_t _2440 = ::halide_cpp_max(_2439, 0);
       int32_t _2441 = _546 * 8;
       int32_t _2442 = _multiplied_no_offsets_s1_y_y * 8;
       int32_t _2443 = ::halide_cpp_min(_2438, 24);
       int32_t _2444 = ::halide_cpp_max(_2443, 16);
       int32_t _2445 = _2444 + -16;
       int32_t _2446 = ::halide_cpp_min(_599, 2);
       int32_t _2447 = _2446 * 16;
       int32_t _2448 = ::halide_cpp_max(_2447, 24);
       int32_t _2449 = _2448 + -24;
       int32_t _2450 = ::halide_cpp_min(_599, 1);
       int32_t _2451 = _2450 * 16;
       int32_t _2452 = ::halide_cpp_max(_2451, 8);
       int32_t _2453 = _2452 + -8;
       for (int _multiplied_no_offsets_s1_k__x_k__x = 0; _multiplied_no_offsets_s1_k__x_k__x < 0 + _596; _multiplied_no_offsets_s1_k__x_k__x++)
       {
        int32_t _2454 = _multiplied_no_offsets_s1_k__x_k__x * 32;
        int32_t _2455 = _556 - _2454;
        for (int _multiplied_no_offsets_s1_y_yi_yi = 0; _multiplied_no_offsets_s1_y_yi_yi < 0 + 8; _multiplied_no_offsets_s1_y_yi_yi++)
        {
         int32_t _2456 = _multiplied_no_offsets_s1_y_yi_yi + _2442;
         int32_t _2457 = _2456 * 4;
         int32_t _2458 = _2457 + 1;
         int32_t _2459 = _2458 * _590;
         int32_t _2460 = _2457 + 2;
         int32_t _2461 = _2460 * _590;
         int32_t _2462 = _2457 + 3;
         int32_t _2463 = _2462 * _590;
         int32_t _2464 = _590 * _2456;
         int32_t _2465 = _2464 * 4;
         int32_t _2466 = ::halide_cpp_min(_2455, 32);
         int32_t _2467 = _604 + _2465;
         int32_t _2468 = _605 + _2465;
         int32_t _2469 = _606 + _2465;
         int32_t _2470 = _607 + _2465;
         int32_t _2471 = _604 + _2463;
         int32_t _2472 = _605 + _2463;
         int32_t _2473 = _606 + _2463;
         int32_t _2474 = _607 + _2463;
         int32_t _2475 = _604 + _2461;
         int32_t _2476 = _605 + _2461;
         int32_t _2477 = _606 + _2461;
         int32_t _2478 = _607 + _2461;
         int32_t _2479 = _604 + _2459;
         int32_t _2480 = _605 + _2459;
         int32_t _2481 = _606 + _2459;
         int32_t _2482 = _607 + _2459;
         for (int _multiplied_no_offsets_s1_k__x_rki = 0; _multiplied_no_offsets_s1_k__x_rki < 0 + _2466; _multiplied_no_offsets_s1_k__x_rki++)
         {
          if (_581)
          {
           int32_t _2483 = _590 * _2456;
           int32_t _2484 = _2483 * 4;
           int32_t _2485 = _2484 + _607;
           uint32x8_t _2486 = uint32x8_t::load(_multiplied_no_offsets, _2485);
           int32_t _2487 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _2488 = _2487 * _40;
           int32_t _2489 = _2488 + _2441;
           int32_t _2490 = _2489 * 4;
           uint8x8_t _2491 = uint8x8_t::load(_mat_b, _2490);
           uint16x8_t _2492 = uint16x8_t::convert_from<uint8x8_t>(_2491);
           int32_t _2493 = _31 * _2456;
           int32_t _2494 = _2493 + _2454;
           int32_t _2495 = _2494 + _multiplied_no_offsets_s1_k__x_rki;
           int32_t _2496 = _2495 * 4;
           uint8_t _2497 = ((const uint8_t *)_mat_a)[_2496];
           uint16_t _2498 = (uint16_t)(_2497);
           uint16x8_t _2499 = uint16x8_t::broadcast(_2498);
           uint16x8_t _2500 = _2492 * _2499;
           uint32x8_t _2501 = uint32x8_t::convert_from<uint16x8_t>(_2500);
           uint32x8_t _2502 = _2486 + _2501;
           int32_t _2503 = _2490 + _40;
           uint8x8_t _2504 = uint8x8_t::load(_mat_b, _2503);
           uint16x8_t _2505 = uint16x8_t::convert_from<uint8x8_t>(_2504);
           int32_t _2506 = _2496 + 1;
           uint8_t _2507 = ((const uint8_t *)_mat_a)[_2506];
           uint16_t _2508 = (uint16_t)(_2507);
           uint16x8_t _2509 = uint16x8_t::broadcast(_2508);
           uint16x8_t _2510 = _2505 * _2509;
           uint32x8_t _2511 = uint32x8_t::convert_from<uint16x8_t>(_2510);
           uint32x8_t _2512 = _2502 + _2511;
           int32_t _2513 = _2489 * 2;
           int32_t _2514 = _2513 + _40;
           int32_t _2515 = _2514 * 2;
           uint8x8_t _2516 = uint8x8_t::load(_mat_b, _2515);
           uint16x8_t _2517 = uint16x8_t::convert_from<uint8x8_t>(_2516);
           int32_t _2518 = _2496 + 2;
           uint8_t _2519 = ((const uint8_t *)_mat_a)[_2518];
           uint16_t _2520 = (uint16_t)(_2519);
           uint16x8_t _2521 = uint16x8_t::broadcast(_2520);
           uint16x8_t _2522 = _2517 * _2521;
           uint32x8_t _2523 = uint32x8_t::convert_from<uint16x8_t>(_2522);
           uint32x8_t _2524 = _2512 + _2523;
           int32_t _2525 = _2490 + _553;
           uint8x8_t _2526 = uint8x8_t::load(_mat_b, _2525);
           uint16x8_t _2527 = uint16x8_t::convert_from<uint8x8_t>(_2526);
           int32_t _2528 = _2496 + 3;
           uint8_t _2529 = ((const uint8_t *)_mat_a)[_2528];
           uint16_t _2530 = (uint16_t)(_2529);
           uint16x8_t _2531 = uint16x8_t::broadcast(_2530);
           uint16x8_t _2532 = _2527 * _2531;
           uint32x8_t _2533 = uint32x8_t::convert_from<uint16x8_t>(_2532);
           uint32x8_t _2534 = _2524 + _2533;
           _2534.store(_multiplied_no_offsets, _2470);
           int32_t _2535 = _2456 * 4;
           int32_t _2536 = _2535 + 1;
           int32_t _2537 = _2536 * _31;
           int32_t _2538 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _2539 = _2538 * 4;
           int32_t _2540 = _2537 + _2539;
           int32_t _2541 = _2536 * _590;
           int32_t _2542 = _2541 + _607;
           uint32x8_t _2543 = uint32x8_t::load(_multiplied_no_offsets, _2542);
           int32_t _2544 = _2538 * _40;
           int32_t _2545 = _2544 + _2441;
           int32_t _2546 = _2545 * 4;
           uint8x8_t _2547 = uint8x8_t::load(_mat_b, _2546);
           uint16x8_t _2548 = uint16x8_t::convert_from<uint8x8_t>(_2547);
           uint8_t _2549 = ((const uint8_t *)_mat_a)[_2540];
           uint16_t _2550 = (uint16_t)(_2549);
           uint16x8_t _2551 = uint16x8_t::broadcast(_2550);
           uint16x8_t _2552 = _2548 * _2551;
           uint32x8_t _2553 = uint32x8_t::convert_from<uint16x8_t>(_2552);
           uint32x8_t _2554 = _2543 + _2553;
           int32_t _2555 = _2546 + _40;
           uint8x8_t _2556 = uint8x8_t::load(_mat_b, _2555);
           uint16x8_t _2557 = uint16x8_t::convert_from<uint8x8_t>(_2556);
           int32_t _2558 = _2540 + 1;
           uint8_t _2559 = ((const uint8_t *)_mat_a)[_2558];
           uint16_t _2560 = (uint16_t)(_2559);
           uint16x8_t _2561 = uint16x8_t::broadcast(_2560);
           uint16x8_t _2562 = _2557 * _2561;
           uint32x8_t _2563 = uint32x8_t::convert_from<uint16x8_t>(_2562);
           uint32x8_t _2564 = _2554 + _2563;
           int32_t _2565 = _2545 * 2;
           int32_t _2566 = _2565 + _40;
           int32_t _2567 = _2566 * 2;
           uint8x8_t _2568 = uint8x8_t::load(_mat_b, _2567);
           uint16x8_t _2569 = uint16x8_t::convert_from<uint8x8_t>(_2568);
           int32_t _2570 = _2540 + 2;
           uint8_t _2571 = ((const uint8_t *)_mat_a)[_2570];
           uint16_t _2572 = (uint16_t)(_2571);
           uint16x8_t _2573 = uint16x8_t::broadcast(_2572);
           uint16x8_t _2574 = _2569 * _2573;
           uint32x8_t _2575 = uint32x8_t::convert_from<uint16x8_t>(_2574);
           uint32x8_t _2576 = _2564 + _2575;
           int32_t _2577 = _2546 + _553;
           uint8x8_t _2578 = uint8x8_t::load(_mat_b, _2577);
           uint16x8_t _2579 = uint16x8_t::convert_from<uint8x8_t>(_2578);
           int32_t _2580 = _2540 + 3;
           uint8_t _2581 = ((const uint8_t *)_mat_a)[_2580];
           uint16_t _2582 = (uint16_t)(_2581);
           uint16x8_t _2583 = uint16x8_t::broadcast(_2582);
           uint16x8_t _2584 = _2579 * _2583;
           uint32x8_t _2585 = uint32x8_t::convert_from<uint16x8_t>(_2584);
           uint32x8_t _2586 = _2576 + _2585;
           _2586.store(_multiplied_no_offsets, _2482);
           int32_t _2587 = _2456 * 4;
           int32_t _2588 = _2587 + 2;
           int32_t _2589 = _2588 * _31;
           int32_t _2590 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _2591 = _2590 * 4;
           int32_t _2592 = _2589 + _2591;
           int32_t _2593 = _2588 * _590;
           int32_t _2594 = _2593 + _607;
           uint32x8_t _2595 = uint32x8_t::load(_multiplied_no_offsets, _2594);
           int32_t _2596 = _2590 * _40;
           int32_t _2597 = _2596 + _2441;
           int32_t _2598 = _2597 * 4;
           uint8x8_t _2599 = uint8x8_t::load(_mat_b, _2598);
           uint16x8_t _2600 = uint16x8_t::convert_from<uint8x8_t>(_2599);
           uint8_t _2601 = ((const uint8_t *)_mat_a)[_2592];
           uint16_t _2602 = (uint16_t)(_2601);
           uint16x8_t _2603 = uint16x8_t::broadcast(_2602);
           uint16x8_t _2604 = _2600 * _2603;
           uint32x8_t _2605 = uint32x8_t::convert_from<uint16x8_t>(_2604);
           uint32x8_t _2606 = _2595 + _2605;
           int32_t _2607 = _2598 + _40;
           uint8x8_t _2608 = uint8x8_t::load(_mat_b, _2607);
           uint16x8_t _2609 = uint16x8_t::convert_from<uint8x8_t>(_2608);
           int32_t _2610 = _2592 + 1;
           uint8_t _2611 = ((const uint8_t *)_mat_a)[_2610];
           uint16_t _2612 = (uint16_t)(_2611);
           uint16x8_t _2613 = uint16x8_t::broadcast(_2612);
           uint16x8_t _2614 = _2609 * _2613;
           uint32x8_t _2615 = uint32x8_t::convert_from<uint16x8_t>(_2614);
           uint32x8_t _2616 = _2606 + _2615;
           int32_t _2617 = _2597 * 2;
           int32_t _2618 = _2617 + _40;
           int32_t _2619 = _2618 * 2;
           uint8x8_t _2620 = uint8x8_t::load(_mat_b, _2619);
           uint16x8_t _2621 = uint16x8_t::convert_from<uint8x8_t>(_2620);
           int32_t _2622 = _2592 + 2;
           uint8_t _2623 = ((const uint8_t *)_mat_a)[_2622];
           uint16_t _2624 = (uint16_t)(_2623);
           uint16x8_t _2625 = uint16x8_t::broadcast(_2624);
           uint16x8_t _2626 = _2621 * _2625;
           uint32x8_t _2627 = uint32x8_t::convert_from<uint16x8_t>(_2626);
           uint32x8_t _2628 = _2616 + _2627;
           int32_t _2629 = _2598 + _553;
           uint8x8_t _2630 = uint8x8_t::load(_mat_b, _2629);
           uint16x8_t _2631 = uint16x8_t::convert_from<uint8x8_t>(_2630);
           int32_t _2632 = _2592 + 3;
           uint8_t _2633 = ((const uint8_t *)_mat_a)[_2632];
           uint16_t _2634 = (uint16_t)(_2633);
           uint16x8_t _2635 = uint16x8_t::broadcast(_2634);
           uint16x8_t _2636 = _2631 * _2635;
           uint32x8_t _2637 = uint32x8_t::convert_from<uint16x8_t>(_2636);
           uint32x8_t _2638 = _2628 + _2637;
           _2638.store(_multiplied_no_offsets, _2478);
           int32_t _2639 = _2456 * 4;
           int32_t _2640 = _2639 + 3;
           int32_t _2641 = _2640 * _31;
           int32_t _2642 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _2643 = _2642 * 4;
           int32_t _2644 = _2641 + _2643;
           int32_t _2645 = _2640 * _590;
           int32_t _2646 = _2645 + _607;
           uint32x8_t _2647 = uint32x8_t::load(_multiplied_no_offsets, _2646);
           int32_t _2648 = _2642 * _40;
           int32_t _2649 = _2648 + _2441;
           int32_t _2650 = _2649 * 4;
           uint8x8_t _2651 = uint8x8_t::load(_mat_b, _2650);
           uint16x8_t _2652 = uint16x8_t::convert_from<uint8x8_t>(_2651);
           uint8_t _2653 = ((const uint8_t *)_mat_a)[_2644];
           uint16_t _2654 = (uint16_t)(_2653);
           uint16x8_t _2655 = uint16x8_t::broadcast(_2654);
           uint16x8_t _2656 = _2652 * _2655;
           uint32x8_t _2657 = uint32x8_t::convert_from<uint16x8_t>(_2656);
           uint32x8_t _2658 = _2647 + _2657;
           int32_t _2659 = _2650 + _40;
           uint8x8_t _2660 = uint8x8_t::load(_mat_b, _2659);
           uint16x8_t _2661 = uint16x8_t::convert_from<uint8x8_t>(_2660);
           int32_t _2662 = _2644 + 1;
           uint8_t _2663 = ((const uint8_t *)_mat_a)[_2662];
           uint16_t _2664 = (uint16_t)(_2663);
           uint16x8_t _2665 = uint16x8_t::broadcast(_2664);
           uint16x8_t _2666 = _2661 * _2665;
           uint32x8_t _2667 = uint32x8_t::convert_from<uint16x8_t>(_2666);
           uint32x8_t _2668 = _2658 + _2667;
           int32_t _2669 = _2649 * 2;
           int32_t _2670 = _2669 + _40;
           int32_t _2671 = _2670 * 2;
           uint8x8_t _2672 = uint8x8_t::load(_mat_b, _2671);
           uint16x8_t _2673 = uint16x8_t::convert_from<uint8x8_t>(_2672);
           int32_t _2674 = _2644 + 2;
           uint8_t _2675 = ((const uint8_t *)_mat_a)[_2674];
           uint16_t _2676 = (uint16_t)(_2675);
           uint16x8_t _2677 = uint16x8_t::broadcast(_2676);
           uint16x8_t _2678 = _2673 * _2677;
           uint32x8_t _2679 = uint32x8_t::convert_from<uint16x8_t>(_2678);
           uint32x8_t _2680 = _2668 + _2679;
           int32_t _2681 = _2650 + _553;
           uint8x8_t _2682 = uint8x8_t::load(_mat_b, _2681);
           uint16x8_t _2683 = uint16x8_t::convert_from<uint8x8_t>(_2682);
           int32_t _2684 = _2644 + 3;
           uint8_t _2685 = ((const uint8_t *)_mat_a)[_2684];
           uint16_t _2686 = (uint16_t)(_2685);
           uint16x8_t _2687 = uint16x8_t::broadcast(_2686);
           uint16x8_t _2688 = _2683 * _2687;
           uint32x8_t _2689 = uint32x8_t::convert_from<uint16x8_t>(_2688);
           uint32x8_t _2690 = _2680 + _2689;
           _2690.store(_multiplied_no_offsets, _2474);
          } // if _581
          else
          {
           int32_t _2691 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _2692 = _2691 * _40;
           int32_t _2693 = _2692 * 4;
           int32_t _2694 = _2693 + _547;
           int32_t _2695 = _590 * _2456;
           int32_t _2696 = _2695 * 4;
           int32_t _2697 = _2696 + _607;
           int32_t _2698 = _31 * _2456;
           int32_t _2699 = _2698 + _2454;
           int32_t _2700 = _2699 + _multiplied_no_offsets_s1_k__x_rki;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2440; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _2701 = _multiplied_no_offsets_s1_x_xi_xii + _2697;
            uint32_t _2702 = _multiplied_no_offsets[_2701];
            int32_t _2703 = _2700 * 4;
            uint8_t _2704 = ((const uint8_t *)_mat_a)[_2703];
            uint16_t _2705 = (uint16_t)(_2704);
            int32_t _2706 = _multiplied_no_offsets_s1_x_xi_xii + _2694;
            uint8_t _2707 = ((const uint8_t *)_mat_b)[_2706];
            uint16_t _2708 = (uint16_t)(_2707);
            uint16_t _2709 = _2705 * _2708;
            uint32_t _2710 = (uint32_t)(_2709);
            uint32_t _2711 = _2702 + _2710;
            int32_t _2712 = _2703 + 1;
            uint8_t _2713 = ((const uint8_t *)_mat_a)[_2712];
            uint16_t _2714 = (uint16_t)(_2713);
            int32_t _2715 = _2706 + _40;
            uint8_t _2716 = ((const uint8_t *)_mat_b)[_2715];
            uint16_t _2717 = (uint16_t)(_2716);
            uint16_t _2718 = _2714 * _2717;
            uint32_t _2719 = (uint32_t)(_2718);
            uint32_t _2720 = _2711 + _2719;
            int32_t _2721 = _2703 + 2;
            uint8_t _2722 = ((const uint8_t *)_mat_a)[_2721];
            uint16_t _2723 = (uint16_t)(_2722);
            int32_t _2724 = _2706 + _552;
            uint8_t _2725 = ((const uint8_t *)_mat_b)[_2724];
            uint16_t _2726 = (uint16_t)(_2725);
            uint16_t _2727 = _2723 * _2726;
            uint32_t _2728 = (uint32_t)(_2727);
            uint32_t _2729 = _2720 + _2728;
            int32_t _2730 = _2703 + 3;
            uint8_t _2731 = ((const uint8_t *)_mat_a)[_2730];
            uint16_t _2732 = (uint16_t)(_2731);
            int32_t _2733 = _2706 + _553;
            uint8_t _2734 = ((const uint8_t *)_mat_b)[_2733];
            uint16_t _2735 = (uint16_t)(_2734);
            uint16_t _2736 = _2732 * _2735;
            uint32_t _2737 = (uint32_t)(_2736);
            uint32_t _2738 = _2729 + _2737;
            int32_t _2739 = _multiplied_no_offsets_s1_x_xi_xii + _2470;
            _multiplied_no_offsets[_2739] = _2738;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _2740 = _2456 * 4;
           int32_t _2741 = _2740 + 1;
           int32_t _2742 = _31 * _2741;
           int32_t _2743 = _590 * _2741;
           int32_t _2744 = _2743 + _607;
           int32_t _2745 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2440; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _2746 = _multiplied_no_offsets_s1_x_xi_xii + _2744;
            uint32_t _2747 = _multiplied_no_offsets[_2746];
            int32_t _2748 = _2745 * 4;
            int32_t _2749 = _2748 + _2742;
            uint8_t _2750 = ((const uint8_t *)_mat_a)[_2749];
            uint16_t _2751 = (uint16_t)(_2750);
            int32_t _2752 = _40 * _2745;
            int32_t _2753 = _2752 * 4;
            int32_t _2754 = _2753 + _547;
            int32_t _2755 = _2754 + _multiplied_no_offsets_s1_x_xi_xii;
            uint8_t _2756 = ((const uint8_t *)_mat_b)[_2755];
            uint16_t _2757 = (uint16_t)(_2756);
            uint16_t _2758 = _2751 * _2757;
            uint32_t _2759 = (uint32_t)(_2758);
            uint32_t _2760 = _2747 + _2759;
            int32_t _2761 = _2749 + 1;
            uint8_t _2762 = ((const uint8_t *)_mat_a)[_2761];
            uint16_t _2763 = (uint16_t)(_2762);
            int32_t _2764 = _2755 + _40;
            uint8_t _2765 = ((const uint8_t *)_mat_b)[_2764];
            uint16_t _2766 = (uint16_t)(_2765);
            uint16_t _2767 = _2763 * _2766;
            uint32_t _2768 = (uint32_t)(_2767);
            uint32_t _2769 = _2760 + _2768;
            int32_t _2770 = _2749 + 2;
            uint8_t _2771 = ((const uint8_t *)_mat_a)[_2770];
            uint16_t _2772 = (uint16_t)(_2771);
            int32_t _2773 = _2755 + _552;
            uint8_t _2774 = ((const uint8_t *)_mat_b)[_2773];
            uint16_t _2775 = (uint16_t)(_2774);
            uint16_t _2776 = _2772 * _2775;
            uint32_t _2777 = (uint32_t)(_2776);
            uint32_t _2778 = _2769 + _2777;
            int32_t _2779 = _2749 + 3;
            uint8_t _2780 = ((const uint8_t *)_mat_a)[_2779];
            uint16_t _2781 = (uint16_t)(_2780);
            int32_t _2782 = _2755 + _553;
            uint8_t _2783 = ((const uint8_t *)_mat_b)[_2782];
            uint16_t _2784 = (uint16_t)(_2783);
            uint16_t _2785 = _2781 * _2784;
            uint32_t _2786 = (uint32_t)(_2785);
            uint32_t _2787 = _2778 + _2786;
            int32_t _2788 = _multiplied_no_offsets_s1_x_xi_xii + _2482;
            _multiplied_no_offsets[_2788] = _2787;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _2789 = _2456 * 4;
           int32_t _2790 = _2789 + 2;
           int32_t _2791 = _31 * _2790;
           int32_t _2792 = _590 * _2790;
           int32_t _2793 = _2792 + _607;
           int32_t _2794 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2440; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _2795 = _multiplied_no_offsets_s1_x_xi_xii + _2793;
            uint32_t _2796 = _multiplied_no_offsets[_2795];
            int32_t _2797 = _2794 * 4;
            int32_t _2798 = _2797 + _2791;
            uint8_t _2799 = ((const uint8_t *)_mat_a)[_2798];
            uint16_t _2800 = (uint16_t)(_2799);
            int32_t _2801 = _40 * _2794;
            int32_t _2802 = _2801 * 4;
            int32_t _2803 = _2802 + _547;
            int32_t _2804 = _2803 + _multiplied_no_offsets_s1_x_xi_xii;
            uint8_t _2805 = ((const uint8_t *)_mat_b)[_2804];
            uint16_t _2806 = (uint16_t)(_2805);
            uint16_t _2807 = _2800 * _2806;
            uint32_t _2808 = (uint32_t)(_2807);
            uint32_t _2809 = _2796 + _2808;
            int32_t _2810 = _2798 + 1;
            uint8_t _2811 = ((const uint8_t *)_mat_a)[_2810];
            uint16_t _2812 = (uint16_t)(_2811);
            int32_t _2813 = _2804 + _40;
            uint8_t _2814 = ((const uint8_t *)_mat_b)[_2813];
            uint16_t _2815 = (uint16_t)(_2814);
            uint16_t _2816 = _2812 * _2815;
            uint32_t _2817 = (uint32_t)(_2816);
            uint32_t _2818 = _2809 + _2817;
            int32_t _2819 = _2798 + 2;
            uint8_t _2820 = ((const uint8_t *)_mat_a)[_2819];
            uint16_t _2821 = (uint16_t)(_2820);
            int32_t _2822 = _2804 + _552;
            uint8_t _2823 = ((const uint8_t *)_mat_b)[_2822];
            uint16_t _2824 = (uint16_t)(_2823);
            uint16_t _2825 = _2821 * _2824;
            uint32_t _2826 = (uint32_t)(_2825);
            uint32_t _2827 = _2818 + _2826;
            int32_t _2828 = _2798 + 3;
            uint8_t _2829 = ((const uint8_t *)_mat_a)[_2828];
            uint16_t _2830 = (uint16_t)(_2829);
            int32_t _2831 = _2804 + _553;
            uint8_t _2832 = ((const uint8_t *)_mat_b)[_2831];
            uint16_t _2833 = (uint16_t)(_2832);
            uint16_t _2834 = _2830 * _2833;
            uint32_t _2835 = (uint32_t)(_2834);
            uint32_t _2836 = _2827 + _2835;
            int32_t _2837 = _multiplied_no_offsets_s1_x_xi_xii + _2478;
            _multiplied_no_offsets[_2837] = _2836;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _2838 = _2456 * 4;
           int32_t _2839 = _2838 + 3;
           int32_t _2840 = _31 * _2839;
           int32_t _2841 = _590 * _2839;
           int32_t _2842 = _2841 + _607;
           int32_t _2843 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2440; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _2844 = _multiplied_no_offsets_s1_x_xi_xii + _2842;
            uint32_t _2845 = _multiplied_no_offsets[_2844];
            int32_t _2846 = _2843 * 4;
            int32_t _2847 = _2846 + _2840;
            uint8_t _2848 = ((const uint8_t *)_mat_a)[_2847];
            uint16_t _2849 = (uint16_t)(_2848);
            int32_t _2850 = _40 * _2843;
            int32_t _2851 = _2850 * 4;
            int32_t _2852 = _2851 + _547;
            int32_t _2853 = _2852 + _multiplied_no_offsets_s1_x_xi_xii;
            uint8_t _2854 = ((const uint8_t *)_mat_b)[_2853];
            uint16_t _2855 = (uint16_t)(_2854);
            uint16_t _2856 = _2849 * _2855;
            uint32_t _2857 = (uint32_t)(_2856);
            uint32_t _2858 = _2845 + _2857;
            int32_t _2859 = _2847 + 1;
            uint8_t _2860 = ((const uint8_t *)_mat_a)[_2859];
            uint16_t _2861 = (uint16_t)(_2860);
            int32_t _2862 = _2853 + _40;
            uint8_t _2863 = ((const uint8_t *)_mat_b)[_2862];
            uint16_t _2864 = (uint16_t)(_2863);
            uint16_t _2865 = _2861 * _2864;
            uint32_t _2866 = (uint32_t)(_2865);
            uint32_t _2867 = _2858 + _2866;
            int32_t _2868 = _2847 + 2;
            uint8_t _2869 = ((const uint8_t *)_mat_a)[_2868];
            uint16_t _2870 = (uint16_t)(_2869);
            int32_t _2871 = _2853 + _552;
            uint8_t _2872 = ((const uint8_t *)_mat_b)[_2871];
            uint16_t _2873 = (uint16_t)(_2872);
            uint16_t _2874 = _2870 * _2873;
            uint32_t _2875 = (uint32_t)(_2874);
            uint32_t _2876 = _2867 + _2875;
            int32_t _2877 = _2847 + 3;
            uint8_t _2878 = ((const uint8_t *)_mat_a)[_2877];
            uint16_t _2879 = (uint16_t)(_2878);
            int32_t _2880 = _2853 + _553;
            uint8_t _2881 = ((const uint8_t *)_mat_b)[_2880];
            uint16_t _2882 = (uint16_t)(_2881);
            uint16_t _2883 = _2879 * _2882;
            uint32_t _2884 = (uint32_t)(_2883);
            uint32_t _2885 = _2876 + _2884;
            int32_t _2886 = _multiplied_no_offsets_s1_x_xi_xii + _2474;
            _multiplied_no_offsets[_2886] = _2885;
           } // for _multiplied_no_offsets_s1_x_xi_xii
          } // if _581 else
          if (_583)
          {
           int32_t _2887 = _590 * _2456;
           int32_t _2888 = _2887 * 4;
           int32_t _2889 = _2888 + _606;
           uint32x8_t _2890 = uint32x8_t::load(_multiplied_no_offsets, _2889);
           int32_t _2891 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _2892 = _2891 * _40;
           int32_t _2893 = _2892 + _2441;
           int32_t _2894 = _2893 * 4;
           int32_t _2895 = _2894 + 8;
           uint8x8_t _2896 = uint8x8_t::load(_mat_b, _2895);
           uint16x8_t _2897 = uint16x8_t::convert_from<uint8x8_t>(_2896);
           int32_t _2898 = _31 * _2456;
           int32_t _2899 = _2898 + _2454;
           int32_t _2900 = _2899 + _multiplied_no_offsets_s1_k__x_rki;
           int32_t _2901 = _2900 * 4;
           uint8_t _2902 = ((const uint8_t *)_mat_a)[_2901];
           uint16_t _2903 = (uint16_t)(_2902);
           uint16x8_t _2904 = uint16x8_t::broadcast(_2903);
           uint16x8_t _2905 = _2897 * _2904;
           uint32x8_t _2906 = uint32x8_t::convert_from<uint16x8_t>(_2905);
           uint32x8_t _2907 = _2890 + _2906;
           int32_t _2908 = _2894 + _40;
           int32_t _2909 = _2908 + 8;
           uint8x8_t _2910 = uint8x8_t::load(_mat_b, _2909);
           uint16x8_t _2911 = uint16x8_t::convert_from<uint8x8_t>(_2910);
           int32_t _2912 = _2901 + 1;
           uint8_t _2913 = ((const uint8_t *)_mat_a)[_2912];
           uint16_t _2914 = (uint16_t)(_2913);
           uint16x8_t _2915 = uint16x8_t::broadcast(_2914);
           uint16x8_t _2916 = _2911 * _2915;
           uint32x8_t _2917 = uint32x8_t::convert_from<uint16x8_t>(_2916);
           uint32x8_t _2918 = _2907 + _2917;
           int32_t _2919 = _2893 * 2;
           int32_t _2920 = _2919 + _40;
           int32_t _2921 = _2920 * 2;
           int32_t _2922 = _2921 + 8;
           uint8x8_t _2923 = uint8x8_t::load(_mat_b, _2922);
           uint16x8_t _2924 = uint16x8_t::convert_from<uint8x8_t>(_2923);
           int32_t _2925 = _2901 + 2;
           uint8_t _2926 = ((const uint8_t *)_mat_a)[_2925];
           uint16_t _2927 = (uint16_t)(_2926);
           uint16x8_t _2928 = uint16x8_t::broadcast(_2927);
           uint16x8_t _2929 = _2924 * _2928;
           uint32x8_t _2930 = uint32x8_t::convert_from<uint16x8_t>(_2929);
           uint32x8_t _2931 = _2918 + _2930;
           int32_t _2932 = _2894 + _553;
           int32_t _2933 = _2932 + 8;
           uint8x8_t _2934 = uint8x8_t::load(_mat_b, _2933);
           uint16x8_t _2935 = uint16x8_t::convert_from<uint8x8_t>(_2934);
           int32_t _2936 = _2901 + 3;
           uint8_t _2937 = ((const uint8_t *)_mat_a)[_2936];
           uint16_t _2938 = (uint16_t)(_2937);
           uint16x8_t _2939 = uint16x8_t::broadcast(_2938);
           uint16x8_t _2940 = _2935 * _2939;
           uint32x8_t _2941 = uint32x8_t::convert_from<uint16x8_t>(_2940);
           uint32x8_t _2942 = _2931 + _2941;
           _2942.store(_multiplied_no_offsets, _2469);
           int32_t _2943 = _2456 * 4;
           int32_t _2944 = _2943 + 1;
           int32_t _2945 = _2944 * _31;
           int32_t _2946 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _2947 = _2946 * 4;
           int32_t _2948 = _2945 + _2947;
           int32_t _2949 = _2944 * _590;
           int32_t _2950 = _2949 + _606;
           uint32x8_t _2951 = uint32x8_t::load(_multiplied_no_offsets, _2950);
           int32_t _2952 = _2946 * _40;
           int32_t _2953 = _2952 + _2441;
           int32_t _2954 = _2953 * 4;
           int32_t _2955 = _2954 + 8;
           uint8x8_t _2956 = uint8x8_t::load(_mat_b, _2955);
           uint16x8_t _2957 = uint16x8_t::convert_from<uint8x8_t>(_2956);
           uint8_t _2958 = ((const uint8_t *)_mat_a)[_2948];
           uint16_t _2959 = (uint16_t)(_2958);
           uint16x8_t _2960 = uint16x8_t::broadcast(_2959);
           uint16x8_t _2961 = _2957 * _2960;
           uint32x8_t _2962 = uint32x8_t::convert_from<uint16x8_t>(_2961);
           uint32x8_t _2963 = _2951 + _2962;
           int32_t _2964 = _2954 + _40;
           int32_t _2965 = _2964 + 8;
           uint8x8_t _2966 = uint8x8_t::load(_mat_b, _2965);
           uint16x8_t _2967 = uint16x8_t::convert_from<uint8x8_t>(_2966);
           int32_t _2968 = _2948 + 1;
           uint8_t _2969 = ((const uint8_t *)_mat_a)[_2968];
           uint16_t _2970 = (uint16_t)(_2969);
           uint16x8_t _2971 = uint16x8_t::broadcast(_2970);
           uint16x8_t _2972 = _2967 * _2971;
           uint32x8_t _2973 = uint32x8_t::convert_from<uint16x8_t>(_2972);
           uint32x8_t _2974 = _2963 + _2973;
           int32_t _2975 = _2953 * 2;
           int32_t _2976 = _2975 + _40;
           int32_t _2977 = _2976 * 2;
           int32_t _2978 = _2977 + 8;
           uint8x8_t _2979 = uint8x8_t::load(_mat_b, _2978);
           uint16x8_t _2980 = uint16x8_t::convert_from<uint8x8_t>(_2979);
           int32_t _2981 = _2948 + 2;
           uint8_t _2982 = ((const uint8_t *)_mat_a)[_2981];
           uint16_t _2983 = (uint16_t)(_2982);
           uint16x8_t _2984 = uint16x8_t::broadcast(_2983);
           uint16x8_t _2985 = _2980 * _2984;
           uint32x8_t _2986 = uint32x8_t::convert_from<uint16x8_t>(_2985);
           uint32x8_t _2987 = _2974 + _2986;
           int32_t _2988 = _2954 + _553;
           int32_t _2989 = _2988 + 8;
           uint8x8_t _2990 = uint8x8_t::load(_mat_b, _2989);
           uint16x8_t _2991 = uint16x8_t::convert_from<uint8x8_t>(_2990);
           int32_t _2992 = _2948 + 3;
           uint8_t _2993 = ((const uint8_t *)_mat_a)[_2992];
           uint16_t _2994 = (uint16_t)(_2993);
           uint16x8_t _2995 = uint16x8_t::broadcast(_2994);
           uint16x8_t _2996 = _2991 * _2995;
           uint32x8_t _2997 = uint32x8_t::convert_from<uint16x8_t>(_2996);
           uint32x8_t _2998 = _2987 + _2997;
           _2998.store(_multiplied_no_offsets, _2481);
           int32_t _2999 = _2456 * 4;
           int32_t _3000 = _2999 + 2;
           int32_t _3001 = _3000 * _31;
           int32_t _3002 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3003 = _3002 * 4;
           int32_t _3004 = _3001 + _3003;
           int32_t _3005 = _3000 * _590;
           int32_t _3006 = _3005 + _606;
           uint32x8_t _3007 = uint32x8_t::load(_multiplied_no_offsets, _3006);
           int32_t _3008 = _3002 * _40;
           int32_t _3009 = _3008 + _2441;
           int32_t _3010 = _3009 * 4;
           int32_t _3011 = _3010 + 8;
           uint8x8_t _3012 = uint8x8_t::load(_mat_b, _3011);
           uint16x8_t _3013 = uint16x8_t::convert_from<uint8x8_t>(_3012);
           uint8_t _3014 = ((const uint8_t *)_mat_a)[_3004];
           uint16_t _3015 = (uint16_t)(_3014);
           uint16x8_t _3016 = uint16x8_t::broadcast(_3015);
           uint16x8_t _3017 = _3013 * _3016;
           uint32x8_t _3018 = uint32x8_t::convert_from<uint16x8_t>(_3017);
           uint32x8_t _3019 = _3007 + _3018;
           int32_t _3020 = _3010 + _40;
           int32_t _3021 = _3020 + 8;
           uint8x8_t _3022 = uint8x8_t::load(_mat_b, _3021);
           uint16x8_t _3023 = uint16x8_t::convert_from<uint8x8_t>(_3022);
           int32_t _3024 = _3004 + 1;
           uint8_t _3025 = ((const uint8_t *)_mat_a)[_3024];
           uint16_t _3026 = (uint16_t)(_3025);
           uint16x8_t _3027 = uint16x8_t::broadcast(_3026);
           uint16x8_t _3028 = _3023 * _3027;
           uint32x8_t _3029 = uint32x8_t::convert_from<uint16x8_t>(_3028);
           uint32x8_t _3030 = _3019 + _3029;
           int32_t _3031 = _3009 * 2;
           int32_t _3032 = _3031 + _40;
           int32_t _3033 = _3032 * 2;
           int32_t _3034 = _3033 + 8;
           uint8x8_t _3035 = uint8x8_t::load(_mat_b, _3034);
           uint16x8_t _3036 = uint16x8_t::convert_from<uint8x8_t>(_3035);
           int32_t _3037 = _3004 + 2;
           uint8_t _3038 = ((const uint8_t *)_mat_a)[_3037];
           uint16_t _3039 = (uint16_t)(_3038);
           uint16x8_t _3040 = uint16x8_t::broadcast(_3039);
           uint16x8_t _3041 = _3036 * _3040;
           uint32x8_t _3042 = uint32x8_t::convert_from<uint16x8_t>(_3041);
           uint32x8_t _3043 = _3030 + _3042;
           int32_t _3044 = _3010 + _553;
           int32_t _3045 = _3044 + 8;
           uint8x8_t _3046 = uint8x8_t::load(_mat_b, _3045);
           uint16x8_t _3047 = uint16x8_t::convert_from<uint8x8_t>(_3046);
           int32_t _3048 = _3004 + 3;
           uint8_t _3049 = ((const uint8_t *)_mat_a)[_3048];
           uint16_t _3050 = (uint16_t)(_3049);
           uint16x8_t _3051 = uint16x8_t::broadcast(_3050);
           uint16x8_t _3052 = _3047 * _3051;
           uint32x8_t _3053 = uint32x8_t::convert_from<uint16x8_t>(_3052);
           uint32x8_t _3054 = _3043 + _3053;
           _3054.store(_multiplied_no_offsets, _2477);
           int32_t _3055 = _2456 * 4;
           int32_t _3056 = _3055 + 3;
           int32_t _3057 = _3056 * _31;
           int32_t _3058 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3059 = _3058 * 4;
           int32_t _3060 = _3057 + _3059;
           int32_t _3061 = _3056 * _590;
           int32_t _3062 = _3061 + _606;
           uint32x8_t _3063 = uint32x8_t::load(_multiplied_no_offsets, _3062);
           int32_t _3064 = _3058 * _40;
           int32_t _3065 = _3064 + _2441;
           int32_t _3066 = _3065 * 4;
           int32_t _3067 = _3066 + 8;
           uint8x8_t _3068 = uint8x8_t::load(_mat_b, _3067);
           uint16x8_t _3069 = uint16x8_t::convert_from<uint8x8_t>(_3068);
           uint8_t _3070 = ((const uint8_t *)_mat_a)[_3060];
           uint16_t _3071 = (uint16_t)(_3070);
           uint16x8_t _3072 = uint16x8_t::broadcast(_3071);
           uint16x8_t _3073 = _3069 * _3072;
           uint32x8_t _3074 = uint32x8_t::convert_from<uint16x8_t>(_3073);
           uint32x8_t _3075 = _3063 + _3074;
           int32_t _3076 = _3066 + _40;
           int32_t _3077 = _3076 + 8;
           uint8x8_t _3078 = uint8x8_t::load(_mat_b, _3077);
           uint16x8_t _3079 = uint16x8_t::convert_from<uint8x8_t>(_3078);
           int32_t _3080 = _3060 + 1;
           uint8_t _3081 = ((const uint8_t *)_mat_a)[_3080];
           uint16_t _3082 = (uint16_t)(_3081);
           uint16x8_t _3083 = uint16x8_t::broadcast(_3082);
           uint16x8_t _3084 = _3079 * _3083;
           uint32x8_t _3085 = uint32x8_t::convert_from<uint16x8_t>(_3084);
           uint32x8_t _3086 = _3075 + _3085;
           int32_t _3087 = _3065 * 2;
           int32_t _3088 = _3087 + _40;
           int32_t _3089 = _3088 * 2;
           int32_t _3090 = _3089 + 8;
           uint8x8_t _3091 = uint8x8_t::load(_mat_b, _3090);
           uint16x8_t _3092 = uint16x8_t::convert_from<uint8x8_t>(_3091);
           int32_t _3093 = _3060 + 2;
           uint8_t _3094 = ((const uint8_t *)_mat_a)[_3093];
           uint16_t _3095 = (uint16_t)(_3094);
           uint16x8_t _3096 = uint16x8_t::broadcast(_3095);
           uint16x8_t _3097 = _3092 * _3096;
           uint32x8_t _3098 = uint32x8_t::convert_from<uint16x8_t>(_3097);
           uint32x8_t _3099 = _3086 + _3098;
           int32_t _3100 = _3066 + _553;
           int32_t _3101 = _3100 + 8;
           uint8x8_t _3102 = uint8x8_t::load(_mat_b, _3101);
           uint16x8_t _3103 = uint16x8_t::convert_from<uint8x8_t>(_3102);
           int32_t _3104 = _3060 + 3;
           uint8_t _3105 = ((const uint8_t *)_mat_a)[_3104];
           uint16_t _3106 = (uint16_t)(_3105);
           uint16x8_t _3107 = uint16x8_t::broadcast(_3106);
           uint16x8_t _3108 = _3103 * _3107;
           uint32x8_t _3109 = uint32x8_t::convert_from<uint16x8_t>(_3108);
           uint32x8_t _3110 = _3099 + _3109;
           _3110.store(_multiplied_no_offsets, _2473);
          } // if _583
          else
          {
           int32_t _3111 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3112 = _3111 * _40;
           int32_t _3113 = _3112 * 4;
           int32_t _3114 = _3113 + _547;
           int32_t _3115 = _590 * _2456;
           int32_t _3116 = _3115 * 4;
           int32_t _3117 = _3116 + _606;
           int32_t _3118 = _31 * _2456;
           int32_t _3119 = _3118 + _2454;
           int32_t _3120 = _3119 + _multiplied_no_offsets_s1_k__x_rki;
           int32_t _3121 = _553 + 8;
           int32_t _3122 = _552 + 8;
           int32_t _3123 = _40 + 8;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2453; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _3124 = _multiplied_no_offsets_s1_x_xi_xii + _3117;
            uint32_t _3125 = _multiplied_no_offsets[_3124];
            int32_t _3126 = _3120 * 4;
            uint8_t _3127 = ((const uint8_t *)_mat_a)[_3126];
            uint16_t _3128 = (uint16_t)(_3127);
            int32_t _3129 = _multiplied_no_offsets_s1_x_xi_xii + _3114;
            int32_t _3130 = _3129 + 8;
            uint8_t _3131 = ((const uint8_t *)_mat_b)[_3130];
            uint16_t _3132 = (uint16_t)(_3131);
            uint16_t _3133 = _3128 * _3132;
            uint32_t _3134 = (uint32_t)(_3133);
            uint32_t _3135 = _3125 + _3134;
            int32_t _3136 = _3126 + 1;
            uint8_t _3137 = ((const uint8_t *)_mat_a)[_3136];
            uint16_t _3138 = (uint16_t)(_3137);
            int32_t _3139 = _3129 + _3123;
            uint8_t _3140 = ((const uint8_t *)_mat_b)[_3139];
            uint16_t _3141 = (uint16_t)(_3140);
            uint16_t _3142 = _3138 * _3141;
            uint32_t _3143 = (uint32_t)(_3142);
            uint32_t _3144 = _3135 + _3143;
            int32_t _3145 = _3126 + 2;
            uint8_t _3146 = ((const uint8_t *)_mat_a)[_3145];
            uint16_t _3147 = (uint16_t)(_3146);
            int32_t _3148 = _3129 + _3122;
            uint8_t _3149 = ((const uint8_t *)_mat_b)[_3148];
            uint16_t _3150 = (uint16_t)(_3149);
            uint16_t _3151 = _3147 * _3150;
            uint32_t _3152 = (uint32_t)(_3151);
            uint32_t _3153 = _3144 + _3152;
            int32_t _3154 = _3126 + 3;
            uint8_t _3155 = ((const uint8_t *)_mat_a)[_3154];
            uint16_t _3156 = (uint16_t)(_3155);
            int32_t _3157 = _3129 + _3121;
            uint8_t _3158 = ((const uint8_t *)_mat_b)[_3157];
            uint16_t _3159 = (uint16_t)(_3158);
            uint16_t _3160 = _3156 * _3159;
            uint32_t _3161 = (uint32_t)(_3160);
            uint32_t _3162 = _3153 + _3161;
            int32_t _3163 = _multiplied_no_offsets_s1_x_xi_xii + _2469;
            _multiplied_no_offsets[_3163] = _3162;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _3164 = _2456 * 4;
           int32_t _3165 = _3164 + 1;
           int32_t _3166 = _31 * _3165;
           int32_t _3167 = _590 * _3165;
           int32_t _3168 = _3167 + _606;
           int32_t _3169 = _553 + 8;
           int32_t _3170 = _552 + 8;
           int32_t _3171 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2453; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _3172 = _multiplied_no_offsets_s1_x_xi_xii + _3168;
            uint32_t _3173 = _multiplied_no_offsets[_3172];
            int32_t _3174 = _3171 * 4;
            int32_t _3175 = _3174 + _3166;
            uint8_t _3176 = ((const uint8_t *)_mat_a)[_3175];
            uint16_t _3177 = (uint16_t)(_3176);
            int32_t _3178 = _40 * _3171;
            int32_t _3179 = _3178 * 4;
            int32_t _3180 = _3179 + _547;
            int32_t _3181 = _3180 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _3182 = _3181 + 8;
            uint8_t _3183 = ((const uint8_t *)_mat_b)[_3182];
            uint16_t _3184 = (uint16_t)(_3183);
            uint16_t _3185 = _3177 * _3184;
            uint32_t _3186 = (uint32_t)(_3185);
            uint32_t _3187 = _3173 + _3186;
            int32_t _3188 = _3175 + 1;
            uint8_t _3189 = ((const uint8_t *)_mat_a)[_3188];
            uint16_t _3190 = (uint16_t)(_3189);
            int32_t _3191 = _3181 + _40;
            int32_t _3192 = _3191 + 8;
            uint8_t _3193 = ((const uint8_t *)_mat_b)[_3192];
            uint16_t _3194 = (uint16_t)(_3193);
            uint16_t _3195 = _3190 * _3194;
            uint32_t _3196 = (uint32_t)(_3195);
            uint32_t _3197 = _3187 + _3196;
            int32_t _3198 = _3175 + 2;
            uint8_t _3199 = ((const uint8_t *)_mat_a)[_3198];
            uint16_t _3200 = (uint16_t)(_3199);
            int32_t _3201 = _3181 + _3170;
            uint8_t _3202 = ((const uint8_t *)_mat_b)[_3201];
            uint16_t _3203 = (uint16_t)(_3202);
            uint16_t _3204 = _3200 * _3203;
            uint32_t _3205 = (uint32_t)(_3204);
            uint32_t _3206 = _3197 + _3205;
            int32_t _3207 = _3175 + 3;
            uint8_t _3208 = ((const uint8_t *)_mat_a)[_3207];
            uint16_t _3209 = (uint16_t)(_3208);
            int32_t _3210 = _3181 + _3169;
            uint8_t _3211 = ((const uint8_t *)_mat_b)[_3210];
            uint16_t _3212 = (uint16_t)(_3211);
            uint16_t _3213 = _3209 * _3212;
            uint32_t _3214 = (uint32_t)(_3213);
            uint32_t _3215 = _3206 + _3214;
            int32_t _3216 = _multiplied_no_offsets_s1_x_xi_xii + _2481;
            _multiplied_no_offsets[_3216] = _3215;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _3217 = _2456 * 4;
           int32_t _3218 = _3217 + 2;
           int32_t _3219 = _31 * _3218;
           int32_t _3220 = _590 * _3218;
           int32_t _3221 = _3220 + _606;
           int32_t _3222 = _553 + 8;
           int32_t _3223 = _552 + 8;
           int32_t _3224 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2453; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _3225 = _multiplied_no_offsets_s1_x_xi_xii + _3221;
            uint32_t _3226 = _multiplied_no_offsets[_3225];
            int32_t _3227 = _3224 * 4;
            int32_t _3228 = _3227 + _3219;
            uint8_t _3229 = ((const uint8_t *)_mat_a)[_3228];
            uint16_t _3230 = (uint16_t)(_3229);
            int32_t _3231 = _40 * _3224;
            int32_t _3232 = _3231 * 4;
            int32_t _3233 = _3232 + _547;
            int32_t _3234 = _3233 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _3235 = _3234 + 8;
            uint8_t _3236 = ((const uint8_t *)_mat_b)[_3235];
            uint16_t _3237 = (uint16_t)(_3236);
            uint16_t _3238 = _3230 * _3237;
            uint32_t _3239 = (uint32_t)(_3238);
            uint32_t _3240 = _3226 + _3239;
            int32_t _3241 = _3228 + 1;
            uint8_t _3242 = ((const uint8_t *)_mat_a)[_3241];
            uint16_t _3243 = (uint16_t)(_3242);
            int32_t _3244 = _3234 + _40;
            int32_t _3245 = _3244 + 8;
            uint8_t _3246 = ((const uint8_t *)_mat_b)[_3245];
            uint16_t _3247 = (uint16_t)(_3246);
            uint16_t _3248 = _3243 * _3247;
            uint32_t _3249 = (uint32_t)(_3248);
            uint32_t _3250 = _3240 + _3249;
            int32_t _3251 = _3228 + 2;
            uint8_t _3252 = ((const uint8_t *)_mat_a)[_3251];
            uint16_t _3253 = (uint16_t)(_3252);
            int32_t _3254 = _3234 + _3223;
            uint8_t _3255 = ((const uint8_t *)_mat_b)[_3254];
            uint16_t _3256 = (uint16_t)(_3255);
            uint16_t _3257 = _3253 * _3256;
            uint32_t _3258 = (uint32_t)(_3257);
            uint32_t _3259 = _3250 + _3258;
            int32_t _3260 = _3228 + 3;
            uint8_t _3261 = ((const uint8_t *)_mat_a)[_3260];
            uint16_t _3262 = (uint16_t)(_3261);
            int32_t _3263 = _3234 + _3222;
            uint8_t _3264 = ((const uint8_t *)_mat_b)[_3263];
            uint16_t _3265 = (uint16_t)(_3264);
            uint16_t _3266 = _3262 * _3265;
            uint32_t _3267 = (uint32_t)(_3266);
            uint32_t _3268 = _3259 + _3267;
            int32_t _3269 = _multiplied_no_offsets_s1_x_xi_xii + _2477;
            _multiplied_no_offsets[_3269] = _3268;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _3270 = _2456 * 4;
           int32_t _3271 = _3270 + 3;
           int32_t _3272 = _31 * _3271;
           int32_t _3273 = _590 * _3271;
           int32_t _3274 = _3273 + _606;
           int32_t _3275 = _553 + 8;
           int32_t _3276 = _552 + 8;
           int32_t _3277 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2453; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _3278 = _multiplied_no_offsets_s1_x_xi_xii + _3274;
            uint32_t _3279 = _multiplied_no_offsets[_3278];
            int32_t _3280 = _3277 * 4;
            int32_t _3281 = _3280 + _3272;
            uint8_t _3282 = ((const uint8_t *)_mat_a)[_3281];
            uint16_t _3283 = (uint16_t)(_3282);
            int32_t _3284 = _40 * _3277;
            int32_t _3285 = _3284 * 4;
            int32_t _3286 = _3285 + _547;
            int32_t _3287 = _3286 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _3288 = _3287 + 8;
            uint8_t _3289 = ((const uint8_t *)_mat_b)[_3288];
            uint16_t _3290 = (uint16_t)(_3289);
            uint16_t _3291 = _3283 * _3290;
            uint32_t _3292 = (uint32_t)(_3291);
            uint32_t _3293 = _3279 + _3292;
            int32_t _3294 = _3281 + 1;
            uint8_t _3295 = ((const uint8_t *)_mat_a)[_3294];
            uint16_t _3296 = (uint16_t)(_3295);
            int32_t _3297 = _3287 + _40;
            int32_t _3298 = _3297 + 8;
            uint8_t _3299 = ((const uint8_t *)_mat_b)[_3298];
            uint16_t _3300 = (uint16_t)(_3299);
            uint16_t _3301 = _3296 * _3300;
            uint32_t _3302 = (uint32_t)(_3301);
            uint32_t _3303 = _3293 + _3302;
            int32_t _3304 = _3281 + 2;
            uint8_t _3305 = ((const uint8_t *)_mat_a)[_3304];
            uint16_t _3306 = (uint16_t)(_3305);
            int32_t _3307 = _3287 + _3276;
            uint8_t _3308 = ((const uint8_t *)_mat_b)[_3307];
            uint16_t _3309 = (uint16_t)(_3308);
            uint16_t _3310 = _3306 * _3309;
            uint32_t _3311 = (uint32_t)(_3310);
            uint32_t _3312 = _3303 + _3311;
            int32_t _3313 = _3281 + 3;
            uint8_t _3314 = ((const uint8_t *)_mat_a)[_3313];
            uint16_t _3315 = (uint16_t)(_3314);
            int32_t _3316 = _3287 + _3275;
            uint8_t _3317 = ((const uint8_t *)_mat_b)[_3316];
            uint16_t _3318 = (uint16_t)(_3317);
            uint16_t _3319 = _3315 * _3318;
            uint32_t _3320 = (uint32_t)(_3319);
            uint32_t _3321 = _3312 + _3320;
            int32_t _3322 = _multiplied_no_offsets_s1_x_xi_xii + _2473;
            _multiplied_no_offsets[_3322] = _3321;
           } // for _multiplied_no_offsets_s1_x_xi_xii
          } // if _583 else
          if (_584)
          {
           int32_t _3323 = _590 * _2456;
           int32_t _3324 = _3323 * 4;
           int32_t _3325 = _3324 + _605;
           uint32x8_t _3326 = uint32x8_t::load(_multiplied_no_offsets, _3325);
           int32_t _3327 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3328 = _3327 * _40;
           int32_t _3329 = _3328 + _2441;
           int32_t _3330 = _3329 * 4;
           int32_t _3331 = _3330 + 16;
           uint8x8_t _3332 = uint8x8_t::load(_mat_b, _3331);
           uint16x8_t _3333 = uint16x8_t::convert_from<uint8x8_t>(_3332);
           int32_t _3334 = _31 * _2456;
           int32_t _3335 = _3334 + _2454;
           int32_t _3336 = _3335 + _multiplied_no_offsets_s1_k__x_rki;
           int32_t _3337 = _3336 * 4;
           uint8_t _3338 = ((const uint8_t *)_mat_a)[_3337];
           uint16_t _3339 = (uint16_t)(_3338);
           uint16x8_t _3340 = uint16x8_t::broadcast(_3339);
           uint16x8_t _3341 = _3333 * _3340;
           uint32x8_t _3342 = uint32x8_t::convert_from<uint16x8_t>(_3341);
           uint32x8_t _3343 = _3326 + _3342;
           int32_t _3344 = _3330 + _40;
           int32_t _3345 = _3344 + 16;
           uint8x8_t _3346 = uint8x8_t::load(_mat_b, _3345);
           uint16x8_t _3347 = uint16x8_t::convert_from<uint8x8_t>(_3346);
           int32_t _3348 = _3337 + 1;
           uint8_t _3349 = ((const uint8_t *)_mat_a)[_3348];
           uint16_t _3350 = (uint16_t)(_3349);
           uint16x8_t _3351 = uint16x8_t::broadcast(_3350);
           uint16x8_t _3352 = _3347 * _3351;
           uint32x8_t _3353 = uint32x8_t::convert_from<uint16x8_t>(_3352);
           uint32x8_t _3354 = _3343 + _3353;
           int32_t _3355 = _3329 * 2;
           int32_t _3356 = _3355 + _40;
           int32_t _3357 = _3356 * 2;
           int32_t _3358 = _3357 + 16;
           uint8x8_t _3359 = uint8x8_t::load(_mat_b, _3358);
           uint16x8_t _3360 = uint16x8_t::convert_from<uint8x8_t>(_3359);
           int32_t _3361 = _3337 + 2;
           uint8_t _3362 = ((const uint8_t *)_mat_a)[_3361];
           uint16_t _3363 = (uint16_t)(_3362);
           uint16x8_t _3364 = uint16x8_t::broadcast(_3363);
           uint16x8_t _3365 = _3360 * _3364;
           uint32x8_t _3366 = uint32x8_t::convert_from<uint16x8_t>(_3365);
           uint32x8_t _3367 = _3354 + _3366;
           int32_t _3368 = _3330 + _553;
           int32_t _3369 = _3368 + 16;
           uint8x8_t _3370 = uint8x8_t::load(_mat_b, _3369);
           uint16x8_t _3371 = uint16x8_t::convert_from<uint8x8_t>(_3370);
           int32_t _3372 = _3337 + 3;
           uint8_t _3373 = ((const uint8_t *)_mat_a)[_3372];
           uint16_t _3374 = (uint16_t)(_3373);
           uint16x8_t _3375 = uint16x8_t::broadcast(_3374);
           uint16x8_t _3376 = _3371 * _3375;
           uint32x8_t _3377 = uint32x8_t::convert_from<uint16x8_t>(_3376);
           uint32x8_t _3378 = _3367 + _3377;
           _3378.store(_multiplied_no_offsets, _2468);
           int32_t _3379 = _2456 * 4;
           int32_t _3380 = _3379 + 1;
           int32_t _3381 = _3380 * _31;
           int32_t _3382 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3383 = _3382 * 4;
           int32_t _3384 = _3381 + _3383;
           int32_t _3385 = _3380 * _590;
           int32_t _3386 = _3385 + _605;
           uint32x8_t _3387 = uint32x8_t::load(_multiplied_no_offsets, _3386);
           int32_t _3388 = _3382 * _40;
           int32_t _3389 = _3388 + _2441;
           int32_t _3390 = _3389 * 4;
           int32_t _3391 = _3390 + 16;
           uint8x8_t _3392 = uint8x8_t::load(_mat_b, _3391);
           uint16x8_t _3393 = uint16x8_t::convert_from<uint8x8_t>(_3392);
           uint8_t _3394 = ((const uint8_t *)_mat_a)[_3384];
           uint16_t _3395 = (uint16_t)(_3394);
           uint16x8_t _3396 = uint16x8_t::broadcast(_3395);
           uint16x8_t _3397 = _3393 * _3396;
           uint32x8_t _3398 = uint32x8_t::convert_from<uint16x8_t>(_3397);
           uint32x8_t _3399 = _3387 + _3398;
           int32_t _3400 = _3390 + _40;
           int32_t _3401 = _3400 + 16;
           uint8x8_t _3402 = uint8x8_t::load(_mat_b, _3401);
           uint16x8_t _3403 = uint16x8_t::convert_from<uint8x8_t>(_3402);
           int32_t _3404 = _3384 + 1;
           uint8_t _3405 = ((const uint8_t *)_mat_a)[_3404];
           uint16_t _3406 = (uint16_t)(_3405);
           uint16x8_t _3407 = uint16x8_t::broadcast(_3406);
           uint16x8_t _3408 = _3403 * _3407;
           uint32x8_t _3409 = uint32x8_t::convert_from<uint16x8_t>(_3408);
           uint32x8_t _3410 = _3399 + _3409;
           int32_t _3411 = _3389 * 2;
           int32_t _3412 = _3411 + _40;
           int32_t _3413 = _3412 * 2;
           int32_t _3414 = _3413 + 16;
           uint8x8_t _3415 = uint8x8_t::load(_mat_b, _3414);
           uint16x8_t _3416 = uint16x8_t::convert_from<uint8x8_t>(_3415);
           int32_t _3417 = _3384 + 2;
           uint8_t _3418 = ((const uint8_t *)_mat_a)[_3417];
           uint16_t _3419 = (uint16_t)(_3418);
           uint16x8_t _3420 = uint16x8_t::broadcast(_3419);
           uint16x8_t _3421 = _3416 * _3420;
           uint32x8_t _3422 = uint32x8_t::convert_from<uint16x8_t>(_3421);
           uint32x8_t _3423 = _3410 + _3422;
           int32_t _3424 = _3390 + _553;
           int32_t _3425 = _3424 + 16;
           uint8x8_t _3426 = uint8x8_t::load(_mat_b, _3425);
           uint16x8_t _3427 = uint16x8_t::convert_from<uint8x8_t>(_3426);
           int32_t _3428 = _3384 + 3;
           uint8_t _3429 = ((const uint8_t *)_mat_a)[_3428];
           uint16_t _3430 = (uint16_t)(_3429);
           uint16x8_t _3431 = uint16x8_t::broadcast(_3430);
           uint16x8_t _3432 = _3427 * _3431;
           uint32x8_t _3433 = uint32x8_t::convert_from<uint16x8_t>(_3432);
           uint32x8_t _3434 = _3423 + _3433;
           _3434.store(_multiplied_no_offsets, _2480);
           int32_t _3435 = _2456 * 4;
           int32_t _3436 = _3435 + 2;
           int32_t _3437 = _3436 * _31;
           int32_t _3438 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3439 = _3438 * 4;
           int32_t _3440 = _3437 + _3439;
           int32_t _3441 = _3436 * _590;
           int32_t _3442 = _3441 + _605;
           uint32x8_t _3443 = uint32x8_t::load(_multiplied_no_offsets, _3442);
           int32_t _3444 = _3438 * _40;
           int32_t _3445 = _3444 + _2441;
           int32_t _3446 = _3445 * 4;
           int32_t _3447 = _3446 + 16;
           uint8x8_t _3448 = uint8x8_t::load(_mat_b, _3447);
           uint16x8_t _3449 = uint16x8_t::convert_from<uint8x8_t>(_3448);
           uint8_t _3450 = ((const uint8_t *)_mat_a)[_3440];
           uint16_t _3451 = (uint16_t)(_3450);
           uint16x8_t _3452 = uint16x8_t::broadcast(_3451);
           uint16x8_t _3453 = _3449 * _3452;
           uint32x8_t _3454 = uint32x8_t::convert_from<uint16x8_t>(_3453);
           uint32x8_t _3455 = _3443 + _3454;
           int32_t _3456 = _3446 + _40;
           int32_t _3457 = _3456 + 16;
           uint8x8_t _3458 = uint8x8_t::load(_mat_b, _3457);
           uint16x8_t _3459 = uint16x8_t::convert_from<uint8x8_t>(_3458);
           int32_t _3460 = _3440 + 1;
           uint8_t _3461 = ((const uint8_t *)_mat_a)[_3460];
           uint16_t _3462 = (uint16_t)(_3461);
           uint16x8_t _3463 = uint16x8_t::broadcast(_3462);
           uint16x8_t _3464 = _3459 * _3463;
           uint32x8_t _3465 = uint32x8_t::convert_from<uint16x8_t>(_3464);
           uint32x8_t _3466 = _3455 + _3465;
           int32_t _3467 = _3445 * 2;
           int32_t _3468 = _3467 + _40;
           int32_t _3469 = _3468 * 2;
           int32_t _3470 = _3469 + 16;
           uint8x8_t _3471 = uint8x8_t::load(_mat_b, _3470);
           uint16x8_t _3472 = uint16x8_t::convert_from<uint8x8_t>(_3471);
           int32_t _3473 = _3440 + 2;
           uint8_t _3474 = ((const uint8_t *)_mat_a)[_3473];
           uint16_t _3475 = (uint16_t)(_3474);
           uint16x8_t _3476 = uint16x8_t::broadcast(_3475);
           uint16x8_t _3477 = _3472 * _3476;
           uint32x8_t _3478 = uint32x8_t::convert_from<uint16x8_t>(_3477);
           uint32x8_t _3479 = _3466 + _3478;
           int32_t _3480 = _3446 + _553;
           int32_t _3481 = _3480 + 16;
           uint8x8_t _3482 = uint8x8_t::load(_mat_b, _3481);
           uint16x8_t _3483 = uint16x8_t::convert_from<uint8x8_t>(_3482);
           int32_t _3484 = _3440 + 3;
           uint8_t _3485 = ((const uint8_t *)_mat_a)[_3484];
           uint16_t _3486 = (uint16_t)(_3485);
           uint16x8_t _3487 = uint16x8_t::broadcast(_3486);
           uint16x8_t _3488 = _3483 * _3487;
           uint32x8_t _3489 = uint32x8_t::convert_from<uint16x8_t>(_3488);
           uint32x8_t _3490 = _3479 + _3489;
           _3490.store(_multiplied_no_offsets, _2476);
           int32_t _3491 = _2456 * 4;
           int32_t _3492 = _3491 + 3;
           int32_t _3493 = _3492 * _31;
           int32_t _3494 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3495 = _3494 * 4;
           int32_t _3496 = _3493 + _3495;
           int32_t _3497 = _3492 * _590;
           int32_t _3498 = _3497 + _605;
           uint32x8_t _3499 = uint32x8_t::load(_multiplied_no_offsets, _3498);
           int32_t _3500 = _3494 * _40;
           int32_t _3501 = _3500 + _2441;
           int32_t _3502 = _3501 * 4;
           int32_t _3503 = _3502 + 16;
           uint8x8_t _3504 = uint8x8_t::load(_mat_b, _3503);
           uint16x8_t _3505 = uint16x8_t::convert_from<uint8x8_t>(_3504);
           uint8_t _3506 = ((const uint8_t *)_mat_a)[_3496];
           uint16_t _3507 = (uint16_t)(_3506);
           uint16x8_t _3508 = uint16x8_t::broadcast(_3507);
           uint16x8_t _3509 = _3505 * _3508;
           uint32x8_t _3510 = uint32x8_t::convert_from<uint16x8_t>(_3509);
           uint32x8_t _3511 = _3499 + _3510;
           int32_t _3512 = _3502 + _40;
           int32_t _3513 = _3512 + 16;
           uint8x8_t _3514 = uint8x8_t::load(_mat_b, _3513);
           uint16x8_t _3515 = uint16x8_t::convert_from<uint8x8_t>(_3514);
           int32_t _3516 = _3496 + 1;
           uint8_t _3517 = ((const uint8_t *)_mat_a)[_3516];
           uint16_t _3518 = (uint16_t)(_3517);
           uint16x8_t _3519 = uint16x8_t::broadcast(_3518);
           uint16x8_t _3520 = _3515 * _3519;
           uint32x8_t _3521 = uint32x8_t::convert_from<uint16x8_t>(_3520);
           uint32x8_t _3522 = _3511 + _3521;
           int32_t _3523 = _3501 * 2;
           int32_t _3524 = _3523 + _40;
           int32_t _3525 = _3524 * 2;
           int32_t _3526 = _3525 + 16;
           uint8x8_t _3527 = uint8x8_t::load(_mat_b, _3526);
           uint16x8_t _3528 = uint16x8_t::convert_from<uint8x8_t>(_3527);
           int32_t _3529 = _3496 + 2;
           uint8_t _3530 = ((const uint8_t *)_mat_a)[_3529];
           uint16_t _3531 = (uint16_t)(_3530);
           uint16x8_t _3532 = uint16x8_t::broadcast(_3531);
           uint16x8_t _3533 = _3528 * _3532;
           uint32x8_t _3534 = uint32x8_t::convert_from<uint16x8_t>(_3533);
           uint32x8_t _3535 = _3522 + _3534;
           int32_t _3536 = _3502 + _553;
           int32_t _3537 = _3536 + 16;
           uint8x8_t _3538 = uint8x8_t::load(_mat_b, _3537);
           uint16x8_t _3539 = uint16x8_t::convert_from<uint8x8_t>(_3538);
           int32_t _3540 = _3496 + 3;
           uint8_t _3541 = ((const uint8_t *)_mat_a)[_3540];
           uint16_t _3542 = (uint16_t)(_3541);
           uint16x8_t _3543 = uint16x8_t::broadcast(_3542);
           uint16x8_t _3544 = _3539 * _3543;
           uint32x8_t _3545 = uint32x8_t::convert_from<uint16x8_t>(_3544);
           uint32x8_t _3546 = _3535 + _3545;
           _3546.store(_multiplied_no_offsets, _2472);
          } // if _584
          else
          {
           int32_t _3547 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3548 = _3547 * _40;
           int32_t _3549 = _3548 * 4;
           int32_t _3550 = _3549 + _547;
           int32_t _3551 = _590 * _2456;
           int32_t _3552 = _3551 * 4;
           int32_t _3553 = _3552 + _605;
           int32_t _3554 = _31 * _2456;
           int32_t _3555 = _3554 + _2454;
           int32_t _3556 = _3555 + _multiplied_no_offsets_s1_k__x_rki;
           int32_t _3557 = _553 + 16;
           int32_t _3558 = _552 + 16;
           int32_t _3559 = _40 + 16;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2445; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _3560 = _multiplied_no_offsets_s1_x_xi_xii + _3553;
            uint32_t _3561 = _multiplied_no_offsets[_3560];
            int32_t _3562 = _3556 * 4;
            uint8_t _3563 = ((const uint8_t *)_mat_a)[_3562];
            uint16_t _3564 = (uint16_t)(_3563);
            int32_t _3565 = _multiplied_no_offsets_s1_x_xi_xii + _3550;
            int32_t _3566 = _3565 + 16;
            uint8_t _3567 = ((const uint8_t *)_mat_b)[_3566];
            uint16_t _3568 = (uint16_t)(_3567);
            uint16_t _3569 = _3564 * _3568;
            uint32_t _3570 = (uint32_t)(_3569);
            uint32_t _3571 = _3561 + _3570;
            int32_t _3572 = _3562 + 1;
            uint8_t _3573 = ((const uint8_t *)_mat_a)[_3572];
            uint16_t _3574 = (uint16_t)(_3573);
            int32_t _3575 = _3565 + _3559;
            uint8_t _3576 = ((const uint8_t *)_mat_b)[_3575];
            uint16_t _3577 = (uint16_t)(_3576);
            uint16_t _3578 = _3574 * _3577;
            uint32_t _3579 = (uint32_t)(_3578);
            uint32_t _3580 = _3571 + _3579;
            int32_t _3581 = _3562 + 2;
            uint8_t _3582 = ((const uint8_t *)_mat_a)[_3581];
            uint16_t _3583 = (uint16_t)(_3582);
            int32_t _3584 = _3565 + _3558;
            uint8_t _3585 = ((const uint8_t *)_mat_b)[_3584];
            uint16_t _3586 = (uint16_t)(_3585);
            uint16_t _3587 = _3583 * _3586;
            uint32_t _3588 = (uint32_t)(_3587);
            uint32_t _3589 = _3580 + _3588;
            int32_t _3590 = _3562 + 3;
            uint8_t _3591 = ((const uint8_t *)_mat_a)[_3590];
            uint16_t _3592 = (uint16_t)(_3591);
            int32_t _3593 = _3565 + _3557;
            uint8_t _3594 = ((const uint8_t *)_mat_b)[_3593];
            uint16_t _3595 = (uint16_t)(_3594);
            uint16_t _3596 = _3592 * _3595;
            uint32_t _3597 = (uint32_t)(_3596);
            uint32_t _3598 = _3589 + _3597;
            int32_t _3599 = _multiplied_no_offsets_s1_x_xi_xii + _2468;
            _multiplied_no_offsets[_3599] = _3598;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _3600 = _2456 * 4;
           int32_t _3601 = _3600 + 1;
           int32_t _3602 = _31 * _3601;
           int32_t _3603 = _590 * _3601;
           int32_t _3604 = _3603 + _605;
           int32_t _3605 = _553 + 16;
           int32_t _3606 = _552 + 16;
           int32_t _3607 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2445; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _3608 = _multiplied_no_offsets_s1_x_xi_xii + _3604;
            uint32_t _3609 = _multiplied_no_offsets[_3608];
            int32_t _3610 = _3607 * 4;
            int32_t _3611 = _3610 + _3602;
            uint8_t _3612 = ((const uint8_t *)_mat_a)[_3611];
            uint16_t _3613 = (uint16_t)(_3612);
            int32_t _3614 = _40 * _3607;
            int32_t _3615 = _3614 * 4;
            int32_t _3616 = _3615 + _547;
            int32_t _3617 = _3616 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _3618 = _3617 + 16;
            uint8_t _3619 = ((const uint8_t *)_mat_b)[_3618];
            uint16_t _3620 = (uint16_t)(_3619);
            uint16_t _3621 = _3613 * _3620;
            uint32_t _3622 = (uint32_t)(_3621);
            uint32_t _3623 = _3609 + _3622;
            int32_t _3624 = _3611 + 1;
            uint8_t _3625 = ((const uint8_t *)_mat_a)[_3624];
            uint16_t _3626 = (uint16_t)(_3625);
            int32_t _3627 = _3617 + _40;
            int32_t _3628 = _3627 + 16;
            uint8_t _3629 = ((const uint8_t *)_mat_b)[_3628];
            uint16_t _3630 = (uint16_t)(_3629);
            uint16_t _3631 = _3626 * _3630;
            uint32_t _3632 = (uint32_t)(_3631);
            uint32_t _3633 = _3623 + _3632;
            int32_t _3634 = _3611 + 2;
            uint8_t _3635 = ((const uint8_t *)_mat_a)[_3634];
            uint16_t _3636 = (uint16_t)(_3635);
            int32_t _3637 = _3617 + _3606;
            uint8_t _3638 = ((const uint8_t *)_mat_b)[_3637];
            uint16_t _3639 = (uint16_t)(_3638);
            uint16_t _3640 = _3636 * _3639;
            uint32_t _3641 = (uint32_t)(_3640);
            uint32_t _3642 = _3633 + _3641;
            int32_t _3643 = _3611 + 3;
            uint8_t _3644 = ((const uint8_t *)_mat_a)[_3643];
            uint16_t _3645 = (uint16_t)(_3644);
            int32_t _3646 = _3617 + _3605;
            uint8_t _3647 = ((const uint8_t *)_mat_b)[_3646];
            uint16_t _3648 = (uint16_t)(_3647);
            uint16_t _3649 = _3645 * _3648;
            uint32_t _3650 = (uint32_t)(_3649);
            uint32_t _3651 = _3642 + _3650;
            int32_t _3652 = _multiplied_no_offsets_s1_x_xi_xii + _2480;
            _multiplied_no_offsets[_3652] = _3651;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _3653 = _2456 * 4;
           int32_t _3654 = _3653 + 2;
           int32_t _3655 = _31 * _3654;
           int32_t _3656 = _590 * _3654;
           int32_t _3657 = _3656 + _605;
           int32_t _3658 = _553 + 16;
           int32_t _3659 = _552 + 16;
           int32_t _3660 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2445; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _3661 = _multiplied_no_offsets_s1_x_xi_xii + _3657;
            uint32_t _3662 = _multiplied_no_offsets[_3661];
            int32_t _3663 = _3660 * 4;
            int32_t _3664 = _3663 + _3655;
            uint8_t _3665 = ((const uint8_t *)_mat_a)[_3664];
            uint16_t _3666 = (uint16_t)(_3665);
            int32_t _3667 = _40 * _3660;
            int32_t _3668 = _3667 * 4;
            int32_t _3669 = _3668 + _547;
            int32_t _3670 = _3669 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _3671 = _3670 + 16;
            uint8_t _3672 = ((const uint8_t *)_mat_b)[_3671];
            uint16_t _3673 = (uint16_t)(_3672);
            uint16_t _3674 = _3666 * _3673;
            uint32_t _3675 = (uint32_t)(_3674);
            uint32_t _3676 = _3662 + _3675;
            int32_t _3677 = _3664 + 1;
            uint8_t _3678 = ((const uint8_t *)_mat_a)[_3677];
            uint16_t _3679 = (uint16_t)(_3678);
            int32_t _3680 = _3670 + _40;
            int32_t _3681 = _3680 + 16;
            uint8_t _3682 = ((const uint8_t *)_mat_b)[_3681];
            uint16_t _3683 = (uint16_t)(_3682);
            uint16_t _3684 = _3679 * _3683;
            uint32_t _3685 = (uint32_t)(_3684);
            uint32_t _3686 = _3676 + _3685;
            int32_t _3687 = _3664 + 2;
            uint8_t _3688 = ((const uint8_t *)_mat_a)[_3687];
            uint16_t _3689 = (uint16_t)(_3688);
            int32_t _3690 = _3670 + _3659;
            uint8_t _3691 = ((const uint8_t *)_mat_b)[_3690];
            uint16_t _3692 = (uint16_t)(_3691);
            uint16_t _3693 = _3689 * _3692;
            uint32_t _3694 = (uint32_t)(_3693);
            uint32_t _3695 = _3686 + _3694;
            int32_t _3696 = _3664 + 3;
            uint8_t _3697 = ((const uint8_t *)_mat_a)[_3696];
            uint16_t _3698 = (uint16_t)(_3697);
            int32_t _3699 = _3670 + _3658;
            uint8_t _3700 = ((const uint8_t *)_mat_b)[_3699];
            uint16_t _3701 = (uint16_t)(_3700);
            uint16_t _3702 = _3698 * _3701;
            uint32_t _3703 = (uint32_t)(_3702);
            uint32_t _3704 = _3695 + _3703;
            int32_t _3705 = _multiplied_no_offsets_s1_x_xi_xii + _2476;
            _multiplied_no_offsets[_3705] = _3704;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _3706 = _2456 * 4;
           int32_t _3707 = _3706 + 3;
           int32_t _3708 = _31 * _3707;
           int32_t _3709 = _590 * _3707;
           int32_t _3710 = _3709 + _605;
           int32_t _3711 = _553 + 16;
           int32_t _3712 = _552 + 16;
           int32_t _3713 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2445; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _3714 = _multiplied_no_offsets_s1_x_xi_xii + _3710;
            uint32_t _3715 = _multiplied_no_offsets[_3714];
            int32_t _3716 = _3713 * 4;
            int32_t _3717 = _3716 + _3708;
            uint8_t _3718 = ((const uint8_t *)_mat_a)[_3717];
            uint16_t _3719 = (uint16_t)(_3718);
            int32_t _3720 = _40 * _3713;
            int32_t _3721 = _3720 * 4;
            int32_t _3722 = _3721 + _547;
            int32_t _3723 = _3722 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _3724 = _3723 + 16;
            uint8_t _3725 = ((const uint8_t *)_mat_b)[_3724];
            uint16_t _3726 = (uint16_t)(_3725);
            uint16_t _3727 = _3719 * _3726;
            uint32_t _3728 = (uint32_t)(_3727);
            uint32_t _3729 = _3715 + _3728;
            int32_t _3730 = _3717 + 1;
            uint8_t _3731 = ((const uint8_t *)_mat_a)[_3730];
            uint16_t _3732 = (uint16_t)(_3731);
            int32_t _3733 = _3723 + _40;
            int32_t _3734 = _3733 + 16;
            uint8_t _3735 = ((const uint8_t *)_mat_b)[_3734];
            uint16_t _3736 = (uint16_t)(_3735);
            uint16_t _3737 = _3732 * _3736;
            uint32_t _3738 = (uint32_t)(_3737);
            uint32_t _3739 = _3729 + _3738;
            int32_t _3740 = _3717 + 2;
            uint8_t _3741 = ((const uint8_t *)_mat_a)[_3740];
            uint16_t _3742 = (uint16_t)(_3741);
            int32_t _3743 = _3723 + _3712;
            uint8_t _3744 = ((const uint8_t *)_mat_b)[_3743];
            uint16_t _3745 = (uint16_t)(_3744);
            uint16_t _3746 = _3742 * _3745;
            uint32_t _3747 = (uint32_t)(_3746);
            uint32_t _3748 = _3739 + _3747;
            int32_t _3749 = _3717 + 3;
            uint8_t _3750 = ((const uint8_t *)_mat_a)[_3749];
            uint16_t _3751 = (uint16_t)(_3750);
            int32_t _3752 = _3723 + _3711;
            uint8_t _3753 = ((const uint8_t *)_mat_b)[_3752];
            uint16_t _3754 = (uint16_t)(_3753);
            uint16_t _3755 = _3751 * _3754;
            uint32_t _3756 = (uint32_t)(_3755);
            uint32_t _3757 = _3748 + _3756;
            int32_t _3758 = _multiplied_no_offsets_s1_x_xi_xii + _2472;
            _multiplied_no_offsets[_3758] = _3757;
           } // for _multiplied_no_offsets_s1_x_xi_xii
          } // if _584 else
          if (_585)
          {
           int32_t _3759 = _590 * _2456;
           int32_t _3760 = _3759 * 4;
           int32_t _3761 = _3760 + _604;
           uint32x8_t _3762 = uint32x8_t::load(_multiplied_no_offsets, _3761);
           int32_t _3763 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3764 = _3763 * _40;
           int32_t _3765 = _3764 + _2441;
           int32_t _3766 = _3765 * 4;
           int32_t _3767 = _3766 + 24;
           uint8x8_t _3768 = uint8x8_t::load(_mat_b, _3767);
           uint16x8_t _3769 = uint16x8_t::convert_from<uint8x8_t>(_3768);
           int32_t _3770 = _31 * _2456;
           int32_t _3771 = _3770 + _2454;
           int32_t _3772 = _3771 + _multiplied_no_offsets_s1_k__x_rki;
           int32_t _3773 = _3772 * 4;
           uint8_t _3774 = ((const uint8_t *)_mat_a)[_3773];
           uint16_t _3775 = (uint16_t)(_3774);
           uint16x8_t _3776 = uint16x8_t::broadcast(_3775);
           uint16x8_t _3777 = _3769 * _3776;
           uint32x8_t _3778 = uint32x8_t::convert_from<uint16x8_t>(_3777);
           uint32x8_t _3779 = _3762 + _3778;
           int32_t _3780 = _3766 + _40;
           int32_t _3781 = _3780 + 24;
           uint8x8_t _3782 = uint8x8_t::load(_mat_b, _3781);
           uint16x8_t _3783 = uint16x8_t::convert_from<uint8x8_t>(_3782);
           int32_t _3784 = _3773 + 1;
           uint8_t _3785 = ((const uint8_t *)_mat_a)[_3784];
           uint16_t _3786 = (uint16_t)(_3785);
           uint16x8_t _3787 = uint16x8_t::broadcast(_3786);
           uint16x8_t _3788 = _3783 * _3787;
           uint32x8_t _3789 = uint32x8_t::convert_from<uint16x8_t>(_3788);
           uint32x8_t _3790 = _3779 + _3789;
           int32_t _3791 = _3765 * 2;
           int32_t _3792 = _3791 + _40;
           int32_t _3793 = _3792 * 2;
           int32_t _3794 = _3793 + 24;
           uint8x8_t _3795 = uint8x8_t::load(_mat_b, _3794);
           uint16x8_t _3796 = uint16x8_t::convert_from<uint8x8_t>(_3795);
           int32_t _3797 = _3773 + 2;
           uint8_t _3798 = ((const uint8_t *)_mat_a)[_3797];
           uint16_t _3799 = (uint16_t)(_3798);
           uint16x8_t _3800 = uint16x8_t::broadcast(_3799);
           uint16x8_t _3801 = _3796 * _3800;
           uint32x8_t _3802 = uint32x8_t::convert_from<uint16x8_t>(_3801);
           uint32x8_t _3803 = _3790 + _3802;
           int32_t _3804 = _3766 + _553;
           int32_t _3805 = _3804 + 24;
           uint8x8_t _3806 = uint8x8_t::load(_mat_b, _3805);
           uint16x8_t _3807 = uint16x8_t::convert_from<uint8x8_t>(_3806);
           int32_t _3808 = _3773 + 3;
           uint8_t _3809 = ((const uint8_t *)_mat_a)[_3808];
           uint16_t _3810 = (uint16_t)(_3809);
           uint16x8_t _3811 = uint16x8_t::broadcast(_3810);
           uint16x8_t _3812 = _3807 * _3811;
           uint32x8_t _3813 = uint32x8_t::convert_from<uint16x8_t>(_3812);
           uint32x8_t _3814 = _3803 + _3813;
           _3814.store(_multiplied_no_offsets, _2467);
           int32_t _3815 = _2456 * 4;
           int32_t _3816 = _3815 + 1;
           int32_t _3817 = _3816 * _31;
           int32_t _3818 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3819 = _3818 * 4;
           int32_t _3820 = _3817 + _3819;
           int32_t _3821 = _3816 * _590;
           int32_t _3822 = _3821 + _604;
           uint32x8_t _3823 = uint32x8_t::load(_multiplied_no_offsets, _3822);
           int32_t _3824 = _3818 * _40;
           int32_t _3825 = _3824 + _2441;
           int32_t _3826 = _3825 * 4;
           int32_t _3827 = _3826 + 24;
           uint8x8_t _3828 = uint8x8_t::load(_mat_b, _3827);
           uint16x8_t _3829 = uint16x8_t::convert_from<uint8x8_t>(_3828);
           uint8_t _3830 = ((const uint8_t *)_mat_a)[_3820];
           uint16_t _3831 = (uint16_t)(_3830);
           uint16x8_t _3832 = uint16x8_t::broadcast(_3831);
           uint16x8_t _3833 = _3829 * _3832;
           uint32x8_t _3834 = uint32x8_t::convert_from<uint16x8_t>(_3833);
           uint32x8_t _3835 = _3823 + _3834;
           int32_t _3836 = _3826 + _40;
           int32_t _3837 = _3836 + 24;
           uint8x8_t _3838 = uint8x8_t::load(_mat_b, _3837);
           uint16x8_t _3839 = uint16x8_t::convert_from<uint8x8_t>(_3838);
           int32_t _3840 = _3820 + 1;
           uint8_t _3841 = ((const uint8_t *)_mat_a)[_3840];
           uint16_t _3842 = (uint16_t)(_3841);
           uint16x8_t _3843 = uint16x8_t::broadcast(_3842);
           uint16x8_t _3844 = _3839 * _3843;
           uint32x8_t _3845 = uint32x8_t::convert_from<uint16x8_t>(_3844);
           uint32x8_t _3846 = _3835 + _3845;
           int32_t _3847 = _3825 * 2;
           int32_t _3848 = _3847 + _40;
           int32_t _3849 = _3848 * 2;
           int32_t _3850 = _3849 + 24;
           uint8x8_t _3851 = uint8x8_t::load(_mat_b, _3850);
           uint16x8_t _3852 = uint16x8_t::convert_from<uint8x8_t>(_3851);
           int32_t _3853 = _3820 + 2;
           uint8_t _3854 = ((const uint8_t *)_mat_a)[_3853];
           uint16_t _3855 = (uint16_t)(_3854);
           uint16x8_t _3856 = uint16x8_t::broadcast(_3855);
           uint16x8_t _3857 = _3852 * _3856;
           uint32x8_t _3858 = uint32x8_t::convert_from<uint16x8_t>(_3857);
           uint32x8_t _3859 = _3846 + _3858;
           int32_t _3860 = _3826 + _553;
           int32_t _3861 = _3860 + 24;
           uint8x8_t _3862 = uint8x8_t::load(_mat_b, _3861);
           uint16x8_t _3863 = uint16x8_t::convert_from<uint8x8_t>(_3862);
           int32_t _3864 = _3820 + 3;
           uint8_t _3865 = ((const uint8_t *)_mat_a)[_3864];
           uint16_t _3866 = (uint16_t)(_3865);
           uint16x8_t _3867 = uint16x8_t::broadcast(_3866);
           uint16x8_t _3868 = _3863 * _3867;
           uint32x8_t _3869 = uint32x8_t::convert_from<uint16x8_t>(_3868);
           uint32x8_t _3870 = _3859 + _3869;
           _3870.store(_multiplied_no_offsets, _2479);
           int32_t _3871 = _2456 * 4;
           int32_t _3872 = _3871 + 2;
           int32_t _3873 = _3872 * _31;
           int32_t _3874 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3875 = _3874 * 4;
           int32_t _3876 = _3873 + _3875;
           int32_t _3877 = _3872 * _590;
           int32_t _3878 = _3877 + _604;
           uint32x8_t _3879 = uint32x8_t::load(_multiplied_no_offsets, _3878);
           int32_t _3880 = _3874 * _40;
           int32_t _3881 = _3880 + _2441;
           int32_t _3882 = _3881 * 4;
           int32_t _3883 = _3882 + 24;
           uint8x8_t _3884 = uint8x8_t::load(_mat_b, _3883);
           uint16x8_t _3885 = uint16x8_t::convert_from<uint8x8_t>(_3884);
           uint8_t _3886 = ((const uint8_t *)_mat_a)[_3876];
           uint16_t _3887 = (uint16_t)(_3886);
           uint16x8_t _3888 = uint16x8_t::broadcast(_3887);
           uint16x8_t _3889 = _3885 * _3888;
           uint32x8_t _3890 = uint32x8_t::convert_from<uint16x8_t>(_3889);
           uint32x8_t _3891 = _3879 + _3890;
           int32_t _3892 = _3882 + _40;
           int32_t _3893 = _3892 + 24;
           uint8x8_t _3894 = uint8x8_t::load(_mat_b, _3893);
           uint16x8_t _3895 = uint16x8_t::convert_from<uint8x8_t>(_3894);
           int32_t _3896 = _3876 + 1;
           uint8_t _3897 = ((const uint8_t *)_mat_a)[_3896];
           uint16_t _3898 = (uint16_t)(_3897);
           uint16x8_t _3899 = uint16x8_t::broadcast(_3898);
           uint16x8_t _3900 = _3895 * _3899;
           uint32x8_t _3901 = uint32x8_t::convert_from<uint16x8_t>(_3900);
           uint32x8_t _3902 = _3891 + _3901;
           int32_t _3903 = _3881 * 2;
           int32_t _3904 = _3903 + _40;
           int32_t _3905 = _3904 * 2;
           int32_t _3906 = _3905 + 24;
           uint8x8_t _3907 = uint8x8_t::load(_mat_b, _3906);
           uint16x8_t _3908 = uint16x8_t::convert_from<uint8x8_t>(_3907);
           int32_t _3909 = _3876 + 2;
           uint8_t _3910 = ((const uint8_t *)_mat_a)[_3909];
           uint16_t _3911 = (uint16_t)(_3910);
           uint16x8_t _3912 = uint16x8_t::broadcast(_3911);
           uint16x8_t _3913 = _3908 * _3912;
           uint32x8_t _3914 = uint32x8_t::convert_from<uint16x8_t>(_3913);
           uint32x8_t _3915 = _3902 + _3914;
           int32_t _3916 = _3882 + _553;
           int32_t _3917 = _3916 + 24;
           uint8x8_t _3918 = uint8x8_t::load(_mat_b, _3917);
           uint16x8_t _3919 = uint16x8_t::convert_from<uint8x8_t>(_3918);
           int32_t _3920 = _3876 + 3;
           uint8_t _3921 = ((const uint8_t *)_mat_a)[_3920];
           uint16_t _3922 = (uint16_t)(_3921);
           uint16x8_t _3923 = uint16x8_t::broadcast(_3922);
           uint16x8_t _3924 = _3919 * _3923;
           uint32x8_t _3925 = uint32x8_t::convert_from<uint16x8_t>(_3924);
           uint32x8_t _3926 = _3915 + _3925;
           _3926.store(_multiplied_no_offsets, _2475);
           int32_t _3927 = _2456 * 4;
           int32_t _3928 = _3927 + 3;
           int32_t _3929 = _3928 * _31;
           int32_t _3930 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3931 = _3930 * 4;
           int32_t _3932 = _3929 + _3931;
           int32_t _3933 = _3928 * _590;
           int32_t _3934 = _3933 + _604;
           uint32x8_t _3935 = uint32x8_t::load(_multiplied_no_offsets, _3934);
           int32_t _3936 = _3930 * _40;
           int32_t _3937 = _3936 + _2441;
           int32_t _3938 = _3937 * 4;
           int32_t _3939 = _3938 + 24;
           uint8x8_t _3940 = uint8x8_t::load(_mat_b, _3939);
           uint16x8_t _3941 = uint16x8_t::convert_from<uint8x8_t>(_3940);
           uint8_t _3942 = ((const uint8_t *)_mat_a)[_3932];
           uint16_t _3943 = (uint16_t)(_3942);
           uint16x8_t _3944 = uint16x8_t::broadcast(_3943);
           uint16x8_t _3945 = _3941 * _3944;
           uint32x8_t _3946 = uint32x8_t::convert_from<uint16x8_t>(_3945);
           uint32x8_t _3947 = _3935 + _3946;
           int32_t _3948 = _3938 + _40;
           int32_t _3949 = _3948 + 24;
           uint8x8_t _3950 = uint8x8_t::load(_mat_b, _3949);
           uint16x8_t _3951 = uint16x8_t::convert_from<uint8x8_t>(_3950);
           int32_t _3952 = _3932 + 1;
           uint8_t _3953 = ((const uint8_t *)_mat_a)[_3952];
           uint16_t _3954 = (uint16_t)(_3953);
           uint16x8_t _3955 = uint16x8_t::broadcast(_3954);
           uint16x8_t _3956 = _3951 * _3955;
           uint32x8_t _3957 = uint32x8_t::convert_from<uint16x8_t>(_3956);
           uint32x8_t _3958 = _3947 + _3957;
           int32_t _3959 = _3937 * 2;
           int32_t _3960 = _3959 + _40;
           int32_t _3961 = _3960 * 2;
           int32_t _3962 = _3961 + 24;
           uint8x8_t _3963 = uint8x8_t::load(_mat_b, _3962);
           uint16x8_t _3964 = uint16x8_t::convert_from<uint8x8_t>(_3963);
           int32_t _3965 = _3932 + 2;
           uint8_t _3966 = ((const uint8_t *)_mat_a)[_3965];
           uint16_t _3967 = (uint16_t)(_3966);
           uint16x8_t _3968 = uint16x8_t::broadcast(_3967);
           uint16x8_t _3969 = _3964 * _3968;
           uint32x8_t _3970 = uint32x8_t::convert_from<uint16x8_t>(_3969);
           uint32x8_t _3971 = _3958 + _3970;
           int32_t _3972 = _3938 + _553;
           int32_t _3973 = _3972 + 24;
           uint8x8_t _3974 = uint8x8_t::load(_mat_b, _3973);
           uint16x8_t _3975 = uint16x8_t::convert_from<uint8x8_t>(_3974);
           int32_t _3976 = _3932 + 3;
           uint8_t _3977 = ((const uint8_t *)_mat_a)[_3976];
           uint16_t _3978 = (uint16_t)(_3977);
           uint16x8_t _3979 = uint16x8_t::broadcast(_3978);
           uint16x8_t _3980 = _3975 * _3979;
           uint32x8_t _3981 = uint32x8_t::convert_from<uint16x8_t>(_3980);
           uint32x8_t _3982 = _3971 + _3981;
           _3982.store(_multiplied_no_offsets, _2471);
          } // if _585
          else
          {
           int32_t _3983 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           int32_t _3984 = _3983 * _40;
           int32_t _3985 = _3984 * 4;
           int32_t _3986 = _3985 + _547;
           int32_t _3987 = _590 * _2456;
           int32_t _3988 = _3987 * 4;
           int32_t _3989 = _3988 + _604;
           int32_t _3990 = _31 * _2456;
           int32_t _3991 = _3990 + _2454;
           int32_t _3992 = _3991 + _multiplied_no_offsets_s1_k__x_rki;
           int32_t _3993 = _553 + 24;
           int32_t _3994 = _552 + 24;
           int32_t _3995 = _40 + 24;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2449; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _3996 = _multiplied_no_offsets_s1_x_xi_xii + _3989;
            uint32_t _3997 = _multiplied_no_offsets[_3996];
            int32_t _3998 = _3992 * 4;
            uint8_t _3999 = ((const uint8_t *)_mat_a)[_3998];
            uint16_t _4000 = (uint16_t)(_3999);
            int32_t _4001 = _multiplied_no_offsets_s1_x_xi_xii + _3986;
            int32_t _4002 = _4001 + 24;
            uint8_t _4003 = ((const uint8_t *)_mat_b)[_4002];
            uint16_t _4004 = (uint16_t)(_4003);
            uint16_t _4005 = _4000 * _4004;
            uint32_t _4006 = (uint32_t)(_4005);
            uint32_t _4007 = _3997 + _4006;
            int32_t _4008 = _3998 + 1;
            uint8_t _4009 = ((const uint8_t *)_mat_a)[_4008];
            uint16_t _4010 = (uint16_t)(_4009);
            int32_t _4011 = _4001 + _3995;
            uint8_t _4012 = ((const uint8_t *)_mat_b)[_4011];
            uint16_t _4013 = (uint16_t)(_4012);
            uint16_t _4014 = _4010 * _4013;
            uint32_t _4015 = (uint32_t)(_4014);
            uint32_t _4016 = _4007 + _4015;
            int32_t _4017 = _3998 + 2;
            uint8_t _4018 = ((const uint8_t *)_mat_a)[_4017];
            uint16_t _4019 = (uint16_t)(_4018);
            int32_t _4020 = _4001 + _3994;
            uint8_t _4021 = ((const uint8_t *)_mat_b)[_4020];
            uint16_t _4022 = (uint16_t)(_4021);
            uint16_t _4023 = _4019 * _4022;
            uint32_t _4024 = (uint32_t)(_4023);
            uint32_t _4025 = _4016 + _4024;
            int32_t _4026 = _3998 + 3;
            uint8_t _4027 = ((const uint8_t *)_mat_a)[_4026];
            uint16_t _4028 = (uint16_t)(_4027);
            int32_t _4029 = _4001 + _3993;
            uint8_t _4030 = ((const uint8_t *)_mat_b)[_4029];
            uint16_t _4031 = (uint16_t)(_4030);
            uint16_t _4032 = _4028 * _4031;
            uint32_t _4033 = (uint32_t)(_4032);
            uint32_t _4034 = _4025 + _4033;
            int32_t _4035 = _multiplied_no_offsets_s1_x_xi_xii + _2467;
            _multiplied_no_offsets[_4035] = _4034;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _4036 = _2456 * 4;
           int32_t _4037 = _4036 + 1;
           int32_t _4038 = _31 * _4037;
           int32_t _4039 = _590 * _4037;
           int32_t _4040 = _4039 + _604;
           int32_t _4041 = _553 + 24;
           int32_t _4042 = _552 + 24;
           int32_t _4043 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2449; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _4044 = _multiplied_no_offsets_s1_x_xi_xii + _4040;
            uint32_t _4045 = _multiplied_no_offsets[_4044];
            int32_t _4046 = _4043 * 4;
            int32_t _4047 = _4046 + _4038;
            uint8_t _4048 = ((const uint8_t *)_mat_a)[_4047];
            uint16_t _4049 = (uint16_t)(_4048);
            int32_t _4050 = _40 * _4043;
            int32_t _4051 = _4050 * 4;
            int32_t _4052 = _4051 + _547;
            int32_t _4053 = _4052 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _4054 = _4053 + 24;
            uint8_t _4055 = ((const uint8_t *)_mat_b)[_4054];
            uint16_t _4056 = (uint16_t)(_4055);
            uint16_t _4057 = _4049 * _4056;
            uint32_t _4058 = (uint32_t)(_4057);
            uint32_t _4059 = _4045 + _4058;
            int32_t _4060 = _4047 + 1;
            uint8_t _4061 = ((const uint8_t *)_mat_a)[_4060];
            uint16_t _4062 = (uint16_t)(_4061);
            int32_t _4063 = _4053 + _40;
            int32_t _4064 = _4063 + 24;
            uint8_t _4065 = ((const uint8_t *)_mat_b)[_4064];
            uint16_t _4066 = (uint16_t)(_4065);
            uint16_t _4067 = _4062 * _4066;
            uint32_t _4068 = (uint32_t)(_4067);
            uint32_t _4069 = _4059 + _4068;
            int32_t _4070 = _4047 + 2;
            uint8_t _4071 = ((const uint8_t *)_mat_a)[_4070];
            uint16_t _4072 = (uint16_t)(_4071);
            int32_t _4073 = _4053 + _4042;
            uint8_t _4074 = ((const uint8_t *)_mat_b)[_4073];
            uint16_t _4075 = (uint16_t)(_4074);
            uint16_t _4076 = _4072 * _4075;
            uint32_t _4077 = (uint32_t)(_4076);
            uint32_t _4078 = _4069 + _4077;
            int32_t _4079 = _4047 + 3;
            uint8_t _4080 = ((const uint8_t *)_mat_a)[_4079];
            uint16_t _4081 = (uint16_t)(_4080);
            int32_t _4082 = _4053 + _4041;
            uint8_t _4083 = ((const uint8_t *)_mat_b)[_4082];
            uint16_t _4084 = (uint16_t)(_4083);
            uint16_t _4085 = _4081 * _4084;
            uint32_t _4086 = (uint32_t)(_4085);
            uint32_t _4087 = _4078 + _4086;
            int32_t _4088 = _multiplied_no_offsets_s1_x_xi_xii + _2479;
            _multiplied_no_offsets[_4088] = _4087;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _4089 = _2456 * 4;
           int32_t _4090 = _4089 + 2;
           int32_t _4091 = _31 * _4090;
           int32_t _4092 = _590 * _4090;
           int32_t _4093 = _4092 + _604;
           int32_t _4094 = _553 + 24;
           int32_t _4095 = _552 + 24;
           int32_t _4096 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2449; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _4097 = _multiplied_no_offsets_s1_x_xi_xii + _4093;
            uint32_t _4098 = _multiplied_no_offsets[_4097];
            int32_t _4099 = _4096 * 4;
            int32_t _4100 = _4099 + _4091;
            uint8_t _4101 = ((const uint8_t *)_mat_a)[_4100];
            uint16_t _4102 = (uint16_t)(_4101);
            int32_t _4103 = _40 * _4096;
            int32_t _4104 = _4103 * 4;
            int32_t _4105 = _4104 + _547;
            int32_t _4106 = _4105 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _4107 = _4106 + 24;
            uint8_t _4108 = ((const uint8_t *)_mat_b)[_4107];
            uint16_t _4109 = (uint16_t)(_4108);
            uint16_t _4110 = _4102 * _4109;
            uint32_t _4111 = (uint32_t)(_4110);
            uint32_t _4112 = _4098 + _4111;
            int32_t _4113 = _4100 + 1;
            uint8_t _4114 = ((const uint8_t *)_mat_a)[_4113];
            uint16_t _4115 = (uint16_t)(_4114);
            int32_t _4116 = _4106 + _40;
            int32_t _4117 = _4116 + 24;
            uint8_t _4118 = ((const uint8_t *)_mat_b)[_4117];
            uint16_t _4119 = (uint16_t)(_4118);
            uint16_t _4120 = _4115 * _4119;
            uint32_t _4121 = (uint32_t)(_4120);
            uint32_t _4122 = _4112 + _4121;
            int32_t _4123 = _4100 + 2;
            uint8_t _4124 = ((const uint8_t *)_mat_a)[_4123];
            uint16_t _4125 = (uint16_t)(_4124);
            int32_t _4126 = _4106 + _4095;
            uint8_t _4127 = ((const uint8_t *)_mat_b)[_4126];
            uint16_t _4128 = (uint16_t)(_4127);
            uint16_t _4129 = _4125 * _4128;
            uint32_t _4130 = (uint32_t)(_4129);
            uint32_t _4131 = _4122 + _4130;
            int32_t _4132 = _4100 + 3;
            uint8_t _4133 = ((const uint8_t *)_mat_a)[_4132];
            uint16_t _4134 = (uint16_t)(_4133);
            int32_t _4135 = _4106 + _4094;
            uint8_t _4136 = ((const uint8_t *)_mat_b)[_4135];
            uint16_t _4137 = (uint16_t)(_4136);
            uint16_t _4138 = _4134 * _4137;
            uint32_t _4139 = (uint32_t)(_4138);
            uint32_t _4140 = _4131 + _4139;
            int32_t _4141 = _multiplied_no_offsets_s1_x_xi_xii + _2475;
            _multiplied_no_offsets[_4141] = _4140;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _4142 = _2456 * 4;
           int32_t _4143 = _4142 + 3;
           int32_t _4144 = _31 * _4143;
           int32_t _4145 = _590 * _4143;
           int32_t _4146 = _4145 + _604;
           int32_t _4147 = _553 + 24;
           int32_t _4148 = _552 + 24;
           int32_t _4149 = _multiplied_no_offsets_s1_k__x_rki + _2454;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _2449; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _4150 = _multiplied_no_offsets_s1_x_xi_xii + _4146;
            uint32_t _4151 = _multiplied_no_offsets[_4150];
            int32_t _4152 = _4149 * 4;
            int32_t _4153 = _4152 + _4144;
            uint8_t _4154 = ((const uint8_t *)_mat_a)[_4153];
            uint16_t _4155 = (uint16_t)(_4154);
            int32_t _4156 = _40 * _4149;
            int32_t _4157 = _4156 * 4;
            int32_t _4158 = _4157 + _547;
            int32_t _4159 = _4158 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _4160 = _4159 + 24;
            uint8_t _4161 = ((const uint8_t *)_mat_b)[_4160];
            uint16_t _4162 = (uint16_t)(_4161);
            uint16_t _4163 = _4155 * _4162;
            uint32_t _4164 = (uint32_t)(_4163);
            uint32_t _4165 = _4151 + _4164;
            int32_t _4166 = _4153 + 1;
            uint8_t _4167 = ((const uint8_t *)_mat_a)[_4166];
            uint16_t _4168 = (uint16_t)(_4167);
            int32_t _4169 = _4159 + _40;
            int32_t _4170 = _4169 + 24;
            uint8_t _4171 = ((const uint8_t *)_mat_b)[_4170];
            uint16_t _4172 = (uint16_t)(_4171);
            uint16_t _4173 = _4168 * _4172;
            uint32_t _4174 = (uint32_t)(_4173);
            uint32_t _4175 = _4165 + _4174;
            int32_t _4176 = _4153 + 2;
            uint8_t _4177 = ((const uint8_t *)_mat_a)[_4176];
            uint16_t _4178 = (uint16_t)(_4177);
            int32_t _4179 = _4159 + _4148;
            uint8_t _4180 = ((const uint8_t *)_mat_b)[_4179];
            uint16_t _4181 = (uint16_t)(_4180);
            uint16_t _4182 = _4178 * _4181;
            uint32_t _4183 = (uint32_t)(_4182);
            uint32_t _4184 = _4175 + _4183;
            int32_t _4185 = _4153 + 3;
            uint8_t _4186 = ((const uint8_t *)_mat_a)[_4185];
            uint16_t _4187 = (uint16_t)(_4186);
            int32_t _4188 = _4159 + _4147;
            uint8_t _4189 = ((const uint8_t *)_mat_b)[_4188];
            uint16_t _4190 = (uint16_t)(_4189);
            uint16_t _4191 = _4187 * _4190;
            uint32_t _4192 = (uint32_t)(_4191);
            uint32_t _4193 = _4184 + _4192;
            int32_t _4194 = _multiplied_no_offsets_s1_x_xi_xii + _2471;
            _multiplied_no_offsets[_4194] = _4193;
           } // for _multiplied_no_offsets_s1_x_xi_xii
          } // if _585 else
         } // for _multiplied_no_offsets_s1_k__x_rki
        } // for _multiplied_no_offsets_s1_y_yi_yi
       } // for _multiplied_no_offsets_s1_k__x_k__x
      } // if _587
     } // if _608
     else
     {
      int32_t _4195 = _multiplied_no_offsets_s1_y_y * 8;
      int32_t _4196 = _598 - _4195;
      int32_t _4197 = _552 + 24;
      int32_t _4198 = _552 + 16;
      for (int _multiplied_no_offsets_s1_x_x = 0; _multiplied_no_offsets_s1_x_x < 0 + _594; _multiplied_no_offsets_s1_x_x++)
      {
       int32_t _4199 = _multiplied_no_offsets_s1_x_x * 32;
       bool _4200 = _4199 < _555;
       bool _4201 = _4199 < _554;
       int32_t _4202 = ::halide_cpp_min(_4196, 8);
       int32_t _4203 = _multiplied_no_offsets_s1_x_x * 8;
       int32_t _4204 = _multiplied_no_offsets_s1_x_x * 2;
       int32_t _4205 = _548 - _4204;
       int32_t _4206 = _600 + _4199;
       int32_t _4207 = _601 + _4199;
       int32_t _4208 = _602 + _4199;
       int32_t _4209 = _603 + _4199;
       for (int _multiplied_no_offsets_s1_k__x_k__x = 0; _multiplied_no_offsets_s1_k__x_k__x < 0 + _596; _multiplied_no_offsets_s1_k__x_k__x++)
       {
        int32_t _4210 = _multiplied_no_offsets_s1_k__x_k__x * 32;
        int32_t _4211 = _556 - _4210;
        int32_t _4212 = ::halide_cpp_min(_4205, 2);
        int32_t _4213 = _4212 * 16;
        int32_t _4214 = ::halide_cpp_max(_4213, 24);
        int32_t _4215 = _4214 + -24;
        int32_t _4216 = _4205 * 16;
        int32_t _4217 = ::halide_cpp_min(_4216, 24);
        int32_t _4218 = _4217 + -16;
        for (int _multiplied_no_offsets_s1_y_yi_yi = 0; _multiplied_no_offsets_s1_y_yi_yi < 0 + _4202; _multiplied_no_offsets_s1_y_yi_yi++)
        {
         int32_t _4219 = _multiplied_no_offsets_s1_y_yi_yi + _4195;
         int32_t _4220 = _4219 * 4;
         int32_t _4221 = _4220 + 1;
         int32_t _4222 = _4221 * _590;
         int32_t _4223 = _4220 + 2;
         int32_t _4224 = _4223 * _590;
         int32_t _4225 = _4220 + 3;
         int32_t _4226 = _4225 * _590;
         int32_t _4227 = _590 * _4219;
         int32_t _4228 = _4227 * 4;
         int32_t _4229 = ::halide_cpp_min(_4211, 32);
         int32_t _4230 = _4206 + _4228;
         int32_t _4231 = _4207 + _4228;
         int32_t _4232 = _4208 + _4228;
         int32_t _4233 = _4209 + _4228;
         int32_t _4234 = _4206 + _4226;
         int32_t _4235 = _4207 + _4226;
         int32_t _4236 = _4208 + _4226;
         int32_t _4237 = _4209 + _4226;
         int32_t _4238 = _4206 + _4224;
         int32_t _4239 = _4207 + _4224;
         int32_t _4240 = _4208 + _4224;
         int32_t _4241 = _4209 + _4224;
         int32_t _4242 = _4206 + _4222;
         int32_t _4243 = _4207 + _4222;
         int32_t _4244 = _4208 + _4222;
         int32_t _4245 = _4209 + _4222;
         for (int _multiplied_no_offsets_s1_k__x_rki = 0; _multiplied_no_offsets_s1_k__x_rki < 0 + _4229; _multiplied_no_offsets_s1_k__x_rki++)
         {
          int32_t _4246 = _590 * _4219;
          int32_t _4247 = _4246 * 4;
          int32_t _4248 = _4247 + _4209;
          uint32x8_t _4249 = uint32x8_t::load(_multiplied_no_offsets, _4248);
          int32_t _4250 = _multiplied_no_offsets_s1_k__x_rki + _4210;
          int32_t _4251 = _4250 * _40;
          int32_t _4252 = _4251 + _4203;
          int32_t _4253 = _4252 * 4;
          uint8x8_t _4254 = uint8x8_t::load(_mat_b, _4253);
          uint16x8_t _4255 = uint16x8_t::convert_from<uint8x8_t>(_4254);
          int32_t _4256 = _31 * _4219;
          int32_t _4257 = _4256 + _4210;
          int32_t _4258 = _4257 + _multiplied_no_offsets_s1_k__x_rki;
          int32_t _4259 = _4258 * 4;
          uint8_t _4260 = ((const uint8_t *)_mat_a)[_4259];
          uint16_t _4261 = (uint16_t)(_4260);
          uint16x8_t _4262 = uint16x8_t::broadcast(_4261);
          uint16x8_t _4263 = _4255 * _4262;
          uint32x8_t _4264 = uint32x8_t::convert_from<uint16x8_t>(_4263);
          uint32x8_t _4265 = _4249 + _4264;
          int32_t _4266 = _4253 + _40;
          uint8x8_t _4267 = uint8x8_t::load(_mat_b, _4266);
          uint16x8_t _4268 = uint16x8_t::convert_from<uint8x8_t>(_4267);
          int32_t _4269 = _4259 + 1;
          uint8_t _4270 = ((const uint8_t *)_mat_a)[_4269];
          uint16_t _4271 = (uint16_t)(_4270);
          uint16x8_t _4272 = uint16x8_t::broadcast(_4271);
          uint16x8_t _4273 = _4268 * _4272;
          uint32x8_t _4274 = uint32x8_t::convert_from<uint16x8_t>(_4273);
          uint32x8_t _4275 = _4265 + _4274;
          int32_t _4276 = _4252 * 2;
          int32_t _4277 = _4276 + _40;
          int32_t _4278 = _4277 * 2;
          uint8x8_t _4279 = uint8x8_t::load(_mat_b, _4278);
          uint16x8_t _4280 = uint16x8_t::convert_from<uint8x8_t>(_4279);
          int32_t _4281 = _4259 + 2;
          uint8_t _4282 = ((const uint8_t *)_mat_a)[_4281];
          uint16_t _4283 = (uint16_t)(_4282);
          uint16x8_t _4284 = uint16x8_t::broadcast(_4283);
          uint16x8_t _4285 = _4280 * _4284;
          uint32x8_t _4286 = uint32x8_t::convert_from<uint16x8_t>(_4285);
          uint32x8_t _4287 = _4275 + _4286;
          int32_t _4288 = _4253 + _553;
          uint8x8_t _4289 = uint8x8_t::load(_mat_b, _4288);
          uint16x8_t _4290 = uint16x8_t::convert_from<uint8x8_t>(_4289);
          int32_t _4291 = _4259 + 3;
          uint8_t _4292 = ((const uint8_t *)_mat_a)[_4291];
          uint16_t _4293 = (uint16_t)(_4292);
          uint16x8_t _4294 = uint16x8_t::broadcast(_4293);
          uint16x8_t _4295 = _4290 * _4294;
          uint32x8_t _4296 = uint32x8_t::convert_from<uint16x8_t>(_4295);
          uint32x8_t _4297 = _4287 + _4296;
          _4297.store(_multiplied_no_offsets, _4233);
          int32_t _4298 = _4219 * 4;
          int32_t _4299 = _4298 + 1;
          int32_t _4300 = _4299 * _31;
          int32_t _4301 = _multiplied_no_offsets_s1_k__x_rki + _4210;
          int32_t _4302 = _4301 * 4;
          int32_t _4303 = _4300 + _4302;
          int32_t _4304 = _4299 * _590;
          int32_t _4305 = _4304 + _4209;
          uint32x8_t _4306 = uint32x8_t::load(_multiplied_no_offsets, _4305);
          int32_t _4307 = _4301 * _40;
          int32_t _4308 = _4307 + _4203;
          int32_t _4309 = _4308 * 4;
          uint8x8_t _4310 = uint8x8_t::load(_mat_b, _4309);
          uint16x8_t _4311 = uint16x8_t::convert_from<uint8x8_t>(_4310);
          uint8_t _4312 = ((const uint8_t *)_mat_a)[_4303];
          uint16_t _4313 = (uint16_t)(_4312);
          uint16x8_t _4314 = uint16x8_t::broadcast(_4313);
          uint16x8_t _4315 = _4311 * _4314;
          uint32x8_t _4316 = uint32x8_t::convert_from<uint16x8_t>(_4315);
          uint32x8_t _4317 = _4306 + _4316;
          int32_t _4318 = _4309 + _40;
          uint8x8_t _4319 = uint8x8_t::load(_mat_b, _4318);
          uint16x8_t _4320 = uint16x8_t::convert_from<uint8x8_t>(_4319);
          int32_t _4321 = _4303 + 1;
          uint8_t _4322 = ((const uint8_t *)_mat_a)[_4321];
          uint16_t _4323 = (uint16_t)(_4322);
          uint16x8_t _4324 = uint16x8_t::broadcast(_4323);
          uint16x8_t _4325 = _4320 * _4324;
          uint32x8_t _4326 = uint32x8_t::convert_from<uint16x8_t>(_4325);
          uint32x8_t _4327 = _4317 + _4326;
          int32_t _4328 = _4308 * 2;
          int32_t _4329 = _4328 + _40;
          int32_t _4330 = _4329 * 2;
          uint8x8_t _4331 = uint8x8_t::load(_mat_b, _4330);
          uint16x8_t _4332 = uint16x8_t::convert_from<uint8x8_t>(_4331);
          int32_t _4333 = _4303 + 2;
          uint8_t _4334 = ((const uint8_t *)_mat_a)[_4333];
          uint16_t _4335 = (uint16_t)(_4334);
          uint16x8_t _4336 = uint16x8_t::broadcast(_4335);
          uint16x8_t _4337 = _4332 * _4336;
          uint32x8_t _4338 = uint32x8_t::convert_from<uint16x8_t>(_4337);
          uint32x8_t _4339 = _4327 + _4338;
          int32_t _4340 = _4309 + _553;
          uint8x8_t _4341 = uint8x8_t::load(_mat_b, _4340);
          uint16x8_t _4342 = uint16x8_t::convert_from<uint8x8_t>(_4341);
          int32_t _4343 = _4303 + 3;
          uint8_t _4344 = ((const uint8_t *)_mat_a)[_4343];
          uint16_t _4345 = (uint16_t)(_4344);
          uint16x8_t _4346 = uint16x8_t::broadcast(_4345);
          uint16x8_t _4347 = _4342 * _4346;
          uint32x8_t _4348 = uint32x8_t::convert_from<uint16x8_t>(_4347);
          uint32x8_t _4349 = _4339 + _4348;
          _4349.store(_multiplied_no_offsets, _4245);
          int32_t _4350 = _4219 * 4;
          int32_t _4351 = _4350 + 2;
          int32_t _4352 = _4351 * _31;
          int32_t _4353 = _multiplied_no_offsets_s1_k__x_rki + _4210;
          int32_t _4354 = _4353 * 4;
          int32_t _4355 = _4352 + _4354;
          int32_t _4356 = _4351 * _590;
          int32_t _4357 = _4356 + _4209;
          uint32x8_t _4358 = uint32x8_t::load(_multiplied_no_offsets, _4357);
          int32_t _4359 = _4353 * _40;
          int32_t _4360 = _4359 + _4203;
          int32_t _4361 = _4360 * 4;
          uint8x8_t _4362 = uint8x8_t::load(_mat_b, _4361);
          uint16x8_t _4363 = uint16x8_t::convert_from<uint8x8_t>(_4362);
          uint8_t _4364 = ((const uint8_t *)_mat_a)[_4355];
          uint16_t _4365 = (uint16_t)(_4364);
          uint16x8_t _4366 = uint16x8_t::broadcast(_4365);
          uint16x8_t _4367 = _4363 * _4366;
          uint32x8_t _4368 = uint32x8_t::convert_from<uint16x8_t>(_4367);
          uint32x8_t _4369 = _4358 + _4368;
          int32_t _4370 = _4361 + _40;
          uint8x8_t _4371 = uint8x8_t::load(_mat_b, _4370);
          uint16x8_t _4372 = uint16x8_t::convert_from<uint8x8_t>(_4371);
          int32_t _4373 = _4355 + 1;
          uint8_t _4374 = ((const uint8_t *)_mat_a)[_4373];
          uint16_t _4375 = (uint16_t)(_4374);
          uint16x8_t _4376 = uint16x8_t::broadcast(_4375);
          uint16x8_t _4377 = _4372 * _4376;
          uint32x8_t _4378 = uint32x8_t::convert_from<uint16x8_t>(_4377);
          uint32x8_t _4379 = _4369 + _4378;
          int32_t _4380 = _4360 * 2;
          int32_t _4381 = _4380 + _40;
          int32_t _4382 = _4381 * 2;
          uint8x8_t _4383 = uint8x8_t::load(_mat_b, _4382);
          uint16x8_t _4384 = uint16x8_t::convert_from<uint8x8_t>(_4383);
          int32_t _4385 = _4355 + 2;
          uint8_t _4386 = ((const uint8_t *)_mat_a)[_4385];
          uint16_t _4387 = (uint16_t)(_4386);
          uint16x8_t _4388 = uint16x8_t::broadcast(_4387);
          uint16x8_t _4389 = _4384 * _4388;
          uint32x8_t _4390 = uint32x8_t::convert_from<uint16x8_t>(_4389);
          uint32x8_t _4391 = _4379 + _4390;
          int32_t _4392 = _4361 + _553;
          uint8x8_t _4393 = uint8x8_t::load(_mat_b, _4392);
          uint16x8_t _4394 = uint16x8_t::convert_from<uint8x8_t>(_4393);
          int32_t _4395 = _4355 + 3;
          uint8_t _4396 = ((const uint8_t *)_mat_a)[_4395];
          uint16_t _4397 = (uint16_t)(_4396);
          uint16x8_t _4398 = uint16x8_t::broadcast(_4397);
          uint16x8_t _4399 = _4394 * _4398;
          uint32x8_t _4400 = uint32x8_t::convert_from<uint16x8_t>(_4399);
          uint32x8_t _4401 = _4391 + _4400;
          _4401.store(_multiplied_no_offsets, _4241);
          int32_t _4402 = _4219 * 4;
          int32_t _4403 = _4402 + 3;
          int32_t _4404 = _4403 * _31;
          int32_t _4405 = _multiplied_no_offsets_s1_k__x_rki + _4210;
          int32_t _4406 = _4405 * 4;
          int32_t _4407 = _4404 + _4406;
          int32_t _4408 = _4403 * _590;
          int32_t _4409 = _4408 + _4209;
          uint32x8_t _4410 = uint32x8_t::load(_multiplied_no_offsets, _4409);
          int32_t _4411 = _4405 * _40;
          int32_t _4412 = _4411 + _4203;
          int32_t _4413 = _4412 * 4;
          uint8x8_t _4414 = uint8x8_t::load(_mat_b, _4413);
          uint16x8_t _4415 = uint16x8_t::convert_from<uint8x8_t>(_4414);
          uint8_t _4416 = ((const uint8_t *)_mat_a)[_4407];
          uint16_t _4417 = (uint16_t)(_4416);
          uint16x8_t _4418 = uint16x8_t::broadcast(_4417);
          uint16x8_t _4419 = _4415 * _4418;
          uint32x8_t _4420 = uint32x8_t::convert_from<uint16x8_t>(_4419);
          uint32x8_t _4421 = _4410 + _4420;
          int32_t _4422 = _4413 + _40;
          uint8x8_t _4423 = uint8x8_t::load(_mat_b, _4422);
          uint16x8_t _4424 = uint16x8_t::convert_from<uint8x8_t>(_4423);
          int32_t _4425 = _4407 + 1;
          uint8_t _4426 = ((const uint8_t *)_mat_a)[_4425];
          uint16_t _4427 = (uint16_t)(_4426);
          uint16x8_t _4428 = uint16x8_t::broadcast(_4427);
          uint16x8_t _4429 = _4424 * _4428;
          uint32x8_t _4430 = uint32x8_t::convert_from<uint16x8_t>(_4429);
          uint32x8_t _4431 = _4421 + _4430;
          int32_t _4432 = _4412 * 2;
          int32_t _4433 = _4432 + _40;
          int32_t _4434 = _4433 * 2;
          uint8x8_t _4435 = uint8x8_t::load(_mat_b, _4434);
          uint16x8_t _4436 = uint16x8_t::convert_from<uint8x8_t>(_4435);
          int32_t _4437 = _4407 + 2;
          uint8_t _4438 = ((const uint8_t *)_mat_a)[_4437];
          uint16_t _4439 = (uint16_t)(_4438);
          uint16x8_t _4440 = uint16x8_t::broadcast(_4439);
          uint16x8_t _4441 = _4436 * _4440;
          uint32x8_t _4442 = uint32x8_t::convert_from<uint16x8_t>(_4441);
          uint32x8_t _4443 = _4431 + _4442;
          int32_t _4444 = _4413 + _553;
          uint8x8_t _4445 = uint8x8_t::load(_mat_b, _4444);
          uint16x8_t _4446 = uint16x8_t::convert_from<uint8x8_t>(_4445);
          int32_t _4447 = _4407 + 3;
          uint8_t _4448 = ((const uint8_t *)_mat_a)[_4447];
          uint16_t _4449 = (uint16_t)(_4448);
          uint16x8_t _4450 = uint16x8_t::broadcast(_4449);
          uint16x8_t _4451 = _4446 * _4450;
          uint32x8_t _4452 = uint32x8_t::convert_from<uint16x8_t>(_4451);
          uint32x8_t _4453 = _4443 + _4452;
          _4453.store(_multiplied_no_offsets, _4237);
          int32_t _4454 = _590 * _4219;
          int32_t _4455 = _4454 * 4;
          int32_t _4456 = _4455 + _4208;
          uint32x8_t _4457 = uint32x8_t::load(_multiplied_no_offsets, _4456);
          int32_t _4458 = _multiplied_no_offsets_s1_k__x_rki + _4210;
          int32_t _4459 = _4458 * _40;
          int32_t _4460 = _4459 + _4203;
          int32_t _4461 = _4460 * 4;
          int32_t _4462 = _4461 + 8;
          uint8x8_t _4463 = uint8x8_t::load(_mat_b, _4462);
          uint16x8_t _4464 = uint16x8_t::convert_from<uint8x8_t>(_4463);
          int32_t _4465 = _31 * _4219;
          int32_t _4466 = _4465 + _4210;
          int32_t _4467 = _4466 + _multiplied_no_offsets_s1_k__x_rki;
          int32_t _4468 = _4467 * 4;
          uint8_t _4469 = ((const uint8_t *)_mat_a)[_4468];
          uint16_t _4470 = (uint16_t)(_4469);
          uint16x8_t _4471 = uint16x8_t::broadcast(_4470);
          uint16x8_t _4472 = _4464 * _4471;
          uint32x8_t _4473 = uint32x8_t::convert_from<uint16x8_t>(_4472);
          uint32x8_t _4474 = _4457 + _4473;
          int32_t _4475 = _4461 + _40;
          int32_t _4476 = _4475 + 8;
          uint8x8_t _4477 = uint8x8_t::load(_mat_b, _4476);
          uint16x8_t _4478 = uint16x8_t::convert_from<uint8x8_t>(_4477);
          int32_t _4479 = _4468 + 1;
          uint8_t _4480 = ((const uint8_t *)_mat_a)[_4479];
          uint16_t _4481 = (uint16_t)(_4480);
          uint16x8_t _4482 = uint16x8_t::broadcast(_4481);
          uint16x8_t _4483 = _4478 * _4482;
          uint32x8_t _4484 = uint32x8_t::convert_from<uint16x8_t>(_4483);
          uint32x8_t _4485 = _4474 + _4484;
          int32_t _4486 = _4460 * 2;
          int32_t _4487 = _4486 + _40;
          int32_t _4488 = _4487 * 2;
          int32_t _4489 = _4488 + 8;
          uint8x8_t _4490 = uint8x8_t::load(_mat_b, _4489);
          uint16x8_t _4491 = uint16x8_t::convert_from<uint8x8_t>(_4490);
          int32_t _4492 = _4468 + 2;
          uint8_t _4493 = ((const uint8_t *)_mat_a)[_4492];
          uint16_t _4494 = (uint16_t)(_4493);
          uint16x8_t _4495 = uint16x8_t::broadcast(_4494);
          uint16x8_t _4496 = _4491 * _4495;
          uint32x8_t _4497 = uint32x8_t::convert_from<uint16x8_t>(_4496);
          uint32x8_t _4498 = _4485 + _4497;
          int32_t _4499 = _4461 + _553;
          int32_t _4500 = _4499 + 8;
          uint8x8_t _4501 = uint8x8_t::load(_mat_b, _4500);
          uint16x8_t _4502 = uint16x8_t::convert_from<uint8x8_t>(_4501);
          int32_t _4503 = _4468 + 3;
          uint8_t _4504 = ((const uint8_t *)_mat_a)[_4503];
          uint16_t _4505 = (uint16_t)(_4504);
          uint16x8_t _4506 = uint16x8_t::broadcast(_4505);
          uint16x8_t _4507 = _4502 * _4506;
          uint32x8_t _4508 = uint32x8_t::convert_from<uint16x8_t>(_4507);
          uint32x8_t _4509 = _4498 + _4508;
          _4509.store(_multiplied_no_offsets, _4232);
          int32_t _4510 = _4219 * 4;
          int32_t _4511 = _4510 + 1;
          int32_t _4512 = _4511 * _31;
          int32_t _4513 = _multiplied_no_offsets_s1_k__x_rki + _4210;
          int32_t _4514 = _4513 * 4;
          int32_t _4515 = _4512 + _4514;
          int32_t _4516 = _4511 * _590;
          int32_t _4517 = _4516 + _4208;
          uint32x8_t _4518 = uint32x8_t::load(_multiplied_no_offsets, _4517);
          int32_t _4519 = _4513 * _40;
          int32_t _4520 = _4519 + _4203;
          int32_t _4521 = _4520 * 4;
          int32_t _4522 = _4521 + 8;
          uint8x8_t _4523 = uint8x8_t::load(_mat_b, _4522);
          uint16x8_t _4524 = uint16x8_t::convert_from<uint8x8_t>(_4523);
          uint8_t _4525 = ((const uint8_t *)_mat_a)[_4515];
          uint16_t _4526 = (uint16_t)(_4525);
          uint16x8_t _4527 = uint16x8_t::broadcast(_4526);
          uint16x8_t _4528 = _4524 * _4527;
          uint32x8_t _4529 = uint32x8_t::convert_from<uint16x8_t>(_4528);
          uint32x8_t _4530 = _4518 + _4529;
          int32_t _4531 = _4521 + _40;
          int32_t _4532 = _4531 + 8;
          uint8x8_t _4533 = uint8x8_t::load(_mat_b, _4532);
          uint16x8_t _4534 = uint16x8_t::convert_from<uint8x8_t>(_4533);
          int32_t _4535 = _4515 + 1;
          uint8_t _4536 = ((const uint8_t *)_mat_a)[_4535];
          uint16_t _4537 = (uint16_t)(_4536);
          uint16x8_t _4538 = uint16x8_t::broadcast(_4537);
          uint16x8_t _4539 = _4534 * _4538;
          uint32x8_t _4540 = uint32x8_t::convert_from<uint16x8_t>(_4539);
          uint32x8_t _4541 = _4530 + _4540;
          int32_t _4542 = _4520 * 2;
          int32_t _4543 = _4542 + _40;
          int32_t _4544 = _4543 * 2;
          int32_t _4545 = _4544 + 8;
          uint8x8_t _4546 = uint8x8_t::load(_mat_b, _4545);
          uint16x8_t _4547 = uint16x8_t::convert_from<uint8x8_t>(_4546);
          int32_t _4548 = _4515 + 2;
          uint8_t _4549 = ((const uint8_t *)_mat_a)[_4548];
          uint16_t _4550 = (uint16_t)(_4549);
          uint16x8_t _4551 = uint16x8_t::broadcast(_4550);
          uint16x8_t _4552 = _4547 * _4551;
          uint32x8_t _4553 = uint32x8_t::convert_from<uint16x8_t>(_4552);
          uint32x8_t _4554 = _4541 + _4553;
          int32_t _4555 = _4521 + _553;
          int32_t _4556 = _4555 + 8;
          uint8x8_t _4557 = uint8x8_t::load(_mat_b, _4556);
          uint16x8_t _4558 = uint16x8_t::convert_from<uint8x8_t>(_4557);
          int32_t _4559 = _4515 + 3;
          uint8_t _4560 = ((const uint8_t *)_mat_a)[_4559];
          uint16_t _4561 = (uint16_t)(_4560);
          uint16x8_t _4562 = uint16x8_t::broadcast(_4561);
          uint16x8_t _4563 = _4558 * _4562;
          uint32x8_t _4564 = uint32x8_t::convert_from<uint16x8_t>(_4563);
          uint32x8_t _4565 = _4554 + _4564;
          _4565.store(_multiplied_no_offsets, _4244);
          int32_t _4566 = _4219 * 4;
          int32_t _4567 = _4566 + 2;
          int32_t _4568 = _4567 * _31;
          int32_t _4569 = _multiplied_no_offsets_s1_k__x_rki + _4210;
          int32_t _4570 = _4569 * 4;
          int32_t _4571 = _4568 + _4570;
          int32_t _4572 = _4567 * _590;
          int32_t _4573 = _4572 + _4208;
          uint32x8_t _4574 = uint32x8_t::load(_multiplied_no_offsets, _4573);
          int32_t _4575 = _4569 * _40;
          int32_t _4576 = _4575 + _4203;
          int32_t _4577 = _4576 * 4;
          int32_t _4578 = _4577 + 8;
          uint8x8_t _4579 = uint8x8_t::load(_mat_b, _4578);
          uint16x8_t _4580 = uint16x8_t::convert_from<uint8x8_t>(_4579);
          uint8_t _4581 = ((const uint8_t *)_mat_a)[_4571];
          uint16_t _4582 = (uint16_t)(_4581);
          uint16x8_t _4583 = uint16x8_t::broadcast(_4582);
          uint16x8_t _4584 = _4580 * _4583;
          uint32x8_t _4585 = uint32x8_t::convert_from<uint16x8_t>(_4584);
          uint32x8_t _4586 = _4574 + _4585;
          int32_t _4587 = _4577 + _40;
          int32_t _4588 = _4587 + 8;
          uint8x8_t _4589 = uint8x8_t::load(_mat_b, _4588);
          uint16x8_t _4590 = uint16x8_t::convert_from<uint8x8_t>(_4589);
          int32_t _4591 = _4571 + 1;
          uint8_t _4592 = ((const uint8_t *)_mat_a)[_4591];
          uint16_t _4593 = (uint16_t)(_4592);
          uint16x8_t _4594 = uint16x8_t::broadcast(_4593);
          uint16x8_t _4595 = _4590 * _4594;
          uint32x8_t _4596 = uint32x8_t::convert_from<uint16x8_t>(_4595);
          uint32x8_t _4597 = _4586 + _4596;
          int32_t _4598 = _4576 * 2;
          int32_t _4599 = _4598 + _40;
          int32_t _4600 = _4599 * 2;
          int32_t _4601 = _4600 + 8;
          uint8x8_t _4602 = uint8x8_t::load(_mat_b, _4601);
          uint16x8_t _4603 = uint16x8_t::convert_from<uint8x8_t>(_4602);
          int32_t _4604 = _4571 + 2;
          uint8_t _4605 = ((const uint8_t *)_mat_a)[_4604];
          uint16_t _4606 = (uint16_t)(_4605);
          uint16x8_t _4607 = uint16x8_t::broadcast(_4606);
          uint16x8_t _4608 = _4603 * _4607;
          uint32x8_t _4609 = uint32x8_t::convert_from<uint16x8_t>(_4608);
          uint32x8_t _4610 = _4597 + _4609;
          int32_t _4611 = _4577 + _553;
          int32_t _4612 = _4611 + 8;
          uint8x8_t _4613 = uint8x8_t::load(_mat_b, _4612);
          uint16x8_t _4614 = uint16x8_t::convert_from<uint8x8_t>(_4613);
          int32_t _4615 = _4571 + 3;
          uint8_t _4616 = ((const uint8_t *)_mat_a)[_4615];
          uint16_t _4617 = (uint16_t)(_4616);
          uint16x8_t _4618 = uint16x8_t::broadcast(_4617);
          uint16x8_t _4619 = _4614 * _4618;
          uint32x8_t _4620 = uint32x8_t::convert_from<uint16x8_t>(_4619);
          uint32x8_t _4621 = _4610 + _4620;
          _4621.store(_multiplied_no_offsets, _4240);
          int32_t _4622 = _4219 * 4;
          int32_t _4623 = _4622 + 3;
          int32_t _4624 = _4623 * _31;
          int32_t _4625 = _multiplied_no_offsets_s1_k__x_rki + _4210;
          int32_t _4626 = _4625 * 4;
          int32_t _4627 = _4624 + _4626;
          int32_t _4628 = _4623 * _590;
          int32_t _4629 = _4628 + _4208;
          uint32x8_t _4630 = uint32x8_t::load(_multiplied_no_offsets, _4629);
          int32_t _4631 = _4625 * _40;
          int32_t _4632 = _4631 + _4203;
          int32_t _4633 = _4632 * 4;
          int32_t _4634 = _4633 + 8;
          uint8x8_t _4635 = uint8x8_t::load(_mat_b, _4634);
          uint16x8_t _4636 = uint16x8_t::convert_from<uint8x8_t>(_4635);
          uint8_t _4637 = ((const uint8_t *)_mat_a)[_4627];
          uint16_t _4638 = (uint16_t)(_4637);
          uint16x8_t _4639 = uint16x8_t::broadcast(_4638);
          uint16x8_t _4640 = _4636 * _4639;
          uint32x8_t _4641 = uint32x8_t::convert_from<uint16x8_t>(_4640);
          uint32x8_t _4642 = _4630 + _4641;
          int32_t _4643 = _4633 + _40;
          int32_t _4644 = _4643 + 8;
          uint8x8_t _4645 = uint8x8_t::load(_mat_b, _4644);
          uint16x8_t _4646 = uint16x8_t::convert_from<uint8x8_t>(_4645);
          int32_t _4647 = _4627 + 1;
          uint8_t _4648 = ((const uint8_t *)_mat_a)[_4647];
          uint16_t _4649 = (uint16_t)(_4648);
          uint16x8_t _4650 = uint16x8_t::broadcast(_4649);
          uint16x8_t _4651 = _4646 * _4650;
          uint32x8_t _4652 = uint32x8_t::convert_from<uint16x8_t>(_4651);
          uint32x8_t _4653 = _4642 + _4652;
          int32_t _4654 = _4632 * 2;
          int32_t _4655 = _4654 + _40;
          int32_t _4656 = _4655 * 2;
          int32_t _4657 = _4656 + 8;
          uint8x8_t _4658 = uint8x8_t::load(_mat_b, _4657);
          uint16x8_t _4659 = uint16x8_t::convert_from<uint8x8_t>(_4658);
          int32_t _4660 = _4627 + 2;
          uint8_t _4661 = ((const uint8_t *)_mat_a)[_4660];
          uint16_t _4662 = (uint16_t)(_4661);
          uint16x8_t _4663 = uint16x8_t::broadcast(_4662);
          uint16x8_t _4664 = _4659 * _4663;
          uint32x8_t _4665 = uint32x8_t::convert_from<uint16x8_t>(_4664);
          uint32x8_t _4666 = _4653 + _4665;
          int32_t _4667 = _4633 + _553;
          int32_t _4668 = _4667 + 8;
          uint8x8_t _4669 = uint8x8_t::load(_mat_b, _4668);
          uint16x8_t _4670 = uint16x8_t::convert_from<uint8x8_t>(_4669);
          int32_t _4671 = _4627 + 3;
          uint8_t _4672 = ((const uint8_t *)_mat_a)[_4671];
          uint16_t _4673 = (uint16_t)(_4672);
          uint16x8_t _4674 = uint16x8_t::broadcast(_4673);
          uint16x8_t _4675 = _4670 * _4674;
          uint32x8_t _4676 = uint32x8_t::convert_from<uint16x8_t>(_4675);
          uint32x8_t _4677 = _4666 + _4676;
          _4677.store(_multiplied_no_offsets, _4236);
          if (_4200)
          {
           int32_t _4678 = _590 * _4219;
           int32_t _4679 = _4678 * 4;
           int32_t _4680 = _4679 + _4207;
           uint32x8_t _4681 = uint32x8_t::load(_multiplied_no_offsets, _4680);
           int32_t _4682 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           int32_t _4683 = _4682 * _40;
           int32_t _4684 = _4683 + _4203;
           int32_t _4685 = _4684 * 4;
           int32_t _4686 = _4685 + 16;
           uint8x8_t _4687 = uint8x8_t::load(_mat_b, _4686);
           uint16x8_t _4688 = uint16x8_t::convert_from<uint8x8_t>(_4687);
           int32_t _4689 = _31 * _4219;
           int32_t _4690 = _4689 + _4210;
           int32_t _4691 = _4690 + _multiplied_no_offsets_s1_k__x_rki;
           int32_t _4692 = _4691 * 4;
           uint8_t _4693 = ((const uint8_t *)_mat_a)[_4692];
           uint16_t _4694 = (uint16_t)(_4693);
           uint16x8_t _4695 = uint16x8_t::broadcast(_4694);
           uint16x8_t _4696 = _4688 * _4695;
           uint32x8_t _4697 = uint32x8_t::convert_from<uint16x8_t>(_4696);
           uint32x8_t _4698 = _4681 + _4697;
           int32_t _4699 = _4685 + _40;
           int32_t _4700 = _4699 + 16;
           uint8x8_t _4701 = uint8x8_t::load(_mat_b, _4700);
           uint16x8_t _4702 = uint16x8_t::convert_from<uint8x8_t>(_4701);
           int32_t _4703 = _4692 + 1;
           uint8_t _4704 = ((const uint8_t *)_mat_a)[_4703];
           uint16_t _4705 = (uint16_t)(_4704);
           uint16x8_t _4706 = uint16x8_t::broadcast(_4705);
           uint16x8_t _4707 = _4702 * _4706;
           uint32x8_t _4708 = uint32x8_t::convert_from<uint16x8_t>(_4707);
           uint32x8_t _4709 = _4698 + _4708;
           int32_t _4710 = _4684 * 2;
           int32_t _4711 = _4710 + _40;
           int32_t _4712 = _4711 * 2;
           int32_t _4713 = _4712 + 16;
           uint8x8_t _4714 = uint8x8_t::load(_mat_b, _4713);
           uint16x8_t _4715 = uint16x8_t::convert_from<uint8x8_t>(_4714);
           int32_t _4716 = _4692 + 2;
           uint8_t _4717 = ((const uint8_t *)_mat_a)[_4716];
           uint16_t _4718 = (uint16_t)(_4717);
           uint16x8_t _4719 = uint16x8_t::broadcast(_4718);
           uint16x8_t _4720 = _4715 * _4719;
           uint32x8_t _4721 = uint32x8_t::convert_from<uint16x8_t>(_4720);
           uint32x8_t _4722 = _4709 + _4721;
           int32_t _4723 = _4685 + _553;
           int32_t _4724 = _4723 + 16;
           uint8x8_t _4725 = uint8x8_t::load(_mat_b, _4724);
           uint16x8_t _4726 = uint16x8_t::convert_from<uint8x8_t>(_4725);
           int32_t _4727 = _4692 + 3;
           uint8_t _4728 = ((const uint8_t *)_mat_a)[_4727];
           uint16_t _4729 = (uint16_t)(_4728);
           uint16x8_t _4730 = uint16x8_t::broadcast(_4729);
           uint16x8_t _4731 = _4726 * _4730;
           uint32x8_t _4732 = uint32x8_t::convert_from<uint16x8_t>(_4731);
           uint32x8_t _4733 = _4722 + _4732;
           _4733.store(_multiplied_no_offsets, _4231);
           int32_t _4734 = _4219 * 4;
           int32_t _4735 = _4734 + 1;
           int32_t _4736 = _4735 * _31;
           int32_t _4737 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           int32_t _4738 = _4737 * 4;
           int32_t _4739 = _4736 + _4738;
           int32_t _4740 = _4735 * _590;
           int32_t _4741 = _4740 + _4207;
           uint32x8_t _4742 = uint32x8_t::load(_multiplied_no_offsets, _4741);
           int32_t _4743 = _4737 * _40;
           int32_t _4744 = _4743 + _4203;
           int32_t _4745 = _4744 * 4;
           int32_t _4746 = _4745 + 16;
           uint8x8_t _4747 = uint8x8_t::load(_mat_b, _4746);
           uint16x8_t _4748 = uint16x8_t::convert_from<uint8x8_t>(_4747);
           uint8_t _4749 = ((const uint8_t *)_mat_a)[_4739];
           uint16_t _4750 = (uint16_t)(_4749);
           uint16x8_t _4751 = uint16x8_t::broadcast(_4750);
           uint16x8_t _4752 = _4748 * _4751;
           uint32x8_t _4753 = uint32x8_t::convert_from<uint16x8_t>(_4752);
           uint32x8_t _4754 = _4742 + _4753;
           int32_t _4755 = _4745 + _40;
           int32_t _4756 = _4755 + 16;
           uint8x8_t _4757 = uint8x8_t::load(_mat_b, _4756);
           uint16x8_t _4758 = uint16x8_t::convert_from<uint8x8_t>(_4757);
           int32_t _4759 = _4739 + 1;
           uint8_t _4760 = ((const uint8_t *)_mat_a)[_4759];
           uint16_t _4761 = (uint16_t)(_4760);
           uint16x8_t _4762 = uint16x8_t::broadcast(_4761);
           uint16x8_t _4763 = _4758 * _4762;
           uint32x8_t _4764 = uint32x8_t::convert_from<uint16x8_t>(_4763);
           uint32x8_t _4765 = _4754 + _4764;
           int32_t _4766 = _4744 * 2;
           int32_t _4767 = _4766 + _40;
           int32_t _4768 = _4767 * 2;
           int32_t _4769 = _4768 + 16;
           uint8x8_t _4770 = uint8x8_t::load(_mat_b, _4769);
           uint16x8_t _4771 = uint16x8_t::convert_from<uint8x8_t>(_4770);
           int32_t _4772 = _4739 + 2;
           uint8_t _4773 = ((const uint8_t *)_mat_a)[_4772];
           uint16_t _4774 = (uint16_t)(_4773);
           uint16x8_t _4775 = uint16x8_t::broadcast(_4774);
           uint16x8_t _4776 = _4771 * _4775;
           uint32x8_t _4777 = uint32x8_t::convert_from<uint16x8_t>(_4776);
           uint32x8_t _4778 = _4765 + _4777;
           int32_t _4779 = _4745 + _553;
           int32_t _4780 = _4779 + 16;
           uint8x8_t _4781 = uint8x8_t::load(_mat_b, _4780);
           uint16x8_t _4782 = uint16x8_t::convert_from<uint8x8_t>(_4781);
           int32_t _4783 = _4739 + 3;
           uint8_t _4784 = ((const uint8_t *)_mat_a)[_4783];
           uint16_t _4785 = (uint16_t)(_4784);
           uint16x8_t _4786 = uint16x8_t::broadcast(_4785);
           uint16x8_t _4787 = _4782 * _4786;
           uint32x8_t _4788 = uint32x8_t::convert_from<uint16x8_t>(_4787);
           uint32x8_t _4789 = _4778 + _4788;
           _4789.store(_multiplied_no_offsets, _4243);
           int32_t _4790 = _4219 * 4;
           int32_t _4791 = _4790 + 2;
           int32_t _4792 = _4791 * _31;
           int32_t _4793 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           int32_t _4794 = _4793 * 4;
           int32_t _4795 = _4792 + _4794;
           int32_t _4796 = _4791 * _590;
           int32_t _4797 = _4796 + _4207;
           uint32x8_t _4798 = uint32x8_t::load(_multiplied_no_offsets, _4797);
           int32_t _4799 = _4793 * _40;
           int32_t _4800 = _4799 + _4203;
           int32_t _4801 = _4800 * 4;
           int32_t _4802 = _4801 + 16;
           uint8x8_t _4803 = uint8x8_t::load(_mat_b, _4802);
           uint16x8_t _4804 = uint16x8_t::convert_from<uint8x8_t>(_4803);
           uint8_t _4805 = ((const uint8_t *)_mat_a)[_4795];
           uint16_t _4806 = (uint16_t)(_4805);
           uint16x8_t _4807 = uint16x8_t::broadcast(_4806);
           uint16x8_t _4808 = _4804 * _4807;
           uint32x8_t _4809 = uint32x8_t::convert_from<uint16x8_t>(_4808);
           uint32x8_t _4810 = _4798 + _4809;
           int32_t _4811 = _4801 + _40;
           int32_t _4812 = _4811 + 16;
           uint8x8_t _4813 = uint8x8_t::load(_mat_b, _4812);
           uint16x8_t _4814 = uint16x8_t::convert_from<uint8x8_t>(_4813);
           int32_t _4815 = _4795 + 1;
           uint8_t _4816 = ((const uint8_t *)_mat_a)[_4815];
           uint16_t _4817 = (uint16_t)(_4816);
           uint16x8_t _4818 = uint16x8_t::broadcast(_4817);
           uint16x8_t _4819 = _4814 * _4818;
           uint32x8_t _4820 = uint32x8_t::convert_from<uint16x8_t>(_4819);
           uint32x8_t _4821 = _4810 + _4820;
           int32_t _4822 = _4800 * 2;
           int32_t _4823 = _4822 + _40;
           int32_t _4824 = _4823 * 2;
           int32_t _4825 = _4824 + 16;
           uint8x8_t _4826 = uint8x8_t::load(_mat_b, _4825);
           uint16x8_t _4827 = uint16x8_t::convert_from<uint8x8_t>(_4826);
           int32_t _4828 = _4795 + 2;
           uint8_t _4829 = ((const uint8_t *)_mat_a)[_4828];
           uint16_t _4830 = (uint16_t)(_4829);
           uint16x8_t _4831 = uint16x8_t::broadcast(_4830);
           uint16x8_t _4832 = _4827 * _4831;
           uint32x8_t _4833 = uint32x8_t::convert_from<uint16x8_t>(_4832);
           uint32x8_t _4834 = _4821 + _4833;
           int32_t _4835 = _4801 + _553;
           int32_t _4836 = _4835 + 16;
           uint8x8_t _4837 = uint8x8_t::load(_mat_b, _4836);
           uint16x8_t _4838 = uint16x8_t::convert_from<uint8x8_t>(_4837);
           int32_t _4839 = _4795 + 3;
           uint8_t _4840 = ((const uint8_t *)_mat_a)[_4839];
           uint16_t _4841 = (uint16_t)(_4840);
           uint16x8_t _4842 = uint16x8_t::broadcast(_4841);
           uint16x8_t _4843 = _4838 * _4842;
           uint32x8_t _4844 = uint32x8_t::convert_from<uint16x8_t>(_4843);
           uint32x8_t _4845 = _4834 + _4844;
           _4845.store(_multiplied_no_offsets, _4239);
           int32_t _4846 = _4219 * 4;
           int32_t _4847 = _4846 + 3;
           int32_t _4848 = _4847 * _31;
           int32_t _4849 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           int32_t _4850 = _4849 * 4;
           int32_t _4851 = _4848 + _4850;
           int32_t _4852 = _4847 * _590;
           int32_t _4853 = _4852 + _4207;
           uint32x8_t _4854 = uint32x8_t::load(_multiplied_no_offsets, _4853);
           int32_t _4855 = _4849 * _40;
           int32_t _4856 = _4855 + _4203;
           int32_t _4857 = _4856 * 4;
           int32_t _4858 = _4857 + 16;
           uint8x8_t _4859 = uint8x8_t::load(_mat_b, _4858);
           uint16x8_t _4860 = uint16x8_t::convert_from<uint8x8_t>(_4859);
           uint8_t _4861 = ((const uint8_t *)_mat_a)[_4851];
           uint16_t _4862 = (uint16_t)(_4861);
           uint16x8_t _4863 = uint16x8_t::broadcast(_4862);
           uint16x8_t _4864 = _4860 * _4863;
           uint32x8_t _4865 = uint32x8_t::convert_from<uint16x8_t>(_4864);
           uint32x8_t _4866 = _4854 + _4865;
           int32_t _4867 = _4857 + _40;
           int32_t _4868 = _4867 + 16;
           uint8x8_t _4869 = uint8x8_t::load(_mat_b, _4868);
           uint16x8_t _4870 = uint16x8_t::convert_from<uint8x8_t>(_4869);
           int32_t _4871 = _4851 + 1;
           uint8_t _4872 = ((const uint8_t *)_mat_a)[_4871];
           uint16_t _4873 = (uint16_t)(_4872);
           uint16x8_t _4874 = uint16x8_t::broadcast(_4873);
           uint16x8_t _4875 = _4870 * _4874;
           uint32x8_t _4876 = uint32x8_t::convert_from<uint16x8_t>(_4875);
           uint32x8_t _4877 = _4866 + _4876;
           int32_t _4878 = _4856 * 2;
           int32_t _4879 = _4878 + _40;
           int32_t _4880 = _4879 * 2;
           int32_t _4881 = _4880 + 16;
           uint8x8_t _4882 = uint8x8_t::load(_mat_b, _4881);
           uint16x8_t _4883 = uint16x8_t::convert_from<uint8x8_t>(_4882);
           int32_t _4884 = _4851 + 2;
           uint8_t _4885 = ((const uint8_t *)_mat_a)[_4884];
           uint16_t _4886 = (uint16_t)(_4885);
           uint16x8_t _4887 = uint16x8_t::broadcast(_4886);
           uint16x8_t _4888 = _4883 * _4887;
           uint32x8_t _4889 = uint32x8_t::convert_from<uint16x8_t>(_4888);
           uint32x8_t _4890 = _4877 + _4889;
           int32_t _4891 = _4857 + _553;
           int32_t _4892 = _4891 + 16;
           uint8x8_t _4893 = uint8x8_t::load(_mat_b, _4892);
           uint16x8_t _4894 = uint16x8_t::convert_from<uint8x8_t>(_4893);
           int32_t _4895 = _4851 + 3;
           uint8_t _4896 = ((const uint8_t *)_mat_a)[_4895];
           uint16_t _4897 = (uint16_t)(_4896);
           uint16x8_t _4898 = uint16x8_t::broadcast(_4897);
           uint16x8_t _4899 = _4894 * _4898;
           uint32x8_t _4900 = uint32x8_t::convert_from<uint16x8_t>(_4899);
           uint32x8_t _4901 = _4890 + _4900;
           _4901.store(_multiplied_no_offsets, _4235);
          } // if _4200
          else
          {
           int32_t _4902 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           int32_t _4903 = _4902 * _40;
           int32_t _4904 = _4903 * 4;
           int32_t _4905 = _4904 + _4199;
           int32_t _4906 = _590 * _4219;
           int32_t _4907 = _4906 * 4;
           int32_t _4908 = _4907 + _4207;
           int32_t _4909 = _31 * _4219;
           int32_t _4910 = _4909 + _4210;
           int32_t _4911 = _4910 + _multiplied_no_offsets_s1_k__x_rki;
           int32_t _4912 = _553 + 16;
           int32_t _4913 = _40 + 16;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _4218; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _4914 = _multiplied_no_offsets_s1_x_xi_xii + _4908;
            uint32_t _4915 = _multiplied_no_offsets[_4914];
            int32_t _4916 = _4911 * 4;
            uint8_t _4917 = ((const uint8_t *)_mat_a)[_4916];
            uint16_t _4918 = (uint16_t)(_4917);
            int32_t _4919 = _multiplied_no_offsets_s1_x_xi_xii + _4905;
            int32_t _4920 = _4919 + 16;
            uint8_t _4921 = ((const uint8_t *)_mat_b)[_4920];
            uint16_t _4922 = (uint16_t)(_4921);
            uint16_t _4923 = _4918 * _4922;
            uint32_t _4924 = (uint32_t)(_4923);
            uint32_t _4925 = _4915 + _4924;
            int32_t _4926 = _4916 + 1;
            uint8_t _4927 = ((const uint8_t *)_mat_a)[_4926];
            uint16_t _4928 = (uint16_t)(_4927);
            int32_t _4929 = _4919 + _4913;
            uint8_t _4930 = ((const uint8_t *)_mat_b)[_4929];
            uint16_t _4931 = (uint16_t)(_4930);
            uint16_t _4932 = _4928 * _4931;
            uint32_t _4933 = (uint32_t)(_4932);
            uint32_t _4934 = _4925 + _4933;
            int32_t _4935 = _4916 + 2;
            uint8_t _4936 = ((const uint8_t *)_mat_a)[_4935];
            uint16_t _4937 = (uint16_t)(_4936);
            int32_t _4938 = _4919 + _4198;
            uint8_t _4939 = ((const uint8_t *)_mat_b)[_4938];
            uint16_t _4940 = (uint16_t)(_4939);
            uint16_t _4941 = _4937 * _4940;
            uint32_t _4942 = (uint32_t)(_4941);
            uint32_t _4943 = _4934 + _4942;
            int32_t _4944 = _4916 + 3;
            uint8_t _4945 = ((const uint8_t *)_mat_a)[_4944];
            uint16_t _4946 = (uint16_t)(_4945);
            int32_t _4947 = _4919 + _4912;
            uint8_t _4948 = ((const uint8_t *)_mat_b)[_4947];
            uint16_t _4949 = (uint16_t)(_4948);
            uint16_t _4950 = _4946 * _4949;
            uint32_t _4951 = (uint32_t)(_4950);
            uint32_t _4952 = _4943 + _4951;
            int32_t _4953 = _multiplied_no_offsets_s1_x_xi_xii + _4231;
            _multiplied_no_offsets[_4953] = _4952;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _4954 = _4219 * 4;
           int32_t _4955 = _4954 + 1;
           int32_t _4956 = _31 * _4955;
           int32_t _4957 = _590 * _4955;
           int32_t _4958 = _4957 + _4207;
           int32_t _4959 = _553 + 16;
           int32_t _4960 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _4218; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _4961 = _multiplied_no_offsets_s1_x_xi_xii + _4958;
            uint32_t _4962 = _multiplied_no_offsets[_4961];
            int32_t _4963 = _4960 * 4;
            int32_t _4964 = _4963 + _4956;
            uint8_t _4965 = ((const uint8_t *)_mat_a)[_4964];
            uint16_t _4966 = (uint16_t)(_4965);
            int32_t _4967 = _40 * _4960;
            int32_t _4968 = _4967 * 4;
            int32_t _4969 = _4968 + _4199;
            int32_t _4970 = _4969 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _4971 = _4970 + 16;
            uint8_t _4972 = ((const uint8_t *)_mat_b)[_4971];
            uint16_t _4973 = (uint16_t)(_4972);
            uint16_t _4974 = _4966 * _4973;
            uint32_t _4975 = (uint32_t)(_4974);
            uint32_t _4976 = _4962 + _4975;
            int32_t _4977 = _4964 + 1;
            uint8_t _4978 = ((const uint8_t *)_mat_a)[_4977];
            uint16_t _4979 = (uint16_t)(_4978);
            int32_t _4980 = _4970 + _40;
            int32_t _4981 = _4980 + 16;
            uint8_t _4982 = ((const uint8_t *)_mat_b)[_4981];
            uint16_t _4983 = (uint16_t)(_4982);
            uint16_t _4984 = _4979 * _4983;
            uint32_t _4985 = (uint32_t)(_4984);
            uint32_t _4986 = _4976 + _4985;
            int32_t _4987 = _4964 + 2;
            uint8_t _4988 = ((const uint8_t *)_mat_a)[_4987];
            uint16_t _4989 = (uint16_t)(_4988);
            int32_t _4990 = _4970 + _4198;
            uint8_t _4991 = ((const uint8_t *)_mat_b)[_4990];
            uint16_t _4992 = (uint16_t)(_4991);
            uint16_t _4993 = _4989 * _4992;
            uint32_t _4994 = (uint32_t)(_4993);
            uint32_t _4995 = _4986 + _4994;
            int32_t _4996 = _4964 + 3;
            uint8_t _4997 = ((const uint8_t *)_mat_a)[_4996];
            uint16_t _4998 = (uint16_t)(_4997);
            int32_t _4999 = _4970 + _4959;
            uint8_t _5000 = ((const uint8_t *)_mat_b)[_4999];
            uint16_t _5001 = (uint16_t)(_5000);
            uint16_t _5002 = _4998 * _5001;
            uint32_t _5003 = (uint32_t)(_5002);
            uint32_t _5004 = _4995 + _5003;
            int32_t _5005 = _multiplied_no_offsets_s1_x_xi_xii + _4243;
            _multiplied_no_offsets[_5005] = _5004;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _5006 = _4219 * 4;
           int32_t _5007 = _5006 + 2;
           int32_t _5008 = _31 * _5007;
           int32_t _5009 = _590 * _5007;
           int32_t _5010 = _5009 + _4207;
           int32_t _5011 = _553 + 16;
           int32_t _5012 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _4218; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _5013 = _multiplied_no_offsets_s1_x_xi_xii + _5010;
            uint32_t _5014 = _multiplied_no_offsets[_5013];
            int32_t _5015 = _5012 * 4;
            int32_t _5016 = _5015 + _5008;
            uint8_t _5017 = ((const uint8_t *)_mat_a)[_5016];
            uint16_t _5018 = (uint16_t)(_5017);
            int32_t _5019 = _40 * _5012;
            int32_t _5020 = _5019 * 4;
            int32_t _5021 = _5020 + _4199;
            int32_t _5022 = _5021 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _5023 = _5022 + 16;
            uint8_t _5024 = ((const uint8_t *)_mat_b)[_5023];
            uint16_t _5025 = (uint16_t)(_5024);
            uint16_t _5026 = _5018 * _5025;
            uint32_t _5027 = (uint32_t)(_5026);
            uint32_t _5028 = _5014 + _5027;
            int32_t _5029 = _5016 + 1;
            uint8_t _5030 = ((const uint8_t *)_mat_a)[_5029];
            uint16_t _5031 = (uint16_t)(_5030);
            int32_t _5032 = _5022 + _40;
            int32_t _5033 = _5032 + 16;
            uint8_t _5034 = ((const uint8_t *)_mat_b)[_5033];
            uint16_t _5035 = (uint16_t)(_5034);
            uint16_t _5036 = _5031 * _5035;
            uint32_t _5037 = (uint32_t)(_5036);
            uint32_t _5038 = _5028 + _5037;
            int32_t _5039 = _5016 + 2;
            uint8_t _5040 = ((const uint8_t *)_mat_a)[_5039];
            uint16_t _5041 = (uint16_t)(_5040);
            int32_t _5042 = _5022 + _4198;
            uint8_t _5043 = ((const uint8_t *)_mat_b)[_5042];
            uint16_t _5044 = (uint16_t)(_5043);
            uint16_t _5045 = _5041 * _5044;
            uint32_t _5046 = (uint32_t)(_5045);
            uint32_t _5047 = _5038 + _5046;
            int32_t _5048 = _5016 + 3;
            uint8_t _5049 = ((const uint8_t *)_mat_a)[_5048];
            uint16_t _5050 = (uint16_t)(_5049);
            int32_t _5051 = _5022 + _5011;
            uint8_t _5052 = ((const uint8_t *)_mat_b)[_5051];
            uint16_t _5053 = (uint16_t)(_5052);
            uint16_t _5054 = _5050 * _5053;
            uint32_t _5055 = (uint32_t)(_5054);
            uint32_t _5056 = _5047 + _5055;
            int32_t _5057 = _multiplied_no_offsets_s1_x_xi_xii + _4239;
            _multiplied_no_offsets[_5057] = _5056;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _5058 = _4219 * 4;
           int32_t _5059 = _5058 + 3;
           int32_t _5060 = _31 * _5059;
           int32_t _5061 = _590 * _5059;
           int32_t _5062 = _5061 + _4207;
           int32_t _5063 = _553 + 16;
           int32_t _5064 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _4218; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _5065 = _multiplied_no_offsets_s1_x_xi_xii + _5062;
            uint32_t _5066 = _multiplied_no_offsets[_5065];
            int32_t _5067 = _5064 * 4;
            int32_t _5068 = _5067 + _5060;
            uint8_t _5069 = ((const uint8_t *)_mat_a)[_5068];
            uint16_t _5070 = (uint16_t)(_5069);
            int32_t _5071 = _40 * _5064;
            int32_t _5072 = _5071 * 4;
            int32_t _5073 = _5072 + _4199;
            int32_t _5074 = _5073 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _5075 = _5074 + 16;
            uint8_t _5076 = ((const uint8_t *)_mat_b)[_5075];
            uint16_t _5077 = (uint16_t)(_5076);
            uint16_t _5078 = _5070 * _5077;
            uint32_t _5079 = (uint32_t)(_5078);
            uint32_t _5080 = _5066 + _5079;
            int32_t _5081 = _5068 + 1;
            uint8_t _5082 = ((const uint8_t *)_mat_a)[_5081];
            uint16_t _5083 = (uint16_t)(_5082);
            int32_t _5084 = _5074 + _40;
            int32_t _5085 = _5084 + 16;
            uint8_t _5086 = ((const uint8_t *)_mat_b)[_5085];
            uint16_t _5087 = (uint16_t)(_5086);
            uint16_t _5088 = _5083 * _5087;
            uint32_t _5089 = (uint32_t)(_5088);
            uint32_t _5090 = _5080 + _5089;
            int32_t _5091 = _5068 + 2;
            uint8_t _5092 = ((const uint8_t *)_mat_a)[_5091];
            uint16_t _5093 = (uint16_t)(_5092);
            int32_t _5094 = _5074 + _4198;
            uint8_t _5095 = ((const uint8_t *)_mat_b)[_5094];
            uint16_t _5096 = (uint16_t)(_5095);
            uint16_t _5097 = _5093 * _5096;
            uint32_t _5098 = (uint32_t)(_5097);
            uint32_t _5099 = _5090 + _5098;
            int32_t _5100 = _5068 + 3;
            uint8_t _5101 = ((const uint8_t *)_mat_a)[_5100];
            uint16_t _5102 = (uint16_t)(_5101);
            int32_t _5103 = _5074 + _5063;
            uint8_t _5104 = ((const uint8_t *)_mat_b)[_5103];
            uint16_t _5105 = (uint16_t)(_5104);
            uint16_t _5106 = _5102 * _5105;
            uint32_t _5107 = (uint32_t)(_5106);
            uint32_t _5108 = _5099 + _5107;
            int32_t _5109 = _multiplied_no_offsets_s1_x_xi_xii + _4235;
            _multiplied_no_offsets[_5109] = _5108;
           } // for _multiplied_no_offsets_s1_x_xi_xii
          } // if _4200 else
          if (_4201)
          {
           int32_t _5110 = _590 * _4219;
           int32_t _5111 = _5110 * 4;
           int32_t _5112 = _5111 + _4206;
           uint32x8_t _5113 = uint32x8_t::load(_multiplied_no_offsets, _5112);
           int32_t _5114 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           int32_t _5115 = _5114 * _40;
           int32_t _5116 = _5115 + _4203;
           int32_t _5117 = _5116 * 4;
           int32_t _5118 = _5117 + 24;
           uint8x8_t _5119 = uint8x8_t::load(_mat_b, _5118);
           uint16x8_t _5120 = uint16x8_t::convert_from<uint8x8_t>(_5119);
           int32_t _5121 = _31 * _4219;
           int32_t _5122 = _5121 + _4210;
           int32_t _5123 = _5122 + _multiplied_no_offsets_s1_k__x_rki;
           int32_t _5124 = _5123 * 4;
           uint8_t _5125 = ((const uint8_t *)_mat_a)[_5124];
           uint16_t _5126 = (uint16_t)(_5125);
           uint16x8_t _5127 = uint16x8_t::broadcast(_5126);
           uint16x8_t _5128 = _5120 * _5127;
           uint32x8_t _5129 = uint32x8_t::convert_from<uint16x8_t>(_5128);
           uint32x8_t _5130 = _5113 + _5129;
           int32_t _5131 = _5117 + _40;
           int32_t _5132 = _5131 + 24;
           uint8x8_t _5133 = uint8x8_t::load(_mat_b, _5132);
           uint16x8_t _5134 = uint16x8_t::convert_from<uint8x8_t>(_5133);
           int32_t _5135 = _5124 + 1;
           uint8_t _5136 = ((const uint8_t *)_mat_a)[_5135];
           uint16_t _5137 = (uint16_t)(_5136);
           uint16x8_t _5138 = uint16x8_t::broadcast(_5137);
           uint16x8_t _5139 = _5134 * _5138;
           uint32x8_t _5140 = uint32x8_t::convert_from<uint16x8_t>(_5139);
           uint32x8_t _5141 = _5130 + _5140;
           int32_t _5142 = _5116 * 2;
           int32_t _5143 = _5142 + _40;
           int32_t _5144 = _5143 * 2;
           int32_t _5145 = _5144 + 24;
           uint8x8_t _5146 = uint8x8_t::load(_mat_b, _5145);
           uint16x8_t _5147 = uint16x8_t::convert_from<uint8x8_t>(_5146);
           int32_t _5148 = _5124 + 2;
           uint8_t _5149 = ((const uint8_t *)_mat_a)[_5148];
           uint16_t _5150 = (uint16_t)(_5149);
           uint16x8_t _5151 = uint16x8_t::broadcast(_5150);
           uint16x8_t _5152 = _5147 * _5151;
           uint32x8_t _5153 = uint32x8_t::convert_from<uint16x8_t>(_5152);
           uint32x8_t _5154 = _5141 + _5153;
           int32_t _5155 = _5117 + _553;
           int32_t _5156 = _5155 + 24;
           uint8x8_t _5157 = uint8x8_t::load(_mat_b, _5156);
           uint16x8_t _5158 = uint16x8_t::convert_from<uint8x8_t>(_5157);
           int32_t _5159 = _5124 + 3;
           uint8_t _5160 = ((const uint8_t *)_mat_a)[_5159];
           uint16_t _5161 = (uint16_t)(_5160);
           uint16x8_t _5162 = uint16x8_t::broadcast(_5161);
           uint16x8_t _5163 = _5158 * _5162;
           uint32x8_t _5164 = uint32x8_t::convert_from<uint16x8_t>(_5163);
           uint32x8_t _5165 = _5154 + _5164;
           _5165.store(_multiplied_no_offsets, _4230);
           int32_t _5166 = _4219 * 4;
           int32_t _5167 = _5166 + 1;
           int32_t _5168 = _5167 * _31;
           int32_t _5169 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           int32_t _5170 = _5169 * 4;
           int32_t _5171 = _5168 + _5170;
           int32_t _5172 = _5167 * _590;
           int32_t _5173 = _5172 + _4206;
           uint32x8_t _5174 = uint32x8_t::load(_multiplied_no_offsets, _5173);
           int32_t _5175 = _5169 * _40;
           int32_t _5176 = _5175 + _4203;
           int32_t _5177 = _5176 * 4;
           int32_t _5178 = _5177 + 24;
           uint8x8_t _5179 = uint8x8_t::load(_mat_b, _5178);
           uint16x8_t _5180 = uint16x8_t::convert_from<uint8x8_t>(_5179);
           uint8_t _5181 = ((const uint8_t *)_mat_a)[_5171];
           uint16_t _5182 = (uint16_t)(_5181);
           uint16x8_t _5183 = uint16x8_t::broadcast(_5182);
           uint16x8_t _5184 = _5180 * _5183;
           uint32x8_t _5185 = uint32x8_t::convert_from<uint16x8_t>(_5184);
           uint32x8_t _5186 = _5174 + _5185;
           int32_t _5187 = _5177 + _40;
           int32_t _5188 = _5187 + 24;
           uint8x8_t _5189 = uint8x8_t::load(_mat_b, _5188);
           uint16x8_t _5190 = uint16x8_t::convert_from<uint8x8_t>(_5189);
           int32_t _5191 = _5171 + 1;
           uint8_t _5192 = ((const uint8_t *)_mat_a)[_5191];
           uint16_t _5193 = (uint16_t)(_5192);
           uint16x8_t _5194 = uint16x8_t::broadcast(_5193);
           uint16x8_t _5195 = _5190 * _5194;
           uint32x8_t _5196 = uint32x8_t::convert_from<uint16x8_t>(_5195);
           uint32x8_t _5197 = _5186 + _5196;
           int32_t _5198 = _5176 * 2;
           int32_t _5199 = _5198 + _40;
           int32_t _5200 = _5199 * 2;
           int32_t _5201 = _5200 + 24;
           uint8x8_t _5202 = uint8x8_t::load(_mat_b, _5201);
           uint16x8_t _5203 = uint16x8_t::convert_from<uint8x8_t>(_5202);
           int32_t _5204 = _5171 + 2;
           uint8_t _5205 = ((const uint8_t *)_mat_a)[_5204];
           uint16_t _5206 = (uint16_t)(_5205);
           uint16x8_t _5207 = uint16x8_t::broadcast(_5206);
           uint16x8_t _5208 = _5203 * _5207;
           uint32x8_t _5209 = uint32x8_t::convert_from<uint16x8_t>(_5208);
           uint32x8_t _5210 = _5197 + _5209;
           int32_t _5211 = _5177 + _553;
           int32_t _5212 = _5211 + 24;
           uint8x8_t _5213 = uint8x8_t::load(_mat_b, _5212);
           uint16x8_t _5214 = uint16x8_t::convert_from<uint8x8_t>(_5213);
           int32_t _5215 = _5171 + 3;
           uint8_t _5216 = ((const uint8_t *)_mat_a)[_5215];
           uint16_t _5217 = (uint16_t)(_5216);
           uint16x8_t _5218 = uint16x8_t::broadcast(_5217);
           uint16x8_t _5219 = _5214 * _5218;
           uint32x8_t _5220 = uint32x8_t::convert_from<uint16x8_t>(_5219);
           uint32x8_t _5221 = _5210 + _5220;
           _5221.store(_multiplied_no_offsets, _4242);
           int32_t _5222 = _4219 * 4;
           int32_t _5223 = _5222 + 2;
           int32_t _5224 = _5223 * _31;
           int32_t _5225 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           int32_t _5226 = _5225 * 4;
           int32_t _5227 = _5224 + _5226;
           int32_t _5228 = _5223 * _590;
           int32_t _5229 = _5228 + _4206;
           uint32x8_t _5230 = uint32x8_t::load(_multiplied_no_offsets, _5229);
           int32_t _5231 = _5225 * _40;
           int32_t _5232 = _5231 + _4203;
           int32_t _5233 = _5232 * 4;
           int32_t _5234 = _5233 + 24;
           uint8x8_t _5235 = uint8x8_t::load(_mat_b, _5234);
           uint16x8_t _5236 = uint16x8_t::convert_from<uint8x8_t>(_5235);
           uint8_t _5237 = ((const uint8_t *)_mat_a)[_5227];
           uint16_t _5238 = (uint16_t)(_5237);
           uint16x8_t _5239 = uint16x8_t::broadcast(_5238);
           uint16x8_t _5240 = _5236 * _5239;
           uint32x8_t _5241 = uint32x8_t::convert_from<uint16x8_t>(_5240);
           uint32x8_t _5242 = _5230 + _5241;
           int32_t _5243 = _5233 + _40;
           int32_t _5244 = _5243 + 24;
           uint8x8_t _5245 = uint8x8_t::load(_mat_b, _5244);
           uint16x8_t _5246 = uint16x8_t::convert_from<uint8x8_t>(_5245);
           int32_t _5247 = _5227 + 1;
           uint8_t _5248 = ((const uint8_t *)_mat_a)[_5247];
           uint16_t _5249 = (uint16_t)(_5248);
           uint16x8_t _5250 = uint16x8_t::broadcast(_5249);
           uint16x8_t _5251 = _5246 * _5250;
           uint32x8_t _5252 = uint32x8_t::convert_from<uint16x8_t>(_5251);
           uint32x8_t _5253 = _5242 + _5252;
           int32_t _5254 = _5232 * 2;
           int32_t _5255 = _5254 + _40;
           int32_t _5256 = _5255 * 2;
           int32_t _5257 = _5256 + 24;
           uint8x8_t _5258 = uint8x8_t::load(_mat_b, _5257);
           uint16x8_t _5259 = uint16x8_t::convert_from<uint8x8_t>(_5258);
           int32_t _5260 = _5227 + 2;
           uint8_t _5261 = ((const uint8_t *)_mat_a)[_5260];
           uint16_t _5262 = (uint16_t)(_5261);
           uint16x8_t _5263 = uint16x8_t::broadcast(_5262);
           uint16x8_t _5264 = _5259 * _5263;
           uint32x8_t _5265 = uint32x8_t::convert_from<uint16x8_t>(_5264);
           uint32x8_t _5266 = _5253 + _5265;
           int32_t _5267 = _5233 + _553;
           int32_t _5268 = _5267 + 24;
           uint8x8_t _5269 = uint8x8_t::load(_mat_b, _5268);
           uint16x8_t _5270 = uint16x8_t::convert_from<uint8x8_t>(_5269);
           int32_t _5271 = _5227 + 3;
           uint8_t _5272 = ((const uint8_t *)_mat_a)[_5271];
           uint16_t _5273 = (uint16_t)(_5272);
           uint16x8_t _5274 = uint16x8_t::broadcast(_5273);
           uint16x8_t _5275 = _5270 * _5274;
           uint32x8_t _5276 = uint32x8_t::convert_from<uint16x8_t>(_5275);
           uint32x8_t _5277 = _5266 + _5276;
           _5277.store(_multiplied_no_offsets, _4238);
           int32_t _5278 = _4219 * 4;
           int32_t _5279 = _5278 + 3;
           int32_t _5280 = _5279 * _31;
           int32_t _5281 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           int32_t _5282 = _5281 * 4;
           int32_t _5283 = _5280 + _5282;
           int32_t _5284 = _5279 * _590;
           int32_t _5285 = _5284 + _4206;
           uint32x8_t _5286 = uint32x8_t::load(_multiplied_no_offsets, _5285);
           int32_t _5287 = _5281 * _40;
           int32_t _5288 = _5287 + _4203;
           int32_t _5289 = _5288 * 4;
           int32_t _5290 = _5289 + 24;
           uint8x8_t _5291 = uint8x8_t::load(_mat_b, _5290);
           uint16x8_t _5292 = uint16x8_t::convert_from<uint8x8_t>(_5291);
           uint8_t _5293 = ((const uint8_t *)_mat_a)[_5283];
           uint16_t _5294 = (uint16_t)(_5293);
           uint16x8_t _5295 = uint16x8_t::broadcast(_5294);
           uint16x8_t _5296 = _5292 * _5295;
           uint32x8_t _5297 = uint32x8_t::convert_from<uint16x8_t>(_5296);
           uint32x8_t _5298 = _5286 + _5297;
           int32_t _5299 = _5289 + _40;
           int32_t _5300 = _5299 + 24;
           uint8x8_t _5301 = uint8x8_t::load(_mat_b, _5300);
           uint16x8_t _5302 = uint16x8_t::convert_from<uint8x8_t>(_5301);
           int32_t _5303 = _5283 + 1;
           uint8_t _5304 = ((const uint8_t *)_mat_a)[_5303];
           uint16_t _5305 = (uint16_t)(_5304);
           uint16x8_t _5306 = uint16x8_t::broadcast(_5305);
           uint16x8_t _5307 = _5302 * _5306;
           uint32x8_t _5308 = uint32x8_t::convert_from<uint16x8_t>(_5307);
           uint32x8_t _5309 = _5298 + _5308;
           int32_t _5310 = _5288 * 2;
           int32_t _5311 = _5310 + _40;
           int32_t _5312 = _5311 * 2;
           int32_t _5313 = _5312 + 24;
           uint8x8_t _5314 = uint8x8_t::load(_mat_b, _5313);
           uint16x8_t _5315 = uint16x8_t::convert_from<uint8x8_t>(_5314);
           int32_t _5316 = _5283 + 2;
           uint8_t _5317 = ((const uint8_t *)_mat_a)[_5316];
           uint16_t _5318 = (uint16_t)(_5317);
           uint16x8_t _5319 = uint16x8_t::broadcast(_5318);
           uint16x8_t _5320 = _5315 * _5319;
           uint32x8_t _5321 = uint32x8_t::convert_from<uint16x8_t>(_5320);
           uint32x8_t _5322 = _5309 + _5321;
           int32_t _5323 = _5289 + _553;
           int32_t _5324 = _5323 + 24;
           uint8x8_t _5325 = uint8x8_t::load(_mat_b, _5324);
           uint16x8_t _5326 = uint16x8_t::convert_from<uint8x8_t>(_5325);
           int32_t _5327 = _5283 + 3;
           uint8_t _5328 = ((const uint8_t *)_mat_a)[_5327];
           uint16_t _5329 = (uint16_t)(_5328);
           uint16x8_t _5330 = uint16x8_t::broadcast(_5329);
           uint16x8_t _5331 = _5326 * _5330;
           uint32x8_t _5332 = uint32x8_t::convert_from<uint16x8_t>(_5331);
           uint32x8_t _5333 = _5322 + _5332;
           _5333.store(_multiplied_no_offsets, _4234);
          } // if _4201
          else
          {
           int32_t _5334 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           int32_t _5335 = _5334 * _40;
           int32_t _5336 = _5335 * 4;
           int32_t _5337 = _5336 + _4199;
           int32_t _5338 = _590 * _4219;
           int32_t _5339 = _5338 * 4;
           int32_t _5340 = _5339 + _4206;
           int32_t _5341 = _31 * _4219;
           int32_t _5342 = _5341 + _4210;
           int32_t _5343 = _5342 + _multiplied_no_offsets_s1_k__x_rki;
           int32_t _5344 = _553 + 24;
           int32_t _5345 = _40 + 24;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _4215; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _5346 = _multiplied_no_offsets_s1_x_xi_xii + _5340;
            uint32_t _5347 = _multiplied_no_offsets[_5346];
            int32_t _5348 = _5343 * 4;
            uint8_t _5349 = ((const uint8_t *)_mat_a)[_5348];
            uint16_t _5350 = (uint16_t)(_5349);
            int32_t _5351 = _multiplied_no_offsets_s1_x_xi_xii + _5337;
            int32_t _5352 = _5351 + 24;
            uint8_t _5353 = ((const uint8_t *)_mat_b)[_5352];
            uint16_t _5354 = (uint16_t)(_5353);
            uint16_t _5355 = _5350 * _5354;
            uint32_t _5356 = (uint32_t)(_5355);
            uint32_t _5357 = _5347 + _5356;
            int32_t _5358 = _5348 + 1;
            uint8_t _5359 = ((const uint8_t *)_mat_a)[_5358];
            uint16_t _5360 = (uint16_t)(_5359);
            int32_t _5361 = _5351 + _5345;
            uint8_t _5362 = ((const uint8_t *)_mat_b)[_5361];
            uint16_t _5363 = (uint16_t)(_5362);
            uint16_t _5364 = _5360 * _5363;
            uint32_t _5365 = (uint32_t)(_5364);
            uint32_t _5366 = _5357 + _5365;
            int32_t _5367 = _5348 + 2;
            uint8_t _5368 = ((const uint8_t *)_mat_a)[_5367];
            uint16_t _5369 = (uint16_t)(_5368);
            int32_t _5370 = _5351 + _4197;
            uint8_t _5371 = ((const uint8_t *)_mat_b)[_5370];
            uint16_t _5372 = (uint16_t)(_5371);
            uint16_t _5373 = _5369 * _5372;
            uint32_t _5374 = (uint32_t)(_5373);
            uint32_t _5375 = _5366 + _5374;
            int32_t _5376 = _5348 + 3;
            uint8_t _5377 = ((const uint8_t *)_mat_a)[_5376];
            uint16_t _5378 = (uint16_t)(_5377);
            int32_t _5379 = _5351 + _5344;
            uint8_t _5380 = ((const uint8_t *)_mat_b)[_5379];
            uint16_t _5381 = (uint16_t)(_5380);
            uint16_t _5382 = _5378 * _5381;
            uint32_t _5383 = (uint32_t)(_5382);
            uint32_t _5384 = _5375 + _5383;
            int32_t _5385 = _multiplied_no_offsets_s1_x_xi_xii + _4230;
            _multiplied_no_offsets[_5385] = _5384;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _5386 = _4219 * 4;
           int32_t _5387 = _5386 + 1;
           int32_t _5388 = _31 * _5387;
           int32_t _5389 = _590 * _5387;
           int32_t _5390 = _5389 + _4206;
           int32_t _5391 = _553 + 24;
           int32_t _5392 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _4215; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _5393 = _multiplied_no_offsets_s1_x_xi_xii + _5390;
            uint32_t _5394 = _multiplied_no_offsets[_5393];
            int32_t _5395 = _5392 * 4;
            int32_t _5396 = _5395 + _5388;
            uint8_t _5397 = ((const uint8_t *)_mat_a)[_5396];
            uint16_t _5398 = (uint16_t)(_5397);
            int32_t _5399 = _40 * _5392;
            int32_t _5400 = _5399 * 4;
            int32_t _5401 = _5400 + _4199;
            int32_t _5402 = _5401 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _5403 = _5402 + 24;
            uint8_t _5404 = ((const uint8_t *)_mat_b)[_5403];
            uint16_t _5405 = (uint16_t)(_5404);
            uint16_t _5406 = _5398 * _5405;
            uint32_t _5407 = (uint32_t)(_5406);
            uint32_t _5408 = _5394 + _5407;
            int32_t _5409 = _5396 + 1;
            uint8_t _5410 = ((const uint8_t *)_mat_a)[_5409];
            uint16_t _5411 = (uint16_t)(_5410);
            int32_t _5412 = _5402 + _40;
            int32_t _5413 = _5412 + 24;
            uint8_t _5414 = ((const uint8_t *)_mat_b)[_5413];
            uint16_t _5415 = (uint16_t)(_5414);
            uint16_t _5416 = _5411 * _5415;
            uint32_t _5417 = (uint32_t)(_5416);
            uint32_t _5418 = _5408 + _5417;
            int32_t _5419 = _5396 + 2;
            uint8_t _5420 = ((const uint8_t *)_mat_a)[_5419];
            uint16_t _5421 = (uint16_t)(_5420);
            int32_t _5422 = _5402 + _4197;
            uint8_t _5423 = ((const uint8_t *)_mat_b)[_5422];
            uint16_t _5424 = (uint16_t)(_5423);
            uint16_t _5425 = _5421 * _5424;
            uint32_t _5426 = (uint32_t)(_5425);
            uint32_t _5427 = _5418 + _5426;
            int32_t _5428 = _5396 + 3;
            uint8_t _5429 = ((const uint8_t *)_mat_a)[_5428];
            uint16_t _5430 = (uint16_t)(_5429);
            int32_t _5431 = _5402 + _5391;
            uint8_t _5432 = ((const uint8_t *)_mat_b)[_5431];
            uint16_t _5433 = (uint16_t)(_5432);
            uint16_t _5434 = _5430 * _5433;
            uint32_t _5435 = (uint32_t)(_5434);
            uint32_t _5436 = _5427 + _5435;
            int32_t _5437 = _multiplied_no_offsets_s1_x_xi_xii + _4242;
            _multiplied_no_offsets[_5437] = _5436;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _5438 = _4219 * 4;
           int32_t _5439 = _5438 + 2;
           int32_t _5440 = _31 * _5439;
           int32_t _5441 = _590 * _5439;
           int32_t _5442 = _5441 + _4206;
           int32_t _5443 = _553 + 24;
           int32_t _5444 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _4215; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _5445 = _multiplied_no_offsets_s1_x_xi_xii + _5442;
            uint32_t _5446 = _multiplied_no_offsets[_5445];
            int32_t _5447 = _5444 * 4;
            int32_t _5448 = _5447 + _5440;
            uint8_t _5449 = ((const uint8_t *)_mat_a)[_5448];
            uint16_t _5450 = (uint16_t)(_5449);
            int32_t _5451 = _40 * _5444;
            int32_t _5452 = _5451 * 4;
            int32_t _5453 = _5452 + _4199;
            int32_t _5454 = _5453 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _5455 = _5454 + 24;
            uint8_t _5456 = ((const uint8_t *)_mat_b)[_5455];
            uint16_t _5457 = (uint16_t)(_5456);
            uint16_t _5458 = _5450 * _5457;
            uint32_t _5459 = (uint32_t)(_5458);
            uint32_t _5460 = _5446 + _5459;
            int32_t _5461 = _5448 + 1;
            uint8_t _5462 = ((const uint8_t *)_mat_a)[_5461];
            uint16_t _5463 = (uint16_t)(_5462);
            int32_t _5464 = _5454 + _40;
            int32_t _5465 = _5464 + 24;
            uint8_t _5466 = ((const uint8_t *)_mat_b)[_5465];
            uint16_t _5467 = (uint16_t)(_5466);
            uint16_t _5468 = _5463 * _5467;
            uint32_t _5469 = (uint32_t)(_5468);
            uint32_t _5470 = _5460 + _5469;
            int32_t _5471 = _5448 + 2;
            uint8_t _5472 = ((const uint8_t *)_mat_a)[_5471];
            uint16_t _5473 = (uint16_t)(_5472);
            int32_t _5474 = _5454 + _4197;
            uint8_t _5475 = ((const uint8_t *)_mat_b)[_5474];
            uint16_t _5476 = (uint16_t)(_5475);
            uint16_t _5477 = _5473 * _5476;
            uint32_t _5478 = (uint32_t)(_5477);
            uint32_t _5479 = _5470 + _5478;
            int32_t _5480 = _5448 + 3;
            uint8_t _5481 = ((const uint8_t *)_mat_a)[_5480];
            uint16_t _5482 = (uint16_t)(_5481);
            int32_t _5483 = _5454 + _5443;
            uint8_t _5484 = ((const uint8_t *)_mat_b)[_5483];
            uint16_t _5485 = (uint16_t)(_5484);
            uint16_t _5486 = _5482 * _5485;
            uint32_t _5487 = (uint32_t)(_5486);
            uint32_t _5488 = _5479 + _5487;
            int32_t _5489 = _multiplied_no_offsets_s1_x_xi_xii + _4238;
            _multiplied_no_offsets[_5489] = _5488;
           } // for _multiplied_no_offsets_s1_x_xi_xii
           int32_t _5490 = _4219 * 4;
           int32_t _5491 = _5490 + 3;
           int32_t _5492 = _31 * _5491;
           int32_t _5493 = _590 * _5491;
           int32_t _5494 = _5493 + _4206;
           int32_t _5495 = _553 + 24;
           int32_t _5496 = _multiplied_no_offsets_s1_k__x_rki + _4210;
           for (int _multiplied_no_offsets_s1_x_xi_xii = 0; _multiplied_no_offsets_s1_x_xi_xii < 0 + _4215; _multiplied_no_offsets_s1_x_xi_xii++)
           {
            int32_t _5497 = _multiplied_no_offsets_s1_x_xi_xii + _5494;
            uint32_t _5498 = _multiplied_no_offsets[_5497];
            int32_t _5499 = _5496 * 4;
            int32_t _5500 = _5499 + _5492;
            uint8_t _5501 = ((const uint8_t *)_mat_a)[_5500];
            uint16_t _5502 = (uint16_t)(_5501);
            int32_t _5503 = _40 * _5496;
            int32_t _5504 = _5503 * 4;
            int32_t _5505 = _5504 + _4199;
            int32_t _5506 = _5505 + _multiplied_no_offsets_s1_x_xi_xii;
            int32_t _5507 = _5506 + 24;
            uint8_t _5508 = ((const uint8_t *)_mat_b)[_5507];
            uint16_t _5509 = (uint16_t)(_5508);
            uint16_t _5510 = _5502 * _5509;
            uint32_t _5511 = (uint32_t)(_5510);
            uint32_t _5512 = _5498 + _5511;
            int32_t _5513 = _5500 + 1;
            uint8_t _5514 = ((const uint8_t *)_mat_a)[_5513];
            uint16_t _5515 = (uint16_t)(_5514);
            int32_t _5516 = _5506 + _40;
            int32_t _5517 = _5516 + 24;
            uint8_t _5518 = ((const uint8_t *)_mat_b)[_5517];
            uint16_t _5519 = (uint16_t)(_5518);
            uint16_t _5520 = _5515 * _5519;
            uint32_t _5521 = (uint32_t)(_5520);
            uint32_t _5522 = _5512 + _5521;
            int32_t _5523 = _5500 + 2;
            uint8_t _5524 = ((const uint8_t *)_mat_a)[_5523];
            uint16_t _5525 = (uint16_t)(_5524);
            int32_t _5526 = _5506 + _4197;
            uint8_t _5527 = ((const uint8_t *)_mat_b)[_5526];
            uint16_t _5528 = (uint16_t)(_5527);
            uint16_t _5529 = _5525 * _5528;
            uint32_t _5530 = (uint32_t)(_5529);
            uint32_t _5531 = _5522 + _5530;
            int32_t _5532 = _5500 + 3;
            uint8_t _5533 = ((const uint8_t *)_mat_a)[_5532];
            uint16_t _5534 = (uint16_t)(_5533);
            int32_t _5535 = _5506 + _5495;
            uint8_t _5536 = ((const uint8_t *)_mat_b)[_5535];
            uint16_t _5537 = (uint16_t)(_5536);
            uint16_t _5538 = _5534 * _5537;
            uint32_t _5539 = (uint32_t)(_5538);
            uint32_t _5540 = _5531 + _5539;
            int32_t _5541 = _multiplied_no_offsets_s1_x_xi_xii + _4234;
            _multiplied_no_offsets[_5541] = _5540;
           } // for _multiplied_no_offsets_s1_x_xi_xii
          } // if _4201 else
         } // for _multiplied_no_offsets_s1_k__x_rki
        } // for _multiplied_no_offsets_s1_y_yi_yi
       } // for _multiplied_no_offsets_s1_k__x_k__x
      } // for _multiplied_no_offsets_s1_x_x
     } // if _608 else
    } // for _multiplied_no_offsets_s1_y_y
    {
     int32_t _5542 = _48 >> 2;
     int32_t _5543 = ::halide_cpp_max(_5542, 4);
     int32_t _5544 = _5543 * 4;
     int64_t _5545 = _5544;
     if ((_5545 > ((int64_t(1) << 31) - 1)) || ((_5545 * sizeof(uint32_t )) > ((int64_t(1) << 31) - 1)))
     {
      halide_error(_ucon, "32-bit signed overflow computing size of allocation row_sums_a\n");
      return -1;
     } // overflow test row_sums_a
     int64_t _5546 = _5545;
     uint32_t *_row_sums_a = (uint32_t  *)halide_malloc(_ucon, sizeof(uint32_t )*_5546);
     if (!_row_sums_a)
     {
      return halide_error_out_of_memory(_ucon);
     }
     HalideFreeHelper _row_sums_a_free(_ucon, _row_sums_a, halide_free);
     // produce row_sums_a
     int32_t _5547 = _48 >> 2;
     int32_t _5548 = _48 + 12;
     int32_t _5549 = _5548 >> 4;
     int32_t _5550 = ::halide_cpp_min(_5547, 4);
     int32_t _5551 = _5550 * 4;
     int32_t _5552 = 16 - _5551;
     int32_t _5553 = _5547 * 4;
     int32_t _5554 = _5553 + -16;
     for (int _row_sums_a_s0_y_y = 0; _row_sums_a_s0_y_y < 0 + _5549; _row_sums_a_s0_y_y++)
     {
      int32_t _5555 = _row_sums_a_s0_y_y * 16;
      int32_t _5556 = ::halide_cpp_min(_5555, _5554);
      {
       uint32_t _sum[16];
       // produce sum
       uint32_t _5557 = (uint32_t)(ADD_UINT64_T_SUFFIX(0));
       uint32x16_t _5558 = uint32x16_t::broadcast(_5557);
       _5558.store(_sum, 0);
       int32_t _5559 = _31 * _5556;
       for (int _sum_s1_fk__x = 0; _sum_s1_fk__x < 0 + _27; _sum_s1_fk__x++)
       {
        uint32x16_t _5560 = uint32x16_t::load(_sum, 0);
        int32_t _5561 = _sum_s1_fk__x + _5559;
        int32x16_t _5562 = int32x16_t::ramp(_5561, _31);
        uint8x16_t _5563 = uint8x16_t::load(_mat_a, _5562);
        uint32x16_t _5564 = uint32x16_t::convert_from<uint8x16_t>(_5563);
        uint32x16_t _5565 = _5560 + _5564;
        _5565.store(_sum, 0);
       } // for _sum_s1_fk__x
       // consume sum
       uint32x16_t _5566 = uint32x16_t::load(_sum, 0);
       int32_t _5567 = _5556 + _5552;
       _5566.store(_row_sums_a, _5567);
      } // alloc _sum
     } // for _row_sums_a_s0_y_y
     // produce output
     // consume row_sums_a
     // consume multiplied_no_offsets
     // consume column_sums_b
     uint32_t _5568 = (uint32_t)(_output_shift);
     int32_t _5569 = _45 >> 4;
     int32_t _5570 = _5569 * 16;
     int32_t _5571 = _48 >> 2;
     int32_t _5572 = 1 << _5568;
     int32_t _5573 = ::halide_cpp_max(_5570, 4);
     int16_t _5574 = (int16_t)(ADD_INT64_T_SUFFIX(0));
     int16_t _5575 = ::halide_cpp_min(_mat_b_offset, _5574);
     int16_t _5576 = (int16_t)(ADD_INT64_T_SUFFIX(-255));
     int16_t _5577 = ::halide_cpp_max(_5575, _5576);
     int16_t _5578 = ::halide_cpp_min(_mat_a_offset, _5574);
     int16_t _5579 = ::halide_cpp_max(_5578, _5576);
     int32_t _5580 = ::halide_cpp_min(_5571, 4);
     int32_t _5581 = ::halide_cpp_min(_5569, 1);
     int32_t _5582 = ::halide_cpp_min(_5570, 4);
     int32_t _5583 = 4 - _5582;
     #pragma omp parallel for
     for (int _output_s0_y_y = 0; _output_s0_y_y < 0 + _5571; _output_s0_y_y++)
     {
      int32_t _5584 = _5572 + -1;
      int32_t _5585 = _output_s0_y_y * 4;
      int32_t _5586 = _5585 + 1;
      int32_t _5587 = _5585 + 2;
      int32_t _5588 = _5585 + 3;
      int32_t _5589 = _output_s0_y_y - _5580;
      int32_t _5590 = _5589 * 4;
      int32_t _5591 = _5584 >> 1;
      int32_t _5592 = _49 * _5588;
      int32_t _5593 = _49 * _5587;
      int32_t _5594 = _49 * _5586;
      int32_t _5595 = _output_s0_y_y * _49;
      int32_t _5596 = _output_s0_y_y * _5573;
      int32_t _5597 = _5596 * 4;
      int32_t _5598 = _5597 + _5583;
      int32_t _5599 = _5590 + 19;
      int32_t _5600 = _5590 + 18;
      int32_t _5601 = _5590 + 17;
      int32_t _5602 = _5590 + 16;
      int32_t _5603 = _5573 * _5588;
      int32_t _5604 = _5603 + _5583;
      int32_t _5605 = _5573 * _5587;
      int32_t _5606 = _5605 + _5583;
      int32_t _5607 = _5573 * _5586;
      int32_t _5608 = _5607 + _5583;
      for (int _output_s0_x_x = 0; _output_s0_x_x < 0 + _5569; _output_s0_x_x++)
      {
       int64_t _5609 = (int64_t)(ADD_INT64_T_SUFFIX(-2147483648));
       int64x16_t _5610 = int64x16_t::broadcast(_5609);
       int64_t _5611 = (int64_t)(ADD_INT64_T_SUFFIX(2147483647));
       int64x16_t _5612 = int64x16_t::broadcast(_5611);
       int32_t _5613 = _output_s0_x_x - _5581;
       int32_t _5614 = _5613 * 16;
       int32_t _5615 = _5614 + 16;
       uint32x16_t _5616 = uint32x16_t::load(_column_sums_b, _5615);
       int32x16_t _5617 = int32x16_t::convert_from<uint32x16_t>(_5616);
       int32_t _5618 = (int32_t)(_5579);
       int32x16_t _5619 = int32x16_t::broadcast(_5618);
       int32x16_t _5620 = _5617 * _5619;
       int32_t _5621 = _output_s0_x_x * 16;
       int32x16_t _5622 = int32x16_t::load(_bias, _5621);
       int32_t _5623 = (int32_t)(_5577);
       int32_t _5624 = _5618 * _5623;
       int32_t _5625 = _5624 * _27;
       uint32_t _5626 = _row_sums_a[_5602];
       int32_t _5627 = (int32_t)(_5626);
       int32_t _5628 = _5623 * _5627;
       int32_t _5629 = _5625 + _5628;
       int32x16_t _5630 = int32x16_t::broadcast(_5629);
       int32x16_t _5631 = _5622 + _5630;
       int32x16_t _5632 = _5620 + _5631;
       int32_t _5633 = _5621 + _5598;
       uint32x16_t _5634 = uint32x16_t::load(_multiplied_no_offsets, _5633);
       int32x16_t _5635 = int32x16_t::convert_from<uint32x16_t>(_5634);
       int32x16_t _5636 = _5632 + _5635;
       int64x16_t _5637 = int64x16_t::convert_from<int32x16_t>(_5636);
       int64_t _5638 = (int64_t)(_output_multiplier);
       int64x16_t _5639 = int64x16_t::broadcast(_5638);
       int64x16_t _5640 = _5637 * _5639;
       int64_t _5641 = (int64_t)(ADD_INT64_T_SUFFIX(1073741824));
       int64x16_t _5642 = int64x16_t::broadcast(_5641);
       int64x16_t _5643 = _5640 + _5642;
       int64_t _5644 = (int64_t)(ADD_INT64_T_SUFFIX(31));
       int64x16_t _5645 = int64x16_t::broadcast(_5644);
       int64x16_t _5646 = _5643 >> _5645;
       int64x16_t _5647 = int64x16_t::min(_5646, _5612);
       int64x16_t _5648 = int64x16_t::max(_5647, _5610);
       int32x16_t _5649 = int32x16_t::convert_from<int64x16_t>(_5648);
       uint8x16_t _5650 = uint8x16_t::broadcast(_output_min);
       uint8x16_t _5651 = uint8x16_t::broadcast(_output_max);
       int32x16_t _5652 = int32x16_t::broadcast(0);
       int32x16_t _5653 = int32x16_t::broadcast(255);
       int32x16_t _5654 = int32x16_t::broadcast(1);
       uint8x16_t _5655 = _5649 < _5652;
       int32x16_t _5656 = int32x16_t::select(_5655, _5654, _5652);
       int32x16_t _5657 = int32x16_t::broadcast(_5591);
       int32x16_t _5658 = _5656 + _5657;
       int32x16_t _5659 = int32x16_t::broadcast(_5584);
       int32x16_t _5660 = _5649 & _5659;
       uint8x16_t _5661 = _5658 < _5660;
       int32x16_t _5662 = int32x16_t::select(_5661, _5654, _5652);
       uint32x16_t _5663 = uint32x16_t::broadcast(_5568);
       int32x16_t _5664 = _5649 >> _5663;
       int32x16_t _5665 = int32x16_t::broadcast(_output_offset);
       int32x16_t _5666 = _5664 + _5665;
       int32x16_t _5667 = _5662 + _5666;
       int32x16_t _5668 = int32x16_t::min(_5667, _5653);
       int32x16_t _5669 = int32x16_t::max(_5668, _5652);
       uint8x16_t _5670 = uint8x16_t::convert_from<int32x16_t>(_5669);
       uint8x16_t _5671 = uint8x16_t::min(_5670, _5651);
       uint8x16_t _5672 = uint8x16_t::max(_5671, _5650);
       int32_t _5673 = _output_s0_x_x * 4;
       int32_t _5674 = _5673 + _5595;
       int32_t _5675 = _5674 * 4;
       _5672.store(_output, _5675);
       int64_t _5676 = (int64_t)(ADD_INT64_T_SUFFIX(-2147483648));
       int64x16_t _5677 = int64x16_t::broadcast(_5676);
       int64_t _5678 = (int64_t)(ADD_INT64_T_SUFFIX(2147483647));
       int64x16_t _5679 = int64x16_t::broadcast(_5678);
       int32_t _5680 = _output_s0_x_x - _5581;
       int32_t _5681 = _5680 * 16;
       int32_t _5682 = _5681 + 16;
       uint32x16_t _5683 = uint32x16_t::load(_column_sums_b, _5682);
       int32x16_t _5684 = int32x16_t::convert_from<uint32x16_t>(_5683);
       int32_t _5685 = (int32_t)(_5579);
       int32x16_t _5686 = int32x16_t::broadcast(_5685);
       int32x16_t _5687 = _5684 * _5686;
       int32_t _5688 = _output_s0_x_x * 16;
       int32x16_t _5689 = int32x16_t::load(_bias, _5688);
       int32_t _5690 = (int32_t)(_5577);
       int32_t _5691 = _5685 * _5690;
       int32_t _5692 = _5691 * _27;
       uint32_t _5693 = _row_sums_a[_5601];
       int32_t _5694 = (int32_t)(_5693);
       int32_t _5695 = _5690 * _5694;
       int32_t _5696 = _5692 + _5695;
       int32x16_t _5697 = int32x16_t::broadcast(_5696);
       int32x16_t _5698 = _5689 + _5697;
       int32x16_t _5699 = _5687 + _5698;
       int32_t _5700 = _5688 + _5608;
       uint32x16_t _5701 = uint32x16_t::load(_multiplied_no_offsets, _5700);
       int32x16_t _5702 = int32x16_t::convert_from<uint32x16_t>(_5701);
       int32x16_t _5703 = _5699 + _5702;
       int64x16_t _5704 = int64x16_t::convert_from<int32x16_t>(_5703);
       int64_t _5705 = (int64_t)(_output_multiplier);
       int64x16_t _5706 = int64x16_t::broadcast(_5705);
       int64x16_t _5707 = _5704 * _5706;
       int64_t _5708 = (int64_t)(ADD_INT64_T_SUFFIX(1073741824));
       int64x16_t _5709 = int64x16_t::broadcast(_5708);
       int64x16_t _5710 = _5707 + _5709;
       int64_t _5711 = (int64_t)(ADD_INT64_T_SUFFIX(31));
       int64x16_t _5712 = int64x16_t::broadcast(_5711);
       int64x16_t _5713 = _5710 >> _5712;
       int64x16_t _5714 = int64x16_t::min(_5713, _5679);
       int64x16_t _5715 = int64x16_t::max(_5714, _5677);
       int32x16_t _5716 = int32x16_t::convert_from<int64x16_t>(_5715);
       uint8x16_t _5717 = uint8x16_t::broadcast(_output_min);
       uint8x16_t _5718 = uint8x16_t::broadcast(_output_max);
       int32x16_t _5719 = int32x16_t::broadcast(0);
       int32x16_t _5720 = int32x16_t::broadcast(255);
       int32x16_t _5721 = int32x16_t::broadcast(1);
       uint8x16_t _5722 = _5716 < _5719;
       int32x16_t _5723 = int32x16_t::select(_5722, _5721, _5719);
       int32x16_t _5724 = int32x16_t::broadcast(_5591);
       int32x16_t _5725 = _5723 + _5724;
       int32x16_t _5726 = int32x16_t::broadcast(_5584);
       int32x16_t _5727 = _5716 & _5726;
       uint8x16_t _5728 = _5725 < _5727;
       int32x16_t _5729 = int32x16_t::select(_5728, _5721, _5719);
       uint32x16_t _5730 = uint32x16_t::broadcast(_5568);
       int32x16_t _5731 = _5716 >> _5730;
       int32x16_t _5732 = int32x16_t::broadcast(_output_offset);
       int32x16_t _5733 = _5731 + _5732;
       int32x16_t _5734 = _5729 + _5733;
       int32x16_t _5735 = int32x16_t::min(_5734, _5720);
       int32x16_t _5736 = int32x16_t::max(_5735, _5719);
       uint8x16_t _5737 = uint8x16_t::convert_from<int32x16_t>(_5736);
       uint8x16_t _5738 = uint8x16_t::min(_5737, _5718);
       uint8x16_t _5739 = uint8x16_t::max(_5738, _5717);
       int32_t _5740 = _5688 + _5594;
       _5739.store(_output, _5740);
       int64_t _5741 = (int64_t)(ADD_INT64_T_SUFFIX(-2147483648));
       int64x16_t _5742 = int64x16_t::broadcast(_5741);
       int64_t _5743 = (int64_t)(ADD_INT64_T_SUFFIX(2147483647));
       int64x16_t _5744 = int64x16_t::broadcast(_5743);
       int32_t _5745 = _output_s0_x_x - _5581;
       int32_t _5746 = _5745 * 16;
       int32_t _5747 = _5746 + 16;
       uint32x16_t _5748 = uint32x16_t::load(_column_sums_b, _5747);
       int32x16_t _5749 = int32x16_t::convert_from<uint32x16_t>(_5748);
       int32_t _5750 = (int32_t)(_5579);
       int32x16_t _5751 = int32x16_t::broadcast(_5750);
       int32x16_t _5752 = _5749 * _5751;
       int32_t _5753 = _output_s0_x_x * 16;
       int32x16_t _5754 = int32x16_t::load(_bias, _5753);
       int32_t _5755 = (int32_t)(_5577);
       int32_t _5756 = _5750 * _5755;
       int32_t _5757 = _5756 * _27;
       uint32_t _5758 = _row_sums_a[_5600];
       int32_t _5759 = (int32_t)(_5758);
       int32_t _5760 = _5755 * _5759;
       int32_t _5761 = _5757 + _5760;
       int32x16_t _5762 = int32x16_t::broadcast(_5761);
       int32x16_t _5763 = _5754 + _5762;
       int32x16_t _5764 = _5752 + _5763;
       int32_t _5765 = _5753 + _5606;
       uint32x16_t _5766 = uint32x16_t::load(_multiplied_no_offsets, _5765);
       int32x16_t _5767 = int32x16_t::convert_from<uint32x16_t>(_5766);
       int32x16_t _5768 = _5764 + _5767;
       int64x16_t _5769 = int64x16_t::convert_from<int32x16_t>(_5768);
       int64_t _5770 = (int64_t)(_output_multiplier);
       int64x16_t _5771 = int64x16_t::broadcast(_5770);
       int64x16_t _5772 = _5769 * _5771;
       int64_t _5773 = (int64_t)(ADD_INT64_T_SUFFIX(1073741824));
       int64x16_t _5774 = int64x16_t::broadcast(_5773);
       int64x16_t _5775 = _5772 + _5774;
       int64_t _5776 = (int64_t)(ADD_INT64_T_SUFFIX(31));
       int64x16_t _5777 = int64x16_t::broadcast(_5776);
       int64x16_t _5778 = _5775 >> _5777;
       int64x16_t _5779 = int64x16_t::min(_5778, _5744);
       int64x16_t _5780 = int64x16_t::max(_5779, _5742);
       int32x16_t _5781 = int32x16_t::convert_from<int64x16_t>(_5780);
       uint8x16_t _5782 = uint8x16_t::broadcast(_output_min);
       uint8x16_t _5783 = uint8x16_t::broadcast(_output_max);
       int32x16_t _5784 = int32x16_t::broadcast(0);
       int32x16_t _5785 = int32x16_t::broadcast(255);
       int32x16_t _5786 = int32x16_t::broadcast(1);
       uint8x16_t _5787 = _5781 < _5784;
       int32x16_t _5788 = int32x16_t::select(_5787, _5786, _5784);
       int32x16_t _5789 = int32x16_t::broadcast(_5591);
       int32x16_t _5790 = _5788 + _5789;
       int32x16_t _5791 = int32x16_t::broadcast(_5584);
       int32x16_t _5792 = _5781 & _5791;
       uint8x16_t _5793 = _5790 < _5792;
       int32x16_t _5794 = int32x16_t::select(_5793, _5786, _5784);
       uint32x16_t _5795 = uint32x16_t::broadcast(_5568);
       int32x16_t _5796 = _5781 >> _5795;
       int32x16_t _5797 = int32x16_t::broadcast(_output_offset);
       int32x16_t _5798 = _5796 + _5797;
       int32x16_t _5799 = _5794 + _5798;
       int32x16_t _5800 = int32x16_t::min(_5799, _5785);
       int32x16_t _5801 = int32x16_t::max(_5800, _5784);
       uint8x16_t _5802 = uint8x16_t::convert_from<int32x16_t>(_5801);
       uint8x16_t _5803 = uint8x16_t::min(_5802, _5783);
       uint8x16_t _5804 = uint8x16_t::max(_5803, _5782);
       int32_t _5805 = _5753 + _5593;
       _5804.store(_output, _5805);
       int64_t _5806 = (int64_t)(ADD_INT64_T_SUFFIX(-2147483648));
       int64x16_t _5807 = int64x16_t::broadcast(_5806);
       int64_t _5808 = (int64_t)(ADD_INT64_T_SUFFIX(2147483647));
       int64x16_t _5809 = int64x16_t::broadcast(_5808);
       int32_t _5810 = _output_s0_x_x - _5581;
       int32_t _5811 = _5810 * 16;
       int32_t _5812 = _5811 + 16;
       uint32x16_t _5813 = uint32x16_t::load(_column_sums_b, _5812);
       int32x16_t _5814 = int32x16_t::convert_from<uint32x16_t>(_5813);
       int32_t _5815 = (int32_t)(_5579);
       int32x16_t _5816 = int32x16_t::broadcast(_5815);
       int32x16_t _5817 = _5814 * _5816;
       int32_t _5818 = _output_s0_x_x * 16;
       int32x16_t _5819 = int32x16_t::load(_bias, _5818);
       int32_t _5820 = (int32_t)(_5577);
       int32_t _5821 = _5815 * _5820;
       int32_t _5822 = _5821 * _27;
       uint32_t _5823 = _row_sums_a[_5599];
       int32_t _5824 = (int32_t)(_5823);
       int32_t _5825 = _5820 * _5824;
       int32_t _5826 = _5822 + _5825;
       int32x16_t _5827 = int32x16_t::broadcast(_5826);
       int32x16_t _5828 = _5819 + _5827;
       int32x16_t _5829 = _5817 + _5828;
       int32_t _5830 = _5818 + _5604;
       uint32x16_t _5831 = uint32x16_t::load(_multiplied_no_offsets, _5830);
       int32x16_t _5832 = int32x16_t::convert_from<uint32x16_t>(_5831);
       int32x16_t _5833 = _5829 + _5832;
       int64x16_t _5834 = int64x16_t::convert_from<int32x16_t>(_5833);
       int64_t _5835 = (int64_t)(_output_multiplier);
       int64x16_t _5836 = int64x16_t::broadcast(_5835);
       int64x16_t _5837 = _5834 * _5836;
       int64_t _5838 = (int64_t)(ADD_INT64_T_SUFFIX(1073741824));
       int64x16_t _5839 = int64x16_t::broadcast(_5838);
       int64x16_t _5840 = _5837 + _5839;
       int64_t _5841 = (int64_t)(ADD_INT64_T_SUFFIX(31));
       int64x16_t _5842 = int64x16_t::broadcast(_5841);
       int64x16_t _5843 = _5840 >> _5842;
       int64x16_t _5844 = int64x16_t::min(_5843, _5809);
       int64x16_t _5845 = int64x16_t::max(_5844, _5807);
       int32x16_t _5846 = int32x16_t::convert_from<int64x16_t>(_5845);
       uint8x16_t _5847 = uint8x16_t::broadcast(_output_min);
       uint8x16_t _5848 = uint8x16_t::broadcast(_output_max);
       int32x16_t _5849 = int32x16_t::broadcast(0);
       int32x16_t _5850 = int32x16_t::broadcast(255);
       int32x16_t _5851 = int32x16_t::broadcast(1);
       uint8x16_t _5852 = _5846 < _5849;
       int32x16_t _5853 = int32x16_t::select(_5852, _5851, _5849);
       int32x16_t _5854 = int32x16_t::broadcast(_5591);
       int32x16_t _5855 = _5853 + _5854;
       int32x16_t _5856 = int32x16_t::broadcast(_5584);
       int32x16_t _5857 = _5846 & _5856;
       uint8x16_t _5858 = _5855 < _5857;
       int32x16_t _5859 = int32x16_t::select(_5858, _5851, _5849);
       uint32x16_t _5860 = uint32x16_t::broadcast(_5568);
       int32x16_t _5861 = _5846 >> _5860;
       int32x16_t _5862 = int32x16_t::broadcast(_output_offset);
       int32x16_t _5863 = _5861 + _5862;
       int32x16_t _5864 = _5859 + _5863;
       int32x16_t _5865 = int32x16_t::min(_5864, _5850);
       int32x16_t _5866 = int32x16_t::max(_5865, _5849);
       uint8x16_t _5867 = uint8x16_t::convert_from<int32x16_t>(_5866);
       uint8x16_t _5868 = uint8x16_t::min(_5867, _5848);
       uint8x16_t _5869 = uint8x16_t::max(_5868, _5847);
       int32_t _5870 = _5818 + _5592;
       _5869.store(_output, _5870);
      } // for _output_s0_x_x
     } // for _output_s0_y_y
     _column_sums_b_free.free();
     _multiplied_no_offsets_free.free();
     _row_sums_a_free.free();
    } // alloc _row_sums_a
   } // alloc _multiplied_no_offsets
  } // alloc _column_sums_b
 } // if _97
 return 0;
}

#ifdef __cplusplus
}  // extern "C"
#endif


#if !defined(SHARED_TYPES_H)
#    define  SHARED_TYPES_H

#if defined(__OPENCL_VERSION__)
#  define IS_CL_DEVICE 1
#else
#  define IS_CL_DEVICE 0
#endif

#define IS_CL_HOST (!IS_CL_DEVICE)


#if IS_CL_DEVICE
#  define CL_PREFIX(type) type
#else
#  include <gpucf/cl.h>
#  define CL_PREFIX(type) cl_ ## type
#endif

#define SHARED_INT   CL_PREFIX(int)
#define SHARED_FLOAT CL_PREFIX(float)


#endif //!defined(SHARED_TYPES_H)

// vim: set ts=4 sw=4 sts=4 expandtab:
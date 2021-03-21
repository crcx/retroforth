/* Add any global #define desired here */

#ifndef BIT64
#define CELL int32_t
#define CELL_MIN INT_MIN + 1
#define CELL_MAX INT_MAX - 1
#else
#define CELL int64_t
#define CELL_MIN LLONG_MIN + 1
#define CELL_MAX LLONG_MAX - 1
#endif

#ifndef IMAGE_SIZE
#define IMAGE_SIZE   524288       /* Amount of RAM, in cells           */
#endif

#ifndef ADDRESSES
#define ADDRESSES    256          /* Depth of address stack            */
#endif

#ifndef STACK_DEPTH
#define STACK_DEPTH  256          /* Depth of data stack               */
#endif

#define TIB            1025       /* Location of TIB                   */

#define D_OFFSET_LINK     0       /* Dictionary Format Info. Update if */
#define D_OFFSET_XT       1       /* you change the dictionary fields. */
#define D_OFFSET_CLASS    2
#define D_OFFSET_NAME     3

#ifdef ENABLE_SOCKETS
#define NUM_DEVICES      10
#else
#define NUM_DEVICES       9       /* Set the number of I/O devices     */
#endif

#define MAX_OPEN_FILES   32

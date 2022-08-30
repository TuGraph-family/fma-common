## Fma-common: utility library for efficient big data programs

Fma-common is a header-only library that implements utility functions for efficient big data programs. The functions are
designed with performance in mind. It uses C++11 and has no external dependencies.

You can refer to the test code in `/test/` for a quick start on how to use this library.

The functionalities can be devided into the following categories:

- [Misc Functions:](#misc-functions-) logging, argument parsing, atomic functions
- [Parallel Utilities:](#parallel-utilities-) concurrent queue, thread pool, pipeline
- [IO:](#io-) file access, text parsing, text writing, simulated RAID

### Misc Functions [^](#fma-common-utility-library-for-efficient-big-data-programs)

Header file      | Functionality
:-----------------|:----------------------------
arg_parse.h      | Command line argument parser
configuration.h  | Configuration loader
env.h            | Accessing environment variables
logging.h        | Logging and CHECK, ASSERT, etc.
string_util.h    | High performance ToString() and string utilities like Strip(), StartsWith()
type_traits.h    | Things like DISABLE_COPY, ENABLE_IF_VOID(T)
utils.h         | Atomic operations for floating point numbers and Sleep()

### Parallel Utilities [^](#fma-common-utility-library-for-efficient-big-data-programs)

Header file       |     Functionalities
 :-----------------|:----------------------------
bounded_queue.h   |  A concurrent queue with capacity limits
pipeline.h        | Pipeline stage that enables multi-thread execution and in-order/out-of-order retirement
thread_pool.h     | Thread pool

### IO [^](#fma-common-utility-library-for-efficient-big-data-programs)

Header file                 |     Functionalities
 :---------------------------|:----------------------------
binary_read_write_helper.h  | Serialize C++ objects into byte stream or deserialize byte stream into C++ objects
file_system.h               | Create/delete/list directory for both local and HDFS file system
[fma_stream.h](/docs/fma_stream.md)              | Uniform interface for accessing local and HDFS files
multi_disk_stream.h         | Storing file on multiple disks in strided manner to speed up IO, like a user-level RAID 0
[string_formatter.h](/docs/text_writer.md#stringformatter-and-textfileformatter-)          | Write data to string in C# style
text_dir_stream.h           | Read a text dir
[text_parser.h](/docs/text_parser.md)            | Parse text files
[text_writer.h](/docs/text_writer.md)            | Write text files
 

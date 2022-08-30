## FmaStream Manual

FmaStream provides uniform interface for reading and writing files in local and HDFS file systems.

- [Features](#features-)
- [Header File](#header-file-)
- [InputFmaStream](#inputfmastream-)
- [OutputFmaStream](#outputfmastream-)
- [Notes On Accessing HDFS Files](#notes-on-accessing-hdfs-files-)

### Features [^](#fmastream-manual)

* Uniform interface for local and HDFS
* Prefetching using background thread
* Supports snappy compression
* Small read/write operation optimization using a buffer

### Header File [^](#fmastream-manual)

The `InputFmaStream/OutputFmaStream` classes are implemented in `fma-common/fma_stream.h`.

### InputFmaStream [^](#fmastream-manual)

`InputFmaStream`  implements a read-only streaming file. It can be used to read file from local file system or HDFS file
system. Internally, it uses a prefetching thread to prefetch contents, so reading sequentially is very efficient.
However, if you need random access, you should disable the prefetching by passing `buf_size=0` to the constructor.

The class is declared as follows:

```c++
class InputFmaStream {
public:
    InputFmaStream();
    InputFmaStream(const std::string& path,  // path to file, use hdfs:/// prefix if HDFS file
        size_t buf_size = 64 << 20,          // buffer size
        bool snappy_compressed = false);     // enable snappy compression

    void Open(const std::string& path,       // open non-compressed file
              size_t buf_size = 64 << 20);
    void Open(const std::string& path,       // open file, possibly compressed
        size_t buf_size,
        bool snappy_compressed);
    void Close();                          // close file
    bool Seek(size_t offset);              // seek to given offset

    size_t Read(void* buf, size_t size);   // read data into buf
    bool Good() const;                     // is file opened correctly?
    size_t Size() const;                   // return size of file
    const std::string& Path() const;       // full path of the file
};

// example
InputFmaStream local_file("/path/to/local/file");
std::string buf(64<<20, 0);
size_t bytes_read = 0;
while(bytes_read = local_file.Read(&buf[0], buf.size()) {
    // bytes_read stores the number of bytes read from the file
}

InputFmaStream hdfs_file("hdfs:///path/to/hdfs/file");
while(bytes_read = hdfs_file.Read(&buf[0], buf.size()) {
    // bytes_read stores the number of bytes read from the file
}
```

### OutputFmaStream [^](#fmastream-manual)

`OutputFmaStream`  implements a write-only streaming file. It can be used to write file to local file system or HDFS
file system. Internally, it buffers the written contents and uses a thread to write the data when the buffer gets full.
So user thread does not wait for the IO, as long as the IO speed can catch up.

The class is declared as follows:

```c++
class OutputFmaStream {
public:
    OutputFmaStream() {}
    OutputFmaStream(const std::string& path,    // path to file, use hdfs:/// prefix if HDFS file
        size_t buf_size = 64 << 20,             // buffer size
        std::ofstream::openmode mode = std::ofstream::trunc, // open mode, trunc or append
        bool snappy_compressed = false,         // whether to use snappy compression
        size_t n_buffers = 1);                  // number of compression buffers to use

    void Open(const std::string& path,
        size_t buf_size = 64 << 20,
        std::ofstream::openmode mode = std::ofstream::trunc);
    void OpenSnappy(const std::string& path,
        size_t n_buffers = 1,
        size_t block_size = 64 << 20,
        std::ofstream::openmode mode = std::ofstream::trunc);
    void OpenNoSnappy(const std::string& path,
        size_t buf_size,
        std::ofstream::openmode mode);
    void Close();
    void Write(const void* buffer, size_t size);

    bool Good() const;
    size_t Size() const;
    const std::string& Path() const;
};

// example
OutputFmaStream local_file("/path/to/local/file");
std::string buffer(1024, 0);
local_file.Write(&buffer[0], buffer.size());

OutputFmaStream hdfs_file("hdfs:///path/to/hdfs/file");
hdfs_file.Write(&buffer[0], buffer.size());
```

### Notes On Accessing HDFS Files [^](#fmastream-manual)

We have two implementations to access HDFS files. One with LibHDFS and the other with pipes.

By default, the pipe-based implementation is used. It uses `hdfs dfs -cat` command and reads the output through a pipe.
The only requirement is that the `hdfs` command must be accesible in the `PATH` environment. If the `hdfs` command is
not accesible in the `PATH` environment, you need to specify the path in [predefs.h](/fma-common/predefs.h#L27). The
downside with this approach is that it takes more memory (around 1GB per file opened) and can fail if system runs very
low on memory.

The LibHDFS-based implementation can be switched on with the `HAS_LIBHDFS=1` macro, by adding `-DHAS_LIBHDFS=1` to your
compiler option. You can refer to [/test/Makefile](/test/Makefile#L13) on how to use it.

To use LibHDFS, you will need to:

1. Provide the correct LibHDFS header file and library in `fma-common/libhdfs`
2. Link with `libjvm.so` in $(JAVA_HOME)/jre/lib/amd64/server
3. Add `libhdfs.so` and `libjvm.so` to LD_LIBRARY_PATH
4. Add hadoop jar files to CLASSPATH. On Linux, you can do it with

```bash
    export CLASSPATH=$CLASSPATH:`hadoop classpath --glob`
```

On Windows, the length of CLASSPATH can easily exceed the maximum length supported by Windows (typically 4K). In that
case, please refer to `fma-common/libhdfs/classpath.txt` and add only those jars to your CLASSPATH. Note that the jars
may have different versions depending on the version of Hadoop. Please make sure the correct jar file path is given.


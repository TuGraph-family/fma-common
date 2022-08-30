## TextWriter Manual

Writing text files is pretty slow, either with fprintf() or std::fstream. In fma-common, we implement our own
high-performance text writers. In our experiments, we can write out integer and floating point values at over 200MB/s
with single thread, and over 1GB/s with multiple threads.

- [Features](#features-)
- [Header File](#header-file-)
- [Using Out-of-box TSV/CSV Writer](#using-out-of-box-tsvcsv-writer-)
- [Customizing Write Behavior With TextWriter](#customizing-write-behavior-with-textwriter-)
- [StringFormatter And TextFileFormatter](#stringformatter-and-textfileformatter-)

### Features [^](#textwriter-manual)

- High performance (>200MB/s with single thread)
- Supports multi-thread acceleration
- Out-of-box TSV/CSV support
- Customizable behavior

### Header File [^](#textwriter-manual)

TextWriter is implemented in `fma-common/text_writer.h`.

### Using Out-of-box TSV/CSV Writer [^](#textwriter-manual)

The easiest way to use TextWriter is to leverage the out-of-box TSV/CSV writer interface.

Assuming we have three vectors, namely `names`, `ages` and `scores` that we want to write out as a TSV file, in which
each vector is written as a column. The following code shows how to write the TSV file with the `WriteTsv()` function.

```c++
#include "fma-common/text_writer.h"

std::vector<std::string> names;
std::vector<int> ages;
std::vector<int> scores;

OutputFmaStream file("/path/to/tsv");
WriteTsv(file,     // file to write to
         4,        // number of threads to use
         65536,    // number of lines to write as a block
         names,    // Now list the vectors to write, every vector
         ages,     //   must have the same length.
         scores);
```

The `Write Tsv()` function template is declared as follows:

```c++
template<typename... T>
inline void WriteTsv(
               OutputFileStream& file, 
               int n_threads, 
               size_t chunk_size, 
               const std::vector<T>&... vs);
```

It takes no less than 4 arguments: the file stream to write to (`file`), the number of concurrent threads to
use (`n_threads`), the number of lines to write as a block (`chunk_size`), and the vectors to write.

Internally, the vector elements are converted into strings using `fma-common::ToString()`, and then stiched together to
form the lines. The lines are then written to the file in order. To speed up the process, multiple threads can be used
to convert the lines, by specifying `n_threads >= 2`. In this case, each thread will process  `chunk_size` lines and
then push it to a dedicated writer thread that writes the lines in order.

Similarly, the `WriteCsv()` function writes vectors as CSV file.

----------
*HINT: n_threads = 4, chunk_size = 65536 is usually a good choice for most cases.*

----------

### Customizing Write Behavior With TextWriter [^](#textwriter-manual)

The `WriteTsv/WriteCsv` interface is easy to use, but a bit restricted. If you want to customize the writing behavior,
you can use the `TextWriter` class directly.

The `TextWriter` template class is declared as follows:

```c++
template<typename F, typename... T>
class TextWriter {
public:
    TextWriter(OutputFileStream& out,       // file to write to
            const F& write_line = F(),      // function to write one line
            int n_writers = 1,              // number of concurrent threads
            size_t chunk_size = 65536);     // size of each concurrent chunk
            
    void Write(size_t n, const T*... data);    // write one line of data
    void Write(const std::vector<T>&... data); // write vectors
};
```

It takes at least two template arguments:

- `F`: The function type used to write one line.
- `T`: The types of the columns.

`F` must be of type `void func(std::string& buf, const T&... data)`. It accepts a list of elements, converts the
elements into one line, and append the line at the end of the string `buf`.

Suppose we want to write a list of edges, each of which has an integer type, which must be converted to string type
with `const std::string& GetEdgeTypeString(int)`. Here is the code to achieve this goal:

```c++
#include "fma-common/text_writer.h"

struct Edge {
    int64_t srcId;
    int64_t dstId;
};

void WriteOneEdge(std::string& buf, const Edge& edge, const int& type) {
	buf += ToString(edge.srcId);  // write source id
	buf += "\t";
	buf += ToString(edge.dstId);  // write destination id
	buf += "\t";
	buf += GetVertexType(type);   // write edge type string
	buf += "\n";                  // end the line
}

std::vector<Edge> edges;
std::vector<int> edge_types;

OutputFmaStream file("/path/to/tsv");
TextWriter<decltype(WriteOneEdge), Edge, int> EdgeWriter(file, WriteOneEdge, 4, 65536);
EdgeWriter.Write(edges, edge_types);  // write all edges with 4 threads
Edge edge = {1, 2}; int edge_type = 0;
EdgeWriter.Write(edge, edge_type);    // write a single edge with single thread
```

### StringFormatter And TextFileFormatter [^](#textwriter-manual)

Writing the `WriteOneEdge()` function can be cumbersome. So we offer you the `StringFormatter` and `TextFileFormatter`.
They enable users to write code like this:

```c++
writer.Write(file, "The first edge is from {} to {}, of type {}", 1, 2, 0);
writer.Write(file, "The second edge if from {} to {}, with type {}", 3, 4, 2);
...
```

The `StringFormatter` prints data into string buffer. It is implemented in `fma-common/string_formatter.h` and has a
declaration as follows:

```c++
class StringFormatter {
public:
    template<typename... Ts>
    static const std::string& Append(std::string& buf, const char* format, const Ts&... data);
    
    template<typename... Ts>
    static const std::string& Format(std::string& buf, const char* format, const Ts&... data);
    
    template<typename... Ts>
    static std::string Format(const char* format, const Ts&... data);
};

// here are some examples:
std::string buf;
OutputFmaStream file;
int a = 200, b = 100;
file.Write(StringFormatter::Format(buf, "Here is the first test: {} * {} = {}", a, b, a + b));
file.Write(StringFormatter::Format(buf, "Here is the second test: {} - {} = {}", a, b, a * b));	// reuses buf, so no memory allocation will happen
```

The first two methods returns a const reference to buf. `Append` appends the formatted string to buf, while `Format`
overwrites the contents in buf. The last method returns a new string.

The `TextFileFormatter` provides similar interface to `StringFormatter`. The only difference is it keeps a string buffer
internally to avoid unnecessary malloc:

```c++
class TextFileFormatter {
    std::string buf_;
public:
    template<typename... Ts>
    size_t Write(OutputFileStream& out, const char* format, const Ts&... data);
};

// exmaple
TextFileFormatter formatter;
formatter.Write(file, "{} {} {}", a, b, c);
formatter.Write(file2, "Formatted string: {} + {} = {}", c, d, e);
```

----------
**NOTE: StringFormatter and TextFileFormatter writes line by line. They are not accelerated with multiple threads. If
you need to use multiple threads to accelerate the output, use TextWriter or use multiple threads to write multiple
files in parallel.**

----------




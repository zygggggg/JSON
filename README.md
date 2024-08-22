# LeptJSON JSON 解析器项目文档
## 概览
LeptJSON 是一个轻量级且高效的 JSON 解析器，用 C 语言实现。它旨在简单易用且速度快。该库提供了基本的功能来解析 JSON 字符串为结构化的格式，便于后续处理和查询。

## 主要功能
JSON 解析：能够解析 JSON 字符串到内部数据结构。
错误处理：支持详细的错误报告。
内存管理：动态分配和释放内存。
字符串编码：支持 UTF-8 编码的字符串。
数组和对象处理：支持解析 JSON 数组和对象。
## 架构设计
LeptJSON 的架构模块化且简单：

头文件 (leptjson.h)：

定义了用于表示 JSON 值的类型。
声明了主要函数和错误码。
实现文件 (leptjson.c)：

包含了 JSON 解析的核心逻辑。
利用了辅助函数来解析特定的 JSON 值，如 true、false 和 null。
实现了跳过空白字符以及错误检查机制。
内存管理：

使用栈来存储解析过程中的临时数据。
动态分配和释放内存以存储解析结果。
解析流程：

lept_parse：解析 JSON 的主函数。初始化上下文，调用解析逻辑并处理错误。
lept_parse_value：根据当前字符确定解析哪种类型的 JSON 值。
lept_parse_literal, lept_parse_number, lept_parse_string, lept_parse_array, lept_parse_object：具体解析函数。
关键实现细节
栈的使用：

lept_context 结构体：维护了一个动态增长的栈，用于存储解析过程中产生的临时数据。
lept_context_push 和 lept_context_pop：提供了向栈中压入和弹出数据的方法。
栈的动态扩展：当栈空间不足时，会自动扩展栈的大小。
字符串解析：

支持 UTF-8 编码的字符串。
使用 lept_encode_utf8 函数将 Unicode 转换为 UTF-8 编码。
数组和对象解析：

数组解析：通过递归解析每个元素。
对象解析：同样递归解析每个成员，并处理键值对。
错误处理：

使用宏 STRING_ERROR 等来快速返回错误码。
在解析失败时，确保清理已分配的内存。
内存管理：

使用 malloc 和 free 进行内存分配和释放。
对于字符串和数组，会在解析完成后复制数据到最终位置。

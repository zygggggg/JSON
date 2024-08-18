这是zygg完成json解析器的历程 做这个c语言项目以巩固数据结构知识 \n
LeptJSON JSON 解析器项目文档
概览
LeptJSON 是一个轻量级且高效的 JSON 解析器，用 C 语言实现。它旨在简单易用且速度快。该库提供了基本的功能来解析 JSON 字符串为结构化的格式，便于后续处理和查询。

主要特性
简单的 API：提供了简洁的接口来解析 JSON。
高效的解析：优化了速度和最小内存使用。
错误处理：返回详细的错误码以帮助诊断解析问题。
可移植性强：用纯 C 语言编写，使其在不同平台上高度可移植。
架构设计
LeptJSON 的架构模块化且简单：

头文件 (leptjson.h)：

定义了用于表示 JSON 值的类型。
声明了主要函数和错误码。
实现文件 (leptjson.c)：

包含了 JSON 解析的核心逻辑。
利用了辅助函数来解析特定的 JSON 值，如 true、false 和 null。
实现了跳过空白字符以及错误检查机制。
关键组件
lept_value 结构体：

表示一个解析后的 JSON 值。
包含了一个字段 type 来指示 JSON 值的类型。
lept_context 结构体：

用来追踪 JSON 字符串中的当前位置。
解析函数：

lept_parse：解析 JSON 的主函数。初始化上下文，调用解析逻辑并处理错误。
lept_parse_true, lept_parse_false, lept_parse_null：辅助函数，用于解析布尔值和 null。
lept_parse_value：确定下一个 JSON 值的类型，并调用相应的解析函数。
工具函数：

lept_parse_whitespace：跳过空白字符。
EXPECT 宏：断言下一个字符与预期匹配。

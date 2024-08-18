这是zygg完成json解析器的历程 做这个c语言项目以巩固数据结构知识
JSON（JavaScript Object Notation）是一个用于数据交换的文本格式，现时的标准为ECMA-404。
虽然 JSON 源至于 JavaScript 语言，但它只是一种数据格式，可用于任何编程语言。现时具类似功能的格式有 XML、YAML，当中以 JSON 的语法最为简单。
例如，一个动态网页想从服务器获得数据时，服务器从数据库查找数据，然后把数据转换成 JSON 文本格式：


{
    "title": "Design Patterns",
    "subtitle": "Elements of Reusable Object-Oriented Software",
    "author": [
        "Erich Gamma",
        "Richard Helm",
        "Ralph Johnson",
        "John Vlissides"
    ],
    "year": 2009,
    "weight": 1.8,
    "hardcover": true,
    "publisher": {
        "Company": "Pearson Education",
        "Country": "India"
    },
    "website": null
}
网页的脚本代码就可以把此 JSON 文本解析为内部的数据结构去使用。

从此例子可看出，JSON 是树状结构，而 JSON 只包含 6 种数据类型：

null: 表示为 null
boolean: 表示为 true 或 false
number: 一般的浮点数表示方式，在下一单元详细说明
string: 表示为 "..."
array: 表示为 [ ... ]
object: 表示为 { ... }
我们要实现的 JSON 库，主要是完成 3 个需求：

把 JSON 文本解析为一个树状数据结构（parse）。
提供接口访问该数据结构（access）。
把数据结构转换成 JSON 文本（stringify）。

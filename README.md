# TinyJsonParser

TinyJsonParser, inspired by both Jan Dugacek's blog[Writing a JSON library in two and half hour, in C++](https://kohutek.eu/open-source/writing-a-json-library-in-two-and-half-hour-in-cpp/) and Milo Yip's [json-tutorial](https://github.com/miloyip/json-tutorial)。

I write this to make a useful tool to fly with JSON format. It's a widely-used and user-friendly format for serialization
and deserializaiton. There are many similar tools among the Github but since I don't want to introduce addtional dependencies into my projects,
and another reason is that parsing JSON is a relatively easy work so I decided to **remaking** a wheel.

# Usage

code in `pdjsontest.cc` shows how it works.
Shared_ptr is heavily used here so there are alot of `std::make_shared<xxx>` redundance. That's a trade-off for memory-safety.
Raw-pointers will make the interface more convenient, more friendly but it may causing crash or memory-leaking when you  wrongly freed pointer from the JSON struct.

# contribution

Any pull request and issue are welcomed!

# License

See **MIT** License.
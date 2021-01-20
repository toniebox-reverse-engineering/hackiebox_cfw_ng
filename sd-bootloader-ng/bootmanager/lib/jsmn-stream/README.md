# jsmn-stream
A streaming JSON parser based on [jsmn](https://github.com/zserge/jsmn) by Serge Zaitsev.
The code has been rewritten to take an input stream character by character and
emit events to user supplied callbacks. Such a parsing scheme is especially
beneficial on embedded systems where the whole parse tree (and possibly the
whole JSON string) cannot be stored in RAM at once.

## Examples
See the [examples](examples) folder.

## License
Like the original jsmn project, this one is licensed under the MIT license.


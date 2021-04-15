# The Drift Programming Language

<p align="center" style="font-weight: bold;">Refined, minimalist, modern, beautiful</p>

- Fourteen keywords
- Twelve object types
- Bytecode virtual machine

Basic grammatical format of drift, please check the instance code `awesome.ft`

## Build

- switch CC field in Makefile and specify your g++ compiler path
- my gcc version is 10.2.0

```
make
make install
```

### To install:

    export PATH=$PWD:$PATH

### To run:
 
    drift                 # REPL MODE
    drift <ft file>	      # FILE MODE

    drift -d              # REPL AND DEBUG MODE
    drift <ft file> -d    # FILE AND DEBUG MODE

    drift -b              # REPL AND DIS BYTECODE
    drift <ft file> -b    # FILE AND DIS BYTECODE

### To clean:

    make clean

## Collaborative development

- Lack of FFI support
- Syntax highlighting support for Virtual Studio Code, `tool` directory
- Standard library and bug testing, etc

## More

For documentation and project architecuture,
visit the official website https://drift-lang.fun/

**It's a toy language, Have Fun!!**

## License
```
Copyright (c) 2021 bingxio（丙杺，黄菁）. All rights reserved.

GNU General Public License, more to see file: LICENSE
https://www.gnu.org/licenses

          THE DRIFT PROGRAMMING LANGUAGE

          https://github.com/bingxio/drift

          https://www.drift-lang.fun/
```
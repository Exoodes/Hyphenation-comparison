# Hyphenation-comparison
This repository is part of my Bachelor thesis [`Judy`](https://sourceforge.net/projects/judy/). 
It contains 2 different programs. 
The first one is called `compare`, which compares Judy data structure and Trie data structure based on hyphenating words with hyphenation patterns.
And second on called the `hyphenator`, which loads patterns and then hyphenates words from the file or terminal input.

## Installation
Needed prerequisites 
- GNU coreutils
- make
- Judy library - download from [here](https://sourceforge.net/projects/judy/) 
- Cprops library - download from [here](https://sourceforge.net/projects/cprops/files/)

Then run `make`

## Usage
- `make run-tests` to run all test
- `make time-test` to run only time complexity testing
- `make memory-test` to run only space complexity testing
- `make hyphenator` crate hyphenator program

### Hyphenator usage
- The first arguments must be  options
    - `-lx` where x can be an arbitrary number higher than 0, it sets `left_hyphen_min` for hyphenating process
    - `-rx` where x can be an arbitrary number higher than 0, it sets `right_hyphen_min` for hyphenating process
    - `-f `file_path` option specifies a file with words to be hyphenated, if not specified, words from terminal will be hyphenated
- After the arguments must be a file with only patterns
- Example of usage for hyphenation from file `./hyphenator -l2 -r2 -f assets/thai_words.dic assets/thai_patterns.tex` or from terminal `./hyphenator -l2 -r2 assets/thai_patterns.tex`
- When hyphenating from the terminal, some commands can be used
    - `:q` Ends the hyphenator program
    - `:lx` Sets the `left_hyphen_min` to number `x`
    - `:rx` Sets the `right_hyphen_min` to number `x`
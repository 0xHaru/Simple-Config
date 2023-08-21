# Simple-Config

A parser for a simple configuration file format.

To use this library you just need to add `config.c` and `config.h` to your project.

## Example

```
# A sample config file for a generic text editor
font: "JetBrainsMono Nerd Font"
font.size: 14
zoom: 1.5
line_numbers: true
bg.color: rgba(255, 255, 255, 1)
```

```c
int
main(void)
{
    CfgError err;
    CfgEntry *entries = malloc(64 * sizeof(CfgEntry));
    Cfg cfg = {.entries = entries, .capacity = 64};

    int res = cfg_parse_file("sample.cfg", &cfg, &err);

    if (res != 0) {
        cfg_fprint_error(stderr, &err);
        free(entries);
        return 1;
    }

    // Default value         --------------------+
    // (if key is not found)                     |
    //                                           v
    char* font = cfg_get_string(&cfg, "font", "Noto Sans Mono");
    int font_size = cfg_get_int(&cfg, "font.size", 12);
    float zoom = cfg_get_float(&cfg, "zoom", 1.0);
    bool line_num = cfg_get_bool(&cfg, "line_numbers", true);
    CfgColor bg = cfg_get_color(&cfg, "bg.color",
                                      (CfgColor){
                                          .r = 255,
                                          .g = 255,
                                          .b = 255,
                                          .a = 1,
                                      });

    free(entries);
    return 0;
}
```

A fully working example can be found in `example.c`, to build it just run `make`.

## Implementations

The program has two implementations:

-   The [first](https://github.com/0xHaru/Simple-Config/tree/master) one uses static arrays to store keys and strings. For this reason, a maximum length must be defined at compile time.

-   The [second](https://github.com/0xHaru/Simple-Config/tree/arena) one uses a [memory arena](https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator) to address this limitation.

Another viable strategy might be to represent keys and strings as pointers to "slices" within the source string.

## Specification

-   A config file must have the `.cfg` file extension

-   A config file consists of zero or more lines

-   A line is composed of a key, a colon (`:`), a value, and a newline (`\n`)

-   A key is a sequence of one or more alphabetic characters, dots (`.`) or underscores (`_`)

-   A value can be one of the following types: string, boolean, integer, float, or color

-   A comment starts with a `#` and can placed on its own line or at the end of an entry

## Data types

-   **String**: A string is enclosed within double quotes (`"`) and can contain alphabetic characters, punctuation, digits, and blanks (spaces or tabs). Double quotes are not allowed inside a string.

-   **Boolean**: A boolean is either `true` or `false`

-   **Integer**: An integer is a sequence of digits, which may be preceded by a hyphen (`-`) to indicate a negative value

-   **Float**: A floating-point number consists of a sequence of digits with a decimal point (`.`), which may be preceded by a hyphen (`-`) to indicate a negative value

-   **Color**: A color is defined in the `rgba(R, G, B, A)` format, where `R`, `G`, and `B` are integers between 0 and 255, and `A` is a floating-point number between 0 and 1

## EBNF grammar

```
cfg    ::= line*
line   ::= key ':' val '\n'
key    ::= (alpha | '.' | '_')+
val    ::= string | bool | int | float | color

string ::= '"' (alpha | punct | digit | blank)+ '"'
alpha  ::= 'a' ... 'z' | 'A' ... 'Z'
punct  ::= '.' | ':' | '~' | '!' | ...
blank  ::= ' ' | '\t'

bool   ::= true | false

int    ::= '-'? digit+
float  ::= '-'? digit+ '.' digit+
digit  ::= '0' ... '9'

color  ::= 'rgba(' digit+ ',' digit+ ',' digit+ ',' digit+ ')'
```

## Acknowledgements

This project was built as a collaborative work by [Haru](https://github.com/0xHaru) and [Cozis](https://github.com/cozis).

## License

This project uses the GPLv3 license.

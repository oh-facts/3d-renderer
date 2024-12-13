## style guide

This is created so I stick to something. This is biased because I created this. Open issue to discuss.

### Conventions

```.c
typedef struct NAMESPACE_StructName NAMESPACE_StructName
struct NAMESPACE_StructName
{

};
```

```.c
function void namespace_functionName();
```

Note: Code inside `base/` is not name spaced.

#### Macros
```.c
#define CONSTANT_FOO (4)
```

```.c
#define doesSomethingLikeAFunction()
```

#### brackets
```.c
switch(type)
{
    case type_a:
    {
        printf("The type is a on this one");
    }break;
    case type_b:
    {
        printf("The type is b on this one");
    }break;

    default:
    {
        printf("WUHAAAAT");
    }break;
}
```
```
if(something)
{
    // 1 or more lines
}
else
{
 
}
```
```
function void foo()
{

}
```

### Misc
- Don't break lines if its too long. Use linewrap.
- NOTE(username), TODO(username) for annotations.
- comment as appropriate.
- No gratituous functions.
- Prefix functions with `function`. It is a typedef for `static`.
- don't use const.
- Common lsps usually don't work.
- tabs over spaces
- don't early return
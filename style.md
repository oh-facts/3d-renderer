## style guide

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
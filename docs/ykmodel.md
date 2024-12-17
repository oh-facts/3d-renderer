# YKModel format

## Header
magic bytes  | 1 | 
version      | 1 |
first        | 4 |
last         | 4 |
next         | 4 |
prev         | 4 |
count        | 4 |
meshes num   | 8 |
mesh array   |   |

## Mesh
vertices num    | 4 |
vertex array    |   |
indices num     | 4 |
index array     |   |
primitive num   | 4 |
primitive array |   |

## Vertex
pos         | 12 |
uv x        | 4  |
normal      | 12 |
uv y        | 4  |
color       | 16 |
tangent     | 12 |
pad         | 4  |

## Primitive
start         | 4  |
count         | 4  |
material path |    |
local pos     | 12 |
local rot     | 16 |
local scale   | 12 |

##  YK Material
color        | 12
diffuse map  |
normal map   |

# YKMaterial format
# Simple3DS v0.1
A simple (and incomplete) .3ds model parser by davideesk

## 3D-Studio File Format spec
http://www.martinreddy.net/gfx/3d/3DS.spec
http://www.martinreddy.net/gfx/3d/MLI.spec

## Parsing
For objects it can parse faces, vertices, and texture coordinates. Normals are not part of the .3ds file spec, so those are generated at run-time.

For materials it only parses the material's name and filename.

## Example Usage

``` c++
#include "Simple3DS.hpp"

/* Somewhere in your code... */
Simple3DS::Model3DS model("Test.3ds", true); // second argument determines if the Y & Z axis should be swapped.

std::cout << "----- Objects -----" << std::endl;
for (size_t i = 0; i < model.data.getNumOfObjects(); i++) {
  Simple3DS::Object* obj = model.data.getObject(i);
  std::cout << "- Object " << i << " -" << std::endl;
  std::cout << "#Triangles: " << obj->getNumOfTriangles() << std::endl;
  std::cout << "#Vertices:  " << obj->getNumOfVertices() << std::endl;
  std::cout << "#UVCoords:  " << obj->getNumOfTexCoords() << std::endl;
  std::cout << "#Normals:   " << obj->getNumOfNormals() << std::endl;
  for (size_t j = 0; j < obj->getNumOfTriangles(); j++) {
    Simple3DS::Triangle* tri = obj->getTriangle(j);
    std::cout << "- Triangle " << j << " -" << std::endl;
    Simple3DS::Vertex* v1 = obj->getVertex(tri->index[0]);
    Simple3DS::Vertex* v2 = obj->getVertex(tri->index[1]);
    Simple3DS::Vertex* v3 = obj->getVertex(tri->index[2]);
    std::cout << "v1: [" << v1->x << ", " << v1->y << ", " << v1->z << "]" << std::endl;
    std::cout << "v2: [" << v2->x << ", " << v2->y << ", " << v2->z << "]" << std::endl;
    std::cout << "v3: [" << v3->x << ", " << v3->y << ", " << v3->z << "]" << std::endl;
    Simple3DS::TextureCoord* tc1 = obj->getTextureCoord(tri->index[0]);
    Simple3DS::TextureCoord* tc2 = obj->getTextureCoord(tri->index[1]);
    Simple3DS::TextureCoord* tc3 = obj->getTextureCoord(tri->index[2]);
    std::cout << "tc1: [" << tc1->u << ", " << tc1->v << "]" << std::endl;
    std::cout << "tc2: [" << tc2->u << ", " << tc2->v << "]" << std::endl;
    std::cout << "tc3: [" << tc3->u << ", " << tc3->v << "]" << std::endl;
    Simple3DS::Normal* n1 = obj->getNormal(tri->index[0]);
    Simple3DS::Normal* n2 = obj->getNormal(tri->index[1]);
    Simple3DS::Normal* n3 = obj->getNormal(tri->index[2]);
    std::cout << "n1: [" << n1->nx << ", " << n1->ny << ", " << n1->nz << "]" << std::endl;
    std::cout << "n2: [" << n2->nx << ", " << n2->ny << ", " << n2->nz << "]" << std::endl;
    std::cout << "n3: [" << n3->nx << ", " << n3->ny << ", " << n3->nz << "]" << std::endl;
  }
}
std::cout << "---- Materials ----" << std::endl;
for (size_t i = 0; i < model.data.getNumOfMaterials(); i++) {
  Simple3DS::Material* mat = model.data.getMaterial(i);
  std::cout << "- Material " << i << " -" << std::endl;
  std::cout << mat->getName() << ", " << mat->getFileName() << std::endl;
}
std::cout << "-------------------" << std::endl;
```

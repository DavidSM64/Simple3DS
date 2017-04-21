#pragma once
#include <fstream>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

#define PRIMARY       0x4D4D				// Primary Chunk, at the beginning of each file
#define OBJECTINFO    0x3D3D				// This gives the version of the mesh and is found right before the material and object information
#define MESHVERSION   0x3D3E
#define VERSION       0x0002				// This gives the version of the .3ds file
#define KEYFRAMES     0xB000				// This is the header for all of the key frame info
#define MATERIAL	  0xAFFF				// This stored the texture info
#define OBJECT		  0x4000				// This stores the faces, vertices, etc...
#define MATNAME       0xA000				// This holds the material name
#define MATDIFFUSE    0xA020				// This holds the color of the object/material
#define MATTRANSP     0xA050				// Material Transparency
#define MATMAP        0xA200				// This is a header for a new material
#define MATOPACITY    0xA210				
#define MATMAPFILE    0xA300				// This holds the file name of the texture
#define MATMAPPARAM	  0xA351				// Mapping Parameters
#define OBJECT_MESH   0x4100				// This lets us know that we are reading a new object
#define OBJECT_VERTICES     0x4110			// The objects vertices
#define OBJECT_FACES		0x4120			// The objects faces
#define OBJECT_MATERIAL		0x4130			// This is found if the object has a material, either texture map or color
#define OBJECT_UV			0x4140			// The UV texture coordinates
#define OBJECT_SMOOTH_GRP	0x4150			// The Smooth group of the mesh
#define OBJECT_LOCAL_COORD	0x4160
#define OBJECT_VISIBLE		0x4165

namespace Simple3DS {
	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	typedef signed char s8;
	typedef signed short s16;
	typedef signed int s32;

	typedef struct _Triangle {
		u16 index[3]; // Indicies
	} Triangle;

	typedef struct _Vertex {
		float x, y, z;
	} Vertex;

	typedef struct _TextureCoord {
		float u, v; 
	} TextureCoord;

	typedef struct _Normal {
		float nx, ny, nz; // (Generated)
	} Normal;

	class Object {
	private:
		vector<Triangle*> triangles;
		vector<Vertex*> vertices;
		vector<TextureCoord*> texuvs;
		vector<Normal*> normals;
	public:
		inline void addTriangle(Triangle* tri) { triangles.push_back(tri); }
		inline void addVertex(Vertex* vert) { vertices.push_back(vert); }
		inline void addTextureCoord(TextureCoord* tc) { texuvs.push_back(tc); }
		inline void addNormal(Normal* nrm) { normals.push_back(nrm); }
		inline void resizeNormals(size_t newSize) { normals.resize(newSize); }
		inline Triangle* getTriangle(int index) { return triangles[index]; }
		inline Vertex* getVertex(int index) { return vertices[index]; }
		inline TextureCoord* getTextureCoord(int index) { return texuvs[index]; }
		inline Normal* getNormal(int index) { return normals[index]; }
		inline size_t getNumOfTriangles() { return triangles.size(); };
		inline size_t getNumOfVertices() { return vertices.size(); };
		inline size_t getNumOfTexCoords() { return texuvs.size(); };
		inline size_t getNumOfNormals() { return normals.size(); };
		inline ~Object() {
			for (size_t i = 0; i < triangles.size(); i++)
				delete triangles[i];
			for (size_t i = 0; i < vertices.size(); i++)
				delete vertices[i];
			for (size_t i = 0; i < texuvs.size(); i++)
				delete texuvs[i];
			for (size_t i = 0; i < normals.size(); i++)
				delete normals[i];
		}
	};

	class Material {
	private:
		string name;
		string filename;
	public:
		inline string getName() { return name; };
		inline string getFileName() { return filename; };
		inline void setName(string n) { name = n; };
		inline void setFileName(string fn) { filename = fn; };
	};

	class ModelData {
	private:
		vector<Object*> objects;
		vector<Material*> materials;
	public:
		bool swapUpAxis = false;
		inline void addObject(Object* obj) { objects.push_back(obj); }
		inline void addMaterial(Material* mat) { materials.push_back(mat); }
		inline Object* getObject(int index) { return objects[index]; }
		inline Material* getMaterial(int index) { return materials[index]; }
		inline size_t getNumOfObjects() { return objects.size(); };
		inline size_t getNumOfMaterials() { return materials.size(); };
		inline ~ModelData() {
			for (size_t i = 0; i < objects.size(); i++)
				delete objects[i];
			for (size_t i = 0; i < materials.size(); i++)
				delete materials[i];
		}
	};

	class Chunk {
	protected:
		u16 id;
		u32 len;
		streampos filePosition;
		vector<Chunk*> children;
		Chunk* parent;
		string storedName;

		vector<int> hasSubNodes {
			OBJECTINFO, OBJECT, MATERIAL, OBJECT_MESH,
			OBJECT_FACES, MATMAP, KEYFRAMES
		};
		inline string parse_string(ifstream& ifs) {
			char c = ' ';
			string str;
			while (c != 0) {
				ifs.read(&c, 1);
				if (c != 0) str += c;
			}
			return str;
		}
		inline int parse_OBJECT(Chunk* child, ifstream& ifs, ModelData& data) {
			data.addObject(new Object());
			child->storedName = parse_string(ifs);
			return child->storedName.size()+1;
		}
		inline int parse_OBJECT_FACES(Chunk* child, ifstream& ifs, ModelData& data) {
			u16 numFaces;
			Object* obj = data.getObject(data.getNumOfObjects()-1);
			ifs.read((char*)&numFaces, 2);
			for (size_t i = 0; i < numFaces; i++) {
				Triangle* tri = new Triangle;
				u16 v;
				ifs.read((char*)&tri->index[0], 2); // First Face Index
				ifs.read((char*)&tri->index[1], 2); // Second Face Index
				ifs.read((char*)&tri->index[2], 2); // Third Face Index
				ifs.read((char*)&v, 2);				// Some visibilty flag (Not needed)
				obj->addTriangle(tri);
			}
			return 2 + numFaces*8;
		}
		inline void parse_OBJECT_VERTICES(Chunk* child, ifstream& ifs, ModelData& data) {
			u16 numVerts;
			Object* obj = data.getObject(data.getNumOfObjects() - 1);
			ifs.read((char*)&numVerts, 2);
			for (size_t i = 0; i < numVerts; i++) {
				Vertex* v = new Vertex;
				ifs.read((char*)&v->x, 4);		// x position of vertex
				if (!data.swapUpAxis) {			// Pass in as normal values.
					ifs.read((char*)&v->y, 4);	// y position of vertex
					ifs.read((char*)&v->z, 4);	// z position of vertex
				} else {						// Need to swap the Y & Z positions.
					ifs.read((char*)&v->z, 4);	// y position of vertex
					ifs.read((char*)&v->y, 4);	// z position of vertex
					v->z = -v->z;				// Negate z value
				}
				obj->addVertex(v);
			}
		}
		inline void parse_OBJECT_UV(Chunk* child, ifstream& ifs, ModelData& data) {
			u16 numTexCords;
			Object* obj = data.getObject(data.getNumOfObjects() - 1);
			ifs.read((char*)&numTexCords, 2);
			for (size_t i = 0; i < numTexCords; i++) {
				TextureCoord* tc = new TextureCoord;
				ifs.read((char*)&tc->u, 4);
				ifs.read((char*)&tc->v, 4);
				obj->addTextureCoord(tc);
			}
		}
		inline int parse_MATMAP(Chunk* child, ifstream& ifs, ModelData& data) {
			u16 a, b, c, d;
			ifs.read((char*)&a, 2);
			ifs.read((char*)&b, 2);
			ifs.read((char*)&c, 2);
			ifs.read((char*)&d, 2);
			return 8;
		}
	public:
		inline ~Chunk() {
			for (size_t i = 0; i < children.size(); i++)
				delete children[i];
		}
		inline void read(ifstream& ifs) {
			filePosition = ifs.tellg();
			ifs.read((char*)&id, 2);
			ifs.read((char*)&len, 4);
		}
		inline bool checkIfHasSubNodes(u16 id) {
			for (size_t i = 0; i < hasSubNodes.size(); i++) 
				if(id == hasSubNodes[i]) 
					return true;
			return false;
		}
		inline void readChildren(ifstream& ifs, int readLeft, ModelData& data) {
			while (readLeft > 0) {
				Chunk* child = new Chunk();
				child->parent = this;
				child->read(ifs);
				if (checkIfHasSubNodes(child->id)) {
					int readLeftChild = child->len - 6;
					switch (child->id) {
						case OBJECT:			readLeftChild -= parse_OBJECT(child, ifs, data); break;
						case OBJECT_FACES:		readLeftChild -= parse_OBJECT_FACES(child, ifs, data); break;
						case MATMAP:			readLeftChild -= parse_MATMAP(child, ifs, data); break;
						case MATERIAL:			data.addMaterial(new Material()); break;
					}
					child->readChildren(ifs, readLeftChild, data);
				} else {
					switch (child->id) {
						case OBJECT_VERTICES:	parse_OBJECT_VERTICES(child, ifs, data); break;
						case OBJECT_UV:			parse_OBJECT_UV(child, ifs, data); break;
						case MATNAME:
							data.getMaterial(data.getNumOfMaterials() - 1)->setName(parse_string(ifs)); break;
						case MATMAPFILE:		
							if(this->id == MATMAP)
								data.getMaterial(data.getNumOfMaterials() - 1)->setFileName(parse_string(ifs)); break;
					}
				}
				ifs.seekg((int)child->filePosition + (child->len), ios::beg);
				readLeft -= child->len;
				children.push_back(child);
			}
		}
		inline u16 getId() { return id; }
		inline u16 getLength() { return len; }
		inline size_t getNumberOfChildren() { return children.size(); }
		inline string to_string() {
			stringstream ss;
			ss << hex << getName() << uppercase << ": " << filePosition <<" [" << getId() << ", " << getLength() << "]" << dec;
			if (children.size() > 0) {
				ss << " {" << endl;
				for (size_t i = 0; i < children.size(); i++) {
					ss << children[i]->to_string() << endl;
				}
				ss << "}";
			}
			return ss.str();
		}
		inline string getName() {
			switch (id) {
				case PRIMARY:				return "Root";
				case VERSION:				return "Version";
				case OBJECTINFO:			return "Object Info";
				case OBJECT_MESH:			return "Object Mesh";
				case KEYFRAMES:				return "Keyframes";
				case MATERIAL:				return "Material";
				case OBJECT:				return "Object";
				case MESHVERSION:			return "Mesh Version";
				case OBJECT_VERTICES:		return "Object Vertices";
				case OBJECT_FACES:			return "Object Faces";
				case OBJECT_MATERIAL:		return "Object Materials";
				case OBJECT_UV:				return "Object UV";
				case OBJECT_SMOOTH_GRP:		return "Smooth Group";
				case OBJECT_LOCAL_COORD:	return "Local Coordinates";
				case OBJECT_VISIBLE:		return "Object Visibilty?";
				case MATNAME:				return "Mat Name";
				case MATDIFFUSE:			return "Mat Diffuse";
				case MATMAP:				return "Mat Texture Map";
				case MATMAPFILE:			return "Mat Texture File";
				case MATMAPPARAM:			return "Mat Texture Parameters";
				case MATTRANSP:				return "Mat Transparency";
				default:					return "UNKNOWN";
			}
		}
	};
	
	/*
	Class that calculates the normals for the model
	Special thanks to Ben Humphrey (DigiBen) for writing his original code in his .3ds loader.
	*/
	class NormalMath {
	private:
		static inline vector<float> MidPoint(float x1, float y1, float z1, float x2, float y2, float z2) {
			vector<float> result;
			result.push_back(x1 - x2);
			result.push_back(y1 - y2);
			result.push_back(z1 - z2);
			return result;
		}
		static inline vector<float> AddPoints(vector<float> v, Normal* nrm) {
			vector<float> result;
			result.push_back(v[0] + nrm->nx);
			result.push_back(v[1] + nrm->ny);
			result.push_back(v[2] + nrm->nz);
			return result;
		}
		static inline Normal CrossProduct(float x1, float y1, float z1, float x2, float y2, float z2) {
			Normal normal;
			normal.nx = ((y1 * z2) - (z1 * y2));
			normal.ny = ((z1 * x2) - (x1 * z2));
			normal.nz = ((x1 * y2) - (y1 * x2));
			return normal;
		}
		static inline Normal* DivideVectorByScaler(vector<float> v, float Scaler) {
			Normal* normal = new Normal();
			normal->nx = v[0] / Scaler;
			normal->ny = v[1] / Scaler;
			normal->nz = v[2] / Scaler;
			return normal;
		}
		static inline void Normalize(Normal* nrm) {
			float Magnitude = sqrt(nrm->nx*nrm->nx + nrm->ny*nrm->ny + nrm->nz*nrm->nz);
			nrm->nx /= Magnitude;
			nrm->ny /= Magnitude;
			nrm->nz /= Magnitude;
		}
	public:
		static inline void CalculateNormals(ModelData& data) {
			for (size_t i = 0; i < data.getNumOfObjects(); i++) {
				Object* obj = data.getObject(i);
				vector<Normal> pNormals(obj->getNumOfTriangles());
				vector<Normal> pTempNormals(obj->getNumOfTriangles());

				for (size_t f = 0; f < obj->getNumOfTriangles(); f++) {
					Vertex* v0 = obj->getVertex(obj->getTriangle(f)->index[0]);
					Vertex* v1 = obj->getVertex(obj->getTriangle(f)->index[1]);
					Vertex* v2 = obj->getVertex(obj->getTriangle(f)->index[2]);

					vector<float> mv0 = MidPoint(v0->x, v0->y, v0->z, v2->x, v2->y, v2->z);
					vector<float> mv1 = MidPoint(v2->x, v2->y, v2->z, v1->x, v1->y, v1->z);

					Normal vNormal = CrossProduct(mv0[0], mv0[1], mv0[2], mv1[0], mv1[1], mv1[2]);
					pTempNormals[f] = vNormal;
					Normalize(&vNormal);
					pNormals[f] = vNormal;
				}

				vector<float> sum = { 0.0, 0.0, 0.0 };
				vector<float> zero = sum;
				int shared = 0;
				for (size_t v = 0; v < obj->getNumOfVertices(); v++) {
					for (size_t f = 0; f < obj->getNumOfTriangles(); f++) {
						if (obj->getTriangle(f)->index[0] == v ||
							obj->getTriangle(f)->index[1] == v ||
							obj->getTriangle(f)->index[2] == v) {
							sum = AddPoints(sum, &pTempNormals[f]);
							shared++;
						}
					}
					Normal* newNrm = DivideVectorByScaler(sum, float(-shared));
					Normalize(newNrm);
					obj->addNormal(newNrm);
					sum = zero;
					shared = 0;
				}
			}
		}
	};

	class Model3DS {
	private:
		Chunk main;
	public:
		ModelData data;
		inline Model3DS(string filename, bool swapUpAxis = false) {
			ifstream ifs(filename, ios::in | ios::binary);
			data.swapUpAxis = swapUpAxis;
			main.read(ifs);
			main.readChildren(ifs, main.getLength() - 6, data);
			//cout << main.to_string() << endl;
			ifs.close();

			NormalMath::CalculateNormals(data);
		}
	};
};
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <vector>
#include <list>
#include <iostream>
#include <cassert>
#include <algorithm>

const float Infinity = 1e8;
template<typename T>
class Vec3
{
public:
    T x, y, z;
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
    Vec3(T xx) : x(xx), y(xx), z(xx) {}
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
    Vec3& normalize()
    {
        T nor2 = length2();
        if (nor2 > 0) {
            T invNor = 1 / sqrt(nor2);
            x *= invNor, y *= invNor, z *= invNor;
        }
        return *this;
    }
    Vec3<T> operator * (const T &f) const { return Vec3<T>(x * f, y * f, z * f); }
    Vec3<T> operator * (const Vec3<T> &v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
    T dot(const Vec3<T> &v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3<T> operator - (const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
    Vec3<T> operator + (const Vec3<T> &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
    Vec3<T>& operator += (const Vec3<T> &v) { x += v.x, y += v.y, z += v.z; return *this; }
    Vec3<T>& operator *= (const Vec3<T> &v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
    Vec3<T> operator - () const { return Vec3<T>(-x, -y, -z); }
    T length2() const { return x * x + y * y + z * z; }
    T length() const { return sqrt(length2()); }
    friend std::ostream & operator << (std::ostream &os, const Vec3<T> &v)
    {
        os << "[" << v.x << " " << v.y << " " << v.z << "]";
        return os;
    }
};

typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

struct Sphere{

        Vec3f color;
        float radius;
        Vec3f center;
        Vec3f emissionColor;
        float transparency;
        float reflection;
	Sphere(Vec3f ct, float r, Vec3f cl, float rf, float tr, Vec3f em = 0): 
	center(ct), radius(r), color(cl), reflection(rf), transparency(tr), emissionColor(em){}
};

unsigned int expandBits(unsigned int v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

unsigned int morton3D(float x, float y, float z)
{
    x = std::min(std::max(x * 1024.0f, 0.0f), 1023.0f);
    y = std::min(std::max(y * 1024.0f, 0.0f), 1023.0f);
    z = std::min(std::max(z * 1024.0f, 0.0f), 1023.0f);
    unsigned int xx = expandBits((unsigned int)x);
    unsigned int yy = expandBits((unsigned int)y);
    unsigned int zz = expandBits((unsigned int)z);
    return xx * 4 + yy * 2 + zz;
}

class Leaf3D
{
public:
	int idx;
	Vec3f bbox_min;
	Vec3f bbox_max;
	Vec3f centroid;
	unsigned int position = -1;

	Leaf3D(Sphere &sphere, int idx_): idx(idx_)
	{
		bbox_min = Vec3f(sphere.center.x-sphere.radius, sphere.center.y-sphere.radius, sphere.center.z-sphere.radius);
		bbox_max = Vec3f(sphere.center.x+sphere.radius, sphere.center.y+sphere.radius, sphere.center.z+sphere.radius);
		centroid = sphere.center;
	}
	
};
	
std::vector<Vec3f> findSpace(std::vector<Leaf3D> &leafs3D)
	{
		int n = leafs3D.size();
		float maxX = leafs3D[0].bbox_max.x, maxY = leafs3D[0].bbox_max.y, maxZ= leafs3D[0].bbox_max.z,
		minX= leafs3D[0].bbox_min.x, minY= leafs3D[0].bbox_min.y, minZ=leafs3D[0].bbox_min.z;
		//std::cout<<"maxZ:" << maxZ<<std::endl;
		for(int i=1; i<n; i++){	
			if (leafs3D[i].bbox_max.x > maxX) maxX = leafs3D[i].bbox_max.x;
			if (leafs3D[i].bbox_max.y > maxY) maxY = leafs3D[i].bbox_max.y;
			if (leafs3D[i].bbox_max.z > maxZ) maxZ = leafs3D[i].bbox_max.z;
			if (leafs3D[i].bbox_min.x < minX) minX = leafs3D[i].bbox_min.x;
			if (leafs3D[i].bbox_min.y < minY) minY = leafs3D[i].bbox_min.y;
			if (leafs3D[i].bbox_min.z < minZ) minZ = leafs3D[i].bbox_min.z;
		}
		std::vector<Vec3f> result(2);
		result[0] = Vec3f(minX, minY, minZ);
		result[1] = Vec3f(maxX, maxY, maxZ);
		return result;
	}

	
void updataLeaf3D(std::vector<Leaf3D> &leafs3D){
		std::vector<Vec3f> space = findSpace(leafs3D);
		//std::cout<<"find the space, min:"<<space[0]<<" max:"<<space[1]<<std::endl;
		float xDiff = space[1].x - space[0].x, yDiff = space[1].y-space[0].y,zDiff = space[1].z-space[0].z;
		float maxD = std::max(xDiff, std::max(yDiff, zDiff));
		float invMaxD = 1.0/maxD;
		//std::cout<<"maxD:"<<maxD<<std::endl;
		Vec3f newMax = space[0] + Vec3f(maxD);
		Vec3f spaceCenter = (newMax+space[0])*0.5;
		//std::cout<<"spceCenter:"<<spaceCenter<<std::endl;
		Vec3f cubeCenter = Vec3f(0.5);
		Vec3f moveVector = cubeCenter-spaceCenter*invMaxD;
		//std::cout<<"movement:"<<moveVector<<std::endl;
		int n = leafs3D.size();
		for(int i=0; i<n; i++)
		{
			Vec3f newCentroid = leafs3D[i].centroid*invMaxD + moveVector;
			//std::cout<<"newCentroid:"<<newCentroid<<std::endl;
			leafs3D[i].position = morton3D(newCentroid.x, newCentroid.y, 1-newCentroid.z); // 1-z or just z?? huh?
			//std::cout<<"position:"<<leafs3D[i].position<<std::endl;
		}
	}
	

class Node
{
public:
	Node *childA;
	Node *childB;
	int childAIdx = Infinity;
	int childBIdx = Infinity;
	Node *parent;
	bool isLeaf = false;
	int objectID = -1;
	Vec3f bbox_min = Infinity;
	Vec3f bbox_max = -Infinity;
}; 


int findSplit(int* sortedMortonCodes, int first, int last){
	unsigned int firstCode = sortedMortonCodes[first];
	unsigned int lastCode= sortedMortonCodes[last];
	
	if(firstCode == lastCode)
		return (first + last) >> 1;
	

	int commonPrefix = /*__builtin_clz*/__lzcnt(firstCode ^ lastCode);
	
	
	int split = first;
	int step = last - first;
	do{
		step = (step+1) >> 1;
		int newSplit = split + step;
	
		if(newSplit < last)
		{
			unsigned int splitCode = sortedMortonCodes[newSplit];
			int splitPrefix = /*__builtin_clz*/__lzcnt(firstCode ^ splitCode);
			if(splitPrefix > commonPrefix)
				split = newSplit;
		}
	}
	while(step>1);

	return split;
}

int sgn(int number)
{
	return (0< number) - (0>number);
}

int longestCommonPrefix(int *sortedMortonCodes, int numberofElements, int index1, int index2, int key1)
{
	if(index2 <0 || index2 >= numberofElements)
		return -1;
	int key2 = sortedMortonCodes[index2];
	return /*__builtin_clz*/__lzcnt(key1^key2);
}

void determineRange(int *sortedMortonCodes, int numObjects, int idx, int *first, int *last)
{
	int key1 = sortedMortonCodes[idx];
	int lcp1 = longestCommonPrefix(sortedMortonCodes, numObjects, idx, idx+1, key1);
	int lcp2 = longestCommonPrefix(sortedMortonCodes, numObjects, idx, idx-1, key1);
	
	int direction =sgn((lcp1 - lcp2));
	int minLcp = longestCommonPrefix(sortedMortonCodes, numObjects, idx, idx-direction, key1);
	int lMax = 2;
	while(longestCommonPrefix(sortedMortonCodes, numObjects, idx, idx+lMax*direction, key1) > minLcp)
		lMax *=2;

	int l = 0;
	int t = lMax;
	while (t>1)
	{
		t = t/2;
		if(longestCommonPrefix(sortedMortonCodes, numObjects, idx, idx+(l+t)*direction, key1)>minLcp)
			l+=t;
	}
	int j = idx+l*direction;
	if(j > idx) *last = j, *first = idx;
	else *last = idx, *first = j;
	
}

void updateParent(Node *a, int&depth)
{
	if(a->parent->bbox_min.x > a->bbox_min.x) a->parent->bbox_min.x = a->bbox_min.x;
	if(a->parent->bbox_min.y > a->bbox_min.y) a->parent->bbox_min.y = a->bbox_min.y;
	if(a->parent->bbox_min.z > a->bbox_min.z) a->parent->bbox_min.z = a->bbox_min.z;
	if(a->parent->bbox_max.x < a->bbox_max.x) a->parent->bbox_max.x = a->bbox_max.x;
	if(a->parent->bbox_max.y < a->bbox_max.y) a->parent->bbox_max.y = a->bbox_max.y;
	if(a->parent->bbox_max.z < a->bbox_max.z) a->parent->bbox_max.z = a->bbox_max.z;
	depth++;
	if(a->parent->parent != nullptr)
		updateParent(a->parent, depth);
}

Node *generateHierarchy(int *sortedMortonCodes, int *sortedObjectIDs, int numObjects,
						std::vector<Leaf3D> leafs3D, int &depth, Node *&leafNodes, Node*&nodes){

	leafNodes = new Node[numObjects];
    nodes = new Node[numObjects-1];

	for(int idx = 0; idx < numObjects; idx++)
	{
		leafNodes[idx].isLeaf = true;
		leafNodes[idx].objectID = sortedObjectIDs[idx];
		leafNodes[idx].bbox_min = leafs3D[leafNodes[idx].objectID].bbox_min;
		leafNodes[idx].bbox_max = leafs3D[leafNodes[idx].objectID].bbox_max;
	}

	for(int idx=0; idx < numObjects-1; idx++)
	{
		int first=-1, last = -1;
		determineRange(sortedMortonCodes, numObjects, idx, &first, &last);
		
		int split = findSplit(sortedMortonCodes, first, last);
	
		Node *childA;
		int childAIdx;
		if (split == first){
			childA = &leafNodes[split];
			childAIdx = -split;
		}
		else{
			childA = &nodes[split];
			childAIdx = split;
		}

		Node *childB;
		int childBIdx;
		if (split + 1 == last){
			childB = &leafNodes[split + 1];
			childBIdx = -(split + 1);
		}
		else{
			childB = &nodes[split + 1];
			childBIdx = split + 1;
		}

		nodes[idx].childA = childA;
		nodes[idx].childB = childB;
		nodes[idx].childAIdx = childAIdx;
		nodes[idx].childBIdx = childBIdx;
		childA->parent = &nodes[idx];
		childB->parent = &nodes[idx];
	}
	nodes[0].parent = nullptr;
	int tmp = 1;
	for(int i=0; i<numObjects; i++)
	{	
		updateParent(&leafNodes[i], tmp);
		if (depth < tmp) depth = tmp;
		tmp = 1;			
	}
	return &nodes[0];
}
// A utility function to get maximum value in arr[]
int getMax(int arr[], int n)
{
    int mx = arr[0];
    for (int i = 1; i < n; i++)
        if (arr[i] > mx)
            mx = arr[i];
    return mx;
}
 
// A function to do counting sort of arr[] according to
// the digit represented by exp.
void countSort(int arr[], int id[], int n, int exp)
{
	using namespace std;
    //int output[n]; // output array
    //int outputid[n];
	int *output = new int[n]; 
	int *outputid = new int[n]; 
    int i, count[10] = {0};
 
    for (i = 0; i < n; i++)
        count[ (arr[i]/exp)%10 ]++;
 
    for (i = 1; i < 10; i++)
        count[i] += count[i - 1];
 
    for (i = n - 1; i >= 0; i--)
    {
        output[count[ (arr[i]/exp)%10 ] - 1] = arr[i];
        outputid[count[ (arr[i]/exp)%10 ] - 1] = id[i];
        count[ (arr[i]/exp)%10 ]--;
    }
 
    for (i = 0; i < n; i++)
    {
        arr[i] = output[i];
	id[i] = outputid[i];
    }

	delete[] output;
	delete[] outputid;
}

void radixsort(int arr[], int id[], int n)
{
	using namespace std;
    int m = getMax(arr, n);
 
    for (int exp = 1; m/exp > 0; exp *= 10)
        countSort(arr, id, n, exp);
}


Node* BVHstart(int&numSphere, double**&mySpheres, int&depth, Node *&leafNodes, Node*&nodes)
{

	std::vector<Sphere> spheres;
    //position, radius, surface color, reflectivity, transparency, emission color
    //spheres.push_back(Sphere(Vec3f( 0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0));
	/*spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 4, Vec3f(1.00, 0.32, 0.36), 1, 0.5));
	spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
	spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
	spheres.push_back(Sphere(Vec3f(-5.5, 0, -15), 3, Vec3f(0.90, 0.90, 0.90), 1, 0.0));
	// light
	spheres.push_back(Sphere(Vec3f(0.0, 20, -30), 3, Vec3f(0.00, 0.00, 0.00), 0, 0.0, Vec3f(3)));*/


	for (int i = 0; i < numSphere; i++){
		spheres.push_back(Sphere(Vec3f(mySpheres[i][0], mySpheres[i][1], mySpheres[i][2]), mySpheres[i][3], Vec3f(0.00, 0.00, 0.00), 0, 0.0, Vec3f(3)));
	}

	int sz = spheres.size();
	std::vector<Leaf3D> leafs3D;

	//int sortedMortonCode[sz];
	//int sortedObjectIDs[sz];
	int* sortedMortonCode = new int[sz];
	int* sortedObjectIDs = new int[sz];
	for(int i=0; i<sz; i++)
	{
		leafs3D.push_back(Leaf3D(spheres[i], i));
	}
	
	updataLeaf3D(leafs3D);	
	for(int i=0; i<sz; i++)
	{
		sortedMortonCode[i] = leafs3D[i].position;
		sortedObjectIDs[i] = i;
	}
	radixsort(sortedMortonCode, sortedObjectIDs, sz);
    	//for(int i=0; i<sz;i++)
		//std::cout<< sortedMortonCode[i]<<" ";
	//std::cout<< std::endl;
	//for(int i=0; i<sz; i++)
		//std::cout<< sortedObjectIDs[i]<<" ";
	//std::cout<< std::endl;
	
	depth = -1;
	Node * result = generateHierarchy(sortedMortonCode, sortedObjectIDs, sz, leafs3D, depth, leafNodes, nodes); 
	delete[] sortedMortonCode;
	delete[] sortedObjectIDs;



	//reader for the BVH 

	/*std::cout << "root is leaf? " << result->isLeaf << std::endl;
	std::cout << "what is your bbox: min:" << result->bbox_min << " max: " << result->bbox_max << std::endl;
	std::cout << "left child is leaf? " << result->childA->isLeaf << std::endl;
	std::cout << "what is your bbox: min:" << result->childA->bbox_min << " max: " << result->childA->bbox_max << std::endl;
	std::cout << "right child is leaf? " << result->childB->isLeaf << std::endl;
	std::cout << "what is your bbox: min:" << result->childB->bbox_min << " max: " << result->childB->bbox_max << std::endl;
	std::cout << "-------------------------" << std::endl;
	std::cout << "left child's left child is leaf? " << result->childA->childA->isLeaf << std::endl;
	std::cout << "what is your bbox: min:" << result->childA->childA->bbox_min << " max: " << result->childA->childA->bbox_max << std::endl;
	std::cout << "left child's right child is leaf? " << result->childA->childB->isLeaf << std::endl;
	std::cout << "Then what is its objectID " << result->childA->childB->objectID << std::endl;
	std::cout << "what is your bbox: min:" << result->childA->childB->bbox_min << " max: " << result->childA->childB->bbox_max << std::endl;
	std::cout << "right child's left child is leaf? " << result->childB->childA->isLeaf << std::endl;
	std::cout << "Then what is its objectID " << result->childB->childA->objectID << std::endl;
	std::cout << "what is your bbox: min:" << result->childB->childA->bbox_min << " max: " << result->childB->childA->bbox_max << std::endl;
	std::cout << "right child's right child is leaf? " << result->childB->childB->isLeaf << std::endl;
	std::cout << "Then what is its objectID " << result->childB->childB->objectID << std::endl;
	std::cout << "what is your bbox: min:" << result->childB->childB->bbox_min << " max: " << result->childB->childB->bbox_max << std::endl;
	std::cout << "-------------------------" << std::endl;
	std::cout << "left child's left child's left child is leaf? " << result->childA->childA->childA->isLeaf << std::endl;
	std::cout << "Then what is its objectID " << result->childA->childA->childA->objectID << std::endl;
	std::cout << "what is your bbox: min:" << result->childA->childA->childA->bbox_min << " max: " << result->childA->childA->childA->bbox_max << std::endl;
	std::cout << "left child's left child's right child is leaf? " << result->childA->childA->childB->isLeaf << std::endl;
	std::cout << "Then what is its objectID " << result->childA->childA->childB->objectID << std::endl;
	std::cout << "what is your bbox: min:" << result->childA->childA->childB->bbox_min << " max: " << result->childA->childA->childB->bbox_max << std::endl;*/
	return result;
}



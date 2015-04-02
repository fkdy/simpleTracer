#include<iostream>
#include<fstream>
#include<cstdlib>
#include<cstdio>
#include<cmath>
#include<vector>

#define _debug 1
#define max_depth 5
template <typename _t>
class vec3
{
public:
    _t x,y,z;
    vec3(){x=0, y=0, z=0;}
    vec3(_t xx):x(xx), y(xx), z(xx) {}
    vec3(_t xx, _t yy, _t zz):x(xx), y(yy), z(zz){}
    _t lengthsq() const {return x*x + y*y + z*z;}
    _t length() const {return sqrt(lengthsq());}
    vec3<_t> normalize() const
    {
	if(lengthsq()==0){
	    std::cerr << "vector is of length 0!" << std::endl;
	    exit(0);
	}
	return vec3<_t>(x/length(), y/length(), z/length());
    }
    _t dot(vec3<_t> v) const{return x*v.x + y*v.y + z*v.z;}
    vec3<_t>& operator += (const vec3<_t>& v)
    {x+=v.x, y+=v.y, z+=v.z; return *this;}
    vec3<_t>& operator -= (const vec3<_t>& v)
    {x-=v.x, y-=v.y, z-=v.z; return *this;}
    vec3<_t> operator + (const vec3<_t>& v) const
    {return vec3<_t>(x+v.x, y+v.y, z+v.z);}
    vec3<_t> operator - (const vec3<_t>& v) const
    {return vec3<_t>(x-v.x, y-v.y, z-v.z);}
    vec3<_t> operator * (const _t& f) const
    {return vec3<_t>(f*x, f*y, f*z);}
};
typedef vec3<float> vec;

class sph
{
public:
    vec center;
    float radius;
    vec surfColor;
    vec emisColor;
    float refl;
    float refr;
    float diffCoeff;
    sph(vec c, float r, vec sc, vec ec, float rfl=0, float rfr=0, float dC=0)
	:center(c), radius(r), surfColor(sc), emisColor(ec), refl(rfl), refr(rfr), diffCoeff(dC)
    {}

    bool hit(vec orig, vec dir, float& length)
    {
	length = INFINITY;
	vec line=center-orig;
	vec dirNorm = dir.normalize();
	float tang=line.dot(dirNorm);
	float dist=sqrt(line.lengthsq()-tang*tang);
	if(tang<=0)
	    return false;
	if(dist >= radius)
	    return false;
	length=tang - sqrt(radius*radius - dist*dist);
	return true;
    }
};

// simple diffuse function
vec diffColor(const vec center, const vec hitPoint, float diffCoeff, std::vector<sph> listSpheres)
{
    vec color=vec(0);
    sph *light;
    for(int i=0; i<listSpheres.size(); i++){
	if(listSpheres[i].emisColor.x>0){
	    light=&listSpheres[i];
	    vec lightDir=light->center - hitPoint ;
	    vec sphNorm =hitPoint - center;
	    lightDir=lightDir.normalize();
	    sphNorm =sphNorm.normalize();
	    color+=light->emisColor*std::max(float(0), lightDir.dot(sphNorm))*diffCoeff;
	}
    }
    return color;
}

vec tracer(vec orig, vec dir, std::vector<sph> listSpheres, const float depth)
{
    // primary ray
    sph *pSphere=NULL;
    float length=INFINITY;
    vec color=vec(0.1f);
    for(int i=0; i<listSpheres.size(); i++){
	float t=INFINITY;
	if(listSpheres[i].hit(orig, dir,t)){
	    //std::cout << "hit: " << t;
	    if(t<length)
		length=t;
	    pSphere=&listSpheres[i];
	    if(pSphere->refl>0 && depth<max_depth){
		pSphere=&listSpheres[i];
		// form secondary ray
		vec secOrig=vec(0);
		vec secDir=vec(0);
		secOrig=orig + dir.normalize()*length;
		vec secNorm=vec(0);
		secNorm=secOrig-pSphere->center;
		secNorm=secNorm.normalize();
		secDir=secNorm*dir.dot(secNorm)*2.0f;
		secDir=dir - secDir;
		vec secColor=tracer(secOrig, secDir, listSpheres, depth+1);
		vec difColor=diffColor(pSphere->center, secOrig, pSphere->diffCoeff, listSpheres);
		//float fact=pow(secNorm.dot(secDir.normalize()),20);
		//std::cout<<"fact: " << secColor.y;
		color=pSphere->surfColor+secColor+difColor; //secColor+pSphere->surfColor*fact;
	    }else if(pSphere->emisColor.x>0 && depth<max_depth){
		color=pSphere->emisColor;
	    }else
		;
	}
    }
    return color;
}

void render(const std::vector<sph> listSpheres)
{
    int width=640, height=480;
    vec *img=new vec[width*height];
    for(int i=0; i<height; i++){
	for(int j=0; j<width; j++){
	    vec orig=vec(-10,0,0);
	    float y= -2+(2+2)*(j+0.5)/width;
	    float z= -1.5+(1.5+1.5)*(i+0.5)/height;
	    vec dir=vec(0, y, z)-orig;
	    img[i*width + j]=tracer(orig, dir, listSpheres, 0);
	}
    }
    std::cout << "write out ppm file (could use eog (eye of gnome) to read)";
    std::ofstream out("./img.ppm", std::ios::out | std::ios::binary);
    out<<"P6\n"<<width <<" " << height << "\n255\n";
    for(int i=0; i<height*width; i++){
	out<<(unsigned char)(std::min(float(1), img[i].x)*255)
	  <<(unsigned char)(std::min(float(1), img[i].y)*255)
	  <<(unsigned char)(std::min(float(1), img[i].z)*255);
    }
    out.close();
    delete[] img;
}

int main(){
#if _debug
    using namespace std;
    vec o(0,0,0);
    vec d=vec(30,0,0);
    float l;
    std::vector<sph> spheres;
    // center, radius, surface color, emission color, refl, refr, diffCoeff
    //spheres.push_back(sph(vec(50,0,0), .5, vec(0.6,.5,1), vec(3), 0));
    spheres.push_back(sph(vec(8,1,-.5), .4, vec(0.3,.3,.3), vec(0), 1, 0, .3f));
    spheres.push_back(sph(vec(8,-2,1.8), .8, vec(0.2,.3,.5), vec(0), 1, 0, .3f));
    spheres.push_back(sph(vec(0,0,12), 1, vec(0.6,.5,1), vec(1), 0, 0, 0));
    vec color=tracer(o, d, spheres, 0);
//    sph sphere(vec(5,1,0), .5, vec(0), vec(0), 1, 0);
//    if(sphere.hit(o, d, l)){
//	cout<<"hit: " << l << " " << endl;
//    }else
//	cout << "miss!"<<endl;
#endif
    render(spheres);
    return 0;
}

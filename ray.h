#pragma once
#include "jmath.h"
#include <stdlib.h>



struct camera {
    v3 position;
    v3 direction;
};

struct image_buffer {
    int Width;
    int Height;
    int Pitch;
    unsigned char* Buffer;
};



typedef v3 rgb;


struct plane {
    v3 N;
    float d;
    v3 Color;
};

struct sphere {
    v3 Center;
    rgb Color;
    float radius;
};


struct world {
    int PlaneCount;
    int SphereCount;
    
    plane* Planes;
    sphere* Spheres;
    
    rgb SkyColor;
    
};



bool IntersectSphere(sphere s, const v3 rayorig, const v3 raydir, float *t0, float *t1){
    v3 l = sub(s.Center, rayorig);
    float tca = dotProduct(l, raydir);
    if (tca < 0) return false;
    float d2 = dotProduct(l,l) - tca * tca;
    if (d2 > s.radius * s.radius) return false;
    float thc = sqrtf((s.radius * s.radius) - d2);
    *t0 = tca - thc;
    *t1 = tca + thc;
    
    return true;
}



//TODO: research this algorithm, and name these things better;

bool IntersectPlane(plane Plane, v3 l0, v3 l, float *t)
{
    // assuming vectors are all normalized
    v3 p0 = {};
    p0.x = Plane.N.x * Plane.d;
    p0.y = Plane.N.y * Plane.d;
    p0.z = Plane.N.z * Plane.d;
    
    float denom = dotProduct(Plane.N, l);
    if (denom > 1e-6 || -denom > 1e-6) {
        v3 p0l0 = p0 - l0;
        *t = (Plane.d - dotProduct(p0l0, Plane.N)) / denom;
        return (t >= 0);
    }
    return false;
}




v3 BufferCoords2FilmPoint(image_buffer* Buffer, camera* Camera, int X, int Y){
    
    v3 Origin = Camera->position;
    v3 CameraZ = -Camera->direction.normal();
    v3 CameraX = crossProduct(CameraZ,v3(0, -1, 0)).normal();
    v3 CameraY = crossProduct(CameraZ,CameraX).normal();
    
    
    float FilmDist = 1.0f;
    float FilmW = (float)Buffer->Width / (float) Buffer->Height;
    float FilmH = 1.0f;
    float HalfFilmW = 0.5f * FilmW;
    float HalfFilmH = 0.5f * FilmH;
    v3 FilmCenter = Origin - (CameraZ * FilmDist);
    
    
    float FilmY = 2.0f * ((float)Y / (float)Buffer->Height) - 1.0f;
    float FilmX = 2.0f * ((float)X / (float)Buffer->Width) - 1.0f;
    v3 FilmP = FilmCenter + CameraY*FilmY*HalfFilmH + FilmX*HalfFilmW*CameraX;
    
    return FilmP;
    
    
}



v3 RayCast(v3 Origin, v3 Direction, world* World) {
    v3 FinalColor = {};
    bool Hit = false;
    float HitDistance = INFINITY;
    
    //cast against spheres
    for (int i = 0; i < World->SphereCount; i++) {
        float t0 = INFINITY;
        float t1 = INFINITY;
        sphere S = World->Spheres[i];
        if (IntersectSphere(S, Origin, Direction, &t0, &t1)) {
            Hit= true;
            if (t0 < 0) t0 = t1;
            if (t0 < HitDistance) {
                HitDistance= t0;
                FinalColor = S.Color;
            }
        }	
    }
    //cast against planes
    
    for (int i = 0; i < World->PlaneCount; i++) {
        plane Plane = World->Planes[i];
        float ThisDistance;
        
        float Denom = dotProduct(Plane.N, Direction);
        if(Denom > 1e-6 || -Denom > 1e-6){
            ThisDistance = (-dotProduct(Plane.N, Origin) - Plane.d ) / Denom;
            if(ThisDistance > 0 && ThisDistance < HitDistance){
                Hit = true;
                HitDistance= ThisDistance;
                FinalColor = Plane.Color;
            }
        }
    }
    
    if (!Hit) {
        
        FinalColor = World->SkyColor;
    }
    
    return FinalColor;
}








void Render(image_buffer* Buffer, world* World, camera* Camera) {
    
    
    v3 Origin = Camera->position;
    v3 CameraZ = -Camera->direction.normal();
    //v3 CameraZ = v3(0, 0, 1);
    v3 CameraX = crossProduct(CameraZ,v3(0, -1, 0)).normal();
    v3 CameraY = crossProduct(CameraZ,CameraX).normal();
    
    
    float FilmDist = 1.0f;
    float FilmW = (float)Buffer->Width / (float) Buffer->Height;
    float FilmH = 1.0f;
    float HalfFilmW = 0.5f * FilmW;
    float HalfFilmH = 0.5f * FilmH;
    v3 FilmCenter = Origin - (CameraZ * FilmDist);
    
    
    unsigned int * Pixel = (unsigned int*) Buffer->Buffer;
    for (int Y = 0; Y < Buffer->Height; Y++){
        float FilmY = 2.0f * ((float)Y / (float)Buffer->Height) - 1.0f;
        for(int X = 0; X < Buffer->Width; X++){
            
            float FilmX = 2.0f * ((float)X / (float)Buffer->Width) - 1.0f;
            
            v3 FilmP = FilmCenter + CameraY*FilmY*HalfFilmH + FilmX*HalfFilmW*CameraX;
            
            //v3 FilmP = BufferCoords2FilmPoint(Buffer, Camera, X, Y);
            
            v3 RayDir = normal(FilmP - Origin);
            
            *Pixel++ = V32RGB(RayCast(Origin,RayDir, World));
        }
    }
}


float lerp(float v0, float v1, float t) {
    return (1 - t) * v0 + t * v1;
}



v3 RandomColor(){
    return v3((float)rand() / (RAND_MAX + 1.0f),
              (float)rand() / (RAND_MAX + 1.0f),
              (float)rand() / (RAND_MAX + 1.0f));
}



sphere* GenerateSpheres(int SphereCount, int RangeX, int RangeY, int RangeZ, int MaxRadius, int MinRadius) {
    sphere* Spheres = (sphere*)malloc(SphereCount * sizeof(sphere));
    
    for (int SphereIndex = 0; SphereIndex < SphereCount; SphereIndex++) {
        Spheres[SphereIndex].Center.x = (float)rand() / (RAND_MAX + 1.0f) * RangeX * 2 - RangeX;
        Spheres[SphereIndex].Center.y = (float)rand() / (RAND_MAX + 1.0f) * RangeY * 2 - RangeY;
        Spheres[SphereIndex].Center.z = (float)rand() / (RAND_MAX + 1.0f) * RangeZ * 2 - RangeZ;
        Spheres[SphereIndex].radius = lerp(MinRadius, MaxRadius, (float)rand() / (RAND_MAX + 1.0f)) ;
        
        Spheres[SphereIndex].Color = RandomColor();
    }
    
    return Spheres;
    
    /*
    sphere* Spheres = (sphere*)malloc(1 * sizeof(sphere));
    for(int i = 0; i < SphereCount; i++){
    
    
        Spheres[i].Center = v3(-20,-20,-20);
        Spheres[i].radius = 10;
        Spheres[i].Color = v3(1,1,1);
    }
    return Spheres;*/
}
/*
New File
*/


#include <math.h>

#define M_PI 3.141592653589793 


struct v2 {
    float x;
    float y;
    
    v2() {
        x = 0;
        y = 0;
    }
    
    v2(float _x, float _y){
        x = _x;
        y = _y;
    }
};



struct v3 {
    float x;
    float y;
    float z;
    
    v3() {
        x = 0;
        y = 0;
        z = 0;
    }
    
    v3(float _x, float _y, float _z) {
        x = _x;
        y = _y;
        z = _z;
    }
    
    inline float length() {
        return sqrtf(x*x + y*y + z*z);
    }
    
    inline v3 normal() {
        v3 result = v3();
        float len = this->length();
        if (len) {
            float divisor = 1 / len;
            result.x = x * divisor;
            result.y = y * divisor;
            result.z = z * divisor;
        }
        return result;
    }
    
    void normalize() {
        float len = this->length();
        if (len) {
            float divisor = 1 / len;
            x *= divisor;
            y *= divisor;
            z *= divisor;
        }
    }
    
    inline v3 operator+(v3 rhs) {
        v3 result;
        result.x = x + rhs.x;
        result.y = y + rhs.y;
        result.z = z + rhs.z;
        return result;
    }
    
    inline v3 operator-(v3 rhs) {
        v3 result;
        result.x = x - rhs.x;
        result.y = y - rhs.y;
        result.z = z - rhs.z;
        return result;
    }
    
    inline v3 operator-(){
        v3 result;
        result.x = -x;
        result.y = -y;
        result.z = -z;
        return result;
    }
};

//scalar multiplication
inline v3 operator*(float scalar, v3 rhs) {
    v3 result;
    result.x = rhs.x * scalar;
    result.y = rhs.y * scalar;
    result.z = rhs.z * scalar;
    return result;
}

inline v3 operator*(v3 lhs, float scalar){
    return scalar * lhs;
}


inline float length(v3 v) {
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}


inline v3 normal(v3 v) {
    float len = length(v);
    if (len) {
        float divisor = 1 / len;
        v.x *= divisor;
        v.y *= divisor;
        v.z *= divisor;
    }
    
    return v;
}


inline v3 crossProduct(v3 first, v3 second) {
    v3 result;
    result.x = first.y * second.z - first.z * second.y;
    result.y = first.z * second.x - first.x * second.z;
    result.z = first.x * second.y - first.y * second.x;
    
    return result;
}



inline float dotProduct(v3 first, v3 second) {
    return first.x * second.x + first.y * second.y + first.z * second.z;
}



//returns angle between two vectors
//TODO: check to see if this code assumes normalized vectors (I think it does)
inline float angle(v3 first, v3 second) {
    return (float)acos(dotProduct(first, second));
}


inline v3 add(v3 first, v3 second) {
    first.x += second.x;
    first.y += second.y;
    first.z += second.z;
    
    return first;
}

inline v3 sub(v3 first, v3 second) {
    first.x -= second.x;
    first.y -= second.y;
    first.z -= second.z;
    
    return first;
}


inline unsigned int V32RGB(v3 v) {
    unsigned int r = (unsigned int)(v.x * 255.0f);
    unsigned int g = (unsigned int)(v.y * 255.0f);
    unsigned int b = (unsigned int)(v.z * 255.0f);
    
    return (r << 16) | (g << 8) | (b);
}


inline v3 RGB2V3(unsigned int rgb) {
    v3 result;
    result.x = ((rgb >> 16) & 0xFF) / 255.0f;
    result.y = ((rgb >> 8) & 0xFF) / 255.0f;
    result.z = (rgb & 0xFF) / 255.0f;
    
    return result;
}


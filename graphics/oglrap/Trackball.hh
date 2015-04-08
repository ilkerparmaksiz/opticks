#pragma once
#include <glm/glm.hpp>  
#include <glm/gtc/quaternion.hpp>

class Trackball {

   public:
       Trackball() :
          m_radius(1.f),
          m_translatefactor(1.f)
       {
          home();
       }

   public:
       void home()
       {
           m_x = 0.f ; 
           m_y = 0.f ; 
           m_z = 0.f ; 
           setOrientation(0.f, 0.f);
       }
       void zoom_to(float x, float y, float dx, float dy)
       {
           m_z += dy*m_translatefactor ;
       } 
       void pan_to(float x, float y, float dx, float dy)
       {
           m_x += dx*m_translatefactor ;
           m_y += dy*m_translatefactor ;
       } 
       void setRadius(float r)
       {
           m_radius = r ; 
       }
       void setTranslateFactor(float tf)
       {
           m_translatefactor = tf ; 
       }

       void setOrientation(float theta, float phi);
       void setOrientation(glm::quat& q);

       glm::quat getOrientation();
       glm::vec3 getTranslation();
       glm::mat4 getOrientationMatrix();
       glm::mat4 getTranslationMatrix();
       glm::mat4 getCombinedMatrix();

   public:
       void Summary(const char* msg);
       void drag_to(float x, float y, float dx, float dy);

   private:
       glm::quat rotate(float x, float y, float dx, float dy);
       static float project(float r, float x, float y);

   private:
       float m_x ;
       float m_y ;
       float m_z ;
       float m_radius ;
       float m_translatefactor ;

       glm::quat m_orientation ; 


};

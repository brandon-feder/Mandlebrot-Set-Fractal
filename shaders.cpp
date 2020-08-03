#include<cmath>
#include "color.cpp"

class Shaders {
    public: 
        static Color shader1(float percent) {
            if(percent == -1) {
                return Color(255, 255, 255, 255);
            } else {
                return lerp( 
                    Color(0, 255, 225, 255),
                    Color(255, 0, 0, 255), 
                    round(10*percent)/10
                );
            }         
        }
    private:
        static Color lerp(Color a, Color b, float t) {

            return Color(
                a.red + (b.red - a.red)*t, 
                a.green + (b.green - a.green)*t, 
                a.blue + (b.blue - a.blue)*t, 
                a.alpha + (b.alpha - a.alpha)*t
            );
        }
};
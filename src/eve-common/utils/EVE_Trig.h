
 /**
  * @name EVE_Trig.h
  *   simple math defines and methods for trig used in EvEmu
  * @Author:         Allan
  * @date:   30 Aug 2015
  */



const double EvE_E = 2.71828182845904523536;
const double EvE_Pi = 3.14159265358979323846;
const double EvE_RadiansInDegrees = 0.01745329251;    //  pi/180
const double EvE_DegreesInRadians = 57.2957795131;   //  180/pi

inline double EvE_DegreesToRadians(double deg) { return (deg * EvE_RadiansInDegrees); }
inline double EvE_RadiansToDegrees(double rad) { return (rad * EvE_DegreesInRadians); }

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

using namespace std;

#define WINDOWDEAPTH 24
size_t bredde=800;
size_t hoejde=600;
float max_dist=200.0;
float min_dist=1.0;
  
/* Definer hvad en trekant er
 * Den har tre koordinater i 3D
 * (myX1,myY1,myZ1), (myX2,myY2,myZ2) og
 * (myX3,myY3,myZ3).
 * Derudover har den en farve angivet som
 * rød, grøn og blå værdier imellem 0 og 255
 * myR, myG og myB.
 */
class Trekant // {{{
{ public:
    Trekant(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, int r, int g, int b);
    virtual ~Trekant();

    float myX1, myY1, myZ1, myX2, myY2, myZ2, myX3, myY3, myZ3;
    int myR, myG, myB;
}; // }}}
Trekant::Trekant(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, int r, int g, int b) // {{{
: myX1(x1)
, myY1(y1)
, myZ1(z1)
, myX2(x2)
, myY2(y2)
, myZ2(z2)
, myX3(x3)
, myY3(y3)
, myZ3(z3)
, myR(r)
, myG(g)
, myB(b)
{
} // }}}
Trekant::~Trekant() // {{{
{
} // }}}

// Indlæs 3d obj fil til trekants liste
vector<Trekant> laes_obj(const string &fname) // {{{
{ ifstream fin(fname);
  vector<Trekant> result;
  vector<float> vs; // Vertices

  string line;
  int r, g, b;
  while (std::getline(fin, line))
  { if (line.substr(0,2)=="v ") // vertex
    { stringstream ss;
      ss << line.substr(2);
      float x, y, z;
      ss >> x;
      ss >> y;
      ss >> z;
      vs.push_back(x);
      vs.push_back(y);
      vs.push_back(z);
    }
    else if (line.substr(0,13)=="usemtl color_") // material (color)
    { size_t col=std::atoi(line.substr(13).c_str());
      b=col%256;
      col=col/256;
      g=col%256;
      col=col/256;
      r=col%256;
    }
    else if (line.substr(0,2)=="f ") // face (triangle)
    { stringstream ss;
      ss << line.substr(2);
      int v1, v2, v3;
      ss >> v1;
      --v1;
      ss >> v2;
      --v2;
      ss >> v3;
      --v3;
      //cout << "laes_obj(" << fname << ") Trekant: ("
      //     << vs[3*v1] << "," << vs[2*v1+1] << "," << vs[3*v1+2] << ")->("
      //     << vs[3*v2] << "," << vs[3*v2+1] << "," << vs[3*v2+2] << ")->("
      //     << vs[3*v3] << "," << vs[3*v3+1] << "," << vs[3*v3+2] << ")." << endl;

      result.push_back(Trekant(vs[3*v1],vs[3*v1+1],vs[3*v1+2],vs[3*v2],vs[3*v2+1],vs[3*v2+2],vs[3*v3],vs[3*v3+1],vs[3*v3+2],r,g,b));
    }
    //else
    //  cout << "Ignoring obj line: " << line << endl;
  }
  fin.close();
  return result;
} // }}}
// Indlæs 3d stl fil til trekants liste
// Eftersom der ikke er farver i stl, angives
// en farve som bruges til trekanterne
vector<Trekant> laes_stl(const string &fname, char r, char g, char b) // {{{
{ ifstream fin(fname);
  vector<Trekant> result;
  fin.ignore(80);
  size_t count=0;
  fin.read((char*)&count,4);
  for (size_t i=0; i<count; ++i)
  { float x0;
    fin.read((char*)&x0,sizeof(float));
    float y0;
    fin.read((char*)&y0,sizeof(float));
    float z0;
    fin.read((char*)&z0,sizeof(float));
    float x1;
    fin.read((char*)&x1,sizeof(float));
    float y1;
    fin.read((char*)&y1,sizeof(float));
    float z1;
    fin.read((char*)&z1,sizeof(float));
    float x2;
    fin.read((char*)&x2,sizeof(float));
    float y2;
    fin.read((char*)&y2,sizeof(float));
    float z2;
    fin.read((char*)&z2,sizeof(float));
    float x3;
    fin.read((char*)&x3,sizeof(float));
    float y3;
    fin.read((char*)&y3,sizeof(float));
    float z3;
    fin.read((char*)&z3,sizeof(float));
    fin.ignore(2);

    result.push_back(Trekant(x1,y1,z1,x2,y2,z2,x3,y3,z3,r,g,b));
  }
  fin.close();
  return result;
} // }}}

// Opdel store trekanter til mindre trekanter
vector<Trekant> smaa_trekanter(const vector<Trekant> &trekanter, float max_afstand=min_dist*1.5) // {{{
{ vector<Trekant> lektier=trekanter;
  vector<Trekant> resultat;

  while (!lektier.empty())
  { Trekant t=lektier.back();
    lektier.pop_back();
    float d12=max(max(abs(t.myX1-t.myX2),abs(t.myY1-t.myY2)),abs(t.myZ1-t.myZ2));
    float d13=max(max(abs(t.myX1-t.myX3),abs(t.myY1-t.myY3)),abs(t.myZ1-t.myZ3));
    float d23=max(max(abs(t.myX2-t.myX3),abs(t.myY2-t.myY3)),abs(t.myZ2-t.myZ3));
    if (d12>max_afstand && d12>=d13 && d12>=d23) // Opdel linjen p1-p2 {{{
    { lektier.push_back(Trekant(t.myX1, t.myY1, t.myZ1,
                                (t.myX1+t.myX2)/2.0, (t.myY1+t.myY2)/2.0, (t.myZ1+t.myZ2)/2.0,
                                t.myX3, t.myY3, t.myZ3,
                                t.myR, t.myG, t.myB));
      lektier.push_back(Trekant((t.myX1+t.myX2)/2.0, (t.myY1+t.myY2)/2.0, (t.myZ1+t.myZ2)/2.0,
                                t.myX2, t.myY2, t.myZ2,
                                t.myX3, t.myY3, t.myZ3,
                                t.myR, t.myG, t.myB));
    } // }}}
    else if (d13>max_afstand && d13>=d12 && d13>=d23) // Opdel linjen p1-p3 {{{
    { lektier.push_back(Trekant(t.myX1, t.myY1, t.myZ1,
                                t.myX2, t.myY2, t.myZ2,
                                (t.myX1+t.myX3)/2.0, (t.myY1+t.myY3)/2.0, (t.myZ1+t.myZ3)/2.0,
                                t.myR, t.myG, t.myB));
      lektier.push_back(Trekant((t.myX1+t.myX3)/2.0, (t.myY1+t.myY3)/2.0, (t.myZ1+t.myZ3)/2.0,
                                t.myX2, t.myY2, t.myZ2,
                                t.myX3, t.myY3, t.myZ3,
                                t.myR, t.myG, t.myB));
    } // }}}
    else if (d23>max_afstand && d23>=d12 && d23>=d13) // Opdel linjen p2-p3 {{{
    { lektier.push_back(Trekant(t.myX1, t.myY1, t.myZ1,
                                t.myX2, t.myY2, t.myZ2,
                                (t.myX2+t.myX3)/2.0, (t.myY2+t.myY3)/2.0, (t.myZ2+t.myZ3)/2.0,
                                t.myR, t.myG, t.myB));
      lektier.push_back(Trekant(t.myX1, t.myY1, t.myZ1,
                                (t.myX2+t.myX3)/2.0, (t.myY2+t.myY3)/2.0, (t.myZ2+t.myZ3)/2.0,
                                t.myX3, t.myY3, t.myZ3,
                                t.myR, t.myG, t.myB));
    } // }}}
    else // Trekanten er lille nok
      resultat.push_back(t);
  }

  return resultat;
} // }}}

/* Definer hvad en tilstand er.
 * I en tilstand gemmes de informationer,
 * der beskriver spillets tilstand.
 * Det inkluderer spillerens position
 * (mig_x, mig_y, mig_z) og horisontale
 * og vertikale retning (mig_h og mig_v).
 * Desuden gemmes hvilke spillertaster,
 * der er trykket ned, og om spillet er
 * afsluttet.
 */
class Tilstand // {{{
{ public:
    Tilstand();
    virtual ~Tilstand();
    bool tast_venstre;
    bool tast_hoejre;
    bool tast_op;
    bool tast_ned;
    bool tast_mellemrum;
    bool tast_a;
    bool tast_s;
    bool tast_d;
    bool tast_w;
    vector<Trekant> trekanter;
    float mig_x;
    float mig_y;
    float mig_z;
    float mig_h;
    float mig_h_cos;
    float mig_h_sin;
    float mig_v;
    float mig_v_cos;
    float mig_v_sin;
    float mig_hastighed_x;
    float mig_hastighed_y;
    float mig_hastighed_z;
    float mig_acceleration_x;
    float mig_acceleration_y;
    float mig_acceleration_z;
    bool frem_blokeret;
    bool tilbage_blokeret;
    bool venstre_blokeret;
    bool hoejre_blokeret;
    bool op_blokeret;
    bool ned_blokeret;
    bool tegn_stereogram;
    bool quit;
}; // }}}
Tilstand::Tilstand() // {{{
: tast_venstre(false)
, tast_hoejre(false)
, tast_op(false)
, tast_ned(false)
, tast_mellemrum(false)
, mig_x(0.0)
, mig_y(0.0)
, mig_z(30.0)
, mig_h(M_PI/2)
, mig_h_cos(0.0)
, mig_h_sin(1.0)
, mig_v(0.0)
, mig_v_cos(1.0)
, mig_v_sin(0.0)
, mig_hastighed_x(0.0)
, mig_hastighed_y(0.0)
, mig_hastighed_z(0.0)
, mig_acceleration_x(0.0)
, mig_acceleration_y(0.0)
, mig_acceleration_z(-0.1)
, frem_blokeret(false)
, tilbage_blokeret(false)
, venstre_blokeret(false)
, hoejre_blokeret(false)
, op_blokeret(false)
, ned_blokeret(false)
, tegn_stereogram(false)
, quit(false)
{
} // }}}
Tilstand::~Tilstand() // {{{
{
} // }}}

// sqr beregner kvadratet af et tal
inline float sqr(float x) // {{{
{ return x*x;
} // }}}

// fade_color beregner hvorledes en
// farve falmer, jo længere væk fra
// spilleren den er
inline unsigned char fade_color(unsigned char col, float d) // {{{
{ if (d>=max_dist)
    return 0;
  if (d<=min_dist)
    return col;
  return (unsigned char)(float(col)/(1.0+18.0*sqr((d-min_dist)/(max_dist-min_dist))));
} // }}}

// Tegn en horisontal linje på dest
// De skærmkoordinater der tegnes på,
// gemmes afstanden i zbuf, så det altid
// er det nærmeste objekt der kan ses
inline void tegn_hline(SDL_Surface *dest, vector<float> &zbuf, int y, int x1, float d1, int x2,float d2, unsigned char r, unsigned char g, unsigned char b) // {{{
{ //cout << "tegn_hline(" << y << "," << x1 << "," << d1 << "," << x2 << "," << d2 << "," << r << "," << g << "," << b <<")" << endl;

  if (x2<x1)
    return tegn_hline(dest,zbuf,y,x2,d2,x1,d1,r,g,b);

  if (y<0)
    return;
  if (y>=int(hoejde))
    return;
  if (x2<0)
    return;
  if (x1>=int(bredde))
    return;

  if (x1<0)
  { d1=d1-(d2-d1)*float(x1)/(float(x2)-float(x1));
    x1=0;
  }
  if (x2>bredde)
  { d2=d2+(d1-d2)*(float(x2)-float(bredde))/(float(x2)-float(x1));
    x2=bredde;
  }
  if (d1<=min_dist && d2<=min_dist)
    return;
  if (d1>=max_dist && d2>=max_dist)
    return;
    
  if (x2>x1)
  { float trind=(d2-d1)/(x2-x1);
    float posd=d1;
    for (size_t x=x1; x<x2; ++x)
    { if (posd>min_dist && posd<zbuf[x+y*bredde])
      { //pixelRGBA(dest,x,y,r,g,b,255);
        pixelRGBA(dest,x,y,fade_color(r,posd),fade_color(g,posd),fade_color(b,posd),255);
        zbuf[x+y*bredde]=posd;
      }
      posd+=trind;
    }
  }
} // }}}

// Tegn en trekant på dest ud fra skærmkoordinater
inline void tegn_trekant2d(SDL_Surface *dest, vector<float> &zbuf, int x1, int y1, float d1, int x2, int y2, float d2, int x3, int y3, float d3, int r, int g, int b) // {{{
{ //if (x1>=0 && x1<bredde && y1>=0 && y1<hoejde)
  //{ pixelRGBA(dest,x1,y1,255,255,255,255);
  //  zbuf[x1+y1*bredde]=0.0;
  //}
  //if (x2>=0 && x2<bredde && y2>=0 && y2<hoejde)
  //{ pixelRGBA(dest,x2,y2,255,255,255,255);
  //  zbuf[x2+y2*bredde]=0.0;
  //}
  //if (x3>=0 && x3<bredde && y3>=0 && y3<hoejde)
  //{ pixelRGBA(dest,x3,y3,255,255,255,255);
  //  zbuf[x3+y3*bredde]=0.0;
  //}
  // Test om vi kan afvise at tegne
  if (d1<=min_dist || d2<=min_dist || d3<=min_dist)
    return;

  if (x1<0 && x2<0 && x3<0)
    return;

  if (x1>=bredde && x2>=bredde && x3>=bredde)
    return;

  if (y1<0 && y2<0 && y3<0)
    return;

  if (y1>=hoejde && y2>=hoejde && y3>=hoejde)
    return;

  // Sorter hjørner
  if (y2<y1)
    return tegn_trekant2d(dest,zbuf,x2,y2,d2,x1,y1,d1,x3,y3,d3,r,g,b);

  if (y3<y1)
    return tegn_trekant2d(dest,zbuf,x3,y3,d3,x2,y2,d2,x1,y1,d1,r,g,b);

  if (y3<y2)
    return tegn_trekant2d(dest,zbuf,x1,y1,d1,x3,y3,d3,x2,y2,d2,r,g,b);

  //Tegn hver hlinje
  float posx2=x1;
  float posx3=x1;
  float posd2=d1;
  float posd3=d1;
  int posy=y1;
  if (y2>y1) // Tegn ned til y2
  { float trinx2=(float(x2)-float(x1))/(float(y2)-float(y1));
    float trind2=(d2-d1)/(float(y2)-float(y1));
    float trinx3=(float(x3)-float(x1))/(float(y3)-float(y1));
    float trind3=(d3-d1)/(float(y3)-float(y1));
    for (int y=0; y<y2-y1; ++y)
    { tegn_hline(dest,zbuf,posy,int(posx2),posd2,int(posx3),posd3,r,g,b);
      posx2+=trinx2;
      posd2+=trind2;
      posx3+=trinx3;
      posd3+=trind3;
      posy+=1;
    }
  }
  posx2=x2;
  posd2=d2;
  if (y3>y2) // Tegn ned til y3
  { float trinx2=(float(x3)-float(x2))/(float(y3)-float(y2));
    float trind2=(d3-d2)/(float(y3)-float(y2));
    float trinx3=(float(x3)-float(x1))/(float(y3)-float(y1));
    float trind3=(d3-d1)/(float(y3)-float(y1));
    for (int y=0; y<y3-y2; ++y)
    { tegn_hline(dest,zbuf,posy,int(posx2),posd2,int(posx3),posd3,r,g,b);
      posx2+=trinx2;
      posd2+=trind2;
      posx3+=trinx3;
      posd3+=trind3;
      posy+=1;
    }
  }
} // }}}

// Tegn en trekant ud fra 3D koordinater
inline void tegn_trekant3d(SDL_Surface *dest, vector<float>&zbuf, Tilstand &t, const Trekant &trekant) // {{{
{ // Forskyd koordinater
  float tx1=trekant.myX1-t.mig_x;
  float ty1=trekant.myY1-t.mig_y;
  float tz1=trekant.myZ1-t.mig_z;
  float tx2=trekant.myX2-t.mig_x;
  float ty2=trekant.myY2-t.mig_y;
  float tz2=trekant.myZ2-t.mig_z;
  float tx3=trekant.myX3-t.mig_x;
  float ty3=trekant.myY3-t.mig_y;
  float tz3=trekant.myZ3-t.mig_z;
  // Roter om z-aksen
  float rx1=tx1*t.mig_h_sin-ty1*t.mig_h_cos;
  float ry1=ty1*t.mig_h_sin+tx1*t.mig_h_cos;
  float rz1=tz1;
  float rx2=tx2*t.mig_h_sin-ty2*t.mig_h_cos;
  float ry2=ty2*t.mig_h_sin+tx2*t.mig_h_cos;
  float rz2=tz2;
  float rx3=tx3*t.mig_h_sin-ty3*t.mig_h_cos;
  float ry3=ty3*t.mig_h_sin+tx3*t.mig_h_cos;
  float rz3=tz3;
  // Roter om x-aksen
  float fx1=rx1;
  float fy1=ry1*t.mig_v_cos+rz1*t.mig_v_sin;
  float fz1=rz1*t.mig_v_cos-ry1*t.mig_v_sin;
  float fx2=rx2;
  float fy2=ry2*t.mig_v_cos+rz2*t.mig_v_sin;
  float fz2=rz2*t.mig_v_cos-ry2*t.mig_v_sin;
  float fx3=rx3;
  float fy3=ry3*t.mig_v_cos+rz3*t.mig_v_sin;
  float fz3=rz3*t.mig_v_cos-ry3*t.mig_v_sin;
  // Gem blokeringer {{{
  if (fy1<5*min_dist && fy1>0 && abs(fx1)<min_dist && abs(fz1)<min_dist)
  { t.frem_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (fy2<5*min_dist && fy2>0 && abs(fx2)<min_dist && abs(fz2)<min_dist)
  { t.frem_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (fy3<5*min_dist && fy3>0 && abs(fx3)<min_dist && abs(fz3)<min_dist)
  { t.frem_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (fy1>-5*min_dist && fy1<0 && abs(fx1)<min_dist && abs(fz1)<min_dist)
  { t.tilbage_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (fy2>-5*min_dist && fy2<0 && abs(fx2)<min_dist && abs(fz2)<min_dist)
  { t.tilbage_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (fy3>-5*min_dist && fy3<0 && abs(fx3)<min_dist && abs(fz3)<min_dist)
  { t.tilbage_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (fx1<5*min_dist && fx1>0 && abs(fy1)<min_dist && abs(fz1)<min_dist)
  { t.hoejre_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (fx2<5*min_dist && fx2>0 && abs(fy2)<min_dist && abs(fz2)<min_dist)
  { t.hoejre_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (fx3<5*min_dist && fx3>0 && abs(fy3)<min_dist && abs(fz3)<min_dist)
  { t.hoejre_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (fx1>-5*min_dist && fx1<0 && abs(fy1)<min_dist && abs(fz1)<min_dist)
  { t.venstre_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (fx2>-5*min_dist && fx2<0 && abs(fy2)<min_dist && abs(fz2)<min_dist)
  { t.venstre_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (fx3>-5*min_dist && fx3<0 && abs(fy3)<min_dist && abs(fz3)<min_dist)
  { t.venstre_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (rz1<5*min_dist && rz1>0 && abs(rx1)<min_dist && abs(ry1)<min_dist)
  { t.op_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (rz2<5*min_dist && rz2>0 && abs(rx2)<min_dist && abs(ry2)<min_dist)
  { t.op_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (rz3<5*min_dist && rz3>0 && abs(rx3)<min_dist && abs(ry3)<min_dist)
  { t.op_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (rz1>-5*min_dist && rz1<0 && abs(rx1)<min_dist && abs(ry1)<min_dist)
  { t.ned_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (rz2>-5*min_dist && rz2<0 && abs(rx2)<min_dist && abs(ry2)<min_dist)
  { t.ned_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  if (rz3>-5*min_dist && rz3<0 && abs(rx3)<min_dist && abs(ry3)<min_dist)
  { t.ned_blokeret=true;
    if (trekant.myR>2.0*trekant.myG && trekant.myR>2.0*trekant.myB)
      cout << "Av av av..." << endl;
    if (trekant.myB>trekant.myR && trekant.myB>trekant.myG)
      cout << "Huraaaa!!!!" << endl;
  }
  // }}}
  // Find skaermkoordinater
  if (fy1<min_dist || fy2<min_dist || fy3<min_dist)
    return;
  int x1=int(0.5*bredde+(fx1/fy1)*bredde);
  int y1=int(0.5*hoejde-(fz1/fy1)*bredde);
  int x2=int(0.5*bredde+(fx2/fy2)*bredde);
  int y2=int(0.5*hoejde-(fz2/fy2)*bredde);
  int x3=int(0.5*bredde+(fx3/fy3)*bredde);
  int y3=int(0.5*hoejde-(fz3/fy3)*bredde);
  tegn_trekant2d(dest,zbuf,x1,y1,fy1,x2,y2,fy2,x3,y3,fy3,trekant.myR,trekant.myG, trekant.myB);
} // }}}
// Bevæg spilleren ud fra tilstanden om
// hvilke taster der er trykket, samt
// spillerens retning
inline void bevaeg(Tilstand &t, size_t ticks=10) // {{{
{ t.mig_acceleration_x*=0.8;
  t.mig_acceleration_y*=0.8;
  t.mig_acceleration_z=-0.01;
  t.mig_hastighed_x*=0.8;
  t.mig_hastighed_y*=0.8;
  //t.mig_hastighed_z*=0.95;

  if (t.tast_op)
  { if (t.frem_blokeret) // Rammer objekt
    { t.mig_hastighed_x=0;
      t.mig_acceleration_x=0;
      t.mig_hastighed_y=0;
      t.mig_acceleration_y=0;
    }
    else                 // Accelerer
    { t.mig_acceleration_x+=t.mig_h_cos*0.2;
      t.mig_acceleration_y+=t.mig_h_sin*0.2;
    }
  }
  if (t.tast_ned)
  { if (t.tilbage_blokeret) // Rammer objekt
    { t.mig_hastighed_x=0;
      t.mig_acceleration_x=0;
      t.mig_hastighed_y=0;
      t.mig_acceleration_y=0;
    }
    else                    // Accelerer
    { t.mig_acceleration_x-=t.mig_h_cos*0.2;
      t.mig_acceleration_y-=t.mig_h_sin*0.2;
    }
  }
  if (t.tast_venstre)
  { if (t.venstre_blokeret) // Rammer objekt
    { t.mig_hastighed_x=0;
      t.mig_acceleration_x=0;
      t.mig_hastighed_y=0;
      t.mig_acceleration_y=0;
    }
    else                    // Accelerer
    { t.mig_acceleration_x-=t.mig_h_sin*0.2;
      t.mig_acceleration_y+=t.mig_h_cos*0.2;
    }
  }
  if (t.tast_hoejre)
  { if (t.hoejre_blokeret) // Rammer objekt
    { t.mig_hastighed_x=0;
      t.mig_acceleration_x=0;
      t.mig_hastighed_y=0;
      t.mig_acceleration_y=0;
    }
    else
    { t.mig_acceleration_x+=t.mig_h_sin*0.2;
      t.mig_acceleration_y-=t.mig_h_cos*0.2;
    }
  }
  if (t.mig_acceleration_z<0)
  { if (t.ned_blokeret || t.mig_z<=0) // Lander
    { t.mig_acceleration_z=0;
      t.mig_hastighed_z=0;
      if (t.tast_mellemrum) // # Hop
        t.mig_hastighed_z=1;
    }
  }
  else if (t.mig_acceleration_z>0)
  { if (t.op_blokeret)
    { t.mig_acceleration_z=0;
      t.mig_hastighed_z=0;
    } 
  }

  t.mig_hastighed_x+=t.mig_acceleration_x;
  t.mig_hastighed_y+=t.mig_acceleration_y;
  t.mig_hastighed_z+=t.mig_acceleration_z;

  t.mig_x+=t.mig_hastighed_x*0.01*ticks;
  t.mig_y+=t.mig_hastighed_y*0.01*ticks;
  t.mig_z+=t.mig_hastighed_z*0.01*ticks;
} // }}}
// Håndter hændelser som tastetryk og
// musebevægelser, og gem ændringerne i
// tilstanden
inline void haandter_haendelse(const SDL_Event &e, Tilstand &t) // {{{
{ switch (e.type)
  { case SDL_KEYDOWN:
      switch (e.key.keysym.sym)
      { case 'a': // LEFT
        case 276: // LEFT
          //cout << "Key Left" << endl;
          t.tast_venstre=true;
          break;
        case 'd': // RIGHT
        case 275: // RIGHT
          //cout << "Key Right" << endl;
          t.tast_hoejre=true;
          break;
        case 's': // DOWN
        case 274: // DOWN
          //cout << "Key Down" << endl;
          t.tast_ned=true;
          break;
        case 'w': // UP
        case 273: // UP
          //cout << "Key Up" << endl;
          t.tast_op=true;
          break;
        case 32: // MELLEMRUM
          //cout << "Key Space" << endl;
          t.tast_mellemrum=true;
          break;
        case 27: // ESCAPE
          t.quit=true;
          break;
	case 'x':
	  t.tegn_stereogram=!t.tegn_stereogram;
        default:
          //cout << "Key Down: " << event.key.keysym.sym << endl;
          break;
      }
      break;
    case SDL_KEYUP:
      switch (e.key.keysym.sym)
      { case 'a': // LEFT
        case 276: // LEFT
          //cout << "Key Left" << endl;
          t.tast_venstre=false;
          break;
        case 'd': // RIGHT
        case 275: // RIGHT
          //cout << "Key Right" << endl;
          t.tast_hoejre=false;
          break;
        case 's': // DOWN
        case 274: // DOWN
          //cout << "Key Down" << endl;
          t.tast_ned=false;
          break;
        case 'w': // UP
        case 273: // UP
          //cout << "Key Up" << endl;
          t.tast_op=false;
          break;
        case 32: // MELLEMRUM
          //cout << "Key Space" << endl;
          t.tast_mellemrum=false;
          break;
        case 27: // ESCAPE
          t.quit=true;
          break;
        default:
          //cout << "Key Up: " << event.key.keysym.sym << endl;
          break;
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      //cout << "Mouse buttton down: " << event.button.x << "," << event.button.y << endl;
      break;
    case SDL_MOUSEBUTTONUP:
      //cout << "Mouse buttton up: " << event.button.x << "," << event.button.y << endl;
      break;
    case SDL_MOUSEMOTION:
      //cout << "Mouse moved to: " << event.button.x << "," << event.button.y << endl;
      //cout << "Mouse relative: " << event.motion.xrel << "," << event.motion.yrel << endl;
      if (e.button.x!=bredde/2 || e.button.y!=hoejde/2)
      { t.mig_h-=(float(e.button.x)-float(bredde/2))*M_PI/3000.0;
        while (t.mig_h<0)
          t.mig_h+=M_PI*2.0;
        while (t.mig_h>M_PI*2.0)
          t.mig_h-=M_PI*2.0;
        t.mig_v-=(float(e.button.y)-float(hoejde/2))*M_PI/3000.0;
        if (t.mig_v>M_PI/2.0)
          t.mig_v=M_PI/2.0;
        if (t.mig_v<-M_PI/2.0)
          t.mig_v=-M_PI/2.0;
        t.mig_h_cos=cos(t.mig_h);
        t.mig_h_sin=sin(t.mig_h);
        t.mig_v_cos=cos(t.mig_v);
        t.mig_v_sin=sin(t.mig_v);
        SDL_WarpMouse(bredde/2,hoejde/2);
      }
      break;
      //case SDL_VIDEORESIZE:
      //  memcpy(msgData,(void*)&event,sizeof(event));
      //  mq_send(SendQueue, msgData, sizeof(event), MSGTYPE_RESIZE);
      //  noMove=false;
      //  break;
    case SDL_QUIT:
      //cout << "Window closed!" << endl;
      t.quit=true;
      break;
    default:
      cerr << "Received unknown event: " << e.type << endl;
      break;
  }
} // }}}

// main er selve programmet
int main(int argc, char **argv) // {{{{
{ // Opret vindue
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *primary = SDL_SetVideoMode(bredde,hoejde,WINDOWDEAPTH,SDL_HWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
  SDL_WM_SetCaption("Christians simple 3D program","Christians simple 3D program");
  SDL_ShowCursor(false);
  vector<float> zbuf(bredde*hoejde,max_dist);
  vector<Uint8> sbuf(bredde*hoejde*3,0);
  for (size_t i=0; i<sbuf.size(); ++i)
    sbuf[i]=rand();
  Tilstand tilstand;

  // Åben 3D-Model
  string fname=argv[1];
  string ext=fname.substr(fname.length()-4);
  if (ext==".stl")
    tilstand.trekanter=laes_stl(fname,0,0,255);
  else if (ext==".obj")
    tilstand.trekanter=laes_obj(fname);
  tilstand.trekanter.push_back(Trekant(-100,-100,0,100,-100,0,100,100,0, 0,100,50));
  tilstand.trekanter.push_back(Trekant(-100,-100,0,-100,100,0,100,100,0,50,100, 0));
  cout << "Verdenen indeholder " << tilstand.trekanter.size() << " trekanter." << endl;
  tilstand.trekanter=smaa_trekanter(tilstand.trekanter);
  cout << "Verdenen indeholder " << tilstand.trekanter.size() << " små trekanter." << endl;
  // Kør spil
  size_t ticks=0;
  size_t frameTicks=1;
  while (!tilstand.quit)
  { // Farv skærmen sort
    SDL_FillRect(primary,NULL,SDL_MapRGB(primary->format,0,0,0));
    for (size_t i=0; i<bredde*hoejde; ++i)
      zbuf[i]=max_dist;

    // Nulstil blokeringer
    tilstand.frem_blokeret=false;
    tilstand.tilbage_blokeret=false;
    tilstand.venstre_blokeret=false;
    tilstand.hoejre_blokeret=false;
    tilstand.op_blokeret=false;
    tilstand.ned_blokeret=false;

    // Tegn billede
    for (size_t t=0; t<tilstand.trekanter.size(); ++t)
    { tegn_trekant3d(primary,zbuf,tilstand,tilstand.trekanter[t]);
    }

    // Tegn stereogram
    if (tilstand.tegn_stereogram)
    { //for (size_t i=0; i<sbuf.size(); ++i)
      //  sbuf[i]=rand();
      for (size_t y=0; y<hoejde; ++y)
      { for (size_t x=0; x<bredde; ++x)
        { if (x<200)
            pixelRGBA(primary,x,y,sbuf[3*(y*bredde+x)],sbuf[3*(y*bredde+x)+1],sbuf[3*(y*bredde+x)+2],255);
          else
          { int src_x=(int(x)-100-int(zbuf[x+y*bredde]/2.0));
            sbuf[3*(y*bredde+x)]=sbuf[3*(y*bredde+src_x)];
            sbuf[3*(y*bredde+x)+1]=sbuf[3*(y*bredde+src_x)+1];
            sbuf[3*(y*bredde+x)+2]=sbuf[3*(y*bredde+src_x)+2];
            pixelRGBA(primary,x,y,sbuf[3*(y*bredde+x)],sbuf[3*(y*bredde+x)+1],sbuf[3*(y*bredde+x)+2],255);
          }
        }
      }
    }
    // Beregn og vis FPS
    // ticks bruges også til hvor
    // meget bevægelse der skal ske
    // imellem hvert billede
    size_t ticks2=SDL_GetTicks();
    frameTicks=ticks2-ticks;
    ticks=ticks2;
    stringstream ss;
    ss << "FPS: " << 1000.0/(float(frameTicks));
    stringRGBA(primary,5,5,ss.str().c_str(),0,0,255,255);
    // Flip opdaterer skærmen med det nye billede
    SDL_Flip(primary);
    
    // Bevæg spilleren
    for (size_t i=frameTicks; i>0;)
    { if (i>10)
      { bevaeg(tilstand,10);
        i-=10;
      }
      else
      { bevaeg(tilstand,i);
        i=0;
      }
    }

    // Håndter handlinger
    SDL_Event event;
    while (SDL_PollEvent(&event))
    { haandter_haendelse(event,tilstand);
    }
  }

  SDL_FreeSurface(primary);
  SDL_Quit();
  return 0;
} // }}}


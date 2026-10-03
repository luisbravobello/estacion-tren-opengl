// ESTACION CENTRAL | OpenGL clasico, GLUT y Visual Studio.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <GL/glut.h>
#include "simulacion.h"
#include "sonido.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fstream>
#include <string>

using sim::PI;
using sim::Color;
int ancho=1440,alto=900,anterior=0,foco=0,vista=0;
bool perspectiva=true,luces=true,pausado=false,trenesActivos=true;
bool ciclo=true,techos=true,rayos=false,ayuda=true,arrastrando=false,verificar=false;
bool rotacion=false,escalaSeno=false,traslacion=false,coloresLuz=false,camaraAuto=false;
bool profundidad=true,multivista=false,mostrarNormales=true;
int mouseX=0,mouseY=0,captura=0;
float giro=36,elevacion=29,distancia=230,objetivoX=-4,objetivoZ=-32,objetivoY=2;
float hora=9,fps=0;
double tiempo=14,tiempoPersonas=0;
GLuint escenaLista=0;
Sonido sonido;
sim::Tren tren[2];
sim::Ambiente ambiente;
struct V3{float x,y,z;};
V3 operator+(V3 a,V3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
V3 operator-(V3 a,V3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
V3 operator*(V3 a,float n){return {a.x*n,a.y*n,a.z*n};}
float punto(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
V3 normalizar(V3 a){return a*(1/std::max(.0001f,std::sqrt(punto(a,a))));}
V3 ojo;
const float centros[]={-10,10,30};
V3 centroPieza(){
    const float t=float(tiempoPersonas);
    return {30+(traslacion?3*std::sin(t):0),4.4f+(traslacion?.6f*std::sin(t*1.4f):0),-17+(traslacion?2*std::cos(t):0)};
}
float escalaPieza(){return escalaSeno?1+.23f*std::sin(float(tiempoPersonas)*2):1;}

void color(float r,float g,float b,float brillo=20){
    glColor3f(r,g,b);const GLfloat e[]={.42f,.42f,.42f,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,e);glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,brillo);
}
void emision(float r=0,float g=0,float b=0){const GLfloat e[]={r,g,b,1};glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,e);}
void caja(float x,float y,float z,float sx,float sy,float sz){
    glPushMatrix();glTranslatef(x,y,z);glScalef(sx,sy,sz);glutSolidCube(1);glPopMatrix();
}
void esfera(float x,float y,float z,float radio,int detalle=12){
    glPushMatrix();glTranslatef(x,y,z);glutSolidSphere(radio,detalle,detalle);glPopMatrix();
}
// Normales explicitas y teselacion para la iluminacion por vertice.
void plano(float x0,float x1,float z0,float z1,float y,float paso){
    glBegin(GL_QUADS);glNormal3f(0,1,0);
    for(float x=x0;x<x1;x+=paso)for(float z=z0;z<z1;z+=paso){
        const float xx=std::min(x1,x+paso),zz=std::min(z1,z+paso);
        glVertex3f(x,y,z);glVertex3f(x,y,zz);glVertex3f(xx,y,zz);glVertex3f(xx,y,z);
    }glEnd();
}
void rotulo(float x,float y,float z,const char* s,float escala=.015f){
    glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);glColor3f(.94f,.95f,.87f);
    glPushMatrix();glTranslatef(x,y,z);glScalef(escala,escala,escala);
    for(;*s;++s)glutStrokeCharacter(GLUT_STROKE_ROMAN,*s);
    glPopMatrix();glPopAttrib();
}

// METODO 1: P/O cambian SOLO la proyeccion; la camara se conserva.
void aplicarProyeccion(int x,int y,int w,int h,bool modo){
    const double aspecto=double(w)/std::max(1,h);
    glViewport(x,y,w,h);glMatrixMode(GL_PROJECTION);glLoadIdentity();
    if(modo)gluPerspective(50,aspecto,.5,1100);
    else{const double semialto=distancia*std::tan(25.0*PI/180);
        glOrtho(-semialto*aspecto,semialto*aspecto,-semialto,semialto,.5,1100);}
    glMatrixMode(GL_MODELVIEW);
}
// METODO 2: transformacion de vista comun a ambas proyecciones.
void colocarCamara(){
    const float a=giro*PI/180,e=elevacion*PI/180;
    ojo={objetivoX+distancia*std::sin(a)*std::cos(e),objetivoY+distancia*std::sin(e),objetivoZ+distancia*std::cos(a)*std::cos(e)};
    glLoadIdentity();gluLookAt(ojo.x,ojo.y,ojo.z,objetivoX,objetivoY,objetivoZ,0,1,0);
}
V3 direccionSol(){const float a=(hora-6)*PI/12;return normalizar({std::cos(a),std::max(.18f,std::sin(a)),.35f});}
V3 posicionLampara(int i){return {centros[i/2],9.7f,i%2?-10.0f:-88.0f};}
// METODO 3: posiciones de luz en el mundo, DESPUES de gluLookAt.
void configurarLuces(){
    if(!luces){glDisable(GL_LIGHTING);return;}glEnable(GL_LIGHTING);glEnable(GL_LIGHT0);
    const GLfloat amb[]={ambiente.ambiente.r,ambiente.ambiente.g,ambiente.ambiente.b,1};glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb);
    const V3 s=direccionSol();const GLfloat pos[]={s.x,s.y,s.z,0}; // w=0: direccional.
    const GLfloat luz[]={ambiente.sol.r,ambiente.sol.g,ambiente.sol.b,1};
    glLightfv(GL_LIGHT0,GL_POSITION,pos);glLightfv(GL_LIGHT0,GL_DIFFUSE,luz);glLightfv(GL_LIGHT0,GL_SPECULAR,luz);
    // Visor local: el brillo especular responde a la posicion real de la camara.
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,perspectiva?GL_TRUE:GL_FALSE);
    for(int i=0;i<6;++i){
        const GLenum id=GL_LIGHT1+i;const V3 p=posicionLampara(i);const GLfloat lp[]={p.x,p.y,p.z,1};
        const Color rgb[]={{1,.20f,.12f},{.18f,1,.36f},{.22f,.40f,1}};
        const float k=coloresLuz?1:ambiente.lamparas;
        const Color c=coloresLuz?rgb[i%3]:Color{1,.77f,.43f};const GLfloat calida[]={c.r*k,c.g*k,c.b*k,1};
        glEnable(id);glLightfv(id,GL_POSITION,lp);glLightfv(id,GL_DIFFUSE,calida);glLightfv(id,GL_SPECULAR,calida);
        glLightf(id,GL_CONSTANT_ATTENUATION,1);glLightf(id,GL_LINEAR_ATTENUATION,.035f);glLightf(id,GL_QUADRATIC_ATTENUATION,.006f);
    }
    glEnable(GL_LIGHT7);const GLfloat faro[]={0,3.2f,tren[0].z+tren[0].sentido*35.2f,1};
    const GLfloat frente[]={0,-.08f,float(tren[0].sentido)},haz[]={.9f,.9f,.68f,1};
    glLightfv(GL_LIGHT7,GL_POSITION,faro);glLightfv(GL_LIGHT7,GL_SPOT_DIRECTION,frente);
    glLightfv(GL_LIGHT7,GL_DIFFUSE,haz);glLightfv(GL_LIGHT7,GL_SPECULAR,haz);
    glLightf(GL_LIGHT7,GL_SPOT_CUTOFF,27);glLightf(GL_LIGHT7,GL_SPOT_EXPONENT,12);
    glLightf(GL_LIGHT7,GL_LINEAR_ATTENUATION,.015f);glLightf(GL_LIGHT7,GL_QUADRATIC_ATTENUATION,.001f);
}
void arbol(float x,float z,float h,int tipo=0){
    color(.31f,.22f,.15f);caja(x,h*.32f,z,.65f,h*.65f,.65f);color(.17f+tipo*.035f,.37f+tipo*.03f,.27f);
    if(tipo%2){esfera(x,h*.76f,z,h*.35f,8);esfera(x-.8f,h*.63f,z+.5f,h*.24f,8);}
    else for(int k=0;k<3;++k){glPushMatrix();glTranslatef(x,h*(.27f+k*.2f),z);glRotatef(-90,1,0,0);glutSolidCone(h*(.38f-k*.075f),h*.6f,8,1);glPopMatrix();}
}
void banco(float x,float z){
    color(.50f,.29f,.14f);caja(x,2.3f,z,1.7f,.25f,4.4f);caja(x-.7f,2.9f,z,.2f,1.2f,4.4f);
    color(.14f,.22f,.24f);caja(x,1.9f,z-1.6f,1.4f,.7f,.25f);caja(x,1.9f,z+1.6f,1.4f,.7f,.25f);
}
void andenes(float zCentro,float largo,bool principal){
    for(int p=0;p<(principal?3:2);++p){
        const float x=centros[p],ini=zCentro-largo/2,fin=zCentro+largo/2;
        color(.47f,.51f,.51f);caja(x,.7f,zCentro,12,1.7f,largo);
        color(.67f,.67f,.59f);plano(x-6,x+6,ini,fin,1.56f,2);
        color(.99f,.72f,.23f);plano(x-5.85f,x-5.35f,ini,fin,1.59f,2);plano(x+5.35f,x+5.85f,ini,fin,1.59f,2);
        for(float z=ini+6;z<fin-4;z+=14){
            color(.09f,.29f,.31f);caja(x,5.7f,z,.5f,8.2f,.5f);caja(x,9.4f,z,9,.3f,.4f);
            if(principal){banco(x-2.8f,z+3);color(.19f,.36f,.37f);caja(x+2.2f,2.2f,z,1,1.4f,1);}
        }
        color(.04f,.16f,.20f);caja(x,7.3f,fin-9,7.5f,2,.23f);
        char s[32];std::snprintf(s,sizeof(s),"%s %d",principal?"CENTRAL":"PARQUE",p+1);rotulo(x-3.3f,6.9f,fin-8.86f,s,.011f);
    }
}
void edificioEstacion(){
    color(.81f,.72f,.54f);caja(-32,6.6f,-50,23,13,67);color(.88f,.82f,.67f);caja(-32,13.4f,-50,25,.75f,70);
    color(.19f,.33f,.35f);caja(-32,14.2f,-50,26,.6f,72);
    for(float z=-76;z<-20;z+=8){
        color(.13f,.28f,.33f,70);caja(-20.42f,6,z,.06f,6,5);
        color(.92f,.85f,.66f);caja(-20.35f,6,z,.1f,.16f,5.1f);caja(-20.35f,6,z,.1f,6,.16f);
    }
    color(.88f,.79f,.60f);caja(-32,18,-20,12,10,9);color(.19f,.32f,.34f);caja(-32,23.2f,-20,13,.8f,10);
    color(.04f,.17f,.22f);caja(-32,10.5f,-15.4f,21,2.8f,.2f);rotulo(-41,9.8f,-15.25f,"ESTACION CENTRAL",.018f);
    color(.10f,.26f,.30f);caja(-32,4,-15.4f,8,7,.25f);
    for(int i=0;i<4;++i){color(.65f,.64f,.56f);caja(-32,.25f+i*.35f,-10-i*.8f,12,.5f,1.5f);}
    color(.18f,.32f,.34f);caja(10,13.2f,-60,49,.7f,5);
    for(int i=0;i<18;++i){color(.56f,.60f,.59f);caja(30,1.7f+i*.66f,-83+i*1.2f,4,.4f,1.4f);caja(-10,1.7f+i*.66f,-83+i*1.2f,4,.4f,1.4f);}
    for(int j=-1;j<=1;j+=2){color(.88f,.72f,.38f);caja(10,15,-60+j*2.5f,49,.16f,.16f);for(int i=-14;i<=34;i+=3)caja(float(i),14.2f,-60+j*2.5f,.13f,1.8f,.13f);}
    color(.85f,.44f,.26f);caja(-12,3.7f,7,4,4.2f,5);color(.97f,.83f,.50f);caja(-12,6,7,4.8f,.45f,5.6f);
    color(.07f,.22f,.28f);caja(-12,4.6f,9.6f,3.2f,1.5f,.05f);
    for(int i=0;i<3;++i){color(.15f,.43f,.51f);caja(-18,2.9f,-31+i*3.2f,1.1f,2.8f,1.8f);}
}
void edificio(float x,float z,float h,int tipo){
    const Color paleta[]={{.70f,.62f,.51f},{.51f,.62f,.63f},{.70f,.48f,.37f},{.62f,.65f,.58f}};
    Color c=paleta[tipo%4];color(c.r,c.g,c.b);caja(x,h/2,z,16,h,17);color(.27f,.33f,.34f);caja(x,h+.4f,z,17,.8f,18);
    color(.12f,.24f,.29f);for(float y=3;y<h-1;y+=4)for(int k=-1;k<=1;++k){caja(x+8.02f,y,z+k*4.5f,.05f,2.1f,2.2f);caja(x+k*4.5f,y,z+8.52f,2.2f,2.1f,.05f);}
}
void crearEntorno(){
    color(.29f,.43f,.30f);plano(-120,110,-290,300,-.32f,12);
    color(.21f,.25f,.27f);plano(-63,-47,-275,280,-.2f,7);color(.66f,.66f,.58f);plano(-47,-43,-275,280,-.12f,7);
    color(.90f,.84f,.66f);for(int z=-270;z<280;z+=12)caja(-55,-.15f,float(z),.18f,.05f,5);
    for(int t=0;t<2;++t){const float x=t*20.0f;
        color(.35f,.37f,.36f);plano(x-3.6f,x+3.6f,-275,280,.05f,4);
        color(.34f,.23f,.16f);for(int z=-272;z<280;z+=3)caja(x,.18f,float(z),5.6f,.22f,.65f);
        color(.62f,.68f,.69f,80);caja(x-1.8f,.44f,2.5f,.17f,.3f,555);caja(x+1.8f,.44f,2.5f,.17f,.3f,555);
    }
    andenes(-50,140,true);andenes(150,100,false);edificioEstacion();
    color(.76f,.73f,.57f);caja(-25,4,150,14,8,25);color(.21f,.35f,.31f);caja(-25,8.5f,150,16,1,27);rotulo(-31,5.5f,162.6f,"PARQUE",.02f);
    for(int k=0;k<10;++k)edificio(-80,float(-220+k*46),float(12+(k*7)%23),k);
    for(int i=0;i<33;++i){const float z=-255.0f+i*16;arbol(47+float(i%3)*6,z,7+float(i%4),i%3);if(i%2==0)arbol(-41,z,6.5f,1);}
    color(.72f,.67f,.52f);plano(57,88,-50,90,-.16f,8);color(.54f,.60f,.56f);caja(72,.5f,30,12,1.4f,12);
    color(.20f,.55f,.63f,85);plano(66.5f,77.5f,24.5f,35.5f,1.22f,2);color(.78f,.77f,.64f);caja(72,2,30,1,3,1);esfera(72,3.6f,30,.8f);
    for(int i=0;i<7;++i){arbol(87,float(-40+i*20),8,1);arbol(58,float(-40+i*20),6,1);}
    for(int i=0;i<10;++i){color(.22f+i%3*.025f,.35f,.31f);glPushMatrix();glTranslatef(130+float(i%2)*25,-2,-280+float(i)*63);glScalef(1,.65f,1.3f);glRotatef(-90,1,0,0);glutSolidCone(42,48+float(i%4)*9,7,1);glPopMatrix();}
    color(.19f,.29f,.33f);caja(30,2.25f,-17,3.4f,1.4f,3.4f);rotulo(27.8f,2.3f,-15.25f,"LUZ",.016f);
}
void cubiertas(){
    if(!techos)return;
    for(int p=0;p<3;++p){color(.12f,.30f,.32f);caja(centros[p],9.9f,-50,8,.4f,129);color(.87f,.75f,.48f);caja(centros[p]-4,9.85f,-50,.15f,.5f,129);}
    for(int p=0;p<2;++p){color(.21f,.38f,.31f);caja(centros[p],9.9f,150,8,.4f,88);}
}
void lamparasYVentanas(){
    const float k=luces?(coloresLuz?1:ambiente.lamparas):0;
    for(int i=0;i<6;++i){V3 p=posicionLampara(i);color(.13f,.23f,.25f);caja(p.x+3.5f,5.5f,p.z,.22f,8,.22f);caja(p.x+1.75f,9.6f,p.z,3.5f,.18f,.18f);
        const Color rgb[]={{1,.20f,.12f},{.18f,1,.36f},{.22f,.40f,1}};
        const Color c=coloresLuz?rgb[i%3]:Color{1,.86f,.58f};
        color(c.r,c.g,c.b);emision(k*c.r,k*c.g,k*c.b);esfera(p.x,p.y,p.z,.48f);emision();}
    emision(.64f*k,.37f*k,.12f*k);color(.86f,.76f,.47f);
    for(int b=0;b<10;++b){const float z=-220.0f+b*46,h=float(12+(b*7)%23);
        for(float y=3;y<h-1;y+=4)for(int w=-1;w<=1;++w)if((b+w+int(y))%3!=0)caja(-71.95f,y,z+w*4.5f,.045f,1.7f,1.8f);}
    emision();
    // Objeto de demostracion: transformaciones de modelo independientes de la camara.
    const V3 c=centroPieza();glPushMatrix();glTranslatef(c.x,c.y,c.z);
    if(rotacion)glRotatef(float(tiempoPersonas)*45,0,1,0);
    const float s=escalaPieza();glScalef(s,s,s);
    color(.56f,.70f,.73f,100);const GLfloat brillo[]={1,1,1,1};glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,brillo);esfera(0,0,0,1.5f,40);
    // Anillo y marcas hacen visible la rotacion de una esfera.
    color(.93f,.56f,.18f,75);glPushMatrix();glRotatef(35,1,0,0);glutSolidTorus(.08,1.65,10,40);glPopMatrix();
    color(.09f,.32f,.37f);caja(0,0,1.49f,.30f,.7f,.12f);glPopMatrix();
}
void rueda(float x,float z,float angulo){
    glPushMatrix();
    glTranslatef(x,1.08f,z);
    glRotatef(90,0,1,0);
    glRotatef(angulo,0,0,1);
    color(.10f,.13f,.14f,60);
    glutSolidTorus(.20,.49,8,16);
    color(.61f,.65f,.64f);
    caja(0,0,0,1,.10f,.1f);
    caja(0,0,0,.1f,1,.1f);
    glPopMatrix();
}
void vagon(int servicio,int i){
    const sim::Tren& t=tren[servicio];const float z=t.z+(i-1.5f)*18;
    glPushMatrix();glTranslatef(servicio*20.0f,0,z);
    color(.13f,.20f,.22f);caja(0,1.5f,0,4.7f,.9f,16.2f);
    color(.90f,.88f,.77f,65);caja(0,3.8f,0,5,3.8f,16.4f);
    color(servicio?.58f:.06f,servicio?.23f:.40f,servicio?.24f:.45f,65);caja(0,2.9f,0,5.06f,1.3f,16.46f);
    color(.18f,.29f,.31f,70);caja(0,5.8f,0,5.2f,.45f,16.6f);
    color(.88f,.59f,.23f);caja(0,2.14f,0,5.08f,.16f,16.5f);
    color(.11f,.21f,.25f,95);const float interior=luces?ambiente.lamparas:0;
    emision(interior*.08f,interior*.10f,interior*.08f);
    for(int lado=-1;lado<=1;lado+=2)for(int w=-1;w<=1;++w)caja(lado*2.52f,4.55f,float(w)*3.3f,.045f,1.35f,2.5f);
    emision();
    for(int lado=-1;lado<=1;lado+=2){
        // Hueco y hojas corredizas; solo se abren durante las paradas.
        color(.035f,.075f,.09f);caja(lado*2.54f,3.55f,6.1f,.03f,2.8f,2.1f);
        for(int hoja=-1;hoja<=1;hoja+=2){const float zz=6.1f+hoja*(.52f+t.puertas*.88f);
            color(.63f,.71f,.68f,40);caja(lado*2.57f,3.55f,zz,.06f,2.8f,1);
            color(.11f,.24f,.28f);caja(lado*2.61f,4.22f,zz,.035f,.95f,.66f);}
        for(int e=-1;e<=1;e+=2)rueda(lado*1.87f,e*5.5f,-t.z*85);
    }
    if(i==0||i==3){const int lado=i==0?-1:1;
        color(.10f,.23f,.27f,95);caja(0,4.55f,lado*8.23f,3.9f,1.6f,.035f);
        const bool frontal=lado==t.sentido;
        for(int f=-1;f<=1;f+=2){color(frontal?1:.8f,frontal?.94f:.12f,frontal?.7f:.07f);
            if(luces)emision(frontal?.85f:.6f,frontal?.78f:.04f,frontal?.45f:.01f);
            esfera(f*1.55f,3,lado*8.28f,.25f);emision();}
    }
    if(i<3){color(.16f,.21f,.22f);caja(0,2.1f,9,1,.5f,1.8f);}glPopMatrix();
}
void extremidad(float x,float y,float angulo,float largo){
    glPushMatrix();glTranslatef(x,y,0);glRotatef(angulo,1,0,0);caja(0,-largo/2,0,.22f,largo,.27f);glPopMatrix();
}
void persona(float x,float z,float rumbo,float fase,int id,bool camina=true){
    const Color ropa[]={{.87f,.39f,.22f},{.16f,.43f,.67f},{.78f,.64f,.22f},{.47f,.29f,.50f},{.24f,.54f,.41f}};
    glPushMatrix();glTranslatef(x,1.58f,z);glRotatef(rumbo,0,1,0);
    const float paso=camina?std::sin(fase)*25:0;
    color(.16f,.21f,.25f);extremidad(-.2f,.9f,paso,.9f);extremidad(.2f,.9f,-paso,.9f);
    Color c=ropa[id%5];color(c.r,c.g,c.b);caja(0,1.4f,0,.78f,1.05f,.43f);
    extremidad(-.5f,1.86f,-paso,.84f);extremidad(.5f,1.86f,paso,.84f);
    color(.73f+(id%3)*.06f,.49f+(id%3)*.05f,.32f+(id%3)*.04f);esfera(0,2.17f,0,.32f,9);
    color(.19f,.13f,.10f);esfera(0,2.34f,-.025f,.28f,8);
    if(id%3==0){color(.25f,.29f,.31f);caja(.75f,.45f,-.1f,.42f,.8f,.65f);}glPopMatrix();
}
void pasajeros(){
    for(int i=0;i<66;++i){const bool parque=i>=45,caminar=i%4!=0;
        float u=std::fmod(float(tiempoPersonas)*.85f+i*7.7f,parque?160.0f:244.0f);
        const float mitad=parque?80.0f:122.0f;
        const float z=(parque?110.0f:-111.0f)+(u<mitad?u:2*mitad-u);
        const float x=centros[parque?i%2:i%3]+(i%2?4.4f:-4.4f);
        persona(x,caminar?z:(parque?118.0f:-103.0f)+(i%9)*10,u<mitad?0.0f:180.0f,float(tiempoPersonas)*5+i,i,caminar);
    }
    // Cruzan el umbral mientras las puertas estan abiertas; se ocultan al entrar.
    for(int s=0;s<2;++s)if(tren[s].estacion>=0&&tren[s].puertas>.95f){
        const float avance=sim::limitar((8-tren[s].restante)/5);
        if(avance<.96f)for(int i=0;i<4;++i)persona(s*20-7+avance*4.2f,tren[s].z+(i-1.5f)*18+6.1f,90,float(tiempoPersonas)*7,i+3,true);
    }
}
void coches(){
    for(int i=0;i<5;++i){float z=std::fmod(float(tiempoPersonas)*7+i*105.0f,540.0f)-270;
        const float x=i%2?-59.0f:-51.0f;if(i%2)z=-z;
        color(i%2?.76f:.22f,.40f,i%2?.27f:.60f,55);caja(x,1.2f,z,3.2f,1.4f,6);
        color(.13f,.25f,.28f,80);caja(x,2.2f,z-.4f,2.8f,1,3.3f);
        color(.09f,.12f,.14f);for(int a=-1;a<=1;a+=2)for(int b=-1;b<=1;b+=2)esfera(x+a*1.55f,.65f,z+b*1.9f,.55f,8);
    }
}
void relojEstacion(){
    glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LINE_BIT);glDisable(GL_LIGHTING);glColor3f(.93f,.88f,.71f);
    glPushMatrix();glTranslatef(-32,18.5f,-15.42f);glBegin(GL_TRIANGLE_FAN);glVertex3f(0,0,0);
    for(int i=0;i<=40;++i){float a=i*2*PI/40;glVertex3f(2.25f*std::cos(a),2.25f*std::sin(a),0);}glEnd();
    glColor3f(.09f,.21f,.25f);glLineWidth(2);glBegin(GL_LINES);
    for(int i=0;i<12;++i){float a=i*PI/6;glVertex3f(1.85f*std::sin(a),1.85f*std::cos(a),.02f);glVertex3f(2.1f*std::sin(a),2.1f*std::cos(a),.02f);}
    const float h=hora*PI/6,m=(hora-std::floor(hora))*2*PI;
    glVertex3f(0,0,.03f);glVertex3f(1.15f*std::sin(h),1.15f*std::cos(h),.03f);
    glVertex3f(0,0,.04f);glVertex3f(1.75f*std::sin(m),1.75f*std::cos(m),.04f);glEnd();glPopMatrix();glPopAttrib();
}

// METODO 4: reflexion especular ideal R = I - 2 dot(I,N) N.
// Las flechas son guias geometricas; las superficies se iluminan con OpenGL.
void flecha(V3 a,V3 b,Color c){
    const V3 d=normalizar(b-a);V3 lateral=normalizar({-d.z,0,d.x});if(std::fabs(d.y)>.98f)lateral={1,0,0};
    glColor3f(c.r,c.g,c.b);glBegin(GL_LINES);glVertex3f(a.x,a.y,a.z);glVertex3f(b.x,b.y,b.z);
    const V3 q=b-d*.8f,r=q+lateral*.3f,s=q-lateral*.3f;
    glVertex3f(b.x,b.y,b.z);glVertex3f(r.x,r.y,r.z);glVertex3f(b.x,b.y,b.z);glVertex3f(s.x,s.y,s.z);glEnd();
}
void rayosDidacticos(){
    if(!rayos&&!mostrarNormales)return;
    const V3 centro=centroPieza();
    // Cara orientada hacia la fuente para evitar un rayo que atraviese la esfera.
    const bool esSol=ambiente.dia>.4f&&!coloresLuz;
    const V3 haciaFuente=esSol?direccionSol():normalizar(posicionLampara(5)-centro);
    const V3 n=normalizar(haciaFuente+V3{.28f,.15f,.20f});const V3 p=centro+n*(1.5f*escalaPieza());
    const V3 origen=esSol?p+direccionSol()*13:posicionLampara(5);
    const V3 incidente=normalizar(p-origen),reflejado=incidente-n*(2*punto(incidente,n));
    glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LINE_BIT);glDisable(GL_LIGHTING);glLineWidth(3);
    flecha(p,p+n*5,{.29f,.92f,.98f});
    if(mostrarNormales)for(int j=0;j<8;++j){const float a=j*PI/4;const V3 nn=normalizar({std::cos(a),.5f,std::sin(a)});
        const V3 pp=centro+nn*(1.5f*escalaPieza());flecha(pp,pp+nn*1.7f,{.29f,.92f,.98f});}
    if(rayos){flecha(origen,p,{1,.73f,.20f});flecha(p,p+reflejado*10,{1,.34f,.48f});}
    glColor3f(1,.91f,.45f);const float t=std::fmod(float(tiempoPersonas)*.5f,1.0f);const V3 viajero=origen+(p-origen)*t;
    if(rayos)esfera(viajero.x,viajero.y,viajero.z,.12f,8);rotulo(p.x+1,p.y+5,p.z,"N",.013f);glPopAttrib();
}
void texto(int x,int y,const char* s,void* fuente=GLUT_BITMAP_HELVETICA_12){glRasterPos2i(x,y);for(;*s;++s)glutBitmapCharacter(fuente,*s);}
void rect(float x,float y,float w,float h,Color c){
    glColor3f(c.r,c.g,c.b);glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+w,y);glVertex2f(x+w,y+h);glVertex2f(x,y+h);glEnd();
}
const char* tema(){if(hora>=6&&hora<12)return "MANANA";if(hora>=12&&hora<19.5f)return "TARDE";return "NOCHE";}
void interfaz(){
    glViewport(0,0,ancho,alto);glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LINE_BIT);
    glDisable(GL_LIGHTING);glDisable(GL_DEPTH_TEST);glDisable(GL_FOG);
    glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();gluOrtho2D(0,ancho,0,alto);
    glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();
    rect(0,float(alto-88),float(ancho),88,{.035f,.09f,.12f});rect(0,0,float(ancho),112,{.035f,.09f,.12f});
    glColor3f(.96f,.78f,.39f);texto(24,alto-31,"ESTACION CENTRAL",GLUT_BITMAP_HELVETICA_18);
    glColor3f(.69f,.83f,.84f);texto(24,alto-53,"SIMULACION FERROVIARIA  /  CENTRAL - PARQUE");
    char s[180];const int hh=int(hora),mm=int((hora-hh)*60);
    std::snprintf(s,sizeof(s),"%02d:%02d  %s  |  %s  |  %s",hh,mm,tema(),ciclo?"CICLO AUTO":"HORA FIJA",pausado?"PAUSA":"EN VIVO");
    glColor3f(.94f,.94f,.84f);texto(24,alto-74,s);
    rect(float(ancho-290),float(alto-62),265,39,{.10f,.27f,.30f});glColor3f(.89f,.96f,.91f);
    texto(ancho-275,alto-46,perspectiva?"PROYECCION: PERSPECTIVA":"PROYECCION: ORTOGRAFICA");
    glColor3f(.64f,.77f,.79f);texto(24,87,"CENTRAL");texto(267,87,"PARQUE");rect(25,67,292,3,{.25f,.41f,.44f});
    rect(25+sim::limitar((tren[0].z+50)/200)*285,62,8,13,{.98f,.73f,.29f});
    std::snprintf(s,sizeof(s),"TREN 01  %s  |  %.0f s  |  %.0f km/h",tren[0].estacion<0?(tren[0].sentido>0?"HACIA PARQUE":"HACIA CENTRAL"):
        (tren[0].puertas>.1f?"PUERTAS ABIERTAS":"EN ANDEN"),tren[0].restante,std::fabs(tren[0].velocidad)*3.6f);
    glColor3f(.92f,.92f,.82f);texto(345,85,s);
    std::snprintf(s,sizeof(s),"Sonido: %s | Luces: %s | %.0f FPS",sonido.disponible()?(sonido.activo?"ON":"OFF"):"SIN DISPOSITIVO",luces?"ON":"OFF",fps);texto(345,65,s);
    glColor3f(.68f,.81f,.83f);
    texto(24,39,"P/O Proyeccion   V Vistas   F Acercar objeto   K Rayos   H Cubiertas   Flechas/arrastrar Camara   +/- Zoom");
    texto(24,19,"J Manana   E Tarde   N Noche   C Ciclo solar   M Sonido   B Bocina   Espacio Pausa   T Trenes   R Reinicio   Esc Salir");
    if(ayuda){
        const int x=ancho-300,y=alto-111;
        rect(float(x-12),float(y-220),288,232,{.06f,.15f,.18f});
        const char* nombres[]={"[1] Rotacion","[2] Escala (seno)","[3] Traslacion (sen/cos)","[4] Iluminacion y normales","[5] Luz de colores","[6] Camara automatica","[7] Proyeccion","[8] Test de profundidad","[9] Viewports"};
        const bool estados[]={rotacion,escalaSeno,traslacion,luces,coloresLuz,camaraAuto,perspectiva,profundidad,multivista};
        glColor3f(.96f,.78f,.39f);texto(x,y-10,"CONTROLES DE LA PRACTICA");
        for(int i=0;i<9;++i){glColor3f(estados[i]?.51f:.65f,estados[i]?.91f:.73f,estados[i]?.68f:.76f);
            texto(x,y-34-i*20,nombres[i]);texto(x+180,y-34-i*20,i==6?(perspectiva?"PERSPECTIVA":"ORTOGRAFICA"):i==8?(multivista?"4 VISTAS":"1 VISTA"):(estados[i]?"ON":"OFF"));}
        glColor3f(.72f,.80f,.79f);texto(x,y-217,"1/2/3 actuan sobre la pieza del laboratorio.");
    }
    if(ayuda){const int y=alto-110;rect(18,float(y-104),530,112,{.06f,.15f,.18f});
        glColor3f(.98f,.77f,.34f);texto(31,y-13,foco==4?"LABORATORIO DE LUZ  /  K: GUIAS DE RAYOS":"OBSERVA Y COMPARA");glColor3f(.89f,.94f,.91f);
        if(foco==4){texto(31,y-36,"Amarillo: luz incidente. Cian: normal. Rosa: reflexion.");
            texto(31,y-55,"La normal orienta la luz difusa; el material controla el brillo.");
            texto(31,y-74,"1/2/3 transforma la pieza. J/E/N cambia la hora.");
            texto(31,y-93,!luces?"Luz desactivada: las flechas muestran solo la geometria.":ambiente.dia>.4f&&!coloresLuz?"Fuente: SOL. Brillo especular: depende de la camara.":"Fuente: LAMPARA. Su intensidad disminuye con la distancia.");
        }else{texto(31,y-36,perspectiva?"Perspectiva: los objetos lejanos se ven mas pequenos.":"Ortografica: objetos iguales conservan su tamano a distinta distancia.");
            texto(31,y-55,perspectiva?"Los rieles convergen. Pulsa O sin mover la camara.":"Los rieles quedan paralelos. Pulsa P para comparar.");
            texto(31,y-74,foco==1?"Tren: acelera, frena y abre las puertas al detenerse.":foco==2?"Pasajeros: caminan y suben durante la parada.":foco==3?"Parque: segunda parada. El tren vuelve a Central.":"F acerca: tren > pasajeros > Parque > laboratorio de luz.");
            texto(31,y-93,"L compara la iluminacion. ? muestra/oculta estos mensajes.");}
    }
    if(multivista){
        const int mitad=(alto-200)/2;
        rect(float(ancho/2-1),112,2,float(alto-200),{.10f,.28f,.30f});rect(0,float(112+mitad),float(ancho),2,{.10f,.28f,.30f});
        glColor3f(1,.85f,.4f);texto(20,112+mitad+15,perspectiva?"PRINCIPAL / PERSPECTIVA":"PRINCIPAL / ORTOGRAFICA");
        texto(ancho/2+15,112+mitad+15,"PLANTA / ORTOGRAFICA");texto(20,128,"FRONTAL / ORTOGRAFICA");texto(ancho/2+15,128,"LATERAL / ORTOGRAFICA");
    }
    if(pausado){glColor3f(1,.78f,.35f);texto(ancho/2-35,alto/2,"PAUSA",GLUT_BITMAP_HELVETICA_18);}
    glPopMatrix();glMatrixMode(GL_PROJECTION);glPopMatrix();glMatrixMode(GL_MODELVIEW);glPopAttrib();
}
void seleccionarFoco(int nuevo){
    foco=nuevo;ayuda=true;
    if(foco==0){distancia=230;giro=36;elevacion=29;objetivoX=-4;objetivoY=2;objetivoZ=-32;}
    if(foco==1){distancia=65;giro=62;elevacion=18;objetivoX=0;objetivoY=3;objetivoZ=tren[0].z;}
    if(foco==2){distancia=28;giro=65;elevacion=17;objetivoX=13;objetivoY=3;objetivoZ=-30;techos=false;}
    if(foco==3){distancia=140;giro=38;elevacion=26;objetivoX=0;objetivoY=3;objetivoZ=150;}
    if(foco==4){distancia=26;giro=45;elevacion=20;objetivoX=30;objetivoY=6;objetivoZ=-16;rayos=true;techos=false;}
}
void seleccionarVista(){
    vista=(vista+1)%4;seleccionarFoco(0);
    if(vista==1){distancia=400;elevacion=52;giro=22;objetivoZ=35;}
    if(vista==2){distancia=175;elevacion=83;giro=0;}
    if(vista==3){distancia=160;elevacion=10;giro=4;objetivoX=4;objetivoY=3;objetivoZ=-35;}
}
void reiniciar(){
    tiempo=14;tiempoPersonas=0;hora=9;ciclo=true;pausado=false;trenesActivos=true;
    perspectiva=true;luces=true;techos=true;rayos=false;vista=0;
    rotacion=false;escalaSeno=false;traslacion=false;coloresLuz=false;camaraAuto=false;
    profundidad=true;multivista=false;mostrarNormales=true;seleccionarFoco(0);
}
bool guardarImagen(const char* nombre){
    std::vector<unsigned char> px(size_t(ancho)*alto*3);glPixelStorei(GL_PACK_ALIGNMENT,1);glReadBuffer(GL_BACK);glFinish();
    glReadPixels(0,0,ancho,alto,GL_RGB,GL_UNSIGNED_BYTE,px.data());std::ofstream f(nombre,std::ios::binary);if(!f)return false;
    f<<"P6\n"<<ancho<<" "<<alto<<"\n255\n";
    for(int y=alto-1;y>=0;--y)f.write(reinterpret_cast<char*>(px.data()+size_t(y)*ancho*3),ancho*3);return bool(f);
}
void prepararCaptura(){
    reiniciar();ciclo=false;sonido.activo=false;tiempoPersonas=12;fps=60;
    if(captura==1)perspectiva=false;
    if(captura==2)hora=18;
    if(captura==3)hora=22;
    if(captura==4){seleccionarFoco(4);hora=17;}
    if(captura==5){seleccionarFoco(4);hora=22;}
    if(captura==6){tiempo=4;tren[0]=sim::evaluarTren(tiempo);seleccionarFoco(1);techos=false;}
    if(captura==7){vista=0;seleccionarVista();hora=10;}
    if(captura==8){seleccionarFoco(2);techos=false;}
    if(captura==9){seleccionarFoco(4);hora=22;luces=false;}
    if(captura==10){seleccionarFoco(4);rotacion=true;escalaSeno=true;traslacion=true;coloresLuz=true;mostrarNormales=true;}
    if(captura==11){multivista=true;ayuda=false;}
    if(captura==12){profundidad=false;}
}
void dibujarVista(int x,int y,int w,int h,bool modo,float a,float e){
    const float antesGiro=giro,antesElevacion=elevacion;const bool antesPerspectiva=perspectiva;
    giro=a;elevacion=e;perspectiva=modo;
    aplicarProyeccion(x,y,w,h,modo);colocarCamara();configurarLuces();
    if(profundidad)glEnable(GL_DEPTH_TEST);else glDisable(GL_DEPTH_TEST);
    const GLfloat niebla[]={ambiente.cielo.r,ambiente.cielo.g,ambiente.cielo.b,1};glFogfv(GL_FOG_COLOR,niebla);
    glFogf(GL_FOG_START,420);glFogf(GL_FOG_END,950);glEnable(GL_FOG);glCallList(escenaLista);
    cubiertas();lamparasYVentanas();for(int s=0;s<2;++s)for(int i=0;i<4;++i)vagon(s,i);
    pasajeros();coches();relojEstacion();rayosDidacticos();
    giro=antesGiro;elevacion=antesElevacion;perspectiva=antesPerspectiva;
}
void display(){
    tren[0]=sim::evaluarTren(tiempo);tren[1]=sim::evaluarTren(tiempo+36);if(foco==1)objetivoZ=tren[0].z;
    ambiente=sim::ambiente(hora);glClearColor(ambiente.cielo.r,ambiente.cielo.g,ambiente.cielo.b,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    const int h=std::max(1,alto-200);
    if(multivista){const int w=ancho/2,hh=h/2;
        dibujarVista(0,112+hh,w,h-hh,perspectiva,giro,elevacion);
        const float distanciaGuardada=distancia;
        distancia=std::min(distancia,180.0f);dibujarVista(w,112+hh,ancho-w,h-hh,false,0,89.9f);
        distancia=std::min(distanciaGuardada,95.0f);dibujarVista(0,112,w,hh,false,0,0);
        distancia=std::min(distanciaGuardada,90.0f);dibujarVista(w,112,ancho-w,hh,false,90,0);
        distancia=distanciaGuardada;
    }else dibujarVista(0,112,ancho,h,perspectiva,giro,elevacion);
    interfaz();
    if(verificar){
        const char* nombres[]={"01_perspectiva.ppm","02_ortografica.ppm","03_tarde.ppm","04_noche.ppm","05_rayos_sol.ppm","06_rayos_lampara.ppm","07_puertas.ppm","08_entorno.ppm","09_personas.ppm","10_sin_luces.ppm","11_transformaciones.ppm","12_viewports.ppm","13_sin_profundidad.ppm"};
        const GLenum error=glGetError();if(error!=GL_NO_ERROR||!guardarImagen(nombres[captura])){std::fprintf(stderr,"Fallo captura %d, GL=%u\n",captura,error);std::exit(2);}
        std::printf("OK %s\n",nombres[captura]);if(++captura==13){std::puts("VERIFICACION OPENGL COMPLETA");std::exit(0);}
        prepararCaptura();glutPostRedisplay();
    }glutSwapBuffers();
}
void teclado(unsigned char k,int,int){
    switch(k){
        case 'p':case 'P':perspectiva=true;break;
        case 'o':case 'O':perspectiva=false;break;
        case 'l':case 'L':luces=!luces;mostrarNormales=luces;break;
        case '1':rotacion=!rotacion;if(foco!=4)seleccionarFoco(4);break;
        case '2':escalaSeno=!escalaSeno;if(foco!=4)seleccionarFoco(4);break;
        case '3':traslacion=!traslacion;if(foco!=4)seleccionarFoco(4);break;
        case '4':luces=!luces;mostrarNormales=luces;break;
        case '5':coloresLuz=!coloresLuz;if(foco!=4)seleccionarFoco(4);break;
        case '6':camaraAuto=!camaraAuto;break;
        case '7':perspectiva=!perspectiva;break;
        case '8':profundidad=!profundidad;break;
        case '9':multivista=!multivista;break;
        case 'j':case 'J':hora=9;ciclo=false;break;
        case 'e':case 'E':hora=18;ciclo=false;break;
        case 'n':case 'N':hora=22;ciclo=false;break;
        case 'c':case 'C':ciclo=!ciclo;break;
        case 'm':case 'M':sonido.activo=!sonido.activo;break;
        case 'b':case 'B':sonido.bocina();break;
        case 't':case 'T':trenesActivos=!trenesActivos;break;
        case ' ':pausado=!pausado;break;
        case 'v':case 'V':seleccionarVista();break;
        case 'f':case 'F':seleccionarFoco((foco+1)%5);break;
        case 'k':case 'K':if(foco!=4)seleccionarFoco(4);else rayos=!rayos;break;
        case 'h':case 'H':techos=!techos;break;
        case '?':ayuda=!ayuda;break;
        case '+':case '=':distancia=std::max(12.0f,distancia*.90f);break;
        case '-':distancia=std::min(550.0f,distancia*1.10f);break;
        case 'w':case 'W':objetivoZ-=6;foco=0;break;
        case 's':case 'S':objetivoZ+=6;foco=0;break;
        case 'a':case 'A':objetivoX-=6;foco=0;break;
        case 'd':case 'D':objetivoX+=6;foco=0;break;
        case 'r':case 'R':reiniciar();break;
        case 27:std::exit(0);
    }glutPostRedisplay();
}
void especiales(int k,int,int){
    if(k==GLUT_KEY_LEFT)giro-=3;
    if(k==GLUT_KEY_RIGHT)giro+=3;
    if(k==GLUT_KEY_UP)elevacion=std::min(85.0f,elevacion+3);
    if(k==GLUT_KEY_DOWN)elevacion=std::max(5.0f,elevacion-3);
    glutPostRedisplay();
}
void raton(int boton,int estado,int x,int y){
    if(boton==GLUT_LEFT_BUTTON){arrastrando=estado==GLUT_DOWN;mouseX=x;mouseY=y;}
    if(estado==GLUT_DOWN&&boton==3)teclado('+',0,0);
    if(estado==GLUT_DOWN&&boton==4)teclado('-',0,0);
}
void movimiento(int x,int y){
    if(arrastrando){giro+=(x-mouseX)*.35f;elevacion=sim::limitar(elevacion+(y-mouseY)*.25f,5,85);mouseX=x;mouseY=y;glutPostRedisplay();}
}
void reshape(int w,int h){ancho=std::max(1,w);alto=std::max(201,h);glutPostRedisplay();}
void timer(int){
    const int ahora=glutGet(GLUT_ELAPSED_TIME);const float dt=std::min(.1f,(ahora-anterior)/1000.0f);anterior=ahora;
    if(dt>0)fps=sim::mezclar(fps,1/dt,.05f);
    if(!pausado){tiempoPersonas+=dt;if(camaraAuto)giro+=dt*12;
        if(trenesActivos){const auto antes=sim::evaluarTren(tiempo);tiempo+=dt;const auto despues=sim::evaluarTren(tiempo);
            if(antes.estacion>=0&&despues.estacion<0)sonido.bocina();
            if(antes.estacion<0&&despues.estacion>=0)sonido.frenar();}
        if(ciclo){hora+=dt*(24.0f/180);if(hora>=24)hora-=24;}
    }
    sonido.velocidad=pausado||!trenesActivos?0:sim::evaluarTren(tiempo).velocidad;
    sonido.dia=sim::ambiente(hora).dia;sonido.actualizar();glutPostRedisplay();glutTimerFunc(16,timer,0);
}
bool probarLogica(){
    for(int i=0;i<=14400;++i){const double t=i*.01;const auto a=sim::evaluarTren(t),b=sim::evaluarTren(t+.001);
        if(a.z<-50.001f||a.z>150.001f||a.puertas<0||a.puertas>1)return false;
        if(a.puertas>0&&(a.estacion<0||a.velocidad!=0))return false;
        if(std::fabs(b.z-a.z)>.013f)return false;
        if(a.estacion<0&&a.puertas!=0)return false;
    }
    if(sim::evaluarTren(4).puertas!=1||sim::evaluarTren(40).puertas!=1)return false;
    const V3 n=normalizar({.24f,.74f,.63f}),i=normalizar({.3f,-.7f,-.5f}),r=i-n*(2*punto(i,n));
    if(std::fabs(punto(r,n)+punto(i,n))>.0001f||std::fabs(punto(r,r)-1)>.0001f)return false;
    for(int h=0;h<2400;++h){const auto a=sim::ambiente(h*.01f);if(a.dia<0||a.dia>1||a.lamparas<0||a.lamparas>1)return false;}
    std::puts("OK recorrido, frenado, puertas, ciclo de luz y reflexion.");return true;
}
bool muestraSonido(){
    Sonido sintetizador;sintetizador.velocidad=8;sintetizador.bocina();std::ofstream f("muestra_sonido.wav",std::ios::binary);if(!f)return false;
    const std::uint32_t n=22050*3*2,tam=n+36,rate=22050,bytes=44100,fmt=16;const std::uint16_t pcm=1,canal=1,align=2,bits=16;
    f.write("RIFF",4);f.write(reinterpret_cast<const char*>(&tam),4);f.write("WAVEfmt ",8);f.write(reinterpret_cast<const char*>(&fmt),4);
    f.write(reinterpret_cast<const char*>(&pcm),2);f.write(reinterpret_cast<const char*>(&canal),2);f.write(reinterpret_cast<const char*>(&rate),4);
    f.write(reinterpret_cast<const char*>(&bytes),4);f.write(reinterpret_cast<const char*>(&align),2);f.write(reinterpret_cast<const char*>(&bits),2);
    f.write("data",4);f.write(reinterpret_cast<const char*>(&n),4);
    for(unsigned k=0;k<n/2;++k){short s=sintetizador.muestra();f.write(reinterpret_cast<const char*>(&s),2);}return bool(f);
}
bool probarControles(){
    reiniciar();const float d=distancia,g=giro,e=elevacion;
    teclado('7',0,0);if(perspectiva||distancia!=d||giro!=g||elevacion!=e)return false;
    teclado('7',0,0);if(!perspectiva)return false;
    teclado('1',0,0);teclado('2',0,0);teclado('3',0,0);
    if(!rotacion||!escalaSeno||!traslacion||foco!=4)return false;
    teclado('4',0,0);if(luces||mostrarNormales)return false;teclado('4',0,0);
    if(!luces||!mostrarNormales)return false;
    teclado('5',0,0);teclado('6',0,0);teclado('8',0,0);teclado('9',0,0);
    if(!coloresLuz||!camaraAuto||profundidad||!multivista)return false;
    teclado('n',0,0);if(hora!=22||ciclo)return false;teclado('e',0,0);if(hora!=18)return false;
    teclado('j',0,0);if(hora!=9)return false;teclado('c',0,0);if(!ciclo)return false;
    teclado(' ',0,0);if(!pausado)return false;teclado('t',0,0);if(trenesActivos)return false;
    reiniciar();std::puts("OK teclas 1-9, misma camara P/O, horas y pausa.");return true;
}
int main(int argc,char** argv){
    if(argc>1&&std::strcmp(argv[1],"--prueba-logica")==0)return probarLogica()?0:2;
    if(argc>1&&std::strcmp(argv[1],"--muestra-sonido")==0)return muestraSonido()?0:2;
    if(argc>1&&std::strcmp(argv[1],"--prueba-audio")==0){
        sonido.activo=false;if(!sonido.abrir()){std::puts("No hay dispositivo de audio disponible");return 2;}
        for(int i=0;i<20;++i){sonido.actualizar();Sleep(16);}
        const bool ok=sonido.disponible();sonido.cerrar();std::puts(ok?"OK dispositivo WinMM y buffers de audio":"Fallo de reproduccion");return ok?0:2;
    }
    verificar=argc>1&&std::strcmp(argv[1],"--verificar")==0;if(verificar&&!probarLogica())return 2;
    int glutArgc=1;glutInit(&glutArgc,argv);glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(ancho,alto);if(verificar)glutInitWindowPosition(-2000,0);
    glutCreateWindow("Estacion Central | Simulacion 3D | OpenGL - GLUT");
    glEnable(GL_DEPTH_TEST);glEnable(GL_NORMALIZE);glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);glShadeModel(GL_SMOOTH);glFogi(GL_FOG_MODE,GL_LINEAR);
    escenaLista=glGenLists(1);glNewList(escenaLista,GL_COMPILE);crearEntorno();glEndList();
    tren[0]=sim::evaluarTren(tiempo);tren[1]=sim::evaluarTren(tiempo+36);
    if(verificar)prepararCaptura();else sonido.abrir();
    glutDisplayFunc(display);glutReshapeFunc(reshape);glutKeyboardFunc(teclado);glutSpecialFunc(especiales);
    glutMouseFunc(raton);glutMotionFunc(movimiento);anterior=glutGet(GLUT_ELAPSED_TIME);
    // Render directo para verificar incluso con una ventana oculta sin eventos de repintado.
    if(verificar){if(!probarControles())return 2;prepararCaptura();for(int i=0;i<13;++i)display();return 0;}
    if(!verificar)glutTimerFunc(16,timer,0);glutMainLoop();return 0;
}

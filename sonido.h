#pragma once
// Sonido sintetizado localmente: no requiere archivos ni bibliotecas de audio externas.
#include <windows.h>
#include <mmsystem.h>
#include <cmath>
#include <cstdint>
#include <algorithm>
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

class Sonido {
    static const int N = 2048, BUFFERS = 3, RATE = 22050;
    HWAVEOUT dispositivo = nullptr;
    WAVEHDR cabeceras[BUFFERS] = {};
    short datos[BUFFERS][N] = {};
    std::uint32_t azar = 12345;
    double reloj = 0, faseMotor = 0, faseRueda = 0;
    float pito = 0, freno = 0, ganancia = 0;
public:
    bool activo = true;
    float velocidad = 0, dia = 1;
    ~Sonido() { cerrar(); }
    bool disponible() const { return dispositivo != nullptr; }
    void bocina() { pito = 1.3f; }
    void frenar() { freno = 1.1f; }
    bool abrir() {
        WAVEFORMATEX formato = {};
        formato.wFormatTag = WAVE_FORMAT_PCM; formato.nChannels = 1;
        formato.nSamplesPerSec = RATE; formato.wBitsPerSample = 16;
        formato.nBlockAlign = 2; formato.nAvgBytesPerSec = RATE * 2;
        if (waveOutOpen(&dispositivo,WAVE_MAPPER,&formato,0,0,CALLBACK_NULL) != MMSYSERR_NOERROR) {
            dispositivo = nullptr; return false;
        }
        for (int i=0;i<BUFFERS;++i) {
            cabeceras[i].lpData = reinterpret_cast<LPSTR>(datos[i]);
            cabeceras[i].dwBufferLength = sizeof(datos[i]);
            if (waveOutPrepareHeader(dispositivo,&cabeceras[i],sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
                cerrar(); return false;
            }
        }
        actualizar(); return dispositivo != nullptr;
    }
    // La misma sintesis permite guardar una muestra WAV para verificarla.
    short muestra() {
        const double pi = 3.141592653589793;
        reloj += 1.0/RATE;
        azar = 1664525u*azar+1013904223u;
        const double ruido = double((azar >> 8)&65535)/32768.0-1;
        const double v = std::min(1.0, std::fabs(double(velocidad))/11.2);
        faseMotor += (38+v*54)/RATE; faseMotor -= std::floor(faseMotor);
        faseRueda += (.8+v*8)/RATE; faseRueda -= std::floor(faseRueda);
        double s = .013*ruido; // brisa
        s += v*(.085*std::sin(2*pi*faseMotor)+.035*ruido);
        s += v*.095*std::exp(-faseRueda*36)*ruido; // juntas de los rieles
        const double ave = std::fmod(reloj,4.7);
        if (dia>.4f && ave<.24) s += .021*std::sin(pi*ave/.24)*std::sin(2*pi*(2400*ave+1300*ave*ave));
        if (dia<.3f) s += .012*std::sin(2*pi*3300*reloj)*std::pow(std::max(0.0,std::sin(2*pi*3.8*reloj)),8);
        if (pito>0) {
            const double env = std::min(1.0,double(pito)*5)*std::min(1.0,double(1.3f-pito)*10);
            s += .13*env*(std::sin(2*pi*311*reloj)+.6*std::sin(2*pi*466*reloj));
            pito = std::max(0.0f,pito-1.0f/RATE);
        }
        if (freno>0) { s += .045*freno*ruido; freno = std::max(0.0f,freno-1.0f/RATE); }
        ganancia += ((activo ? .65f : 0)-ganancia)*.002f;
        return static_cast<short>(std::max(-.9,std::min(.9,s*ganancia))*32767);
    }
    void actualizar() {
        if (!dispositivo) return;
        for (int i=0;i<BUFFERS;++i) {
            if (!(cabeceras[i].dwFlags & WHDR_INQUEUE)) {
                for (int k=0;k<N;++k) datos[i][k]=muestra();
                if (waveOutWrite(dispositivo,&cabeceras[i],sizeof(WAVEHDR))!=MMSYSERR_NOERROR) {
                    cerrar(); return;
                }
            }
        }
    }
    void cerrar() {
        if (!dispositivo) return;
        waveOutReset(dispositivo);
        for (int i=0;i<BUFFERS;++i) if (cabeceras[i].dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(dispositivo,&cabeceras[i],sizeof(WAVEHDR));
        waveOutClose(dispositivo); dispositivo=nullptr;
    }
};

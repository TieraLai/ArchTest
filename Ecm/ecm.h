#ifndef ECM_H
#define ECM_H

#ifdef __cplusplus
extern "C" {
#endif

//callback functions define
typedef void (*NEC_RtCyclicCallback)(int value);// For cyclic process. 
void registerCallback(NEC_RtCyclicCallback cb); // Register
void startECM();                                // start polling

#ifdef __cplusplus
}
#endif

#endif // ECM_H
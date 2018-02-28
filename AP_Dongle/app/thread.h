/*
 * thread.h
 *
 *  Created on: 2018Äê2ÔÂ28ÈÕ
 *      Author: ggg
 */

#ifndef APP_THREAD_H_
#define APP_THREAD_H_


extern void TIM_SetSoftInterrupt(UINT8 enable, void (*p)(void));
extern void TIM_SoftInterrupt(void);
extern void semaphore_init(void);


#endif /* APP_THREAD_H_ */

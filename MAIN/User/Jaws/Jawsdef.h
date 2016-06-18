#ifndef JAWSDEF_H
#define JAWSDEF_H

typedef enum
{
    X_AXIS=0,
    Y_AXIS=1,
    Z_AXIS=2,
    E_AXIS=3
} AxisEnum;

#define  FORCE_INLINE       __attribute__((always_inline)) inline

#define  CRITICAL_SECTION_START     __disable_irq();
#define  CRITICAL_SECTION_END       __enable_irq();

#define  millis()   HAL_GetTick()


#endif /* JAWSDEF_H */

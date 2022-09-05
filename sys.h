#include <stdint.h>
#include  <MK66F18.h>

typedef int16_t __attribute__((__may_alias__)) aliased_int16_t;

#define LOW 0
#define HIGH 1

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define M_SQRT2 1.4142135623730950488016887
#define M_SQRT3 1.7320508075688772935274463

#define SERIAL  0
#define DISPLAY 1

#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)


void _WAIT(uint32_t ms);

void _WAIT_us(uint32_t us);

void _WAIT_t(uint32_t t);

void _WAIT_NOP (uint32_t stepDelay);

void WriteDAC0(int val);

void WriteDAC1(int val);

void WriteDAC00(uint16_t val);

void WriteDAC01(uint16_t val);

void WriteDAC000(uint16_t val);

void WriteDAC001(uint16_t val);

void WriteDAC0000(int val);

void WriteDAC0001(int val);

int GetWDay(int d, int mon, int yr, int cent);

int number_days(int m, int y);
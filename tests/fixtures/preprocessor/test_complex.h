#define PI 3.14159
#define SQUARE(x) ((x) * (x))
#ifdef MATH_EXTENDED
#define CIRCLE_AREA(r) (PI * SQUARE(r))
#else
#define CIRCLE_AREA(r) (3.14 * SQUARE(r))
#endif

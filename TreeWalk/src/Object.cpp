#include "Object.h"

bool operator==(const Object& a, const Object& b)
{
	return a.data == b.data;
}

#include <iostream>
#include <vector>
#include "wavefront.h"



int main()
{
	wavefront::Object obj;
	obj = wavefront::parse_obj_file("sample.obj");
	obj.print();
}
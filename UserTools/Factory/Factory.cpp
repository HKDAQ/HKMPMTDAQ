#include "Factory.h"

Tool* Factory(std::string tool) {
Tool* ret=0;

// if (tool=="Type") tool=new Type;
if (tool=="DummyTool") ret=new DummyTool;
if (tool=="Logger") ret=new Logger;
if (tool=="Monitor") ret=new Monitor;
if (tool=="SlowControl") ret=new SlowControl;

return ret;
}

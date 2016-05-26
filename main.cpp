#include "AppManager.h"

int main(int argc, char** argv)
{
	AppManager app;
	
	app.ReadCMD(argc, argv);

	app.Execute();
	
	return 0;
}

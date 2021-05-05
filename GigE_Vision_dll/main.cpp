#include <iostream>
#include "GigeManager.hpp"


int main()
{
	GigeManager gige;
	gige.Init("TLSimu.cti");
	gige.useInterface(0);
	gige.useDevice(1);
	gige.cameraInit();
	gige.acquirerPreparing();
	gige.startAcquisition();

	int payloadSize = gige.imageSize();

	int* image = new int[payloadSize];
	
	while (true)
	{
		gige.timer();
		if (gige.getImage(image, payloadSize))
		{
			for (int i = 0; i < payloadSize; i++)
			{
				std::cout << ' ' << image[i];
			}
		}
		std::cout << std::endl;
		system("pause");
	}
	
	return 0;
}

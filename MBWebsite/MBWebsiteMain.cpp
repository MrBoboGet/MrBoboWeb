#include "MPGMemeSite.h"
#include <MrBoboSockets/MrBoboSockets.h>
int main()
{
	MBSockets::Init();

	return(MBWebsite::MBGWebsiteMain());
}

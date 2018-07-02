#ifndef WRAPSERVERSOCKET_H_
#define WRAPSERVERSOCKET_H_
#include "core/sox/selsel.h"

namespace core{
using sox::TimeClickCallbackIf;

class WrapServerStart
{
public:
	static void init();
	static void register_time_click_callback(TimeClickCallbackIf* If);
	static void run();
	static void stop();
    static bool isRunning();

};
}
#endif /*WRAPSERVERSOCKET_H_*/

#pragma once

#include <iostream>
#include <memory>
#include <filesystem>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Hazel/Log.h"

#ifdef HZ_PLATFORM_WINDOWS
	#include <Windows.h>

#elif def HZ_PLATFORM_LINUX
	#include <unistd.h>
#endif
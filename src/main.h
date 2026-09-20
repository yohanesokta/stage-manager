#ifdef _WIN32

#include <Windows.h>
#include <WinUser.h>
#include <iostream>
#include <vector>
#include <string>

#define FEATURES_WINDOWS_PROGRAM_SIZE 2

void getAllWindowVisibleTitle();

#else

#error "This code is only supported on Windows."

#endif